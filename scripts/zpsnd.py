#!/usr/bin/env python3
"""zpsnd - bake samples and a text score into a ZPA sound pack.

    zpsnd.py bake  score.zps -o out.zpa [--listing out.lst]
    zpsnd.py trace out.zpa --ticks N [--mailbox TICK=VALUE ...]
    zpsnd.py info  out.zpa

A .zps file declares samples (synthesized or from a WAV) and a score. `bake`
produces the STL image, SST image, score bytes and BIOS header described in
docs/apu/zpa.txt. `trace` runs the baked score through a reference interpreter
and prints the MMP register writes per tick: the golden trace the BIOS engines
(LLE assembly, HLE C++) must reproduce.

Stdlib only. See docs/apu/zpa.txt for the format and docs/apu/sst.txt for the
SST block layout this writes.
"""

import argparse
import math
import os
import struct
import sys
import wave
from math import gcd

MIX_RATE = 48000
TICK_RATE = 120
SST_BASE = 0x9000
SST_SIZE = 0x4000
STL_BASE = 0x7000
MAX_SAMPLES = 256               # NOTE's sample index is one byte
SCORE_BASE = 0x3020
SCORE_MAX = 0x6000 - SCORE_BASE  # bytes the BIOS checksums after the header
MAGIC_ZPA = 0x0B
CHANNELS = 16

# score ops
OP_END, OP_NOTE, OP_PITCH, OP_VOL, OP_PAN, OP_OFF, OP_SET, OP_POST, OP_NOP = \
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08
OP_MARK, OP_REP, OP_JUMP, OP_JZ, OP_JEQ, OP_WAIT = \
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15

OP_NAMES = {
    OP_END: "END", OP_NOTE: "NOTE", OP_PITCH: "PITCH", OP_VOL: "VOL",
    OP_PAN: "PAN", OP_OFF: "OFF", OP_SET: "SET", OP_POST: "POST",
    OP_NOP: "NOP", OP_MARK: "MARK", OP_REP: "REP", OP_JUMP: "JUMP",
    OP_JZ: "JZ", OP_JEQ: "JEQ", OP_WAIT: "WAIT",
}
# argument layout per op: b = byte, w = big-endian 16-bit (signed ones marked s)
OP_ARGS = {
    OP_END: "", OP_NOTE: "bbwbb", OP_PITCH: "bsb", OP_VOL: "bsb",
    OP_PAN: "bsb", OP_OFF: "b", OP_SET: "bwbb", OP_POST: "b", OP_NOP: "",
    OP_MARK: "", OP_REP: "b", OP_JUMP: "w", OP_JZ: "w", OP_JEQ: "bw",
    OP_WAIT: "",
}


class ZpsndError(Exception):
    pass


# --------------------------------------------------------------------------
# sample synthesis and conversion
# --------------------------------------------------------------------------

def parse_kv(tokens, line_no):
    params = {}
    for tok in tokens:
        if "=" not in tok:
            raise ZpsndError("line %d: expected key=value, got '%s'" % (line_no, tok))
        k, v = tok.split("=", 1)
        params[k] = v
    return params


def fnum(params, key, default=None, line_no=0):
    if key not in params:
        if default is None:
            raise ZpsndError("line %d: missing %s=" % (line_no, key))
        return default
    try:
        return float(params[key])
    except ValueError:
        raise ZpsndError("line %d: %s=%s is not a number" % (line_no, key, params[key]))


def parse_pos(tok, rate, line_no):
    """A position in a sample: seconds ('0.25') or a sample index ('600n')."""
    try:
        if tok.endswith("n"):
            return int(tok[:-1])
        return int(round(float(tok) * rate))
    except ValueError:
        raise ZpsndError("line %d: bad position '%s'" % (line_no, tok))


def lcg_noise(n, seed):
    """Deterministic white noise in [-1, 1) so a build is reproducible."""
    x = seed & 0xFFFFFFFF
    out = []
    for _ in range(n):
        x = (x * 1664525 + 1013904223) & 0xFFFFFFFF
        out.append(((x >> 8) & 0xFFFF) / 32768.0 - 1.0)
    return out


def one_pole_lowpass(x, fc, rate, poles):
    a = 1.0 - math.exp(-2.0 * math.pi * fc / rate)
    for _ in range(poles):
        y = 0.0
        for i, v in enumerate(x):
            y += a * (v - y)
            x[i] = y
    return x


def stable_seed(name):
    h = 2166136261
    for ch in name.encode():
        h = ((h ^ ch) * 16777619) & 0xFFFFFFFF
    return h


def cycle_wave(kind, n, amp):
    out = []
    for i in range(n):
        ph = i / float(n)
        if kind == "sine":
            v = math.sin(2.0 * math.pi * ph)
        elif kind == "saw":
            v = 2.0 * ph - 1.0
        elif kind == "square":
            v = 1.0 if ph < 0.5 else -1.0
        else:  # tri
            v = 4.0 * ph - 1.0 if ph < 0.5 else 3.0 - 4.0 * ph
        out.append(v * amp)
    return out


def read_wav_mono(path, line_no):
    try:
        w = wave.open(path, "rb")
    except (OSError, wave.Error) as e:
        raise ZpsndError("line %d: cannot read %s: %s" % (line_no, path, e))
    ch, width, rate, n = w.getnchannels(), w.getsampwidth(), w.getframerate(), w.getnframes()
    raw = w.readframes(n)
    w.close()
    vals = []
    if width == 1:
        vals = [(b - 128) / 128.0 for b in raw]
    elif width == 2:
        vals = [v / 32768.0 for v in struct.unpack("<%dh" % (len(raw) // 2), raw)]
    elif width == 3:
        for i in range(0, len(raw) - 2, 3):
            v = raw[i] | (raw[i + 1] << 8) | (raw[i + 2] << 16)
            vals.append((v - (1 << 24) if v & 0x800000 else v) / 8388608.0)
    elif width == 4:
        vals = [v / 2147483648.0 for v in struct.unpack("<%di" % (len(raw) // 4), raw)]
    else:
        raise ZpsndError("line %d: unsupported WAV sample width %d" % (line_no, width))
    if ch > 1:
        vals = [sum(vals[i:i + ch]) / ch for i in range(0, len(vals) - ch + 1, ch)]
    return vals, rate


def resample(x, sr_in, sr_out, half=12):
    """Windowed-sinc resampler (Blackman), cutoff at the lower Nyquist."""
    if sr_in == sr_out or not x:
        return list(x)
    ratio = sr_out / float(sr_in)
    fc = min(1.0, ratio)
    reach = int(math.ceil(half / fc))
    n_out = int(len(x) * ratio)
    out = []
    for i in range(n_out):
        t = i / ratio
        c = int(math.floor(t))
        acc = 0.0
        for k in range(c - reach + 1, c + reach + 1):
            if 0 <= k < len(x):
                d = (t - k) * fc
                if abs(d) >= half:
                    continue
                s = 1.0 if d == 0 else math.sin(math.pi * d) / (math.pi * d)
                u = d / half
                win = 0.42 + 0.5 * math.cos(math.pi * u) + 0.08 * math.cos(2.0 * math.pi * u)
                acc += x[k] * s * win * fc
        out.append(acc)
    return out


class Sample:
    def __init__(self, name):
        self.name = name
        self.data = []          # int8 values, length a multiple of 12
        self.rate = MIX_RATE
        self.cycle = None       # cycle length in samples (tone samples only)
        self.loop_start = 0     # sample index (the STL loop word)
        self.one_shot = False
        self.index = 0
        self.sst_offset = 0

    def native_pitch(self):
        return int(round(self.rate * 4096.0 / MIX_RATE))


def finish_sample(s, floats, params, line_no):
    """Apply loop/crossfade/trim, normalize and quantize to int8."""
    rate = s.rate
    loop = params.get("loop", "none")
    xfade = int(fnum(params, "xfade", 0, line_no))
    n = len(floats)
    if n == 0:
        raise ZpsndError("line %d: sample '%s' is empty" % (line_no, s.name))

    if loop == "none":
        s.one_shot = True
        pad = (-n) % 12
        floats = floats + [0.0] * pad
        s.loop_start = 0
    else:
        if loop == "whole":
            start, end = 0, n
        else:
            if ".." not in loop:
                raise ZpsndError("line %d: loop= must be none, whole or A..B" % line_no)
            a, b = loop.split("..", 1)
            start, end = parse_pos(a, rate, line_no), parse_pos(b, rate, line_no)
        end = min(end, n)
        end -= end % 12                      # a sector is a multiple of 12 and wraps at its end
        if end - start < 12:
            raise ZpsndError("line %d: loop of '%s' is shorter than 12 samples after rounding"
                             % (line_no, s.name))
        if xfade:
            if xfade > start or xfade > end - start:
                raise ZpsndError("line %d: xfade=%d does not fit before the loop start / inside "
                                 "the loop" % (line_no, xfade))
            floats = list(floats[:end])
            for i in range(xfade):
                t = (i + 1) / float(xfade + 1)       # 0 -> 1 across the fade
                floats[end - xfade + i] = (floats[end - xfade + i] * (1.0 - t)
                                           + floats[start - xfade + i] * t)
        floats = floats[:end]
        s.loop_start = start

    peak = max(abs(v) for v in floats) or 1.0
    gain = fnum(params, "gain", 1.0, line_no)
    if "norm" in params:
        gain = fnum(params, "norm", 0.95, line_no) / peak
    out = []
    for v in floats:
        q = int(round(v * gain * 127.0))
        out.append(max(-128, min(127, q)))
    s.data = out
    return s


def build_sample(name, kind, params, base_dir, line_no):
    s = Sample(name)
    if kind in ("sine", "saw", "square", "tri"):
        cycle = int(fnum(params, "cycle", 0, line_no))
        if cycle < 2:
            raise ZpsndError("line %d: %s needs cycle=N (samples per cycle)" % (line_no, kind))
        s.cycle = cycle
        s.rate = int(fnum(params, "rate", MIX_RATE, line_no))
        cyc = cycle_wave(kind, cycle, fnum(params, "amp", 0.9, line_no))
        reps = 12 // gcd(cycle, 12)          # repeat to a multiple of 12 so it wraps cleanly
        p = dict(params)
        p["loop"] = "whole"
        return finish_sample(s, cyc * reps, p, line_no)

    if kind == "noise":
        s.rate = int(fnum(params, "rate", 8000, line_no))
        length = fnum(params, "len", 0.5, line_no)
        x = lcg_noise(int(length * s.rate), int(fnum(params, "seed", stable_seed(name), line_no)))
        if "lp" in params:
            one_pole_lowpass(x, fnum(params, "lp", 1000, line_no), s.rate,
                             int(fnum(params, "poles", 2, line_no)))
        if "hp" in params:
            low = one_pole_lowpass(list(x), fnum(params, "hp", 100, line_no), s.rate, 1)
            x = [a - b for a, b in zip(x, low)]
        attack = fnum(params, "attack", 0.0, line_no)
        decay = fnum(params, "decay", 0.0, line_no)
        for i in range(len(x)):
            t = i / float(s.rate)
            env = 1.0
            if attack > 0:
                env = min(1.0, t / attack)
            if decay > 0:
                env *= math.exp(-t / decay)
            x[i] *= env
        p = dict(params)
        p.setdefault("norm", "0.95")
        return finish_sample(s, x, p, line_no)

    if kind == "wav":
        if "file" not in params:
            raise ZpsndError("line %d: wav sample needs file=" % line_no)
        path = params["file"]
        if not os.path.isabs(path):
            path = os.path.join(base_dir, path)
        x, in_rate = read_wav_mono(path, line_no)
        s.rate = int(fnum(params, "rate", 16000, line_no))
        start = parse_pos(params.get("start", "0"), in_rate, line_no)
        end = parse_pos(params["end"], in_rate, line_no) if "end" in params else len(x)
        x = resample(x[start:end], in_rate, s.rate)
        return finish_sample(s, x, params, line_no)

    raise ZpsndError("line %d: unknown sample kind '%s' (noise, sine, saw, square, tri, wav)"
                     % (line_no, kind))


def bake_banks(samples):
    """Return (stl_bytes, sst_bytes) and fill in each sample's offsets."""
    if len(samples) > MAX_SAMPLES:
        raise ZpsndError("more than %d samples" % MAX_SAMPLES)
    sst = bytearray()
    stl = bytearray()
    for i, s in enumerate(samples):
        s.index = i
        s.sst_offset = len(sst)
        nblocks = len(s.data) // 12
        for b in range(nblocks):
            last = (b == nblocks - 1)
            l_nybble = 0
            if last:
                l_nybble = 0x4 | (0x8 if s.one_shot else 0)    # W, plus Z for a one-shot
            sst += bytes([0x00, l_nybble, 0x00, 0x00])
            sst += bytes(v & 0xFF for v in s.data[b * 12:(b + 1) * 12])
        stl += struct.pack("<HH", SST_BASE + s.sst_offset, s.loop_start)
    if len(sst) > SST_SIZE:
        raise ZpsndError("samples need %d bytes of SST, only %d available" % (len(sst), SST_SIZE))
    return bytes(stl), bytes(sst)


# --------------------------------------------------------------------------
# score compilation
# --------------------------------------------------------------------------

def parse_time(tok, line_no):
    """'0.5' seconds, '60t' ticks. Returns ticks."""
    try:
        if tok.endswith("t"):
            return int(tok[:-1])
        return int(math.floor(float(tok) * TICK_RATE + 0.5))
    except ValueError:
        raise ZpsndError("line %d: bad time '%s'" % (line_no, tok))


def clamp88(v):
    return max(0, min(0xFFFF, v))


class Chan:
    def __init__(self):
        self.sample = None
        self.pitch = None
        self.vol = None        # 8.8
        self.pan = None        # 8.8
        self.end = {"pitch": 0, "vol": 0, "pan": 0}

    def snapshot(self):
        return (self.sample, self.pitch, self.vol, self.pan)


def plan_slide(param, start, target, ticks, line_no):
    """Chunks [(ticks, delta)] that take `start` to `target` in exactly `ticks`.

    The engine adds an integer delta per tick (pitch: raw units, vol/pan: 1/256),
    so a straight line is built from two speeds: T-r ticks at d and r ticks at d+1,
    where d, r = divmod(|diff|, T). The total is exactly `diff`, so it lands on the
    target with no rounding drift. A slide smaller than T units is r ticks of +-1
    (it finishes early). Chunks are at most 255 ticks (the event's tick field).
    """
    if ticks < 1:
        raise ZpsndError("line %d: slide needs at least 1 tick" % line_no)
    diff = target - start
    if diff == 0:
        return [], start
    sign = 1 if diff > 0 else -1
    d, r = divmod(abs(diff), ticks)
    steps = []
    if d > 0 and ticks - r > 0:
        steps.append((ticks - r, sign * d))
    if r > 0:
        steps.append((r, sign * (d + 1)))
    if any(abs(dl) > 32767 for _, dl in steps):
        need = int(math.ceil(abs(diff) / 32767.0))
        raise ZpsndError("line %d: %s slide too fast: needs at least %d ticks" % (line_no, param, need))
    chunks = []
    for n, dl in steps:
        while n > 255:
            chunks.append((255, dl))
            n -= 255
        if n:
            chunks.append((n, dl))
    return chunks, target


class Compiler:
    def __init__(self, samples):
        self.samples = samples
        self.by_name = {s.name: s for s in samples}
        self.chans = [Chan() for _ in range(CHANNELS)]
        self.items = []          # ("label", name) or event dicts, in author order
        self.time = 0
        self.seq = 0
        self.labels_seen = False
        self.first_after_label = False
        self.label_state = {}    # name -> snapshot
        self.mark_states = []    # stack of snapshots for mark/rep
        self.reachable = True

    # -- helpers --------------------------------------------------------
    def chan(self, tok, line_no):
        try:
            c = int(tok)
        except ValueError:
            raise ZpsndError("line %d: bad channel '%s'" % (line_no, tok))
        if not 0 <= c < CHANNELS:
            raise ZpsndError("line %d: channel %d out of range 0-%d" % (line_no, c, CHANNELS - 1))
        return c

    def state(self):
        return tuple(ch.snapshot() for ch in self.chans)

    def finished(self, line_no, what):
        for i, ch in enumerate(self.chans):
            for p, e in ch.end.items():
                if e > self.time:
                    raise ZpsndError(
                        "line %d: %s while channel %d's %s slide is still running (ends at tick %d, "
                        "now tick %d): add a NOP so the slide can finish first"
                        % (line_no, what, i, p, e, self.time))

    def pitch_value(self, params, sample, line_no, current=None):
        """raw=N, hz=F (tone samples), semis=S (relative to native), pitch=native|N;
        falls back to `current` (which may be None)."""
        if "raw" in params:
            return int(params["raw"], 0)
        if "hz" in params:
            if sample is None or sample.cycle is None:
                raise ZpsndError("line %d: hz= needs a tone sample (sine/saw/square/tri)" % line_no)
            return int(round(float(params["hz"]) * sample.cycle * 4096.0 / MIX_RATE))
        if "semis" in params:
            if sample is None:
                raise ZpsndError("line %d: semis= needs a sample on the channel" % line_no)
            return int(round(sample.native_pitch() * 2.0 ** (float(params["semis"]) / 12.0)))
        if "pitch" in params:
            if params["pitch"] == "native":
                if sample is None:
                    raise ZpsndError("line %d: pitch=native needs a sample" % line_no)
                return sample.native_pitch()
            return int(params["pitch"], 0)
        return current

    def add(self, kind, line_no, delta, **kw):
        self.time += delta
        self.seq += 1
        ev = {"kind": kind, "time": self.time, "seq": self.seq, "line": line_no}
        ev.update(kw)
        self.items.append(ev)
        self.first_after_label = False
        return ev

    # -- one score line -------------------------------------------------
    def line(self, tokens, line_no):
        if tokens[0] == "label":
            if len(tokens) != 2:
                raise ZpsndError("line %d: label NAME" % line_no)
            name = tokens[1]
            if name in self.label_state:
                raise ZpsndError("line %d: label '%s' defined twice" % (line_no, name))
            self.finished(line_no, "label '%s'" % name)
            snap = self.state()
            if self.reachable:
                pending = self.label_state.get("<fwd>" + name)
                if pending is not None and pending != snap:
                    raise ZpsndError("line %d: a jump to '%s' arrives in a different channel state "
                                     "than falling into it; put a SET after the label" % (line_no, name))
            else:
                pending = self.label_state.get("<fwd>" + name)
                if pending is not None:
                    snap = pending
                    for ch, st in zip(self.chans, pending):
                        ch.sample, ch.pitch, ch.vol, ch.pan = st
            self.label_state[name] = snap
            self.items.append(("label", name))
            self.labels_seen = True
            self.first_after_label = True
            self.reachable = True
            return

        spec, verb, args = tokens[0], tokens[1] if len(tokens) > 1 else "", tokens[2:]
        if spec[0] == "+":
            delta = parse_time(spec[1:], line_no)
        elif spec[0] == "@":
            if self.labels_seen:
                raise ZpsndError("line %d: after a label use +TIME, not @TIME (a jump back restarts "
                                 "timing from the jump)" % line_no)
            delta = parse_time(spec[1:], line_no) - self.time
            if delta < 0:
                raise ZpsndError("line %d: events must be in time order" % line_no)
        else:
            raise ZpsndError("line %d: expected @TIME or +TIME, got '%s'" % (line_no, spec))
        getattr(self, "v_" + verb, self.unknown)(verb, args, delta, line_no)

    def unknown(self, verb, args, delta, line_no):
        raise ZpsndError("line %d: unknown verb '%s'" % (line_no, verb))

    # -- verbs ------------------------------------------------------------
    def v_note(self, verb, args, delta, line_no):
        if len(args) < 2:
            raise ZpsndError("line %d: note CH SAMPLE [pitch=.. vol=.. pan=..]" % line_no)
        c = self.chan(args[0], line_no)
        if args[1] not in self.by_name:
            raise ZpsndError("line %d: unknown sample '%s'" % (line_no, args[1]))
        smp = self.by_name[args[1]]
        params = parse_kv(args[2:], line_no)
        pitch = self.pitch_value(params, smp, line_no)
        if pitch is None:
            pitch = smp.native_pitch()
        if not 0 <= pitch <= 0xFFFF:
            raise ZpsndError("line %d: pitch out of range" % line_no)
        vol = int(float(params.get("vol", 255)))
        pan = int(float(params.get("pan", 128)))
        if not (0 <= vol <= 255 and 0 <= pan <= 255):
            raise ZpsndError("line %d: vol/pan must be 0-255" % line_no)
        ch = self.chans[c]
        ch.sample, ch.pitch, ch.vol, ch.pan = smp, pitch, vol << 8, pan << 8
        ch.end = {"pitch": 0, "vol": 0, "pan": 0}      # a note cancels slides
        self.add("op", line_no, delta, op=OP_NOTE, args=[c, smp.index, pitch, vol, pan])

    def v_set(self, verb, args, delta, line_no):
        c = self.chan(args[0], line_no)
        ch = self.chans[c]
        if ch.pitch is None:
            raise ZpsndError("line %d: set on channel %d before any note" % (line_no, c))
        params = parse_kv(args[1:], line_no)
        pitch = self.pitch_value(params, ch.sample, line_no, current=ch.pitch)
        vol = int(float(params["vol"])) if "vol" in params else ch.vol >> 8
        pan = int(float(params["pan"])) if "pan" in params else ch.pan >> 8
        if not (0 <= vol <= 255 and 0 <= pan <= 255 and 0 <= pitch <= 0xFFFF):
            raise ZpsndError("line %d: value out of range" % line_no)
        ch.pitch, ch.vol, ch.pan = pitch, vol << 8, pan << 8
        ch.end = {"pitch": 0, "vol": 0, "pan": 0}      # set cancels slides
        self.add("op", line_no, delta, op=OP_SET, args=[c, pitch, vol, pan])

    def v_slide(self, verb, args, delta, line_no):
        if len(args) < 3:
            raise ZpsndError("line %d: slide CH pitch|vol|pan to=.. over=SECONDS" % line_no)
        c = self.chan(args[0], line_no)
        param = args[1]
        if param not in ("pitch", "vol", "pan"):
            raise ZpsndError("line %d: slide what? pitch, vol or pan" % line_no)
        ch = self.chans[c]
        if ch.pitch is None:
            raise ZpsndError("line %d: slide on channel %d before any note" % (line_no, c))
        params = parse_kv(args[2:], line_no)
        if "over" not in params:
            raise ZpsndError("line %d: slide needs over=SECONDS (or Nt)" % line_no)
        ticks = parse_time(params["over"], line_no)
        start_time = self.time + delta           # the tick this event fires on
        if ch.end[param] > start_time:
            raise ZpsndError("line %d: channel %d's %s slide is still running until tick %d"
                             % (line_no, c, param, ch.end[param]))
        if param == "pitch":
            start = ch.pitch
            pp = {k: v for k, v in params.items() if k not in ("over", "to")}
            if "to" in params:
                pp["raw"] = params["to"]
            target = self.pitch_value(pp, ch.sample, line_no)
            if target is None or not 0 <= target <= 0xFFFF:
                raise ZpsndError("line %d: pitch slide needs a target in range (to=/raw=/hz=/semis=)"
                                 % line_no)
        else:
            if "to" not in params:
                raise ZpsndError("line %d: slide needs to=0..255" % line_no)
            tv = float(params["to"])
            if not 0 <= tv <= 255:
                raise ZpsndError("line %d: to=%s out of range 0-255" % (line_no, params["to"]))
            start = ch.vol if param == "vol" else ch.pan
            target = int(round(tv * 256))
        chunks, final = plan_slide(param, start, target, ticks, line_no)
        if not chunks:
            return                               # already there: nothing to emit
        op = {"pitch": OP_PITCH, "vol": OP_VOL, "pan": OP_PAN}[param]
        spent = 0
        parent = None
        for n, dl in chunks:
            if parent is None:
                parent = self.add("op", line_no, delta, op=op, args=[c, dl, n])
            else:
                # chained continuation: lands among the author's events by time (see encode())
                self.items.append({"kind": "op", "time": start_time + spent, "seq": parent["seq"],
                                   "line": line_no, "op": op, "args": [c, dl, n]})
            spent += n
        ch.end[param] = start_time + spent
        if param == "pitch":
            ch.pitch = final
        elif param == "vol":
            ch.vol = final
        else:
            ch.pan = final

    def v_off(self, verb, args, delta, line_no):
        c = self.chan(args[0], line_no)
        self.chans[c].end = {"pitch": 0, "vol": 0, "pan": 0}
        self.add("op", line_no, delta, op=OP_OFF, args=[c])

    def v_post(self, verb, args, delta, line_no):
        self.add("op", line_no, delta, op=OP_POST, args=[int(args[0], 0) & 0xFF])

    def v_nop(self, verb, args, delta, line_no):
        self.add("op", line_no, delta, op=OP_NOP, args=[])

    def v_wait(self, verb, args, delta, line_no):
        self.add("op", line_no, delta, op=OP_WAIT, args=[])

    def v_mark(self, verb, args, delta, line_no):
        if len(self.mark_states) >= 4:
            raise ZpsndError("line %d: marks nest at most 4 deep" % line_no)
        self.finished_after_delta(delta, line_no, "mark")
        self.mark_states.append(self.state())
        self.add("op", line_no, delta, op=OP_MARK, args=[])

    def finished_after_delta(self, delta, line_no, what):
        self.time += delta
        try:
            self.finished(line_no, what)
        finally:
            self.time -= delta

    def v_rep(self, verb, args, delta, line_no):
        if not self.mark_states:
            raise ZpsndError("line %d: rep without a mark" % line_no)
        n = int(args[0])
        if not 1 <= n <= 255:
            raise ZpsndError("line %d: rep count must be 1-255" % line_no)
        self.finished_after_delta(delta, line_no, "rep")
        if self.mark_states.pop() != self.state():
            raise ZpsndError("line %d: the channel state at 'rep' differs from the state at its "
                             "'mark'; put a SET after the mark" % line_no)
        self.add("op", line_no, delta, op=OP_REP, args=[n])

    def jump_common(self, name, delta, line_no, verb):
        self.finished_after_delta(delta, line_no, verb)
        snap = self.state()
        if name in self.label_state:
            if self.label_state[name] != snap:
                raise ZpsndError("line %d: the channel state at '%s %s' differs from the state at "
                                 "label '%s'; put a SET after the label so the loop is stable"
                                 % (line_no, verb, name, name))
        else:
            prev = self.label_state.get("<fwd>" + name)
            if prev is not None and prev != snap:
                raise ZpsndError("line %d: jumps to '%s' arrive in different channel states"
                                 % (line_no, name))
            self.label_state["<fwd>" + name] = snap

    def v_jump(self, verb, args, delta, line_no):
        self.jump_common(args[0], delta, line_no, "jump")
        self.add("op", line_no, delta, op=OP_JUMP, args=[("label", args[0])])
        self.reachable = False

    def v_jz(self, verb, args, delta, line_no):
        self.jump_common(args[0], delta, line_no, "jz")
        self.add("op", line_no, delta, op=OP_JZ, args=[("label", args[0])])

    def v_jeq(self, verb, args, delta, line_no):
        self.jump_common(args[1], delta, line_no, "jeq")
        self.add("op", line_no, delta, op=OP_JEQ,
                 args=[int(args[0], 0) & 0xFF, ("label", args[1])])

    def v_end(self, verb, args, delta, line_no):
        self.add("op", line_no, delta, op=OP_END, args=[])
        self.reachable = False

    # -- finish ------------------------------------------------------------
    def encode(self):
        # Segments between control-flow items are ordered by time (slide
        # continuations land among the author's events); markers stay put.
        markers = (OP_MARK, OP_REP, OP_JUMP, OP_JZ, OP_JEQ, OP_END, OP_WAIT)
        segments, cur = [], []
        for it in self.items:
            if isinstance(it, tuple) or it.get("op") in markers:
                segments.append(sorted(cur, key=lambda e: (e["time"], e["seq"])))
                segments.append([it])
                cur = []
            else:
                cur.append(it)
        segments.append(sorted(cur, key=lambda e: (e["time"], e["seq"])))
        ordered = [it for seg in segments for it in seg]

        out = bytearray()
        labels, fixups, listing = {}, [], []
        prev_time = 0
        for it in ordered:
            if isinstance(it, tuple):
                labels[it[1]] = len(out)
                listing.append("%04X  label %s" % (len(out), it[1]))
                continue
            delta = it["time"] - prev_time
            if delta < 0:
                raise ZpsndError("line %d: event is out of order after slide expansion" % it["line"])
            prev_time = it["time"]
            at = len(out)
            if delta < 128:
                out.append(delta)
            elif delta < 32768:
                out += bytes([0x80 | (delta >> 8), delta & 0xFF])
            else:
                raise ZpsndError("line %d: gap of %d ticks is too long (max 32767)" % (it["line"], delta))
            out.append(it["op"])
            for kind, val in zip(OP_ARGS[it["op"]], it["args"]):
                if isinstance(val, tuple):
                    fixups.append((len(out), val[1], it["line"]))
                    out += b"\0\0"
                elif kind == "b":
                    out.append(val & 0xFF)
                elif kind == "w":
                    out += struct.pack(">H", val & 0xFFFF)
                else:
                    out += struct.pack(">h", val)
            listing.append("%04X  +%-5d %-5s %s" % (at, delta, OP_NAMES[it["op"]],
                                                    " ".join(str(a) for a in it["args"])))
        for pos, name, line in fixups:
            if name not in labels:
                raise ZpsndError("line %d: unknown label '%s'" % (line, name))
            out[pos:pos + 2] = struct.pack(">H", labels[name])
        if not any(isinstance(i, dict) and i.get("op") == OP_END for i in self.items):
            raise ZpsndError("the score has no END")
        if len(out) > SCORE_MAX:
            raise ZpsndError("score is %d bytes, the BIOS checksums only %d" % (len(out), SCORE_MAX))
        return bytes(out), listing


def parse_zps(text, base_dir):
    samples, comp = [], None
    in_score = False
    for line_no, raw in enumerate(text.splitlines(), 1):
        line = raw.split(";", 1)[0].strip()
        if not line:
            continue
        tokens = line.split()
        if not in_score:
            if tokens[0] == "tickrate":
                if int(tokens[1]) != TICK_RATE:
                    raise ZpsndError("line %d: only tickrate %d is supported" % (line_no, TICK_RATE))
            elif tokens[0] == "sample":
                if len(tokens) < 3:
                    raise ZpsndError("line %d: sample NAME KIND key=value..." % line_no)
                if any(s.name == tokens[1] for s in samples):
                    raise ZpsndError("line %d: sample '%s' defined twice" % (line_no, tokens[1]))
                samples.append(build_sample(tokens[1], tokens[2],
                                            parse_kv(tokens[3:], line_no), base_dir, line_no))
            elif tokens[0] == "score":
                bake_banks(samples)             # assigns sample indices before the score uses them
                comp = Compiler(samples)
                in_score = True
            else:
                raise ZpsndError("line %d: unexpected '%s' before 'score'" % (line_no, tokens[0]))
        else:
            comp.line(tokens, line_no)
    if comp is None:
        raise ZpsndError("no 'score' section")
    return samples, comp


# --------------------------------------------------------------------------
# container
# --------------------------------------------------------------------------

def bios_header(score):
    padded = score + bytes(SCORE_MAX - len(score))
    header = bytearray(32)
    header[0] = MAGIC_ZPA
    header[4] = SCORE_BASE & 0xFF
    header[5] = SCORE_BASE >> 8
    header[31] = sum(padded) & 0xFF
    return bytes(header)


def build_container(stl, sst, score):
    head = b"ZPA1" + struct.pack("<BBHHHHH", TICK_RATE, 0, len(stl), len(sst), len(score), 0, 0)
    return head + stl + sst + score + bios_header(score)


def parse_container(blob):
    if blob[:4] != b"ZPA1":
        raise ZpsndError("not a ZPA1 pack")
    tick, flags, nstl, nsst, nscore, _, _ = struct.unpack("<BBHHHHH", blob[4:16])
    p = 16
    stl, sst, score = blob[p:p + nstl], blob[p + nstl:p + nstl + nsst], \
        blob[p + nstl + nsst:p + nstl + nsst + nscore]
    header = blob[p + nstl + nsst + nscore:p + nstl + nsst + nscore + 32]
    return {"tick": tick, "stl": stl, "sst": sst, "score": score, "header": header}


# --------------------------------------------------------------------------
# reference interpreter
# --------------------------------------------------------------------------

def decode_event(score, ptr):
    """Return (delta, op, args, next_ptr) for the event at ptr."""
    b = score[ptr]
    ptr += 1
    if b & 0x80:
        delta = ((b & 0x7F) << 8) | score[ptr]
        ptr += 1
    else:
        delta = b
    op = score[ptr]
    ptr += 1
    args = []
    for kind in OP_ARGS[op]:
        if kind == "b":
            args.append(score[ptr])
            ptr += 1
        elif kind == "w":
            args.append((score[ptr] << 8) | score[ptr + 1])
            ptr += 2
        else:
            args.append(struct.unpack(">h", score[ptr:ptr + 2])[0])
            ptr += 2
    return delta, op, args, ptr


def trace_score(pack, max_ticks, mailbox=None):
    """Reference engine: run the score and return the MMP register writes per tick.

    This defines the behaviour every engine (LLE BIOS, HLE BIOS) must reproduce.
    Per tick: run every event whose delta has elapsed, advance all slides one
    step, then write whatever channel values changed. mailbox maps tick -> byte
    the CPU wrote to $1000 at the start of that tick.
    """
    score = pack["score"]
    mail = sorted((mailbox or {}).items())
    mbox = 0
    lines = []
    zero_slides = lambda: {"pitch": [0, 0], "vol": [0, 0], "pan": [0, 0]}
    chans = [{"stl": 0, "pending": 0, "pitch": None, "vol": 0, "pan": 0,
              "slide": zero_slides(), "written": {}} for _ in range(CHANNELS)]

    def write(tick, c, field, value):
        lines.append("t%04d ch%-2d %-5s %d" % (tick, c, field, value))

    def stop_channel(tick, c):
        if chans[c]["stl"] or chans[c]["pending"]:
            write(tick, c, "stl", 0)
        chans[c]["stl"] = chans[c]["pending"] = 0

    ptr = 0                                   # start of the next event to run
    cnt = decode_event(score, 0)[0]           # its delta, counted from tick 0
    frames = []                               # REP stack: [loop_start_ptr, left or None]
    waiting = False                           # a WAIT is holding the score

    for tick in range(max_ticks):
        while mail and mail[0][0] <= tick:
            mbox = mail.pop(0)[1]
        if waiting and mbox != 0:
            waiting = False
        guard = 0
        while not waiting and cnt == 0:
            guard += 1
            if guard > 10000:
                raise ZpsndError("tick %d: over 10000 events in one tick (a zero-delta loop?)" % tick)
            _, op, args, nxt = decode_event(score, ptr)
            ptr = nxt
            jump = None
            if op == OP_END:
                for c in range(CHANNELS):
                    stop_channel(tick, c)
                lines.append("t%04d END" % tick)
                return lines
            elif op == OP_NOTE:
                c, idx, pitch, vol, pan = args
                ch = chans[c]
                ch["slide"] = zero_slides()
                ch["pitch"], ch["vol"], ch["pan"] = pitch, vol << 8, pan << 8
                ch["written"] = {}
                if ch["stl"]:
                    write(tick, c, "stl", 0)          # same address would not retrigger
                ch["stl"] = 0
                ch["pending"] = STL_BASE + idx * 4
            elif op == OP_SET:
                c, pitch, vol, pan = args
                ch = chans[c]
                ch["slide"] = zero_slides()
                ch["pitch"], ch["vol"], ch["pan"] = pitch, vol << 8, pan << 8
            elif op in (OP_PITCH, OP_VOL, OP_PAN):
                c, dl, n = args
                key = {OP_PITCH: "pitch", OP_VOL: "vol", OP_PAN: "pan"}[op]
                chans[c]["slide"][key] = [n, dl]
            elif op == OP_OFF:
                chans[args[0]]["slide"] = zero_slides()
                stop_channel(tick, args[0])
            elif op == OP_POST:
                lines.append("t%04d POST  %d" % (tick, args[0]))
            elif op == OP_MARK:
                frames.append([ptr, None])
            elif op == OP_REP:
                if not frames:
                    raise ZpsndError("tick %d: REP without MARK" % tick)
                if frames[-1][1] is None:
                    frames[-1][1] = args[0]
                if frames[-1][1] > 0:
                    frames[-1][1] -= 1
                    jump = frames[-1][0]
                else:
                    frames.pop()
            elif op == OP_JUMP:
                jump = args[0]
            elif op == OP_JZ:
                jump = args[0] if mbox == 0 else None
            elif op == OP_JEQ:
                jump = args[1] if mbox == args[0] else None
            elif op == OP_WAIT:
                waiting = mbox == 0
            if jump is not None:
                ptr = jump
            if ptr >= len(score):
                raise ZpsndError("tick %d: the score ran off its end without END" % tick)
            cnt = decode_event(score, ptr)[0]         # next delta counts from now (or the jump)

        for c, ch in enumerate(chans):
            for key in ("pitch", "vol", "pan"):
                n, dl = ch["slide"][key]
                if n > 0:
                    ch["slide"][key][0] = n - 1
                    ch[key] = max(0, min(0xFFFF, ch[key] + dl))
            if ch["pitch"] is None:
                continue
            now = {"pitch": ch["pitch"], "vol": ch["vol"] >> 8, "pan": ch["pan"] >> 8}
            for key in ("pitch", "vol", "pan"):
                if ch["written"].get(key) != now[key]:
                    ch["written"][key] = now[key]
                    write(tick, c, key, now[key])
            if ch["pending"]:
                ch["stl"] = ch["pending"]
                write(tick, c, "stl", ch["stl"])
                ch["pending"] = 0
        if not waiting and cnt > 0:
            cnt -= 1
    return lines


# --------------------------------------------------------------------------
# command line
# --------------------------------------------------------------------------

def cmd_bake(args):
    with open(args.score) as f:
        text = f.read()
    samples, comp = parse_zps(text, os.path.dirname(os.path.abspath(args.score)))
    stl, sst = bake_banks(samples)
    score, listing = comp.encode()
    blob = build_container(stl, sst, score)
    with open(args.output, "wb") as f:
        f.write(blob)
    print("%s: %d samples, STL %d B, SST %d B of %d, score %d B of %d" %
          (args.output, len(samples), len(stl), len(sst), SST_SIZE, len(score), SCORE_MAX))
    for s in samples:
        print("  [%d] %-12s %5d samples @ %5d Hz  %s  SST $%04X  STL $%04X" %
              (s.index, s.name, len(s.data), s.rate,
               "one-shot" if s.one_shot else "loop from %d" % s.loop_start,
               SST_BASE + s.sst_offset, STL_BASE + s.index * 4))
    if args.listing:
        with open(args.listing, "w") as f:
            f.write("\n".join(listing) + "\n")


def cmd_trace(args):
    with open(args.pack, "rb") as f:
        pack = parse_container(f.read())
    mailbox = {}
    for item in args.mailbox or []:
        t, v = item.split("=")
        mailbox[int(t)] = int(v, 0)
    for line in trace_score(pack, args.ticks, mailbox):
        print(line)


def cmd_info(args):
    with open(args.pack, "rb") as f:
        pack = parse_container(f.read())
    print("tick rate %d Hz, STL %d B, SST %d B, score %d B, header checksum $%02X" %
          (pack["tick"], len(pack["stl"]), len(pack["sst"]), len(pack["score"]),
           pack["header"][31]))


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    sub = ap.add_subparsers(dest="cmd", required=True)
    b = sub.add_parser("bake")
    b.add_argument("score")
    b.add_argument("-o", "--output", required=True)
    b.add_argument("--listing")
    b.set_defaults(fn=cmd_bake)
    t = sub.add_parser("trace")
    t.add_argument("pack")
    t.add_argument("--ticks", type=int, default=600)
    t.add_argument("--mailbox", action="append", metavar="TICK=VALUE")
    t.set_defaults(fn=cmd_trace)
    i = sub.add_parser("info")
    i.add_argument("pack")
    i.set_defaults(fn=cmd_info)
    args = ap.parse_args(argv)
    try:
        args.fn(args)
    except ZpsndError as e:
        print("zpsnd: %s" % e, file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())

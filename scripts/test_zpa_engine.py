#!/usr/bin/env python3
"""Tests for the APU BIOS sequencer engine (ZPbootROM/apu/internalBIOS/engine.uas).

Every case builds a ZPA pack, runs it through the zpsnd reference interpreter
(the golden behaviour, docs/apu/zpa.txt) and through the real BIOS on the real
APU core (ZeroPoint's test_zpa_engine harness), and requires the MMP register
writes to be identical, tick for tick. Scores are hand-encoded here so they can
exercise what the baker never emits (clamping, deep REP nesting, odd ops).

    python3 scripts/test_zpa_engine.py

Needs a built ZeroPoint (build/bin/test_zpa_engine, or set TEST_ZPA_ENGINE) and
the ZPbootROM checkout next to ZPdevtools (or set ZPBOOTROM).
"""

import os
import shutil
import struct
import subprocess
import sys
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import zpsnd  # noqa: E402

ROOT = os.path.abspath(os.path.join(HERE, "..", ".."))
HARNESS = os.environ.get("TEST_ZPA_ENGINE") or os.path.join(ROOT, "ZeroPoint", "build", "bin", "test_zpa_engine")
BOOTROM = os.environ.get("ZPBOOTROM") or os.path.join(ROOT, "ZPbootROM")
APUASM_SRC = os.path.join(HERE, "..", "src", "apuasm.c")

OPS = zpsnd.OP_ARGS


def enc(events):
    """events: list of (delta, op, *args) -> score bytes."""
    out = bytearray()
    for ev in events:
        delta, op, args = ev[0], ev[1], ev[2:]
        if delta < 128:
            out.append(delta)
        else:
            out += bytes([0x80 | (delta >> 8), delta & 0xFF])
        out.append(op)
        assert len(args) == len(OPS[op]), (op, args)
        for kind, a in zip(OPS[op], args):
            if kind == "b":
                out.append(a)
            elif kind == "w":
                out += struct.pack(">H", a)
            else:
                out += struct.pack(">h", a)
    return bytes(out)


def offsets(events):
    """Byte offset of each event in the encoded score (for JUMP/JZ targets)."""
    pos, offs = 0, []
    for ev in events:
        offs.append(pos)
        pos += len(enc([ev]))
    return offs


class EngineTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        if not os.path.exists(HARNESS):
            raise unittest.SkipTest("build ZeroPoint's test_zpa_engine (or set TEST_ZPA_ENGINE)")
        cls.tmp = tempfile.mkdtemp()
        asm = os.path.join(cls.tmp, "apuasm")
        subprocess.run(["gcc", "-O1", "-o", asm, APUASM_SRC], check=True)
        cls.bios = os.path.join(cls.tmp, "bios.bin")
        r = subprocess.run([asm, os.path.join(BOOTROM, "apu", "internalBIOS", "launch.uas"), cls.bios],
                           capture_output=True, text=True)
        if r.returncode != 0:
            raise AssertionError("BIOS does not assemble:\n" + r.stderr)

    @classmethod
    def tearDownClass(cls):
        shutil.rmtree(cls.tmp, ignore_errors=True)

    def check(self, score, ticks, mailbox=None):
        """Run both engines on `score` and require identical traces."""
        pack_bytes = zpsnd.build_container(b"", b"", score)
        pack = zpsnd.parse_container(pack_bytes)
        want = zpsnd.trace_score(pack, ticks, mailbox)
        path = os.path.join(self.tmp, "t.zpa")
        with open(path, "wb") as f:
            f.write(pack_bytes)
        args = [HARNESS, self.bios, path, str(ticks)] + ["%d=%d" % kv for kv in sorted((mailbox or {}).items())]
        r = subprocess.run(args, capture_output=True, text=True, timeout=120)
        self.assertEqual(r.returncode, 0, r.stderr)
        got = r.stdout.splitlines()
        self.assertEqual(want, got, "engine trace differs from the reference (%d vs %d writes)" % (len(want), len(got)))
        return got

    # -- ops ------------------------------------------------------------

    def test_note_set_off_post(self):
        N, S, O, P = zpsnd.OP_NOTE, zpsnd.OP_SET, zpsnd.OP_OFF, zpsnd.OP_POST
        ev = [
            (0, N, 0, 0, 0x1000, 200, 100),
            (3, N, 0, 1, 0x1000, 200, 100),    # retrigger: stl 0 first, then the new address
            (2, S, 0, 0x2345, 7, 250),
            (2, S, 0, 0x2345, 7, 250),          # same values: no writes at all
            (1, P, 42),
            (1, N, 15, 255, 0x0800, 1, 2),      # channel 15, sample 255 ($7000 + 255*4 = $73fc)
            (4, O, 0),
            (1, O, 0),                          # already off: nothing to stop
            (2, O, 15),
            (2, zpsnd.OP_END),
        ]
        self.check(enc(ev), 40)

    def test_slides_and_clamping(self):
        N, PI, VO, PA = zpsnd.OP_NOTE, zpsnd.OP_PITCH, zpsnd.OP_VOL, zpsnd.OP_PAN
        ev = [
            (0, N, 1, 0, 0xFFF0, 250, 5),
            (0, PI, 1, 0x0020, 3),               # up past $FFFF: clamps
            (0, VO, 1, 0x0300, 3),               # 8.8 volume past 255: clamps
            (0, PA, 1, -0x0200, 4),              # pan below 0: clamps
            (8, N, 2, 3, 0x0010, 0, 255),
            (0, PI, 2, -0x0030, 2),              # pitch below 0
            (0, VO, 2, 0x00A5, 5),
            (0, PA, 2, 0x0101, 3),
            (6, zpsnd.OP_END),
        ]
        self.check(enc(ev), 30)

    def test_slide_replaced_mid_flight_and_cancelled_by_set(self):
        N, PI, S = zpsnd.OP_NOTE, zpsnd.OP_PITCH, zpsnd.OP_SET
        ev = [
            (0, N, 0, 0, 0x1000, 128, 128),
            (0, PI, 0, 0x0010, 20),
            (5, PI, 0, -0x0008, 10),              # replaces the running slide
            (4, S, 0, 0x1800, 128, 128),          # SET cancels slides
            (4, zpsnd.OP_END),
        ]
        self.check(enc(ev), 30)

    def test_rep_nesting_and_zero_count(self):
        N, M, R, PI = zpsnd.OP_NOTE, zpsnd.OP_MARK, zpsnd.OP_REP, zpsnd.OP_PITCH
        ev = [
            (0, N, 0, 0, 0x1000, 100, 100),
            (0, M),
            (1, PI, 0, 5, 1),
            (0, M),
            (1, PI, 0, 1, 1),
            (1, R, 2),                            # inner: 3 passes
            (1, R, 3),                            # outer: 4 passes
            (0, M),
            (1, R, 0),                            # REP 0: falls through at once
            (2, zpsnd.OP_END),
        ]
        self.check(enc(ev), 120)

    def test_rep_four_deep(self):
        N, M, R, PI = zpsnd.OP_NOTE, zpsnd.OP_MARK, zpsnd.OP_REP, zpsnd.OP_PITCH
        ev = [(0, N, 0, 0, 0x1000, 100, 100)] + [(0, M)] * 4 + [(1, PI, 0, 1, 1)] + \
             [(0, R, 1)] * 4 + [(1, zpsnd.OP_END)]
        self.check(enc(ev), 80)

    def test_jump_loop_and_mailbox_jumps(self):
        N, J, JZ, JEQ, PI, E = zpsnd.OP_NOTE, zpsnd.OP_JUMP, zpsnd.OP_JZ, zpsnd.OP_JEQ, zpsnd.OP_PITCH, zpsnd.OP_END
        head = [(0, N, 0, 0, 0x1000, 100, 100), (2, PI, 0, 1, 1)]
        # loop: pitch tick, JZ back while the mailbox is 0, then JEQ 5 to an alternate ending
        base = len(enc(head))
        body = [(1, PI, 0, 2, 1), (1, JZ, 0), (1, JEQ, 5, 0), (1, E)]
        offs = offsets(head + body)
        body[1] = (1, JZ, offs[2])               # to the first body event (PITCH)
        body[2] = (1, JEQ, 5, offs[len(head) + 3])   # on 5: jump to END
        self.assertEqual(base, offs[2])
        ev = head + body
        self.check(enc(ev), 60, {20: 3})          # 3: JEQ 5 not taken, falls to END
        self.check(enc(ev), 60, {20: 5})

    def test_wait_holds_until_the_mailbox(self):
        N, W, PI = zpsnd.OP_NOTE, zpsnd.OP_WAIT, zpsnd.OP_PITCH
        ev = [(0, N, 0, 0, 0x1000, 100, 100), (3, W), (2, PI, 0, 9, 2), (2, zpsnd.OP_END)]
        self.check(enc(ev), 80, {25: 1})
        self.check(enc(ev), 80, {0: 1})           # already non-zero: WAIT passes straight through

    def test_long_deltas(self):
        N, PI, P = zpsnd.OP_NOTE, zpsnd.OP_PITCH, zpsnd.OP_POST
        ev = [(0, N, 0, 0, 0x1000, 1, 1), (127, P, 1), (128, P, 2), (300, PI, 0, 1, 3), (1000, P, 3),
              (2000, zpsnd.OP_END)]
        self.check(enc(ev), 3500)

    def test_zero_delta_burst_on_all_channels(self):
        N, VO = zpsnd.OP_NOTE, zpsnd.OP_VOL
        ev = [(0, N, c, c, 0x1000 + c, 16 * c, 255 - c) for c in range(16)]
        ev += [(0, VO, c, 0x0010 * c - 0x40, 6) for c in range(16)]
        ev += [(7, zpsnd.OP_END)]
        self.check(enc(ev), 16)

    # -- the real thing -------------------------------------------------

    def test_the_jingle(self):
        score = os.path.join(BOOTROM, "apu", "jingle", "jingle.zps")
        if not os.path.exists(score):
            self.skipTest("no jingle.zps")
        samples, comp = zpsnd.parse_zps(open(score).read(), os.path.dirname(score))
        stl, sst = zpsnd.bake_banks(samples)
        sc, _ = comp.encode()
        for mb in ({700: 1}, {431: 2}, {}):
            ticks = (max(mb) + 60) if mb else 500
            self.check(sc, ticks, mb)


if __name__ == "__main__":
    unittest.main()

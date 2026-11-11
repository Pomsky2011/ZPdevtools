#!/usr/bin/env python3
"""Tests for zpsnd.py. Run: python3 scripts/test_zpsnd.py"""

import os
import random
import sys
import tempfile
import unittest

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import zpsnd  # noqa: E402


def bake(text):
    samples, comp = zpsnd.parse_zps(text, ".")
    stl, sst = zpsnd.bake_banks(samples)
    score, listing = comp.encode()
    return samples, stl, sst, score, listing


def pack_of(text):
    samples, stl, sst, score, _ = bake(text)
    return zpsnd.parse_container(zpsnd.build_container(stl, sst, score))


TONES = "sample saw saw cycle=96\nsample sine sine cycle=96\n"


class SampleBaking(unittest.TestCase):
    def test_block_layout_and_loop_flags(self):
        samples, stl, sst, _, _ = bake(TONES + "score\n@0 note 0 saw\n+0 end\n")
        saw = samples[0]
        self.assertEqual(len(saw.data), 96)
        blocks = [sst[i * 16:(i + 1) * 16] for i in range(8)]
        for b in blocks[:-1]:
            self.assertEqual(b[:4], bytes([0, 0, 0, 0]))
        self.assertEqual(blocks[-1][:4], bytes([0, 0x04, 0, 0]))      # W only: loops
        self.assertEqual(stl[0:4], bytes([0x00, 0x90, 0x00, 0x00]))   # $9000, loop word 0

    def test_samples_are_signed_twos_complement(self):
        samples, _, sst, _, _ = bake(TONES + "score\n@0 note 0 saw\n+0 end\n")
        first = [v for v in sst[4:16]]
        # a saw cycle starts at -amp, so its first bytes are >= 0x80
        self.assertTrue(first[0] >= 0x80)
        self.assertEqual(first[0], samples[0].data[0] & 0xFF)

    def test_one_shot_sets_z_and_w_and_pads_to_12(self):
        text = "sample n noise rate=8000 len=0.0125\nscore\n@0 note 0 n\n+0 end\n"
        samples, stl, sst, _, _ = bake(text)
        self.assertEqual(len(samples[0].data) % 12, 0)
        self.assertTrue(samples[0].one_shot)
        self.assertEqual(sst[-16 + 1], 0x0C)                           # Z | W
        self.assertEqual(stl[2:4], bytes([0, 0]))

    def test_cycle_repeats_to_a_multiple_of_12(self):
        samples, _, _, _, _ = bake("sample s sine cycle=64\nscore\n@0 note 0 s hz=100\n+0 end\n")
        self.assertEqual(len(samples[0].data), 192)                    # lcm(64, 12)
        self.assertEqual(samples[0].data[:64], samples[0].data[64:128])

    def test_loop_point_and_length(self):
        text = ("sample r noise rate=4000 len=1.0 lp=300 loop=0.15..1.0 xfade=100\n"
                "score\n@0 note 0 r\n+0 end\n")
        samples, stl, _, _, _ = bake(text)
        r = samples[0]
        self.assertEqual(len(r.data) % 12, 0)
        self.assertEqual(r.loop_start, 600)
        self.assertEqual(int.from_bytes(stl[2:4], "little"), 600)
        self.assertEqual(len(r.data), 3996)                            # 4000 rounded down

    def test_crossfade_makes_the_seam_continuous(self):
        text = ("sample r noise rate=4000 len=1.0 loop=0.15..1.0 xfade=200\n"
                "score\n@0 note 0 r\n+0 end\n")
        r = bake(text)[0][0]
        # the last sample must continue into the loop start like the sample before it does
        self.assertLessEqual(abs(r.data[-1] - r.data[r.loop_start - 1]), 2)

    def test_deterministic(self):
        text = TONES + "sample n noise rate=8000 len=0.1\nscore\n@0 note 0 n\n+0 end\n"
        a, b = bake(text), bake(text)
        self.assertEqual((a[1], a[2], a[3]), (b[1], b[2], b[3]))

    def test_sst_overflow_is_an_error(self):
        with self.assertRaises(zpsnd.ZpsndError):
            bake("sample big noise rate=48000 len=0.5\nscore\n@0 note 0 big\n+0 end\n")


class SlidePlanning(unittest.TestCase):
    def test_lands_exactly_in_exactly_t_ticks(self):
        rng = random.Random(1)
        for _ in range(500):
            start, target = rng.randrange(0, 60000), rng.randrange(0, 60000)
            ticks = rng.randrange(1, 700)
            chunks, final = zpsnd.plan_slide("pitch", start, target, ticks, 0)
            self.assertEqual(final, target)
            self.assertEqual(start + sum(n * d for n, d in chunks), target)
            self.assertTrue(all(1 <= n <= 255 for n, _ in chunks))
            total = sum(n for n, _ in chunks)
            if abs(target - start) >= ticks:
                self.assertEqual(total, ticks)
            else:
                self.assertEqual(total, abs(target - start))           # small slide finishes early

    def test_no_change_emits_nothing(self):
        self.assertEqual(zpsnd.plan_slide("vol", 500, 500, 10, 0), ([], 500))

    def test_too_fast_is_an_error(self):
        with self.assertRaises(zpsnd.ZpsndError):
            zpsnd.plan_slide("vol", 0, 255 * 256, 1, 0)


class ScoreErrors(unittest.TestCase):
    def err(self, body, samples=TONES):
        with self.assertRaises(zpsnd.ZpsndError) as cm:
            bake(samples + "score\n" + body)
        return str(cm.exception)

    def test_no_end(self):
        self.assertIn("no END", self.err("@0 note 0 saw\n"))

    def test_slide_before_note(self):
        self.assertIn("before any note", self.err("@0 slide 0 vol to=10 over=0.1\n+0 end\n"))

    def test_overlapping_slides_on_one_parameter(self):
        self.assertIn("still running",
                      self.err("@0 note 0 saw vol=0\n+0 slide 0 vol to=200 over=1.0\n"
                               "+0.1 slide 0 vol to=0 over=1.0\n+0 end\n"))

    def test_label_while_a_slide_is_running(self):
        self.assertIn("still running",
                      self.err("@0 note 0 saw vol=0\n+0 slide 0 vol to=200 over=1.0\n"
                               "label L\n+0 jz L\n+0 end\n"))

    def test_loop_must_return_to_the_same_state(self):
        msg = self.err("@0 note 0 saw vol=0\nlabel L\n+0 slide 0 vol to=200 over=0.1\n"
                       "+0.2 jz L\n+0 end\n")
        self.assertIn("differs", msg)

    def test_unknown_label(self):
        self.assertIn("unknown label", self.err("@0 note 0 saw\n+0 jz nowhere\n+0 end\n"))

    def test_absolute_time_after_a_label(self):
        self.assertIn("+TIME", self.err("@0 note 0 saw\nlabel L\n@1 nop\n+0 jz L\n+0 end\n"))

    def test_unknown_sample_and_channel(self):
        self.assertIn("unknown sample", self.err("@0 note 0 nope\n+0 end\n"))
        self.assertIn("out of range", self.err("@0 note 16 saw\n+0 end\n"))


class Encoding(unittest.TestCase):
    def test_round_trip(self):
        _, _, _, score, _ = bake(TONES + "score\n@0 note 2 saw hz=100 vol=10 pan=20\n"
                                         "+0.5 off 2\n+0 post 7\n+0 end\n")
        ptr, ops = 0, []
        while ptr < len(score):
            delta, op, args, ptr = zpsnd.decode_event(score, ptr)
            ops.append((delta, op, args))
        self.assertEqual(ops[0], (0, zpsnd.OP_NOTE, [2, 0, 819, 10, 20]))
        self.assertEqual(ops[1], (60, zpsnd.OP_OFF, [2]))
        self.assertEqual(ops[2], (0, zpsnd.OP_POST, [7]))
        self.assertEqual(ops[3][1], zpsnd.OP_END)

    def test_long_delta_uses_two_bytes(self):
        _, _, _, long_score, _ = bake(TONES + "score\n@0 note 0 saw\n+1000t nop\n+0 end\n")
        ptr = 8
        d, op, _, _ = zpsnd.decode_event(long_score, ptr)
        self.assertEqual((d, op), (1000, zpsnd.OP_NOP))
        self.assertEqual(long_score[ptr] & 0x80, 0x80)

    def test_container_and_bios_header(self):
        pack = pack_of(TONES + "score\n@0 note 0 saw\n+0 end\n")
        h = pack["header"]
        self.assertEqual(h[0], 0x0B)
        self.assertEqual((h[4], h[5]), (0x20, 0x30))                   # entry $3020
        padded = pack["score"] + bytes(zpsnd.SCORE_MAX - len(pack["score"]))
        self.assertEqual(h[31], sum(padded) & 0xFF)


class ReferenceTrace(unittest.TestCase):
    SCORE = (TONES + "score\n@0 note 0 saw hz=100 vol=0 pan=128\n"
                     "+0 slide 0 vol to=100 over=0.5\n+0.5 nop\n"
                     "label hold\n+0.25 jz hold\n+0 end\n")

    def values(self, trace, ch, field):
        return [(int(l[1:5]), int(l.split()[-1])) for l in trace
                if l.startswith("t") and (" ch%-2d " % ch) in l and (" %-5s " % field) in l]

    def test_note_then_slide_reaches_target(self):
        trace = zpsnd.trace_score(pack_of(self.SCORE), 200, {50: 1})
        vols = dict(self.values(trace, 0, "vol"))
        self.assertEqual(vols[0], 1)                                   # first step on tick 0
        self.assertEqual(vols[59], 100)                                # 60 ticks later: target

    def test_hold_loops_until_the_mailbox_then_ends_and_silences(self):
        trace = zpsnd.trace_score(pack_of(self.SCORE), 400, {200: 1})
        end = [l for l in trace if l.endswith("END")][0]
        end_tick = int(end[1:5])
        self.assertGreaterEqual(end_tick, 200)
        self.assertLess(end_tick, 200 + 31)                            # within one 0.25 s hold loop
        self.assertIn("t%04d ch0  stl   0" % end_tick, trace)

    def test_loops_forever_without_a_result(self):
        trace = zpsnd.trace_score(pack_of(self.SCORE), 1000)
        self.assertFalse([l for l in trace if l.endswith("END")])

    def test_retrigger_writes_stl_zero_first(self):
        text = TONES + "score\n@0 note 0 saw\n+0.1 note 0 saw\n+0.1 end\n"
        trace = zpsnd.trace_score(pack_of(text), 30)
        stl = [l for l in trace if "stl" in l]
        # note, retrigger (0 first, or the same address would not restart), then END silences it
        self.assertEqual([l.split()[-1] for l in stl], ["28672", "0", "28672", "0"])

    def test_rep_runs_the_body_count_plus_one_times(self):
        text = (TONES + "score\n@0 note 0 saw vol=0\n+0 mark\n+0.1 post 1\n+0 rep 2\n+0 end\n")
        trace = zpsnd.trace_score(pack_of(text), 100)
        self.assertEqual(len([l for l in trace if "POST" in l]), 3)

    def test_wait_holds_until_the_mailbox(self):
        text = TONES + "score\n@0 note 0 saw\n+0 wait\n+0 post 9\n+0 end\n"
        trace = zpsnd.trace_score(pack_of(text), 100, {40: 1})
        post = [l for l in trace if "POST" in l][0]
        self.assertEqual(int(post[1:5]), 40)


ZPA_PLAY = os.environ.get("ZPA_PLAY") or os.path.join(
    os.path.dirname(os.path.abspath(__file__)), "..", "..", "ZeroPoint", "build", "bin", "zpa_play")


@unittest.skipUnless(os.path.exists(ZPA_PLAY), "build ZeroPoint's zpa_play (or set ZPA_PLAY)")
class MixerPlayback(unittest.TestCase):
    """What zpsnd bakes is what the real mixer plays (ZeroPoint's zpa_play)."""

    TEXT = (TONES + "sample crack noise rate=9000 len=0.05 hp=300 lp=3500\n"
            "sample rumble noise rate=4000 len=0.2 lp=300 loop=0.05..0.2 xfade=40\n"
            "score\n@0 note 0 saw\n+0 end\n")

    @classmethod
    def setUpClass(cls):
        import subprocess
        cls.samples, stl, sst, score, _ = bake(cls.TEXT)
        cls.dir = tempfile.TemporaryDirectory()
        cls.path = os.path.join(cls.dir.name, "t.zpa")
        with open(cls.path, "wb") as f:
            f.write(zpsnd.build_container(stl, sst, score))
        cls.subprocess = subprocess

    @classmethod
    def tearDownClass(cls):
        cls.dir.cleanup()

    def play(self, index, frames):
        out = self.subprocess.run([ZPA_PLAY, self.path, str(index), str(frames)],
                                  capture_output=True, text=True, check=True).stdout.split()
        return [int(v) for v in out]

    def test_tone_loops_forever(self):
        saw = self.samples[0].data
        self.assertEqual(self.play(0, 3 * len(saw) + 5),
                         (saw * 4)[:3 * len(saw) + 5])

    def test_one_shot_plays_once_then_is_silent(self):
        crack = self.samples[2].data
        got = self.play(2, len(crack) + 50)
        self.assertEqual(got[:len(crack)], crack)
        self.assertEqual(got[len(crack):], [0] * 50)

    def test_loop_point_wraps_to_the_loop_start(self):
        r = self.samples[3]
        n = len(r.data)
        got = self.play(3, n + 2 * (n - r.loop_start) + 7)
        self.assertEqual(got[:n], r.data)
        tail = r.data[r.loop_start:]
        expect = (tail * 4)[:len(got) - n]
        self.assertEqual(got[n:], expect)

    def test_pitch_scales_playback_speed(self):
        saw = self.samples[0].data
        got = self.subprocess.run([ZPA_PLAY, self.path, "0", "24", "0x2000"],
                                  capture_output=True, text=True, check=True).stdout.split()
        self.assertEqual([int(v) for v in got], saw[0:48:2])          # 2.0x: every other sample


class CommandLine(unittest.TestCase):
    def test_bake_info_trace_and_the_real_jingle(self):
        jingle = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                              "..", "..", "ZPbootROM", "apu", "jingle", "jingle.zps")
        if not os.path.exists(jingle):
            self.skipTest("ZPbootROM checkout not next to ZPdevtools")
        with tempfile.TemporaryDirectory() as d:
            out = os.path.join(d, "j.zpa")
            self.assertEqual(zpsnd.main(["bake", jingle, "-o", out]), 0)
            self.assertEqual(zpsnd.main(["info", out]), 0)
            with open(out, "rb") as f:
                pack = zpsnd.parse_container(f.read())
            trace = zpsnd.trace_score(pack, 900, {700: 1})
            self.assertTrue([l for l in trace if l.endswith("END")])
            self.assertLessEqual(len(pack["sst"]), zpsnd.SST_SIZE)


if __name__ == "__main__":
    unittest.main(verbosity=1)

#!/usr/bin/env python3
"""Tests for png2zp.py. Run: python3 scripts/test_png2zp.py"""

import base64
import json
import os
import re
import struct
import sys
import tempfile
import unittest
import zlib

from PIL import Image

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import png2zp  # noqa: E402

TRANSP = None


def indexed(path, w, h, pixels, colours, transparent=None):
    """Write an indexed PNG: pixels are palette indices into `colours` (a list of (r, g, b))."""
    im = Image.new("P", (w, h))
    flat = []
    for c in colours:
        flat.extend(c)
    im.putpalette(flat)
    im.putdata(pixels)
    kw = {"transparency": transparent} if transparent is not None else {}
    im.save(path, **kw)


class Work(unittest.TestCase):
    def setUp(self):
        self._t = tempfile.TemporaryDirectory()
        self.dir = self._t.name
        self.out = os.path.join(self.dir, "out")
        self._memo = {}

    def tearDown(self):
        self._t.cleanup()

    def p(self, name):
        return os.path.join(self.dir, name)

    def manifest(self, spec):
        path = self.p("assets.json")
        with open(path, "w") as f:
            json.dump(spec, f)
        return path

    def convert(self, spec, quantize=0):
        a = png2zp.build(self.manifest(spec), quantize)
        png2zp.write(a, self.out)
        self._memo = {}
        return a

    # ---- reading the result back, independent of the converter's own tables ----
    def blob(self):
        if "blob" not in self._memo:
            with open(os.path.join(self.out, "assets.bin"), "rb") as f:
                self._memo["blob"] = f.read()
        return self._memo["blob"]

    def header(self):
        if "header" not in self._memo:
            with open(os.path.join(self.out, "assets.h")) as f:
                text = f.read()
            self._memo["header"] = dict((m.group(1), int(m.group(2), 0))
                                        for m in re.finditer(r"#define ASSET_(\w+) (0x[0-9A-F]+|\d+)", text))
        return self._memo["header"]

    def palette(self):
        h, b = self.header(), self.blob()
        base = h["PAL"] - 0x110000
        return struct.unpack("<256H", b[base:base + 512])

    def tile(self, bank, tid):
        h, b = self.header(), self.blob()
        off = h["TILES%d" % bank] - 0x110000 + 64 * tid
        return b[off:off + 64]

    def cell(self, name, n):
        h, b = self.header(), self.blob()
        off = h[name] - 0x110000 + 4 * n
        w0, w1 = struct.unpack("<HH", b[off:off + 4])
        return w0 & 255, w1 & 3, bool(w1 & 8), bool(w1 & 16)

    def render_map(self, layer, bank):
        """The 512x512 picture of a map as RGB555-ish palette words (0 = transparent)."""
        pal = self.palette()
        img = {}
        for cy in range(64):
            for cx in range(64):
                tid, b, hf, vf = self.cell("MAP%d" % layer, cy * 64 + cx)
                self.assertEqual(b, bank)
                t = self.tile(b, tid)
                for y in range(8):
                    for x in range(8):
                        sx, sy = 7 - x if hf else x, 7 - y if vf else y
                        i = t[sy * 8 + sx]
                        img[(cx * 8 + x, cy * 8 + y)] = pal[i] if i else 0
        return img


def word(r, g, b):
    return png2zp.pal_word(png2zp.rgb555(r, g, b))


def pattern(w, h, ncol, seed=1):
    out = []
    s = seed
    for _ in range(w * h):
        s = (s * 1103515245 + 12345) & 0x7FFFFFFF
        out.append(1 + (s >> 8) % ncol)
    return out


COLS = [(255, 0, 255)] + [(i * 8 % 256, (i * 40) % 256, (i * 90) % 256) for i in range(1, 40)]


class RoundTrip(Work):
    def test_bg_layers_survive_pixel_for_pixel(self):
        w = h = 512
        px0 = pattern(8, 8, 5)                     # a repeating 8x8 tile pattern: only 1 distinct tile after the repeat
        big = []
        for y in range(h):
            for x in range(w):
                big.append(px0[(y % 8) * 8 + x % 8] if (x // 8 + y // 8) % 3 else px0[(x % 8) * 8 + y % 8])
        indexed(self.p("g.png"), w, h, big, COLS)
        kinds = [pattern(8, 8, 3, 20 + k) for k in range(4)]       # four random 8x8 tiles, scattered over the map
        sky = [kinds[((x // 8) * 7 + (y // 8) * 3) % 4][(y % 8) * 8 + x % 8] for y in range(h) for x in range(w)]
        sky = [0 if v == 2 else v for v in sky]
        indexed(self.p("c.png"), w, h, sky, COLS, transparent=0)
        self.convert({"bg0": {"image": "g.png"}, "bg1": {"image": "c.png"}})
        pal = self.palette()
        g0 = self.render_map(0, 0)
        g1 = self.render_map(1, 2)
        for y in range(0, h, 7):
            for x in range(0, w, 5):
                c = COLS[big[y * w + x]]
                self.assertEqual(g0[(x, y)], word(*c))
                v = sky[y * w + x]
                self.assertEqual(g1[(x, y)], 0 if v == 0 else word(*COLS[v]))
        self.assertEqual(pal[0], 0)
        self.assertTrue(self.header()["BG1_HAS_ALPHA"] and not self.header()["BG0_HAS_ALPHA"])

    def test_palette_rules(self):
        # 40 colours + transparent: no opaque colour may land on index 0, 16, 32, ...; transparent is always index 0
        indexed(self.p("a.png"), 8, 8, [0] * 8 + list(range(1, 40)) + [1] * 17, COLS, transparent=0)
        self.convert({"bg1": {"image": "a.png", "fill": "clear"}})
        pal = self.palette()
        for i in range(0, 256, 16):
            self.assertEqual(pal[i], 0)
        self.assertEqual(sum(1 for e in pal if e), 39)
        self.assertTrue(all(e & 1 for e in pal if e))
        t = self.tile(2, 0)
        self.assertEqual(list(t[:8]), [0] * 8)
        self.assertTrue(all(v % 16 for v in t[8:]))

    def test_artist_palette_order_is_kept(self):
        # colour A is palette index 2 in the PNG, B is index 1: B gets the lower index
        indexed(self.p("a.png"), 8, 8, [2] * 32 + [1] * 32, [(0, 0, 0), (0, 200, 0), (200, 0, 0)])
        self.convert({"bg0": {"image": "a.png"}})
        pal = self.palette()
        self.assertEqual(pal[1], word(0, 200, 0))
        self.assertEqual(pal[2], word(200, 0, 0))

    def test_dedupe_and_flip_fold(self):
        base = pattern(8, 8, 6, 3)
        flip_h = [base[y * 8 + 7 - x] for y in range(8) for x in range(8)]
        flip_v = [base[(7 - y) * 8 + x] for y in range(8) for x in range(8)]
        row = []
        for y in range(8):
            row += base[y * 8:y * 8 + 8] + flip_h[y * 8:y * 8 + 8] + flip_v[y * 8:y * 8 + 8] + base[y * 8:y * 8 + 8]
        indexed(self.p("a.png"), 32, 8, row, COLS)
        a = self.convert({"bg0": {"image": "a.png", "flips": True}})
        self.assertEqual(len(a.banks[0].tiles), 1)
        self.assertEqual([self.cell("MAP0", i)[2:] for i in range(4)], [(False, False), (True, False), (False, True), (False, False)])
        a = self.convert({"bg0": {"image": "a.png"}})
        self.assertEqual(len(a.banks[0].tiles), 3)             # without folding: base, h-flipped, v-flipped

    def test_fill_repeat_and_clear(self):
        indexed(self.p("a.png"), 16, 8, pattern(16, 8, 4), COLS)
        self.convert({"bg0": {"image": "a.png"}})
        for cx in range(64):
            self.assertEqual(self.cell("MAP0", 5 * 64 + cx)[0], self.cell("MAP0", cx % 2)[0])
        self.convert({"bg0": {"image": "a.png", "fill": "clear"}})
        self.assertEqual(self.tile(0, self.cell("MAP0", 2)[0]), bytes(64))
        self.assertEqual(self.tile(0, self.cell("MAP0", 5 * 64)[0]), bytes(64))

    def test_sprite_frames_and_cells(self):
        # 2 frames of 2x1 tiles, side by side
        px = []
        for y in range(8):
            for f in range(4):
                px += [1 + f] * 8
        indexed(self.p("s.png"), 32, 8, px, COLS, transparent=0)
        a = self.convert({"sprites": [{"name": "dude", "image": "s.png", "frame_w": 2, "frame_h": 1},
                                      {"name": "other", "image": "s.png"}]})
        h = self.header()
        self.assertEqual((h["SPR_DUDE_CELL"], h["SPR_DUDE_W"], h["SPR_DUDE_H"], h["SPR_DUDE_FRAMES"]), (0, 2, 1, 2))
        self.assertEqual((h["SPR_OTHER_CELL"], h["SPR_OTHER_W"], h["SPR_OTHER_H"], h["SPR_OTHER_FRAMES"]), (4, 4, 1, 1))
        self.assertEqual(h["CELLS_N"], 8)
        self.assertEqual(len(a.banks[1].tiles), 4)             # the second sprite reuses the first's tiles
        self.assertEqual([self.cell("CELLS", i)[0] for i in range(8)], [0, 1, 2, 3, 0, 1, 2, 3])

    def test_font_and_box(self):
        gl = []
        for i in range(96):
            gl.append(bytes([1 + i % 5] * 64))
        w = 16 * 8
        px = [0] * (w * 48)
        for i, g in enumerate(gl):
            cx, cy = (i % 16) * 8, (i // 16) * 8
            for y in range(8):
                for x in range(8):
                    px[(cy + y) * w + cx + x] = g[0] if (x, y) != (3, 3) else 6 + i % 20
        indexed(self.p("f.png"), w, 48, px, COLS)
        indexed(self.p("fr.png"), 8, 8, [30] * 64, COLS)
        a = self.convert({"font": {"image": "f.png"}, "frame": "fr.png",
                          "boxes": [{"name": "m", "w": 6, "h": 4, "border": True, "pad": 1, "lines": ["AB", "C"]}]})
        ids = a.font_ids
        want = {(0, 0): "F", (1, 1): "A", (2, 1): "B", (3, 1): " ", (1, 2): "C", (2, 2): " "}
        for (cx, cy), ch in want.items():
            if ch == "F":
                self.assertEqual(self.cell("BOX_M", cy * 6 + cx)[0], a.frame_id)
            else:
                self.assertEqual(self.cell("BOX_M", cy * 6 + cx)[0], ids[ch])
        self.assertEqual(self.cell("BOX_M", 3 * 6 + 5)[0], a.frame_id)
        self.assertEqual(self.header()["BOX_M_N"], 24)
        self.assertEqual(self.cell("BOX_M", 0)[1], 3)


class Errors(Work):
    def err(self, spec, quantize=0):
        with self.assertRaises(png2zp.AssetError) as c:
            png2zp.build(self.manifest(spec), quantize)
        return str(c.exception)

    def test_rgb_png_needs_quantize(self):
        Image.new("RGB", (8, 8), (1, 2, 3)).save(self.p("a.png"))
        msg = self.err({"bg0": {"image": "a.png"}})
        self.assertIn("indexed", msg)
        self.assertIn("--quantize", msg)

    def test_size_not_multiple_of_eight(self):
        indexed(self.p("a.png"), 12, 8, [1] * 96, COLS)
        self.assertIn("multiple of 8", self.err({"bg0": {"image": "a.png"}}))

    def test_too_many_colours(self):
        for name, base in (("a.png", 0), ("b.png", 8)):         # two images of 200 different colours each: 400 > 240
            im = Image.new("RGB", (16, 16))
            im.putdata([((i % 8) * 32 + base, ((i // 8) % 8) * 32 + base, (i // 64) * 32 + base) if i < 200 else (0, 0, 0)
                        for i in range(256)])
            im.save(self.p(name))
        self.assertIn("240", self.err({"bg0": {"image": "a.png"}, "bg1": {"image": "b.png"}}, quantize=200))
        self.assertIn("1..240", self.err({"bg0": {"image": "a.png"}}, quantize=300))

    def test_bank_full_and_flip_hint(self):
        tiles = [[(i // 16) % 20 + 1] * 8 + [i % 16 + 1] * 56 for i in range(300)]
        px = []
        for y in range(8):
            for t in tiles:
                px += t[y * 8:y * 8 + 8] if False else [t[0], t[1], t[2], t[3], t[4], t[5], t[6], t[7]]
        # 300 tiles in a row is wider than 512 px; use a 2D arrangement instead: 20 x 15
        w = 20 * 8
        px = [0] * (w * 15 * 8)
        for n in range(300):
            cx, cy = n % 20, n // 20
            for y in range(8):
                for x in range(8):
                    px[(cy * 8 + y) * w + cx * 8 + x] = 1 + ((n * 7 + x * 3 + y * 5 + (n >> 4)) % 30) if (x + y + n) % 2 else 1 + n % 29
        indexed(self.p("a.png"), w, 15 * 8, px, COLS)
        msg = self.err({"bg0": {"image": "a.png"}})
        self.assertIn("full", msg)
        self.assertIn("flips", msg)

    def test_transparent_font_is_refused(self):
        indexed(self.p("f.png"), 128, 48, [0] * (128 * 48), COLS, transparent=0)
        indexed(self.p("fr.png"), 8, 8, [1] * 64, COLS)
        self.assertIn("transparent", self.err({"font": {"image": "f.png"}, "frame": "fr.png"}))

    def test_box_text_must_fit_and_be_in_font(self):
        indexed(self.p("f.png"), 128, 48, [1] * (128 * 48), COLS)
        indexed(self.p("fr.png"), 8, 8, [2] * 64, COLS)
        base = {"font": {"image": "f.png", "first": 65}, "frame": "fr.png"}
        self.assertIn("space", self.err(dict(base, boxes=[{"name": "m", "w": 4, "h": 3, "lines": ["A"]}])))
        base["font"]["first"] = 32
        self.assertIn("too wide", self.err(dict(base, boxes=[{"name": "m", "w": 4, "h": 3, "border": True, "lines": ["ABCDE"]}])))
        self.assertIn("not in the font", self.err(dict(base, boxes=[{"name": "m", "w": 40, "h": 3, "lines": ["é"]}])))


class Quantize(Work):
    def test_rgba_exact_when_few_colours(self):
        im = Image.new("RGBA", (16, 8), (0, 0, 0, 0))
        for x in range(8):
            for y in range(8):
                im.putpixel((x, y), (200, 40, 40, 255) if (x + y) % 2 else (40, 200, 40, 255))
        im.save(self.p("s.png"))
        self.convert({"sprites": [{"name": "a", "image": "s.png"}]}, quantize=48)
        pal = self.palette()
        self.assertEqual(sorted(e for e in pal if e), sorted([word(200, 40, 40), word(40, 200, 40)]))
        self.assertEqual(list(self.tile(1, 1)), [0] * 64)             # the transparent half

    def test_rgb_quantizes_down(self):
        im = Image.new("RGB", (32, 32))
        im.putdata([((i * 8) % 256, (i * 3) % 256, (i * 11) % 256) for i in range(32 * 32)])
        im.save(self.p("a.png"))
        self.convert({"bg0": {"image": "a.png"}}, quantize=16)
        pal = self.palette()
        used = {e for e in pal if e}
        self.assertLessEqual(len(used), 16)
        self.assertGreater(len(used), 4)

    def test_manifest_quantize_key(self):
        Image.new("RGB", (8, 8), (9, 99, 199)).save(self.p("a.png"))
        self.convert({"quantize": 8, "bg0": {"image": "a.png"}})
        self.assertEqual(self.palette()[1], word(9, 99, 199))


class Tiled(Work):
    def tileset(self, cols=2, rows=2):
        # 4 distinct tiles 1..4, each a single colour with one marker pixel so flips are visible
        w, h = cols * 8, rows * 8
        px = []
        for y in range(h):
            for x in range(w):
                n = (y // 8) * cols + x // 8
                px.append(1 + n if not (x % 8 == 0 and y % 8 == 0) else 9)
        indexed(self.p("ts.png"), w, h, px, COLS)

    def map_json(self, data, encoding=None, compression=None, **extra):
        layer = {"type": "tilelayer", "name": "fg", "width": 3, "height": 2, "data": data}
        if encoding:
            layer["encoding"] = encoding
        if compression:
            layer["compression"] = compression
        m = {"orientation": "orthogonal", "tilewidth": 8, "tileheight": 8, "width": 3, "height": 2, "infinite": False,
             "layers": [layer],
             "tilesets": [{"firstgid": 1, "name": "ts", "image": "ts.png", "tilewidth": 8, "tileheight": 8, "columns": 2,
                           "tilecount": 4, "margin": 0, "spacing": 0}]}
        m.update(extra)
        with open(self.p("m.tmj"), "w") as f:
            json.dump(m, f)

    def test_csv_map_with_flips_and_empty(self):
        self.tileset()
        self.map_json([1, 2, 0, 3 | png2zp.GID_H, 4 | png2zp.GID_V, 1 | png2zp.GID_H | png2zp.GID_V])
        a = self.convert({"bg1": {"tiled": "m.tmj", "layer": "fg"}})
        self.assertEqual(len(a.banks[2].tiles), 5)             # 4 tiles + the clear one
        ids = [self.cell("MAP1", i) for i in range(3)]
        self.assertEqual(self.tile(2, ids[2][0]), bytes(64))   # gid 0
        row2 = [self.cell("MAP1", 64 + i) for i in range(3)]
        self.assertEqual([c[2:] for c in row2], [(True, False), (False, True), (True, True)])
        self.assertEqual(row2[2][0], ids[0][0])                # gid 1 flipped both ways is gid 1's tile with the two bits set
        self.assertNotEqual(row2[0][0], ids[0][0])

    def test_base64_zlib_and_gzip(self):
        self.tileset()
        gids = [1, 2, 3, 4, 1, 2]
        raw = struct.pack("<6I", *gids)
        self.map_json(base64.b64encode(zlib.compress(raw)).decode(), "base64", "zlib")
        self.convert({"bg1": {"tiled": "m.tmj"}})
        first = [self.cell("MAP1", i)[0] for i in range(3)]
        gz = zlib.compressobj(9, zlib.DEFLATED, 16 + zlib.MAX_WBITS)
        self.map_json(base64.b64encode(gz.compress(raw) + gz.flush()).decode(), "base64", "gzip")
        self.convert({"bg1": {"tiled": "m.tmj"}})
        self.assertEqual([self.cell("MAP1", i)[0] for i in range(3)], first)
        self.map_json(base64.b64encode(raw).decode(), "base64")
        self.convert({"bg1": {"tiled": "m.tmj"}})
        self.assertEqual([self.cell("MAP1", i)[0] for i in range(3)], first)

    def test_external_tileset_and_rejections(self):
        self.tileset()
        ts = {"name": "ts", "image": "ts.png", "tilewidth": 8, "tileheight": 8, "columns": 2, "tilecount": 4, "margin": 0, "spacing": 0}
        with open(self.p("ts.tsj"), "w") as f:
            json.dump(ts, f)
        self.map_json([1, 2, 3, 4, 1, 2], tilesets=[{"firstgid": 1, "source": "ts.tsj"}])
        self.convert({"bg1": {"tiled": "m.tmj"}})
        self.assertEqual(len(png2zp.build(self.p("assets.json")).banks[2].tiles), 4)
        for extra, word_ in (({"infinite": True}, "infinite"), ({"tilewidth": 16}, "8x8"), ({"orientation": "isometric"}, "orthogonal")):
            self.map_json([1, 2, 3, 4, 1, 2], **extra)
            with self.assertRaises(png2zp.AssetError) as c:
                png2zp.build(self.manifest({"bg1": {"tiled": "m.tmj"}}))
            self.assertIn(word_, str(c.exception))
        self.map_json([1 | png2zp.GID_D, 2, 3, 4, 1, 2])
        with self.assertRaises(png2zp.AssetError) as c:
            png2zp.build(self.manifest({"bg1": {"tiled": "m.tmj"}}))
        self.assertIn("rotated", str(c.exception))

    def test_tileset_margin_and_spacing(self):
        # a 2x1 tileset with a 1 px margin and 2 px spacing: 1+8+2+8+1 = 20 wide, 10 tall
        w, h = 20, 10
        px = [0] * (w * h)
        for y in range(8):
            for x in range(8):
                px[(1 + y) * w + 1 + x] = 3
                px[(1 + y) * w + 11 + x] = 4
        indexed(self.p("ts.png"), w, h, px, COLS)
        self.map_json([1, 2, 1, 2, 1, 2], tilesets=[{"firstgid": 1, "name": "ts", "image": "ts.png", "tilewidth": 8, "tileheight": 8,
                                                      "columns": 2, "tilecount": 2, "margin": 1, "spacing": 2}])
        self.convert({"bg0": {"tiled": "m.tmj"}})
        t0, t1 = self.tile(0, self.cell("MAP0", 0)[0]), self.tile(0, self.cell("MAP0", 1)[0])
        self.assertEqual(set(t0), {2})                         # palette 1 is the PNG's index 0 (opaque magenta), then 3, then 4
        self.assertEqual(set(t1), {3})


class Cli(Work):
    def test_cli_writes_files_and_reports_errors(self):
        indexed(self.p("a.png"), 8, 8, [1] * 64, COLS)
        path = self.manifest({"bg0": {"image": "a.png"}})
        self.assertEqual(png2zp.main([path, "-o", self.out, "--rom-base", "0x200000"]), 0)
        self.assertEqual(self.header()["PAL"], 0x200000)
        self.assertEqual(len(self.blob()) % 256, 0)
        path = self.manifest({"bg0": {"image": "nope.png"}})
        self.assertEqual(png2zp.main([path, "-o", self.out]), 1)


if __name__ == "__main__":
    unittest.main()

#!/usr/bin/env python3
"""Checks frames dumped from the game template (with its sample art) against an independent reconstruction of what they must show.

  check_frames.py <build-dir> <frame.rgba> [<frame.rgba> ...]

The example's assets are fully known (build/assets.bin, where build/assets.h says each piece sits), so this composites the whole
picture itself in the order the microcode draws it: BG0 (opaque), the bird (a sprite BETWEEN the layers), BG1 (see-through where a
pixel's palette entry has alpha 0), the hero (above both), then the window's text box (above everything, never scrolling). The game
advances one step per frame; a dumped frame's step is read off where BG1 is (its scroll is the step, mod 512), and the frame must then equal that
step's reconstruction exactly, pixel for pixel, and the gap between the frame number and the step must be the same for every frame. Exit status 0 when every frame does.
"""
import re
import struct
import sys

build = sys.argv[1]
blob = open(build + "/assets.bin", "rb").read()
defs = dict((m.group(1), int(m.group(2), 16)) for m in re.finditer(r"#define ASSET_(\w+) 0x([0-9A-F]+)UL", open(build + "/assets.h").read()))
counts = dict((m.group(1), int(m.group(2))) for m in re.finditer(r"#define ASSET_(\w+_N) (\d+)", open(build + "/assets.h").read()))
ROM_BASE = 0x110000


def at(name):
    return defs[name] - ROM_BASE


def color32(c16):
    """The latched colour of a 16 bit palette entry (Display::color16To32), as (r, g, b, a)."""
    r5, g5, b5 = (c16 >> 1) & 31, (c16 >> 6) & 31, (c16 >> 11) & 31
    ex = lambda v: (v << 3) | (v >> 2)
    return (ex(r5), ex(g5), ex(b5), 255 if c16 & 1 else 0)


pal16 = struct.unpack("<256H", blob[at("PAL"):at("PAL") + 512])
pal = [color32(c) for c in pal16]
tiles = {}                                  # (bank, id) -> 64 palette indices
for bank in range(4):
    base = at("TILES%d" % bank)
    for tid in range(counts["TILES%d_N" % bank]):
        tiles[(bank, tid)] = blob[base + 64 * tid: base + 64 * tid + 64]


def cell(table_off, n):
    w0, w1 = struct.unpack("<HH", blob[table_off + 4 * n: table_off + 4 * n + 4])
    return w1 & 3, w0 & 255                 # (bank, tile id): the key of `tiles`


maps = [at("MAP0"), at("MAP1")]
sprite_cells = [cell(at("CELLS"), i) for i in range(counts["CELLS_N"])]
win_cells = [cell(at("BOX_MSG"), i) for i in range(counts["BOX_MSG_N"])]
BOX_COL, BOX_ROW, BOX_W, BOX_H = 2, 26, 28, 5
HERO = (120, 136)                           # main.c: it never moves, nothing presses the pad


def bg(layer, x, y, sx, sy):
    wx, wy = (x + sx) & 511, (y + sy) & 511
    t = tiles[cell(maps[layer], (wy >> 3) * 64 + (wx >> 3))]
    return t[(wy & 7) * 8 + (wx & 7)]


def sprite(x, y, sx, sy, w, h, first):
    """The palette index of a w x h tile sprite at (sx, sy) at pixel (x, y), or None outside it."""
    px, py = x - sx, y - sy
    if not (0 <= px < 8 * w and 0 <= py < 8 * h):
        return None
    return tiles[sprite_cells[first + (py >> 3) * w + (px >> 3)]][(py & 7) * 8 + (px & 7)]


def expected(t):
    bx = (t * 2) % 288 - 16
    by = 36 + ((t >> 3) & 7)
    frame = []
    for y in range(256):
        for x in range(256):
            c = pal[bg(0, x, y, t >> 1, 0)]                    # BG0: opaque
            b = sprite(x, y, bx, by, 2, 1, 4)                  # the bird: between the layers
            if b is not None and pal[b][3]:
                c = pal[b]
            i = bg(1, x, y, t, 0)                              # BG1: see-through at alpha 0
            if pal[i][3]:
                c = pal[i]
            h = sprite(x, y, HERO[0], HERO[1], 2, 2, 0)        # the hero: above both
            if h is not None and pal[h][3]:
                c = pal[h]
            cx, cy = (x >> 3) - BOX_COL, (y >> 3) - BOX_ROW
            if 0 <= cx < BOX_W and 0 <= cy < BOX_H:            # the text box: solid, above everything, does not scroll
                c = pal[tiles[win_cells[cy * BOX_W + cx]][(y & 7) * 8 + (x & 7)]]
            frame.append(c)
    return frame


# Rows that show BG1 and nothing that moves independently of it: the clouds (rows 24-63) and the trees and bushes (rows 112-135), without the bird
# (y 36-50) or the hero (y 136-151). Over these a BG1 scroll position is nearly unambiguous, but not quite (a scroll that puts every cloud off the
# sampled columns fits many), so find_steps returns every scroll that fits and the caller checks each against the whole frame.
BG1_ROWS = [y for y in range(24, 64, 3) if not 36 <= y <= 51] + list(range(112, 136, 4))


def find_steps(got, n):
    """Candidate game steps for a frame: every BG1 scroll (= the step mod 512) that explains the BG1 rows, as steps no later than frame n."""
    out = []
    for sc in range(512):
        for y in BG1_ROWS:
            ok_row = True
            for x in range(0, 256, 3):
                i = bg(1, x, y, sc, 0)
                c = pal[i] if pal[i][3] else pal[bg(0, x, y, 0, 0)] if y < 64 else None
                if c is None:
                    if pal[i][3] and pal[i] != got[y * 256 + x]:      # below the sky BG0 is not uniform: only the BG1 pixels themselves are compared
                        ok_row = False
                        break
                elif c != got[y * 256 + x]:
                    ok_row = False
                    break
            if not ok_row:
                break
        else:
            out.extend(range(sc, n + 1, 512))
    return out


ok = True
offsets = []
for path in sys.argv[2:]:
    n = int(re.search(r"_(\d+)\.rgba$", path).group(1))
    raw = open(path, "rb").read()
    got = [tuple(raw[i:i + 4]) for i in range(0, len(raw), 4)]
    best = None
    # the game loop starts some frames after power-on (the uploads, and on the real Boot ROM its whole splash), so frame n shows step n - lag
    for step in find_steps(got, n):
        for t in (step, step - 1, step + 1):
            if t < 0:
                continue
            bad = sum(1 for a, b in zip(got, expected(t)) if a != b)
            if best is None or bad < best[0]:
                best = (bad, t)
            if bad == 0:
                break
        if best and best[0] == 0:
            break
    if best is None:
        print("  FAIL: frame %d: BG1 is not where the game would put it at any step (nothing drawn, or the game is not running)" % n)
        ok = False
        continue
    status = "ok  " if best[0] == 0 else "FAIL"
    print("  %s: frame %d is game step %d (start lag %d)%s" % (status, n, best[1], n - best[1], "" if best[0] == 0 else ", closest match: %d pixels differ" % best[0]))
    ok = ok and best[0] == 0
    offsets.append(n - best[1])
# one game step per raster: the lag between the frame number and the game step must not change from frame to frame
if len(set(offsets)) > 1:
    print("  FAIL: the start lag changes (%s): the game loop is not running exactly one step per frame" % offsets)
    ok = False
elif ok and offsets:
    print("  ok  : one game step per frame (the lag stays %d)" % offsets[0])
sys.exit(0 if ok and offsets else 1)

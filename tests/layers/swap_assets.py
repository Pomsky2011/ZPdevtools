#!/usr/bin/env python3
"""The game template with other art: proves that swapping PNGs changes what is drawn with no code change, and that mirrored tiles folded into
one (the cell's flip bits) draw mirrored on the PPU.

  swap_assets.py gen <dir> <template-assets-dir>      writes <dir>/assets.json and PNGs: BG0 is ONE asymmetric tile, placed flipped
                                                      four ways at random over the map (flips: true), BG1 empty, the rest copied
  swap_assets.py check <dir> <build-dir> <frame.rgba> ...    each frame must show BG0 exactly as the source PNG says, at some scroll

The check ignores the sprites and the window: it looks at the rows the hero, the bird and the box do not touch (y 60..130).
"""
import json
import os
import random
import re
import shutil
import sys

from PIL import Image

W = 512
TILE = [[1 + ((x * 3 + y * 5 + (x >> 1) * y) % 6) for x in range(8)] for y in range(8)]     # asymmetric both ways
PALETTE = [(255, 0, 255), (200, 40, 40), (40, 200, 40), (40, 40, 200), (230, 230, 40), (40, 230, 230), (230, 40, 230)]


def variants():
    rng = random.Random(5)
    return [[rng.randrange(4) for _ in range(64)] for _ in range(64)]


def px(cx, cy, x, y, v):
    sx = 7 - x if v & 1 else x
    sy = 7 - y if v & 2 else y
    return TILE[sy][sx]


def gen(out, template_assets):
    os.makedirs(out, exist_ok=True)
    v = variants()
    im = Image.new("P", (W, W))
    im.putpalette([c for rgb in PALETTE for c in rgb])
    im.putdata([px(x // 8, y // 8, x % 8, y % 8, v[y // 8][x // 8]) for y in range(W) for x in range(W)])
    im.save(os.path.join(out, "ground.png"))
    clear = Image.new("P", (W, W), 0)
    clear.putpalette([255, 0, 255])
    clear.save(os.path.join(out, "clouds.png"), transparency=0)
    for name in ("hero.png", "bird.png", "font.png", "frame.png"):
        shutil.copy(os.path.join(template_assets, name), os.path.join(out, name))
    m = json.load(open(os.path.join(template_assets, "assets.json")))
    m["bg0"] = {"image": "ground.png", "flips": True}
    m["bg1"] = {"image": "clouds.png"}
    json.dump(m, open(os.path.join(out, "assets.json"), "w"), indent=2)


def color32(r, g, b):
    ex = lambda c: ((c >> 3) << 3) | (c >> 5)          # 8 bit -> 5 bit -> the latched 8 bit value (v << 3 | v >> 2)
    return (ex(r), ex(g), ex(b), 255)


def check(build, frames):
    header = open(os.path.join(build, "assets.h")).read()
    n0 = int(re.search(r"ASSET_TILES0_N (\d+)", header).group(1))
    if n0 != 1:
        print("  FAIL: BG0 has %d tiles in bank 0, the four mirrored placements should fold into 1" % n0)
        return 1
    v = variants()
    src = [[color32(*PALETTE[px(x // 8, y // 8, x % 8, y % 8, v[y // 8][x // 8])]) for x in range(W)] for y in range(64, 128)]
    bad_all = 0
    for path in frames:
        raw = open(path, "rb").read()
        got = [[tuple(raw[(y * 256 + x) * 4:(y * 256 + x) * 4 + 4]) for x in range(256)] for y in range(64, 128)]
        best = None
        for sx in range(W):                           # BG0 scrolls by t >> 1, a different amount every couple of frames
            bad = 0
            for y in (0, 17, 33, 63):
                row = src[y]
                bad += sum(1 for x in range(256) if row[(x + sx) & 511] != got[y][x])
                if bad > 8:
                    break
            if best is None or bad < best[0]:
                best = (bad, sx)
            if bad == 0:
                break
        bad = sum(1 for y in range(64) for x in range(256) if src[y][(x + best[1]) & 511] != got[y][x])
        print("  %s: %s shows BG0 at scroll %d (%d pixels differ)" % ("ok  " if bad == 0 else "FAIL", os.path.basename(path), best[1], bad))
        bad_all += bad
    return 1 if bad_all else 0


if __name__ == "__main__":
    if sys.argv[1] == "gen":
        gen(sys.argv[2], sys.argv[3])
    else:
        sys.exit(check(sys.argv[3], sys.argv[4:]))

#!/usr/bin/env python3
"""Generates the assets of the layers-game example: build/assets.bin and build/assets.h (where each piece sits in ROM).

Everything is drawn by this script, so the example has no binary files to carry around. Palette: 256 colours as 16 bit
BBBBBGGGGGRRRRR1 (bit 0 is alpha; entry 0, 16, 32.. have alpha 0, which is what makes a tile pixel see-through). Tiles: 8x8,
one palette index per byte. Four tile banks: 0 opaque ground tiles for BG0, 1 sprite tiles, 2 see-through tiles for BG1, 3 the
text box (a small font and its frame).

  gen_assets.py [build-dir]
"""
import os
import struct
import sys

OUT = sys.argv[1] if len(sys.argv) > 1 else "build"
ROM_BASE = 0x110000          # where the Makefile places assets.bin (zplink --data)


def rgb(r, g, b):
    return ((b >> 3) << 11) | ((g >> 3) << 6) | ((r >> 3) << 1) | 1


COL = {}
palette = [0] * 256          # index 0 stays 0: alpha 0, see-through


def color(index, r, g, b, name):
    assert index % 16 != 0, "indices 0, 16, 32.. are the see-through ones"
    palette[index] = rgb(r, g, b)
    COL[name] = index


color(1, 80, 150, 255, "sky")
color(2, 130, 190, 255, "sky2")
color(3, 255, 255, 255, "cloud")
color(4, 200, 215, 240, "cloud2")
color(5, 70, 170, 60, "grass")
color(6, 40, 120, 40, "grass2")
color(7, 150, 100, 50, "dirt")
color(8, 100, 65, 30, "dirt2")
color(9, 230, 60, 60, "hero")
color(10, 140, 20, 30, "hero2")
color(11, 255, 255, 255, "white")
color(12, 10, 10, 10, "black")
color(13, 20, 20, 60, "box")
color(14, 230, 230, 255, "border")
color(15, 255, 240, 120, "text")
color(17, 255, 210, 40, "bird")
color(18, 230, 120, 20, "bird2")
color(20, 30, 140, 50, "bush")
color(21, 20, 100, 35, "bush2")
color(22, 110, 70, 30, "trunk")


def tile(fn):
    """An 8x8 tile from fn(x, y) -> palette index."""
    return bytes(fn(x, y) for y in range(8) for x in range(8))


def solid(i):
    return tile(lambda x, y: i)


# ---- bank 0: opaque ground tiles for BG0 ----------------------------------------------------------------------------------------
def grass(x, y):
    if y < 3:
        return COL["grass2"] if (x * 5 + y * 3) % 7 == 0 else COL["grass"]
    return COL["dirt2"] if (x * 3 + y * 5) % 11 == 0 else COL["dirt"]


def dirt(x, y):
    return COL["dirt2"] if (x * 7 + y * 3) % 9 == 0 else COL["dirt"]


bank0 = [solid(COL["sky"]), solid(COL["sky2"]), tile(grass), tile(dirt), solid(COL["dirt2"])]
T0 = {"sky": 0, "sky2": 1, "grass": 2, "dirt": 3, "dirt2": 4}


# ---- bank 2: see-through tiles for BG1 (palette index 0 shows what is behind) ------------------------------------------------------
def cloud_l(x, y):
    if y < 2 or y > 6:
        return 0
    edge = 3 - (y if y < 4 else 7 - y)
    return 0 if x < edge else (COL["cloud2"] if y > 4 else COL["cloud"])


def cloud_m(x, y):
    if y < 1 or y > 6:
        return 0
    return COL["cloud2"] if y > 4 else COL["cloud"]


def cloud_r(x, y):
    return cloud_l(7 - x, y)


def bush_top(x, y):
    d = abs(x - 3.5)
    if y < 2 + d * 0.9:
        return 0
    return COL["bush2"] if (x + y) % 4 == 0 else COL["bush"]


def bush_body(x, y):
    return COL["bush2"] if (x * 3 + y) % 5 == 0 else COL["bush"]


def trunk(x, y):
    return COL["trunk"] if 2 <= x <= 5 else 0


bank2 = [tile(cloud_l), tile(cloud_m), tile(cloud_r), tile(bush_top), tile(bush_body), tile(trunk), solid(0), solid(0)]
T2 = {"cl": 0, "cm": 1, "cr": 2, "bush_top": 3, "bush": 4, "trunk": 5, "blank": 7}

# ---- bank 1: sprites: a 2x2 hero (tiles 0-3) and a 2x1 bird (tiles 4-5) ---------------------------------------------------------------
hero = [[0] * 16 for _ in range(16)]
for y in range(16):
    for x in range(16):
        inside = (1 <= x <= 14 and 1 <= y <= 14) and not ((x in (1, 14)) and (y in (1, 14)))
        if inside:
            edge = x in (1, 14) or y in (1, 14)
            hero[y][x] = COL["hero2"] if edge else COL["hero"]
for ex in (4, 9):
    for dy in range(2):
        for dx in range(3):
            hero[5 + dy][ex + dx] = COL["white"]
    hero[6][ex + 1] = COL["black"]
    hero[6][ex + 2] = COL["black"]
for x in range(5, 11):
    hero[11][x] = COL["hero2"]
hero[10][4] = hero[10][11] = COL["hero2"]


def sub(img, tx, ty):
    return bytes(img[ty * 8 + y][tx * 8 + x] for y in range(8) for x in range(8))


bird = [[0] * 16 for _ in range(8)]
for y in range(8):
    for x in range(16):
        dx, dy = (x - 7.5) / 7.0, (y - 4.5) / 3.2
        if dx * dx + dy * dy <= 1.0:
            bird[y][x] = COL["bird2"] if (y >= 5 and 3 <= x <= 10) else COL["bird"]
bird[3][11] = COL["black"]
bird[4][14] = bird[4][15] = COL["bird2"]
bank1 = [sub(hero, 0, 0), sub(hero, 1, 0), sub(hero, 0, 1), sub(hero, 1, 1), sub(bird, 0, 0), sub(bird, 1, 0)]
CELLS = [(i, 1) for i in range(6)]       # sprite cells: (tile id, bank)

# ---- bank 3: the text box: 12 letters of a 5x7 font, a blank and a frame tile ----------------------------------------------------
FONT = {
    "Z": ["#####", "....#", "...#.", "..#..", ".#...", "#....", "#####"],
    "E": ["#####", "#....", "#....", "####.", "#....", "#....", "#####"],
    "R": ["####.", "#...#", "#...#", "####.", "#.#..", "#..#.", "#...#"],
    "O": [".###.", "#...#", "#...#", "#...#", "#...#", "#...#", ".###."],
    "P": ["####.", "#...#", "#...#", "####.", "#....", "#....", "#...."],
    "I": ["#####", "..#..", "..#..", "..#..", "..#..", "..#..", "#####"],
    "N": ["#...#", "##..#", "##..#", "#.#.#", "#..##", "#..##", "#...#"],
    "T": ["#####", "..#..", "..#..", "..#..", "..#..", "..#..", "..#.."],
    "L": ["#....", "#....", "#....", "#....", "#....", "#....", "#####"],
    "A": [".###.", "#...#", "#...#", "#####", "#...#", "#...#", "#...#"],
    "Y": ["#...#", "#...#", ".#.#.", "..#..", "..#..", "..#..", "..#.."],
    "S": [".####", "#....", "#....", ".###.", "....#", "....#", "####."],
}
LETTERS = "ZEROPINTLAYS"


def glyph(ch):
    rows = FONT[ch]
    return tile(lambda x, y: COL["text"] if (y < 7 and 1 <= x <= 5 and rows[y][x - 1] == "#") else COL["box"])


bank3 = [glyph(c) for c in LETTERS] + [solid(COL["box"]), solid(COL["border"])]
T3_BLANK, T3_BORDER = len(LETTERS), len(LETTERS) + 1


def attr(bank, hflip=0, vflip=0, transparent=0):
    return 0x1000 | (bank & 3) | (0x04 if transparent else 0) | (0x08 if hflip else 0) | (0x10 if vflip else 0)


def cell(tile_id, bank):
    return struct.pack("<HH", tile_id, attr(bank))


# ---- the maps: 64 x 64 cells of (tile id word, attribute word) ------------------------------------------------------------------
def map0():
    rows = []
    for ty in range(64):
        if ty < 8:
            t = T0["sky"]
        elif ty < 20:
            t = T0["sky2"]
        elif ty == 20:
            t = T0["grass"]
        elif ty < 24:
            t = T0["dirt"]
        else:
            t = T0["dirt2"]
        rows.append(cell(t, 0) * 64)
    return b"".join(rows)


def map1():
    grid = [[cell(T2["blank"], 2)] * 64 for _ in range(64)]
    for cx, cy, w in ((3, 3, 3), (14, 5, 4), (27, 3, 3), (38, 6, 4), (50, 4, 3), (58, 7, 3)):    # clouds: left, middle(s), right
        grid[cy][cx] = cell(T2["cl"], 2)
        for k in range(1, w - 1):
            grid[cy][cx + k] = cell(T2["cm"], 2)
        grid[cy][cx + w - 1] = cell(T2["cr"], 2)
    for bx in (6, 20, 33, 47, 60):                                                                # bushes on the grass line (row 20)
        grid[18][bx] = cell(T2["bush_top"], 2)
        grid[19][bx] = cell(T2["bush"], 2)
    for tx in (11, 41):                                                                           # a tree: trunk and a bush on top
        for ty in (15, 16, 17, 18, 19):
            grid[ty][tx] = cell(T2["trunk"], 2)
        grid[14][tx] = cell(T2["bush_top"], 2)
    return b"".join(b"".join(r) for r in grid)


# ---- the text box: 28 x 5 cells (col 2, row 26), a frame round two lines of text --------------------------------------------------
BOX_W, BOX_H = 28, 5


def boxcells():
    out = []
    lines = {1: "ZEROPOINT", 3: "LAYERS"}
    for ty in range(BOX_H):
        for tx in range(BOX_W):
            if ty in (0, BOX_H - 1) or tx in (0, BOX_W - 1):
                out.append(cell(T3_BORDER, 3))
                continue
            text = lines.get(ty, "")
            k = tx - 2
            out.append(cell(LETTERS.index(text[k]) if 0 <= k < len(text) else T3_BLANK, 3))
    return b"".join(out)


# ---- the ROM image: every piece 256-aligned -------------------------------------------------------------------------------------
blob = bytearray()
addr = {}


def put(name, data):
    while len(blob) % 256:
        blob.append(0)
    addr[name] = ROM_BASE + len(blob)
    blob.extend(data)


put("PAL", b"".join(struct.pack("<H", c) for c in palette))
put("TILES0", b"".join(bank0))
put("TILES2", b"".join(bank2))
put("TILES1", b"".join(bank1))
put("TILES3", b"".join(bank3))
put("CELLS", b"".join(cell(t, b) for t, b in CELLS))
put("WINCELLS", boxcells())
put("MAP0", map0())
put("MAP1", map1())
while len(blob) % 256:
    blob.append(0)

os.makedirs(OUT, exist_ok=True)
open(os.path.join(OUT, "assets.bin"), "wb").write(bytes(blob))
with open(os.path.join(OUT, "assets.h"), "w") as f:
    f.write("/* generated by gen_assets.py: where the assets sit in ROM, and how many of each (do not edit) */\n")
    for name, a in addr.items():
        f.write("#define ASSET_%s 0x%06XUL\n" % (name, a))
    f.write("#define ASSET_TILES0_N %d\n#define ASSET_TILES1_N %d\n#define ASSET_TILES2_N %d\n#define ASSET_TILES3_N %d\n" %
            (len(bank0), len(bank1), len(bank2), len(bank3)))
    f.write("#define ASSET_CELLS_N %d\n#define ASSET_WINCELLS_N %d\n" % (len(CELLS), BOX_W * BOX_H))
print("%s: %d bytes of assets" % (os.path.join(OUT, "assets.bin"), len(blob)))

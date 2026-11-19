#!/usr/bin/env python3
"""Builds the layers demo cartridge: build/demo.zpb (signed with the local dev key; loads in the emulator).

  build_demo.py [--out DIR]

Two scrolling 64x64 background layers (sky + ground; clouds + a foreground strip, scrolling twice as fast) and three sprites of
different sizes (a ball, a 2x2 box and a 4x2 ship) over them, all running on the PPU microcode in ../gen_layers.py, and a window layer above everything: a textbox over the bottom rows (opaque, so
the PPU leaves out the BG under it) that never scrolls, and a small frosted-glass lens over the ground.

Cartridge layout: the CPU program (generated here, DEF88186 assembly) sits at $8000. The PPU segment of the ROM (bank $10 up)
carries one blob: the PPU program, the palette, the sprite cells, the tile pixels, both tilemaps, and 256 precomputed 4 KiB
frame blocks (sprite lists + scroll, from gen_frames.c). The CPU loads the PPU program (COP #$F7), DMAs the assets into PPU RAM,
uploads the tiles through the PPU's tile-upload mailbox, sets RUN, then once per frame DMAs the next frame block into the idle
list buffer and flips LISTBASE.

Signing: v2 by default now (`--v3` for the old scheme): the console's BLAKE2s engine hashes the whole ROM at boot in ~0.1 s, so there is
no per-chunk verification and the CPU program has no COP #$FF calls. The v3 notes below are why the demo used to need them.

Signing (v3, `--v3`): code + chunked-data manifest. The real Boot ROM hashes a v1 ROM's whole payload in software, ~9 min per 8 MB, so a
2 MB demo sat there for minutes before the CPU ran. With v3 it hashes only the CPU program (CODESIZE, ~1.5 s) and a small manifest;
every 16 KiB chunk of the PPU blob stays poison until the CPU asks the Boot ROM to check it (COP #$FF, zp_verify_chunk), which it
does right before each read of that chunk. CODESIZE is chosen so that chunk 0 starts exactly at the blob.
"""
import argparse
import math
import shutil
import tempfile
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
LAYERS = os.path.dirname(HERE)
TOOLS = os.path.join(LAYERS, "..", "..", "executables")
sys.path.insert(0, LAYERS)
import gen_layers as G  # noqa: E402  (layout constants shared with the microcode)

CODESIZE = 0xC000                                   # the CPU program is hashed at boot; must cover it (asserted in build())
CHUNK = 0x4000
PPU_ROM = 0x100000                                   # the PPU segment's ROM address (bank $10)
FIRST_CHUNK = (PPU_ROM - CODESIZE) // CHUNK          # manifest index of the blob's first 16 KiB
assert (PPU_ROM - CODESIZE) % CHUNK == 0

FRAME_BLOCK = 256                                   # bytes per frame block (gen_frames.c)
FRAMES = 256                                         # animation frames, one per 1/60 s; 256 blocks = one 64 KiB bank

# ---- art ----------------------------------------------------------------------------------------------------------------
PALETTE = {  # index -> (r, g, b); 0 is transparent
    1: (70, 140, 255), 2: (100, 165, 255), 3: (135, 190, 255), 4: (170, 210, 255),
    5: (110, 215, 90), 6: (60, 160, 60), 7: (160, 105, 60), 8: (115, 72, 42), 9: (175, 175, 185), 10: (120, 120, 135),
    11: (255, 255, 255), 12: (215, 228, 248), 13: (125, 82, 42), 14: (34, 122, 52), 15: (74, 180, 80),
    16: (230, 50, 50), 17: (150, 20, 30), 18: (250, 220, 60), 19: (240, 140, 40), 20: (20, 20, 30), 21: (255, 255, 255),
    22: (60, 100, 230), 23: (30, 50, 150), 24: (80, 230, 230), 25: (190, 190, 200), 26: (255, 200, 0),
    27: (24, 32, 72), 28: (240, 240, 255), 29: (255, 226, 120), 30: (225, 235, 255),   # window: panel, border, text, glass fill
}


def make_palette():
    out = bytearray()
    for i in range(256):
        if i == 0:
            v = 0                                   # alpha 0: transparent
        else:
            r, g, b = PALETTE.get(i, (0, 0, 0))
            v = ((b >> 3) << 11) | ((g >> 3) << 6) | ((r >> 3) << 1) | 1
        out += bytes((v & 0xFF, v >> 8))
    return bytes(out)


def canvas(w, h):
    return [[0] * w for _ in range(h)]


def disc(c, cx, cy, r, color):
    for y in range(len(c)):
        for x in range(len(c[0])):
            if (x + 0.5 - cx) ** 2 + (y + 0.5 - cy) ** 2 <= r * r:
                c[y][x] = color


def rect(c, x0, y0, x1, y1, color):
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            c[y][x] = color


def tiles_of(c):
    """Cut a canvas (multiples of 8) into 8x8 tiles, row-major; each tile is 64 palette indices."""
    h, w = len(c), len(c[0])
    out = []
    for ty in range(h // 8):
        for tx in range(w // 8):
            out.append(bytes(c[ty * 8 + y][tx * 8 + x] for y in range(8) for x in range(8)))
    return out


def solid(i):
    return bytes([i] * 64)


def build_tiles():
    """Returns {(bank, id): 64 bytes} and the named ids the maps and sprites use."""
    t = {}
    # bank 0: BG0
    for i in range(4):
        t[(0, i)] = solid(1 + i)                                                   # sky bands
    t[(0, 4)] = bytes((5 if (y < 3 and (x + y) % 4) else 6) for y in range(8) for x in range(8))        # grass
    t[(0, 5)] = bytes((8 if (x * 3 + y * 5) % 7 == 0 else 7) for y in range(8) for x in range(8))        # dirt
    t[(0, 6)] = bytes((10 if (y in (0, 4) or (y < 4 and x == 3) or (y >= 4 and x == 7)) else 9) for y in range(8) for x in range(8))  # brick
    # bank 1: BG1 (index 0 = see-through)
    t[(1, 0)] = bytes(64)
    cl = canvas(24, 16)
    for cx, cy, r in ((6, 10, 5), (12, 7, 6), (18, 10, 5), (12, 11, 5)):
        disc(cl, cx, cy, r, 11)
    for y in range(11, 16):
        for x in range(24):
            if cl[y][x]:
                cl[y][x] = 12
    for i, tl in enumerate(tiles_of(cl)):
        t[(1, 1 + i)] = tl                                                         # 3x2 cloud: ids 1-6
    t[(1, 7)] = bytes((14 if (x % 3 != 1 or y > 3 + (x * 5) % 3) else 0) for y in range(8) for x in range(8))   # grass blades
    t[(1, 8)] = bytes((15 if (x * 7 + y * 3) % 9 == 0 else 14) for y in range(8) for x in range(8))             # foreground green
    # bank 2: sprites
    ball = canvas(8, 8)
    disc(ball, 4, 4, 3.9, 16)
    disc(ball, 4.5, 5, 3.0, 17)
    disc(ball, 3.0, 2.6, 1.4, 11)
    t[(2, 0)] = tiles_of(ball)[0]
    box = canvas(16, 16)
    rect(box, 0, 0, 15, 15, 23)
    rect(box, 1, 1, 14, 14, 22)
    rect(box, 3, 4, 5, 8, 21); rect(box, 10, 4, 12, 8, 21)                        # eyes
    rect(box, 4, 6, 5, 8, 20); rect(box, 11, 6, 12, 8, 20)                        # pupils
    rect(box, 5, 11, 10, 11, 20); box[10][4] = 20; box[10][11] = 20               # mouth
    for i, tl in enumerate(tiles_of(box)):
        t[(2, 1 + i)] = tl                                                         # ids 1-4
    ship = canvas(32, 16)
    for y in range(16):
        for x in range(32):
            if ((x - 16) / 14.0) ** 2 + ((y - 9) / 5.5) ** 2 <= 1:
                ship[y][x] = 18 if y < 9 else 19
    rect(ship, 2, 4, 8, 8, 19); rect(ship, 3, 5, 7, 7, 26)                       # tail fin
    disc(ship, 21, 6.5, 3.2, 24)                                                  # cockpit
    for x in range(9, 28):
        ship[9][x] = 20                                                           # hull line
    rect(ship, 12, 12, 18, 14, 25)                                                # belly plate
    rect(ship, 29, 8, 31, 10, 26)                                                 # nose light
    for i, tl in enumerate(tiles_of(ship)):
        t[(2, 5 + i)] = tl                                                         # ids 5-12
    # bank 3: the window. 0 = panel, 1-8 = border pieces (TL T TR L R BL B BR), 9.. = the letters the message uses, 40 = glass fill
    def border(top, bottom, left, right):
        return bytes((28 if ((top and y < 2) or (bottom and y > 5) or (left and x < 2) or (right and x > 5)) else 27) for y in range(8) for x in range(8))
    t[(3, 0)] = solid(27)
    for i, args in enumerate(((1, 0, 1, 0), (1, 0, 0, 0), (1, 0, 0, 1), (0, 0, 1, 0), (0, 0, 0, 1), (0, 1, 1, 0), (0, 1, 0, 0), (0, 1, 0, 1))):
        t[(3, 1 + i)] = border(*args)
    for ch in sorted(set("".join(MESSAGE)) - {" "}):
        t[(3, GLYPH0 + ord(ch) - ord("A"))] = bytes((29 if (1 <= x <= 5 and 1 <= y <= 7 and FONT[ch][y - 1][x - 1] == "1") else 27) for y in range(8) for x in range(8))
    t[(3, 40)] = solid(30)
    return t


# a 5x7 font for the letters the window text uses (one 8x8 tile each)
FONT = {c: rows.split(",") for c, rows in {
    "A": "01110,10001,10001,11111,10001,10001,10001", "B": "11110,10001,10001,11110,10001,10001,11110", "C": "01110,10001,10000,10000,10000,10001,01110",
    "D": "11110,10001,10001,10001,10001,10001,11110", "E": "11111,10000,10000,11110,10000,10000,11111", "F": "11111,10000,10000,11110,10000,10000,10000",
    "G": "01110,10001,10000,10111,10001,10001,01111", "H": "10001,10001,10001,11111,10001,10001,10001", "I": "01110,00100,00100,00100,00100,00100,01110",
    "J": "00111,00010,00010,00010,00010,10010,01100", "K": "10001,10010,10100,11000,10100,10010,10001", "L": "10000,10000,10000,10000,10000,10000,11111",
    "M": "10001,11011,10101,10101,10001,10001,10001", "N": "10001,11001,10101,10011,10001,10001,10001", "O": "01110,10001,10001,10001,10001,10001,01110",
    "P": "11110,10001,10001,11110,10000,10000,10000", "Q": "01110,10001,10001,10001,10101,10010,01101", "R": "11110,10001,10001,11110,10100,10010,10001",
    "S": "01111,10000,10000,01110,00001,00001,11110", "T": "11111,00100,00100,00100,00100,00100,00100", "U": "10001,10001,10001,10001,10001,10001,01110",
    "V": "10001,10001,10001,10001,10001,01010,00100", "W": "10001,10001,10001,10101,10101,11011,10001", "X": "10001,10001,01010,00100,01010,10001,10001",
    "Y": "10001,10001,01010,00100,00100,00100,00100", "Z": "11111,00001,00010,00100,01000,10000,11111",
}.items()}
GLYPH0 = 9                                          # tile id of "A" in bank 3; the other letters follow in alphabet order
MESSAGE = ("THIS BOX NEVER SCROLLS", "BG UNDER IT IS NEVER DRAWN")   # at most 27 letters: the cells at both ends of a row are the border and a blank
BOX_COL, BOX_ROW, BOX_W, BOX_H = 1, 26, 30, 5       # the textbox on the window grid (gen_frames.c has the same numbers)


def make_window_cells():
    """The textbox's cells (BOX_W x BOX_H, 4 bytes each): a border round two lines of text."""
    rows = []
    for r in range(BOX_H):
        line = MESSAGE[0] if r == 1 else MESSAGE[1] if r == 3 else ""
        out = bytearray()
        for c in range(BOX_W):
            edge_t, edge_b, edge_l, edge_r = r == 0, r == BOX_H - 1, c == 0, c == BOX_W - 1
            if edge_t or edge_b:
                tid = (1 if edge_l else 3 if edge_r else 2) if edge_t else (6 if edge_l else 8 if edge_r else 7)   # TL T TR / BL B BR
            elif edge_l or edge_r:
                tid = 4 if edge_l else 5
            else:
                k = c - 2
                ch = line[k] if 0 <= k < len(line) else " "
                tid = 0 if ch == " " else GLYPH0 + ord(ch) - ord("A")
            out += cell(tid, 3)
        rows.append(bytes(out))
    return b"".join(rows)


def make_glass_fill():
    return cell(40, 3) * 32                                                         # the glass fill row: 32 identical cells


def cell(tile, bank):
    return bytes((tile & 0xFF, 0, bank & 3, 0x10))      # word0 = tile ID, word1 = bank | $1000


def make_maps():
    bg0, bg1 = bytearray(), bytearray()
    for row in range(64):
        for col in range(64):
            if row < 6: tid = 0
            elif row < 12: tid = 1
            elif row < 17: tid = 2
            elif row < 22: tid = 3
            elif row == 22: tid = 4
            else: tid = 6 if (col * 5 + row * 3) % 11 == 0 and row < 41 else 5
            bg0 += cell(tid, 0)
    clouds = {}
    for cx, cy in ((3, 3), (14, 6), (26, 2), (37, 5), (49, 3), (58, 7)):
        for dy in range(2):
            for dx in range(3):
                clouds[((cx + dx) % 64, cy + dy)] = 1 + dy * 3 + dx
    for row in range(64):
        for col in range(64):
            if row == 28: tid = 7
            elif 29 <= row <= 40: tid = 8
            else: tid = clouds.get((col, row), 0)
            bg1 += cell(tid, 1)
    return bytes(bg0), bytes(bg1)


def make_sprite_cells():
    out = bytearray()
    for tid in range(13):                                                          # 0 ball, 1-4 box, 5-12 ship, all bank 2
        out += cell(tid, 2)
    return bytes(out).ljust(256, b"\0")


# ---- cartridge ---------------------------------------------------------------------------------------------------------------
class Asm:
    def __init__(self):
        self.lines = []

    def __call__(self, s):
        self.lines.append("    " + s)

    def label(self, name):
        self.lines.append(name + ":")

    def raw(self, s):
        self.lines.append(s)


SCRIPTS = os.path.join(LAYERS, "..", "..", "scripts")
APU_CODE, APU_TABLES = 0x10000, 0x11000             # the ADPCM decoder's APU program and tables (bank 1, below the PPU segment)


def make_adpcm(audio, rate, seconds, workdir):
    """Encodes `audio` and generates the APU decoder (scripts/build_adpcm_rom.py's steps). Returns what the CPU side and the linker need."""
    sys.path.insert(0, SCRIPTS)
    import ima_adpcm as ima
    import gen_adpcm_apu as gen
    pitch = round(rate * 4096 / 48000)
    real_rate = pitch * 48000 / 4096
    nat = ima._probe_frames(audio, real_rate, seconds)
    frames = max(1, round(nat / ima.PAGE_FRAMES)) * ima.PAGE_FRAMES
    print("adpcm: %s, %d frames at %.1f Hz stretched to %d (%.2f s)" % (os.path.basename(audio), nat, real_rate, frames, frames / real_rate))
    left, right = ima.read_pcm(audio, real_rate, frames, seconds)
    stream, il, ir = ima.encode_stereo(left, right)
    pages = frames // ima.PAGE_FRAMES
    code = gen.assemble(gen.generate(pitch, pages, il, ir), workdir, gen.default_apuasm())
    tables = gen.tables_image()
    pad = lambda d: d + bytes(-len(d) % 256)
    code, tables = pad(code), pad(tables)
    assert len(code) <= APU_TABLES - APU_CODE and len(tables) <= 0x10000 - (APU_TABLES - APU_CODE)
    return dict(stream=stream, code=code, tables=tables, pages=pages, in_page0=gen.IN_PAGE0, in_pages=gen.IN_PAGES, rate=real_rate, seconds=frames / real_rate)


def build(outdir, v3=False, adpcm=None, rate=40000, seconds=None):
    os.makedirs(outdir, exist_ok=True)
    if adpcm and v3:
        sys.exit("--adpcm needs the v2 scheme: the feeder does not verify chunks")
    b = lambda name: os.path.join(outdir, name)
    run = lambda *cmd: subprocess.run(cmd, check=True, capture_output=True, text=True)

    # microcode, frame blocks
    run(sys.executable, os.path.join(LAYERS, "gen_layers.py"), "-o", b("layers.bin"))
    prog = open(b("layers.bin"), "rb").read()
    pages = (len(prog) + 255) // 256
    prog = prog.ljust(pages * 256, b"\0")
    assert len(prog) <= 0x1000
    run("cc", "-O2", "-o", b("gen_frames"), os.path.join(HERE, "gen_frames.c"), "-lm")
    res = subprocess.run([b("gen_frames"), b("frames.bin"), b("window.bin")], capture_output=True, text=True)
    sys.stderr.write(res.stderr)
    if res.returncode:
        sys.exit("gen_frames dropped sprite pieces: the demo must fit its budget")
    frames = open(b("frames.bin"), "rb").read()
    assert len(frames) == FRAMES * FRAME_BLOCK

    # assets
    tiles = build_tiles()
    bg0, bg1 = make_maps()
    PAL_OFF, CELL_OFF, TILE_OFF, WIN_OFF, MAP0_OFF, MAP1_OFF, FRAME_OFF = 0x1000, 0x1200, 0x1300, 0x3000, 0x4000, 0x8000, 0x10000
    wintab = open(b("window.bin"), "rb").read()
    wincells, glassfill = make_window_cells(), make_glass_fill()
    assert len(wintab) == G.WIN_TABLE_BYTES and len(wincells) == 4 * BOX_W * BOX_H and len(wincells) <= G.WINCELLS_END - G.WINCELLS
    WINTAB_OFF, WINCELLS_OFF, GLFILL_OFF = WIN_OFF, WIN_OFF + 0x200, WIN_OFF + 0x600
    tile_keys = sorted(tiles)
    blob = bytearray(prog)
    def put(off, data):
        nonlocal blob
        blob = blob.ljust(off, b"\0")
        assert len(blob) == off, "asset overlap at %x" % off
        blob += data
    put(PAL_OFF, make_palette())
    put(CELL_OFF, make_sprite_cells())
    put(TILE_OFF, b"".join(tiles[k] for k in tile_keys))
    put(WINTAB_OFF, wintab)
    put(WINCELLS_OFF, wincells)
    put(GLFILL_OFF, glassfill)
    put(MAP0_OFF, bg0)
    put(MAP1_OFF, bg1)
    assert FRAME_OFF % 0x10000 == 0 and len(frames) == 0x10000, "the frame loop addresses the blocks as bank + k"
    put(FRAME_OFF, frames)
    open(b("blob.bin"), "wb").write(blob)
    ROM = PPU_ROM
    snd = None
    if adpcm:
        snd = make_adpcm(adpcm, rate, seconds, outdir)
        STREAM_ROM = PPU_ROM + len(blob)
        assert STREAM_ROM % 256 == 0 and STREAM_ROM + len(snd["stream"]) <= 0x800000, \
            "the track (%d KiB) does not fit after the demo (%d KiB): lower --rate or use --seconds" % (len(snd["stream"]) // 1024, len(blob) // 1024)
        for name, data in (("apu.bin", snd["code"]), ("tables.bin", snd["tables"]), ("stream.bin", snd["stream"])):
            open(b(name), "wb").write(data)

    # CPU program
    a = Asm()
    a.raw(".org $8000")
    a.label("main")
    a("sep #$30                      ; 8-bit A/X/Y: cpuasm's immediates are one byte (see zp_dma.asm)")
    if adpcm:
        a("sdb #$80                      ; DB = work RAM: the ADPCM feeder's variables are plain absolutes")

    def verify_blob(off, length):
        """Asks the Boot ROM to check every chunk of the blob that [off, off+length) touches (a no-op once a chunk is verified)."""
        if not v3:
            return                                                    # v2: the whole ROM was hashed at boot, nothing is gated
        for c in range(off // CHUNK, (off + length - 1) // CHUNK + 1):
            a("lda #$%02X" % (FIRST_CHUNK + c))
            a("jsr vchunk")

    verify_blob(0, pages * 256)                                       # COP #$F7 reads the PPU program through the gated bus
    a("lda #$10")
    a("ldx #$00")
    a("ldy #$%02X" % pages)
    a("cop #$F7                      ; load and start the PPU program (the blob's first pages)")

    def dma(src, dst, length, chan=0):
        verify_blob(src - ROM, (length + 255) & ~255)                 # zp_dma_load rounds up to 256 bytes
        for off, v in enumerate((src & 255, (src >> 8) & 255, (src >> 16) & 255, dst & 255, (dst >> 8) & 255, (dst >> 16) & 255, length & 255, length >> 8, chan)):
            a("lda #$%02X" % v)
            a("sta $8000%02X" % (0x10 + off))
        a("jsr zp_dma_load")

    ppu = lambda addr: "$B0%04X" % addr
    if snd:
        # the APU may be mid-way through the BIOS jingle: reset and halt it first (a reset clears APU RAM, so before the uploads)
        a("lda #$03"); a("sta $D80010                    ; APUCTRL: reset + halt")
        dma(APU_CODE, 0xA00200, len(snd["code"]))
        dma(APU_TABLES, 0xA09000, len(snd["tables"]))
        a("lda #$00"); a("sta $A01000"); a("sta $A01800   ; both mailboxes clear")
        a("sta $0100"); a("sta $0108"); a("sta $010A"); a("sta $010B   ; feeder: wr page, src mid, pages sent (16-bit)")
        a("lda #$%02X" % (STREAM_ROM >> 16)); a("sta $0109             ; src bank")
        a("jsr feed_pump                 ; prefill the ring; the decoder starts once the setup is done")
    dma(ROM + PAL_OFF, 0xB00000 + G.PAL, 512)
    dma(ROM + CELL_OFF, 0xB00000 + G.SPRCELLS, 256)
    dma(ROM + MAP0_OFF, 0xB00000 + G.MAP[0], 0x4000)
    dma(ROM + MAP1_OFF, 0xB00000 + G.MAP[1], 0x4000)
    dma(ROM + FRAME_OFF, 0xB00000 + G.LISTS, FRAME_BLOCK)        # frame 0 into list buffer 0
    # the window: its table, the glass fill row, the textbox's cells. zp_dma_load rounds every length up to 256 bytes and the blob is zero-padded, so
    # each of these spills zeros past its end: the cells go last because they sit right after the fill row and the table's spill lands in the idle WIN1.
    dma(ROM + WINTAB_OFF, 0xB00000 + G.WIN0, G.WIN_TABLE_BYTES)
    dma(ROM + GLFILL_OFF, 0xB00000 + G.GLFILL, len(glassfill))
    dma(ROM + WINCELLS_OFF, 0xB00000 + G.WINCELLS, len(wincells))

    verify_blob(TILE_OFF, 64 * len(tile_keys))
    # tiles: the PPU selects each (bank, id); the CPU writes its 64 bytes through the $0300 window
    treq, tbank, tid = G.ctrl("TILEREQ"), G.ctrl("TILEBANK"), G.ctrl("TILEID")
    for n, (bank, tile) in enumerate(tile_keys):
        a("lda #$%02X" % bank); a("sta %s" % ppu(tbank)); a("lda #$00"); a("sta %s" % ppu(tbank + 1))
        a("lda #$%02X" % tile); a("sta %s" % ppu(tid)); a("lda #$00"); a("sta %s" % ppu(tid + 1))
        a("lda #$01"); a("sta %s" % ppu(treq))
        a.label("tw%d" % n); a("lda %s" % ppu(treq)); a("cmp #$02"); a("beq tg%d" % n); a("bra tw%d" % n)
        a.label("tg%d" % n); a("ldx #$00")
        a.label("tc%d" % n)
        a("lda $%06X,X" % (ROM + TILE_OFF + 64 * n)); a("sta $B00300,X"); a("inx"); a("txa"); a("cmp #$40"); a("beq td%d" % n); a("bra tc%d" % n)
        a.label("td%d" % n); a("lda #$03"); a("sta %s" % ppu(treq))
        a.label("tz%d" % n); a("lda %s" % ppu(treq)); a("beq tn%d" % n); a("bra tz%d" % n)
        a.label("tn%d" % n)

    # configuration, then go
    def w16(addr, v):
        a("lda #$%02X" % (v & 255)); a("sta %s" % ppu(addr)); a("lda #$%02X" % (v >> 8)); a("sta %s" % ppu(addr + 1))
    w16(G.ctrl("NCOLS"), 33)
    w16(G.ctrl("SPREN"), 1)
    w16(G.ctrl("L1FLAGS"), G.F_TRANSPARENT)              # BG1 is transparent wherever its tiles have alpha 0
    w16(G.ctrl("LISTBASE"), G.LISTS)
    w16(G.ctrl("WINBASE"), G.WIN0)                       # the window layer: static, so the table is flipped in once
    w16(G.ctrl("RUN"), 1)
    a("lda #$01"); a("sta $800030                    ; k: the next frame block to send")
    a("lda #$80"); a("sta $D80050                    ; enable the V-blank timer: one animation step per 1/60 s of console time")
    a("lda #$%02X" % (G.LISTS2 >> 8)); a("sta $800031 ; high byte of the list buffer we fill next")

    if snd:
        a("lda #$01"); a("sta $D8001A                    ; APU fast mode (after the uploads: DMA to the APU window runs at half rate in it)")
        a("lda #$00"); a("sta $D80012"); a("lda #$05"); a("sta $D80013   ; APU PC = $0500")
        a("lda #$04"); a("sta $D80010                    ; resume: the decoder runs")

    # one iteration per PPU frame: DMA the next frame block into the idle buffer, flip to it, wait for the PPU to take it
    a.label("frame_loop")
    if v3:
        a("lda $800030"); a("lsr A"); a("lsr A"); a("lsr A"); a("lsr A"); a("lsr A"); a("lsr A"); a("clc"); a("adc #$%02X" % (FIRST_CHUNK + FRAME_OFF // CHUNK))
        a("jsr vchunk                    ; frame block k sits in chunk FRAME_OFF/16K + k/64 (64 blocks of 256 bytes each; a no-op once checked)")
    a("lda $800030"); a("sta $800011      ; src mid = k & 255 (256-byte blocks), low byte 0")
    a("lda #$%02X" % ((ROM + FRAME_OFF) >> 16)); a("sta $800012      ; src bank: all the frame blocks fit one 64 KiB bank")
    a("lda #$00"); a("sta $800010"); a("sta $800013")
    a("lda $800031"); a("sta $800014")
    a("lda #$B0"); a("sta $800015")
    a("lda #$00"); a("sta $800016"); a("lda #$%02X" % (FRAME_BLOCK >> 8)); a("sta $800017"); a("lda #$00"); a("sta $800018")
    a("jsr zp_dma_load")
    a("lda #$00"); a("sta %s" % ppu(G.ctrl("LISTBASE")))
    a("lda $800031"); a("sta %s" % ppu(G.ctrl("LISTBASE") + 1))
    a("lda #$00"); a("sta %s" % ppu(G.ctrl("FRAMES"))); a("sta %s" % ppu(G.ctrl("FRAMES") + 1))
    a.label("wait_frame")
    a("lda %s" % ppu(G.ctrl("FRAMES"))); a("beq wait_frame ; the PPU counts a frame when it snapshots LISTBASE: the other buffer is free")
    a("lda $800031"); a("xor #$10"); a("sta $800031")
    a("lda $800030"); a("inc A"); a("sta $800030")
    a.label("tick_wait")
    if snd:
        a("jsr feed_pump                 ; top up the ADPCM ring while waiting for the tick")
    a("lda $D80051"); a("and #$80"); a("beq tick_wait ; hold the step until the V-blank timer ticks")
    a("lda #$80"); a("sta $D80051                  ; acknowledge it")
    a("bra frame_loop")
    if snd:
        # feed_pump: one 256-byte page per DMA from ROM into the APU's input ring ($A0:IN_PAGE0.., IN_PAGES pages), until the ring is full
        # (never to the brim: a page is only written when the one after it is not the page the APU reads). Mailboxes: $A0:1000 = the page
        # we fill next, published after each page lands; $A0:1800 = the page the APU is reading. Needs DB=$80 ($0100.. are plain absolutes;
        # cpuasm has no long CMP/INC).   $0100 wr page  $0102 next  $0103 APU read page  $0108/9 ROM src mid/bank  $010A/B pages sent
        a.label("feed_pump")
        a("lda $0100"); a("clc"); a("adc #$01"); a("cmp #$%02X" % snd["in_pages"]); a("beq fp_wrap"); a("bra fp_nowrap")
        a.label("fp_wrap"); a("lda #$00")
        a.label("fp_nowrap"); a("sta $0102")
        a("lda $A01800"); a("sta $0103")
        a("lda $0102"); a("cmp $0103"); a("beq fp_done")
        a("lda #$00"); a("sta $800010"); a("lda $0108"); a("sta $800011"); a("lda $0109"); a("sta $800012")
        a("lda #$00"); a("sta $800013")
        a("lda $0100"); a("clc"); a("adc #$%02X" % snd["in_page0"]); a("sta $800014")
        a("lda #$A0"); a("sta $800015")
        a("lda #$00"); a("sta $800016"); a("lda #$01"); a("sta $800017"); a("lda #$00"); a("sta $800018")
        a("jsr zp_dma_load               ; blocks until the page has landed")
        a("inc $0108"); a("bne fp_nc1"); a("inc $0109")
        a.label("fp_nc1")
        a("lda $0102"); a("sta $0100"); a("sta $A01000    ; publish")
        a("inc $010A"); a("bne fp_nc2"); a("inc $010B")
        a.label("fp_nc2")
        a("lda $010A"); a("cmp #$%02X" % (snd["pages"] & 0xFF)); a("bne fp_pump_again")
        a("lda $010B"); a("cmp #$%02X" % (snd["pages"] >> 8)); a("bne fp_pump_again")
        a("lda #$00"); a("sta $010A"); a("sta $010B"); a("sta $0108")          # end of the track: back to the start (the APU reloads its state at the same page)
        a("lda #$%02X" % (STREAM_ROM >> 16)); a("sta $0109")
        a.label("fp_pump_again"); a("bra feed_pump")
        a.label("fp_done"); a("rts")
    # vchunk: 8-bit A = manifest chunk index -> zp_verify_chunk (which wants a 16-bit A; cpuasm has no XBA or 16-bit #imm, so via RAM)
    a.label("vchunk")
    a("sta $800040"); a("lda #$00"); a("sta $800041")
    a("rep #$20"); a("lda $800040")
    a("jsr zp_verify_chunk")
    a("sep #$20")
    a("rts")
    a.raw(".include \"%s\"" % os.path.join(TOOLS, "..", "lib", "zp_dma.asm"))
    a.raw(".include \"%s\"" % os.path.join(TOOLS, "..", "lib", "zp_cop.asm"))
    open(b("demo.asm"), "w").write("\n".join(a.lines) + "\n")

    run(os.path.join(TOOLS, "cpuasm"), b("demo.asm"), b("demo.bin"))
    assert 0x8000 + os.path.getsize(b("demo.bin")) <= CODESIZE, "the CPU program outgrew CODESIZE: it would be hashed as data"
    run(os.path.join(TOOLS, "zplink"), "-o", b("demo.elf"), "--cpu", b("demo.bin") + "@0x8000", "--ppu", b("blob.bin") + "@0x100000", "--entry", "0x8000",
        *((("--cpu", b("apu.bin") + "@0x%X" % APU_CODE, "--cpu", b("tables.bin") + "@0x%X" % APU_TABLES,
           "--data", b("stream.bin") + "@0x%X" % STREAM_ROM) if snd else ())))
    run(os.path.join(TOOLS, "zpbuild"), b("demo.elf"), "-o", b("demo.zpb"), "--title", "LAYERS DEMO", "--dev", "ZP", "--codesize", str(CODESIZE), "--trailer", "3" if v3 else "2")
    print("%s: %d bytes of CPU code, %d tiles, ROM blob %d KiB" % (b("demo.zpb"), os.path.getsize(b("demo.bin")), len(tile_keys), len(blob) // 1024))


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", default=os.path.join(HERE, "build"))
    ap.add_argument("--v3", action="store_true", help="sign with the chunked-manifest scheme (per-chunk COP #$FF verification)")
    ap.add_argument("--adpcm", metavar="AUDIO", help="also play AUDIO (anything ffmpeg reads) on the APU through the ADPCM decoder, looping (v2 only)")
    ap.add_argument("--rate", type=int, default=40000, help="ADPCM sample rate (default 40000: a 173 s track then fits beside the demo)")
    ap.add_argument("--seconds", type=float, help="only the first N seconds of the audio")
    args = ap.parse_args()
    build(args.out, args.v3, args.adpcm, args.rate, args.seconds)

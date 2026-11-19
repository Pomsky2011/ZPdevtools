#!/usr/bin/env python3
"""Two scrolling background layers + sprites as PPU microcode, v2: runs on the PPU's tile-row queue
(ZeroPoint docs/ppu.md "Tile-row queue"; design in docs/ppu/layers.md).

v1 (gen_layers_v1.py) issued every tile itself (33 cycles per cell, issue-bound, and a late band flickered). v2 launches whole
map rows at the queue's port and lets the hardware hold each one until its rows are writable, so the PPU only does ~40 cycles
of work per row command and runs ahead of the beam by however much the 128-cell queue holds.

  gen_layers.py [-o out.bin] [--asm out.asm]

v2 also has one standstill "window" layer above everything (docs/ppu/layers.md "Window layer"): per band one ink span (tile-aligned cells,
opaque = culls the BG under it, or overlay = transparent over what is below) and one glass span (a blended fill).
"""
import argparse
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
PPUASM = os.path.join(HERE, "..", "..", "executables", "ppuasm")

# ---- PPU memory layout (mirrored in zp_layers_layout.h). Write-ahead mode makes $C000-$FFFF open bus. -------------
CODE_BASE = 0x0400      # $0000 holds a stub; $00E0 / $00F0-$00FF / $0200-$0207 / $0300-$036F are live ports
PAL = 0x1C00            # 256 x BBGR16 (512 B), loaded once with PALETTE256
CTRL = 0x1E00           # control block, words (CTRL_WORDS)
STACK = 0x1F00          # grows UP, 256 B
SPRCELLS = 0x1800       # sprite tile cells (4 B each) the CPU fills: room for 256 (1 KiB), $1800-$1BFF
WIN0 = 0x0E00           # window tables (header + WIN_BANDS records), double-buffered like the frame blocks: the CPU fills the idle one and flips WINBASE
WIN1 = 0x0F90
GLFILL = 0x1120         # the glass fill: one row of 32 identical cells the CPU fills (128 B)
WINCELLS = 0x11A0       # free pool for the cells of window boxes, up to WINCELLS_END (nothing reads it but the boxes' own SRC pointers)
WINCELLS_END = 0x1800
WIN_HDR_BYTES = 4       # table header: word GLMODE (1 average, 2 subtract, 3 add), word reserved (alpha, see docs)
WIN_REC_BYTES = 12      # per band: INKCNT INKCOL INKSRC INKFLG GLCNT GLCOL (words)
WIN_TABLE_BYTES = WIN_HDR_BYTES + 32 * WIN_REC_BYTES
LISTS = 0x2000          # sprite pieces, 32 bands x LIST_STRIDE B, two buffers: the CPU fills the idle one and flips LISTBASE
LISTS2 = 0x3000
MAP = (0x4000, 0x8000)  # 64x64 cells x 4 B = 16 KiB each
TILE_MODE_FLAGS = 0x20  # queue flag bits 4-5 = tile mode 2 (16BBGR 8bpp: the 256-entry palette)
F_CLIP, F_WRAP, F_TRANSPARENT = 0x01, 0x08, 0x40
BANDS = 32
MAXP = 8                # sprite pieces per band the PPU will launch (a header above this is clamped)
BAND_BUDGET = 1300      # blitter channel-cycles of sprite work the list builder admits per band (zp_layers_sprites.h): bands first go late at ~1550 under the worst fine scroll, see docs/ppu/layers.md
LIST_STRIDE = 128       # the builder's dense form: per band a word count, then MAXP records of 7 words in port order (2 + 8 x 14 = 114 B)
RECORD_BYTES = 14       # what the PPU reads is the packed stream: per band a word count, then count records, bands back to back
LIST_RECORD_WORDS = 7   # src, count, x, y, clip0, clip1, flags
QUEUE_CELLS = 128

CTRL_WORDS = ["NCOLS", "SPREN", "FRAMES", "L1FLAGS", "TILEREQ", "TILEBANK", "TILEID", "RUN", "LISTBASE", "WINBASE"]
SCROLL_OFF = 0          # the four scroll words SCX0 SCY0 SCX1 SCY1 start the frame block, then (STREAM_OFF) the sprite stream: one flip moves scroll and sprites together
STREAM_OFF = 8

REGS = dict(
    ORIG=53, CLAMP=54, C14=55,
    ZERO=4, C2=2, C8=3, ONE=5, C512=7, C256=8, C64=9, C128=10, CMAXP=11, FOUR=12, PENDP=13, PORTB=14, FBASE=15,
    SRC=16, CNT=17, PX=18, PY=19, CL0=20, CL1=21, FLG=22,
    SHX0=23, SHY0=24, SHX1=25, SHY1=26, NCOLS=27, SPREN=28, BAND=29, ROW0=30, KA=31, KB=32, MAPB=33, SCX=34, SCY=35,
    WY=36, TROW=37, O=38, TX=39, FX=40, T=41, T2=42, T3=43, REC=44, NREC=45, PORT=46, CULL=47, L1F=48, LAYERF=49, PFRAME=50, FRAMEP=51, LBASE=52,
    ICOL=6, WINBASE=56, WINR=57, ICNT=58,
    SP=61, PC=62, DP=63,
)

lines = []


def e(s=""):
    lines.append(s)


def r(name):
    return "R%d" % REGS[name] if REGS[name] < 59 else name


def li(reg, value):
    """Load a 16-bit constant or label address (8 cycles)."""
    e("    TARREG 2, LSB, %s" % r(reg))
    e("    TARREG 3, MSB, %s" % r(reg))
    if isinstance(value, str):
        e("    SETBYTE 2, %s" % value)
        e("    SETBYTE 3, %s" % value)
    else:
        e("    SETBYTE 2, 0x%02X" % (value & 0xFF))
        e("    SETBYTE 3, 0x%02X" % ((value >> 8) & 0xFF))


def mov(dst, src):
    e("    CLR %s" % r(dst))
    e("    ADD %s, %s" % (r(dst), r(src)))


def ins(op, a, b=None):
    e("    %s %s%s" % (op, r(a), (", " + r(b)) if b else ""))


def ldw(dst, addr):
    li("T3", addr)
    e("    SETDP %s" % r("T3"))
    e("    MOV %s" % r(dst))


def stw(addr, src):
    li("T3", addr)
    e("    SETDP %s" % r("T3"))
    e("    MOVDP %s" % r(src))


def ldp(dst, base, off):
    """dst = the word at (register `base`) + off. Clobbers T3."""
    mov("T3", base)
    for _ in range(off // 2):
        ins("ADD", "T3", "C2")
    e("    SETDP %s" % r("T3"))
    e("    MOV %s" % r(dst))


def launch(tag, guard=False):
    """Queue one row: wait until CNT more cells fit, then SRC CNT PX PY CL0 CL1 FLG -> the port and the launch. Inline: a JSR + RET is ~26 PPU
    cycles and the producer, not the blitter, is the bottleneck on a band of many launches. The frame word ($035E) is not written here, once
    per frame is enough (the port keeps its words), see frame_loop. `guard`: a count the queue could never hold (a corrupt sprite list) skips the
    wait, since room that cannot exist would block forever; the hardware drops that launch and flags it."""
    if guard:
        e("    CMP %s, %s" % (r("CNT"), r("C128")))
        e("    JMG ln_go_%s" % tag)
    label("ln_poll_%s" % tag)
    e("    SETDP %s" % r("PENDP")); e("    MOV %s" % r("T"))
    ins("ADD", "T", "CNT")
    e("    CMP %s, %s" % (r("T"), r("C128")))
    e("    JNG ln_go_%s" % tag)
    e("    JMR ln_poll_%s" % tag)
    label("ln_go_%s" % tag)
    e("    SETDP %s" % r("PORTB"))
    for name in ("SRC", "CNT", "PX", "PY", "CL0", "CL1", "FLG"):
        e("    MOVDP %s" % r(name)); e("    ADD DP, %s" % r("C2"))
    e("    ADD DP, %s" % r("C2"))                          # past the frame word
    e("    MOVDP %s" % r("ONE"))                           # $0360: the launch


def ctrl(name):
    return CTRL + 2 * CTRL_WORDS.index(name)


def label(name):
    e("%s:" % name)


def emit_program():
    e("; generated by gen_layers.py (v2, tile-row queue) - do not edit")
    e("    JMR main")
    for _ in range((CODE_BASE - 10) // 2):
        e("    DEFCALL R0, R0")      # a zero word, never executed: the loader copies this image through the PPU window, where a NOP ($1000) would write $10 to the live ports below $0400

    # ---- init ------------------------------------------------------------------------------------------------------
    label("main")
    li("SP", STACK)
    for name, val in (("ZERO", 0), ("C2", 2), ("C8", 8), ("ONE", 1), ("C512", 512), ("C256", 256), ("C64", 64), ("C128", 128),
                      ("CMAXP", MAXP), ("C14", RECORD_BYTES), ("FOUR", 4), ("PENDP", 0x0360), ("PORTB", 0x0350), ("FBASE", TILE_MODE_FLAGS | F_WRAP), ("FRAMEP", 0x0366)):
        li(name, val)
    li("T", 0x0362)                  # write-ahead mode: $C000-$FFFF are open bus from here on
    e("    SETDP %s" % r("T"))
    e("    MOVDP %s" % r("ONE"))

    # ---- idle until the CPU says RUN: serve its tile uploads (the tile window at $0300 is keyed on the PPU's live bank/ID, so the
    # PPU selects the tile, the CPU writes its 64 bytes, then tells us it is done). TILEREQ: 1 = CPU asks, 2 = selected, 3 = CPU done.
    label("idle_loop")
    ldw("T", ctrl("TILEREQ"))
    e("    CMP %s, %s" % (r("T"), r("ONE")))
    e("    JNZ idle_run")
    ldw("T2", ctrl("TILEBANK")); e("    SETTILEBANK %s" % r("T2"))
    ldw("T2", ctrl("TILEID")); e("    SETTILE %s, %d" % (r("T2"), 2))
    li("T2", 2); stw(ctrl("TILEREQ"), "T2")
    label("idle_wait_done")
    ldw("T", ctrl("TILEREQ")); li("T2", 3)
    e("    CMP %s, %s" % (r("T"), r("T2")))
    e("    JNZ idle_wait_done")
    stw(ctrl("TILEREQ"), "ZERO")
    e("    JMR idle_loop")
    label("idle_run")
    ldw("T", ctrl("RUN"))
    e("    CMP %s, %s" % (r("T"), r("ZERO")))
    e("    JMZ idle_loop")
    li("T", PAL)                     # the CPU has put the palette in place by now
    e("    SETDP %s" % r("T"))
    e("    PALETTE256")
    # The frame counter starts level with the hardware's *now*, when the CPU says RUN (not at power-on: a cartridge can spend many
    # frames uploading first); the first frame_loop makes it the next frame.
    e("    SETDP %s" % r("FRAMEP")); e("    MOV %s" % r("PFRAME"))

    # ---- one frame: snapshot the CPU-visible registers, then produce its 32 bands -----------------------------------------
    label("frame_loop")
    ldw("NCOLS", ctrl("NCOLS")); ldw("SPREN", ctrl("SPREN")); ldw("L1F", ctrl("L1FLAGS")); ldw("LBASE", ctrl("LISTBASE")); ldw("WINBASE", ctrl("WINBASE"))
    li("T2", SCROLL_OFF); mov("T", "LBASE"); ins("ADD", "T", "T2")           # scroll comes from the same block as the sprite lists
    for name in ("SHX0", "SHY0", "SHX1", "SHY1"):
        e("    SETDP %s" % r("T")); e("    MOV %s" % r(name)); ins("ADD", "T", "C2")
    mov("REC", "LBASE"); ins("ADD", "REC", "C8")                              # REC walks the sprite stream across the bands: it starts after the scroll words
    ldw("T", ctrl("FRAMES")); e("    INC %s" % r("T")); stw(ctrl("FRAMES"), "T")
    e("    INC %s" % r("PFRAME"))
    li("T", 0x035E); e("    SETDP %s" % r("T")); e("    MOVDP %s" % r("PFRAME"))     # every row of this frame names it (the port keeps the word)
    e("    CLR %s" % r("BAND"))

    label("band_loop")
    mov("ROW0", "BAND"); ins("MUL", "ROW0", "C8")
    # Rows are frame-local; every launch names the frame (PFRAME) it belongs to, so the hardware places it - nothing here
    # depends on where the beam is, and a launch that waits for queue space cannot land a frame off.

    # This band's window record (WINBASE = 0: no window). An opaque ink span (INKFLG = 0) hides the BG under it, so CULL tells do_layer to leave
    # those columns out; an overlay span (INKFLG = T) shows what is below, so it never culls.
    e("    CLR %s" % r("ICNT")); e("    CLR %s" % r("CULL"))
    e("    CMP %s, %s" % (r("WINBASE"), r("ZERO")))
    e("    JMZ win_read_done")
    mov("WINR", "WINBASE"); ins("ADD", "WINR", "ROW0"); ins("ADD", "WINR", "FOUR")      # + 8 per band + the 4-byte header
    mov("T", "BAND"); ins("MUL", "T", "FOUR"); ins("ADD", "WINR", "T")                  # + 4 per band: 12 bytes a record
    ldp("ICNT", "WINR", 0)
    e("    CMP %s, %s" % (r("ICNT"), r("ZERO")))
    e("    JMZ win_read_done")
    ldp("ICOL", "WINR", 2)
    ldp("T", "WINR", 6)
    e("    CMP %s, %s" % (r("T"), r("ZERO")))
    e("    JNZ win_read_done")
    e("    INC %s" % r("CULL"))
    label("win_read_done")

    # BG0 then BG1: queue order is z-order
    mov("SCX", "SHX0"); mov("SCY", "SHY0"); li("MAPB", MAP[0]); e("    CLR %s" % r("LAYERF"))
    e("    JSR do_layer")
    mov("SCX", "SHX1"); mov("SCY", "SHY1"); li("MAPB", MAP[1]); mov("LAYERF", "L1F")
    e("    JSR do_layer")

    e("    CMP %s, %s" % (r("SPREN"), r("ZERO")))
    e("    JMZ sprites_done")
    e("    JSR do_sprites")
    label("sprites_done")
    e("    CMP %s, %s" % (r("WINBASE"), r("ZERO")))
    e("    JMZ band_next")
    e("    JSR do_window")                                  # above everything, so last
    label("band_next")

    e("    INC %s" % r("BAND"))
    li("T", BANDS)
    e("    CMP %s, %s" % (r("BAND"), r("T")))
    e("    JNZ band_loop")
    e("    JMR frame_loop")

    # ---- do_layer: one layer's pieces for the band: SCX SCY MAPB LAYERF ROW0 NCOLS in. Works out the layer's row state, then draws
    # the screen cells [KA, KB) of it (do_range); under an opaque window span only the cells the span does not hide. -----------------------
    label("do_layer")
    mov("WY", "ROW0"); ins("ADD", "WY", "SCY")
    mov("T", "WY"); ins("INTDIV", "T", "C512"); ins("MUL", "T", "C512"); ins("SUB", "WY", "T")     # world row mod 512
    mov("TROW", "WY"); ins("INTDIV", "TROW", "C8")
    mov("O", "TROW"); ins("MUL", "O", "C8"); mov("T", "WY"); ins("SUB", "T", "O"); mov("O", "T")   # fine Y 0..7
    mov("TX", "SCX"); ins("INTDIV", "TX", "C8")
    mov("FX", "TX"); ins("MUL", "FX", "C8"); mov("T", "SCX"); ins("SUB", "T", "FX"); mov("FX", "T")
    e("    CMP %s, %s" % (r("CULL"), r("ZERO")))
    e("    JNZ dl_culled")
    e("    CLR %s" % r("KA")); mov("KB", "NCOLS")
    e("    JSR do_range")
    e("    RET")
    # Screen cell k covers x = 8k - FX .. 8k - FX + 7; the span hides x = 8 ICOL .. 8 (ICOL + ICNT) - 1. With a fine X the cell at ICOL still shows its
    # left FX pixels and the cell at ICOL + ICNT its right ones, so the left piece is [0, ICOL + (FX != 0)) and the right one [ICOL + ICNT, NCOLS).
    label("dl_culled")
    e("    CLR %s" % r("KA")); mov("KB", "ICOL")
    e("    CMP %s, %s" % (r("FX"), r("ZERO")))
    e("    JMZ dl_fx0")
    e("    INC %s" % r("KB"))
    label("dl_fx0")
    e("    CMP %s, %s" % (r("KB"), r("NCOLS")))
    e("    JNG dl_left")
    mov("KB", "NCOLS")
    label("dl_left")
    e("    JSR do_range")
    mov("KA", "ICOL"); ins("ADD", "KA", "ICNT"); mov("KB", "NCOLS")
    e("    JSR do_range")
    e("    RET")

    # ---- do_range: the layer's screen cells [KA, KB) of this band (nothing if empty): one queue row per half of the tile row ------------
    label("do_range")
    e("    CMP %s, %s" % (r("KB"), r("KA")))
    e("    JMG dr_go")
    e("    RET")
    label("dr_go")
    mov("CNT", "KB"); ins("SUB", "CNT", "KA")
    mov("PX", "KA"); ins("MUL", "PX", "C8"); ins("SUB", "PX", "FX")
    mov("T3", "TX")                                                            # first world column (< 64 already when KA = 0), wrapped to the 64-cell row
    e("    CMP %s, %s" % (r("KA"), r("ZERO")))
    e("    JMZ dr_col")
    ins("ADD", "T3", "KA")
    mov("T2", "T3"); ins("INTDIV", "T2", "C64"); ins("MUL", "T2", "C64"); ins("SUB", "T3", "T2")
    label("dr_col")
    ins("MUL", "T3", "FOUR")
    # half 1: tile row TROW, top at ROW0 - O, clipped to [ROW0, ROW0 + 8 - O) when O != 0
    mov("PY", "ROW0"); ins("SUB", "PY", "O")
    mov("SRC", "TROW"); ins("MUL", "SRC", "C256"); ins("ADD", "SRC", "MAPB"); ins("ADD", "SRC", "T3")
    mov("FLG", "FBASE"); ins("ADD", "FLG", "LAYERF")
    e("    CLR %s" % r("CL0")); e("    CLR %s" % r("CL1"))
    e("    CMP %s, %s" % (r("O"), r("ZERO")))
    e("    JMZ dr_go1")
    e("    INC %s" % r("FLG"))                       # F_CLIP
    mov("CL0", "ROW0")
    mov("CL1", "CL0"); ins("ADD", "CL1", "C8"); ins("SUB", "CL1", "O")
    label("dr_go1")
    launch("r1")
    e("    CMP %s, %s" % (r("O"), r("ZERO")))
    e("    JMZ dr_done")
    # half 2: tile row TROW + 1 (mod 64), top at ROW0 + 8 - O, clipped to [that, ROW0 + 8)
    mov("T", "TROW"); e("    INC %s" % r("T"))
    mov("T2", "T"); ins("INTDIV", "T2", "C64"); ins("MUL", "T2", "C64"); ins("SUB", "T", "T2")
    mov("SRC", "T"); ins("MUL", "SRC", "C256"); ins("ADD", "SRC", "MAPB"); ins("ADD", "SRC", "T3")
    mov("PY", "ROW0"); ins("ADD", "PY", "C8"); ins("SUB", "PY", "O")
    mov("CL0", "PY")
    mov("CL1", "ROW0"); ins("ADD", "CL1", "C8")
    launch("r2")
    label("dr_done")
    e("    RET")

    # ---- do_window: the band's window pass, after everything else. The glass span first (a fill row, blended), then the ink span over it. The
    # queue snapshots the blend settings at launch, so they are pushed just before the glass launch and put back right after it: nothing else
    # is ever blended. Alpha stays 15 (every slot): the blit applies the *live* translucency to its colour when it retires, so a lower
    # alpha would leak into whatever retires between the push and the restore (docs/ppu/layers.md "Window layer"). ---------------------------
    label("do_window")
    ldp("CNT", "WINR", 8)
    e("    CMP %s, %s" % (r("CNT"), r("ZERO")))
    e("    JMZ dw_ink")
    li("T2", 0xFFFF); li("DP", 0x00FE); e("    MOVDP %s" % r("T2"))                           # translucency values all 15
    ldp("T2", "WINBASE", 0); li("T", 0x0800); ins("ADD", "T2", "T"); li("DP", 0x00FC); e("    MOVDP %s" % r("T2"))   # mode | push the blend
    ldp("PX", "WINR", 10); ins("MUL", "PX", "C8")
    li("SRC", GLFILL); mov("PY", "ROW0"); e("    CLR %s" % r("CL0")); e("    CLR %s" % r("CL1"))
    li("FLG", TILE_MODE_FLAGS)
    launch("g")
    li("T2", 0xFFFF); li("DP", 0x00FE); e("    MOVDP %s" % r("T2"))
    li("T2", 0x0800); li("DP", 0x00FC); e("    MOVDP %s" % r("T2"))                           # blend mode 0 again
    label("dw_ink")
    e("    CMP %s, %s" % (r("ICNT"), r("ZERO")))
    e("    JMZ dw_done")
    ldp("SRC", "WINR", 4); mov("CNT", "ICNT"); mov("PX", "ICOL"); ins("MUL", "PX", "C8")
    mov("PY", "ROW0"); e("    CLR %s" % r("CL0")); e("    CLR %s" % r("CL1"))
    ldp("FLG", "WINR", 6); li("T", TILE_MODE_FLAGS); ins("ADD", "FLG", "T")
    launch("i")
    label("dw_done")
    e("    RET")

    # ---- do_sprites: the band's piece list from the packed stream: [word count][count x 14 B]; REC is a running pointer, so an empty band
    # costs two bytes of ROM. A count above MAXP is clamped and the surplus skipped (the builder never makes one). ---------------------
    label("do_sprites")
    e("    SETDP %s" % r("REC")); e("    MOV %s" % r("ORIG"))
    ins("ADD", "REC", "C2")
    mov("NREC", "ORIG")
    e("    CMP %s, %s" % (r("NREC"), r("CMAXP")))
    e("    JNG sp_count_ok")
    mov("NREC", "CMAXP")                              # admission: at most MAXP pieces per band
    label("sp_count_ok")
    mov("CLAMP", "NREC")
    label("sp_loop")
    e("    CMP %s, %s" % (r("NREC"), r("ZERO")))
    e("    JMZ sp_done")
    e("    SETDP %s" % r("REC"))
    for name in ("SRC", "CNT", "PX", "PY", "CL0", "CL1", "FLG"):         # the record is the port's words in order
        e("    MOV %s" % r(name)); e("    ADD DP, %s" % r("C2"))
    mov("REC", "DP")
    launch("s", guard=True)
    e("    DEC %s" % r("NREC"))
    e("    JMR sp_loop")
    label("sp_done")
    mov("T", "ORIG"); ins("SUB", "T", "CLAMP"); ins("MUL", "T", "C14"); ins("ADD", "REC", "T")   # skip what was clamped away
    e("    RET")


def write_layout_header(path):
    with open(path, "w") as f:
        f.write("// generated by gen_layers.py - PPU layout of the two-layer + sprite microcode (docs/ppu/layers.md)\n")
        f.write("#pragma once\n")
        for n, v in (("CODE_BASE", CODE_BASE), ("PAL", PAL), ("CTRL", CTRL), ("STACK", STACK), ("SPRCELLS", SPRCELLS), ("LISTS", LISTS), ("LISTS2", LISTS2),
                     ("MAP0", MAP[0]), ("MAP1", MAP[1]), ("WIN0", WIN0), ("WIN1", WIN1), ("GLFILL", GLFILL), ("WINCELLS", WINCELLS), ("WINCELLS_END", WINCELLS_END)):
            f.write("#define ZPL_%s 0x%04X\n" % (n, v))
        for n, v in (("BANDS", BANDS), ("SCROLL_OFF", SCROLL_OFF), ("STREAM_OFF", STREAM_OFF), ("MAXP", MAXP), ("BAND_BUDGET", BAND_BUDGET), ("LIST_STRIDE", LIST_STRIDE), ("LIST_RECORD_WORDS", LIST_RECORD_WORDS),
                     ("QUEUE_CELLS", QUEUE_CELLS), ("TILE_MODE_FLAGS", TILE_MODE_FLAGS), ("WIN_HDR_BYTES", WIN_HDR_BYTES), ("WIN_REC_BYTES", WIN_REC_BYTES),
                     ("WIN_TABLE_BYTES", WIN_TABLE_BYTES)):
            f.write("#define ZPL_%s %d\n" % (n, v))
        f.write("#define ZPL_F_CLIP 0x%02X\n#define ZPL_F_WRAP 0x%02X\n#define ZPL_F_TRANSPARENT 0x%02X\n" % (F_CLIP, F_WRAP, F_TRANSPARENT))
        for i, n in enumerate(CTRL_WORDS):
            f.write("#define ZPL_CTRL_%s 0x%04X\n" % (n, CTRL + 2 * i))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("-o", "--out", default=os.path.join(HERE, "build", "layers.bin"))
    ap.add_argument("--asm", default=None)
    args = ap.parse_args()
    os.makedirs(os.path.dirname(args.out), exist_ok=True)
    emit_program()
    asm = args.asm or os.path.splitext(args.out)[0] + ".asm"
    with open(asm, "w") as f:
        f.write("\n".join(lines) + "\n")
    write_layout_header(os.path.join(HERE, "zp_layers_layout.h"))
    res = subprocess.run([PPUASM, asm, "-o", args.out], capture_output=True, text=True)
    sys.stdout.write(res.stdout)
    sys.stderr.write(res.stderr)
    if res.returncode != 0:
        sys.exit(res.returncode)
    size = os.path.getsize(args.out)
    print("%s: %d bytes of code (must stay below $%04X)" % (args.out, size, WIN0))
    if size > WIN0:
        sys.exit("code overruns the window tables")


if __name__ == "__main__":
    main()

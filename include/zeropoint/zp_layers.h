/*
 * zp_layers.h - C library for the layers microcode (ucode/layers, docs/ppu/layers.md): two scrolling 64x64 tile
 * layers, sprites with three priorities, and a standstill window layer (textbox / HUD / glass), driven from the CPU.
 *
 * It does what the layers demo CPU program does, as a library: boots the microcode, loads palette, tiles, maps
 * and sprite cells, and then once a frame builds the sprite stream, sends it to the PPU and flips the frame block.
 * Header only, like the rest of the SDK (ZP_INLINE), and it compiles with both zpcc and def88186cc (-DZP_CC_DOS).
 *
 *     #include <zeropoint/zp_layers.h>
 *
 *     int main()
 *     {
 *         zpl_init(0x10, 0x0000, 4);                        // microcode: 4 pages of ROM at $10:0000
 *         zpl_palette_load(0x110000UL);                     // 512 bytes
 *         zpl_tiles_load(0, 0, 0x110200UL, 3);              // bank 0, tile ids 0.., 64 bytes each
 *         zpl_cells_load(0, 0x110400UL, 4);                 // the four cells of a 2x2 sprite
 *         zpl_map_load(0, 0x120000UL);                      // BG0, 16 KiB
 *         zpl_start();
 *         zpl_sprite_set(0, 100, 60, 2, 2, 0, 0);           // slot, x, y, w, h (in 8x8 tiles), first cell, flags
 *         for (;;) {
 *             zpl_scroll(0, 1, 0);
 *             zpl_frame();                                  // wait for the 60 Hz tick, send the frame
 *         }
 *     }
 *
 * What it keeps in RAM. The C compilers have no writable globals, so the state of the library is one block of Work RAM
 * at ZPL_RAM (default $80:4000, ZPL_RAM_SIZE bytes: define ZPL_RAM before including this to move it). Keep it clear of your own data.
 *
 * Order matters. Everything that goes into PPU memory (palette, tiles, maps, cells) must be loaded before
 * zpl_start(): the PPU serves tile uploads only until it is told to run, and the first frame uses whatever is there.
 * Maps and sprite cells are read as they are drawn, so change them only through zpl_map_set()/zpl_cell_set() when you accept a
 * glitch for a frame, or double buffer them yourself.
 *
 * Counts. Pieces: a sprite is cut into one piece per tile row per 8-line band it touches; at most ZPL_MAXP (15) pieces
 * fit in a band, and ZPL_MAX_PIECES (278) in a frame (the frame block is 4 KiB). What does not fit is dropped (the
 * pieces of the later sprites, last band first) and counted: zpl_dropped(). Up to ZPL_MAX_SPRITES sprites and ZPL_MAX_BOXES
 * window boxes. The library does not account for blitter time: at the shipped budget the piece cap is what binds
 * (docs/ppu/layers.md, "Never lag").
 */
#ifndef ZEROPOINT_ZP_LAYERS_H
#define ZEROPOINT_ZP_LAYERS_H

#include "zp_types.h"
#include "zp_layers_layout.h"
#ifndef ZPL_HOST_TEST           /* the host test builds only the stream builder: no console hardware */
#include "zp_mmio.h"
#include "zp_dma.h"
#include "zp_ppu.h"
#include "zp_timer.h"
#include "zp_cop.h"
#endif

/* ---- sizes and addresses ----------------------------------------------------------------------------------------- */
#ifndef ZPL_RAM
#define ZPL_RAM 0x4000          /* offset in bank $80 of the state of the library */
#endif
#ifndef ZPL_MAX_SPRITES
#define ZPL_MAX_SPRITES 64
#endif
#define ZPL_MAX_BOXES 8
#define ZPL_MAX_PIECES 278      /* (4096 - 8 - 32 x (2 + 4)) / 14: what a 4 KiB frame block holds, with every band extended */
#define ZPL_RAM_SIZE 0x1900

#ifndef ZPL_DMA_CHAN
#define ZPL_DMA_CHAN 0
#endif

/* sprite flags (low bits as in zp_layers_sprites.h) */
#ifndef ZPL_SPR_HFLIP
#define ZPL_SPR_HFLIP 0x01
#define ZPL_SPR_VFLIP 0x02
#define ZPL_SPR_PRIO_MASK 0x0C
#define ZPL_SPR_PRIO_MID  0x04   /* between BG0 and BG1 */
#define ZPL_SPR_PRIO_BACK 0x08   /* behind BG0 (shows only where BG0 is transparent: zpl_layer_transparent) */
#endif
#define ZPL_FLIP_X_FLAG 0x02     /* the flag bits of the queue port */
#define ZPL_FLIP_Y_FLAG 0x04

/* map / sprite cell attribute word: ZPL_ATTR(bank, hflip, vflip, transparent) */
#define ZPL_ATTR(bank, h, v, t)  (0x1000 | ((bank) & 3) | ((t) ? 0x04 : 0) | ((h) ? 0x08 : 0) | ((v) ? 0x10 : 0))

/* window boxes (docs/ppu/layers.md "Window layer") */
#ifndef ZPL_BOX_SOLID
#define ZPL_BOX_SOLID 1      /* opaque ink: the BG under it is not drawn */
#define ZPL_BOX_OVERLAY 2    /* ink with transparency: shows what is below */
#define ZPL_BOX_GLASS 3      /* a blended fill */
#define ZPL_GLASS_AVERAGE 1
#define ZPL_GLASS_SUBTRACT 2
#define ZPL_GLASS_ADD 3
#define ZPL_WIN_COLS 32
#endif

/* ---- library state (all in Work RAM at ZPL_RAM) --------------------------------------------------------------------- */
/* def88186cc takes one declarator per struct member, so no "short x, y;" below. */
struct zpl_spr { short x; short y; unsigned short w; unsigned short h; unsigned short cells; unsigned short flags; };   /* 12 bytes */
struct zpl_box { unsigned short kind; unsigned short col; unsigned short row; unsigned short w; unsigned short h; unsigned short cells; };   /* 12 bytes */
struct zpl_ctx {
    unsigned short scroll[4];        /* SCX0 SCY0 SCX1 SCY1 */
    unsigned short l0flags;          /* queue flag bits ORed into BG0 (ZPL_F_TRANSPARENT = $40) */
    unsigned short l1flags;          /* ... and into BG1 */
    unsigned short spren;
    unsigned short active;           /* PPU address of the frame block the PPU reads (or is about to) */
    unsigned short winactive;        /* PPU address of the window table in use, 0 = no window */
    unsigned short pending;          /* a flip is written and the PPU has not taken it yet */
    unsigned short started;
    unsigned short spr_hi;           /* one past the highest sprite slot ever set */
    unsigned short dropped;          /* pieces left out of the last frame */
    unsigned short pieces;           /* pieces admitted so far in this pass */
    unsigned short sdirty;           /* a sprite changed since the stream was built */
    unsigned short built;            /* the stream in RAM is valid */
    unsigned short stream_bytes;     /* length of the last stream, without the 8 scroll bytes */
    unsigned short glass_mode;
    unsigned short nbox;
    unsigned short wdirty;
    unsigned short pool_next;
    struct zpl_box box[ZPL_MAX_BOXES];
};

#define ZPL_O_SPR   0x0100       /* sprites: ZPL_MAX_SPRITES x 12 */
#define ZPL_O_CNT   0x0400       /* per band and group: pieces counted (32 x 3 words) */
#define ZPL_O_WPTR  0x04C0       /* per band and group: where the next piece goes (32 x 3 words) */
#define ZPL_O_TOTAL 0x0580       /* per band: pieces admitted (32 words) */
#define ZPL_O_WTAB  0x0600       /* the window table being built (ZPL_WIN_TABLE_BYTES) */
#define ZPL_O_BUF   0x0800       /* the frame block being built: 8 scroll bytes, then the stream (up to $1000 bytes) */
#define ZPL_O_SCR   0x1800       /* 256 bytes: a ROM chunk staged for a byte copy (zp_peek8 cannot read ROM above bank 0) */

/* A pointer into the state block. It is a function because zpcc cannot turn an integer CONSTANT into a pointer (it can from a
 * variable), and so that the host test can point it at an array. */
#ifdef ZPL_HOST_TEST
ZP_INLINE unsigned char* zpl_mem(int off) { return zpl_host_mem + off; }
#else
ZP_INLINE unsigned char* zpl_mem(int off) { return (unsigned char*)(ZPL_RAM + off); }
#endif
#define ZPL_MEM(off) zpl_mem(off)

/* The state and the sprite table as struct pointers. These are functions that cast a local VARIABLE: def88186cc crashes when a call
 * result or a constant is cast straight to a struct pointer, and refuses "f()->member", so callers do  c = zpl_ctxp(); c->member. */
ZP_INLINE struct zpl_ctx* zpl_ctxp(ZP_VOID)
{
    unsigned char* b;
    b = ZPL_MEM(0);
    return (struct zpl_ctx*)b;
}
ZP_INLINE struct zpl_spr* zpl_sprp(int i)
{
    unsigned char* b;
    struct zpl_spr* s;
    b = ZPL_MEM(ZPL_O_SPR);
    s = (struct zpl_spr*)b;
    return s + i;                       /* not "(struct zpl_spr*)b + i": def88186cc scales that by 2 bytes, not sizeof */
}
#define ZPL_OFS(p) ((unsigned short)((unsigned char*)(p) - ZPL_MEM(0)))     /* a pointer into the state block as an offset (it fits a word) */

ZP_INLINE void zpl_zero(unsigned char* p, int n)
{
    int i;
    for (i = 0; i < n; i = i + 1) p[i] = 0;
}

/* ================================================================================================================= *
 * The frame block builder (pure memory work; the same on the console and on a host, which is how it is tested)
 * ================================================================================================================= */

/* A pass is split into small functions: zpcc gives every temporary its own stack slot and a frame over 255 bytes is an error. */

/* Admits one piece of group g (0 above both layers, 1 between, 2 behind BG0) into `band`, or says no: a band holds at most
 * ZPL_MAXP pieces and a frame ZPL_MAX_PIECES. Counting (fill = 0) also tallies it. Returns band * 3 + g, or -1. */
ZP_INLINE int zpl_admit(int band, int g, int fill)
{
    struct zpl_ctx* c;
    unsigned short* total;
    unsigned short* cnt;
    c = zpl_ctxp();
    total = (unsigned short*)ZPL_MEM(ZPL_O_TOTAL);
    if ((int)total[band] >= ZPL_MAXP || (int)c->pieces >= ZPL_MAX_PIECES) return -1;
    total[band] = total[band] + 1;
    c->pieces = c->pieces + 1;
    if (fill == 0) {
        cnt = (unsigned short*)ZPL_MEM(ZPL_O_CNT);
        cnt[band * 3 + g] = cnt[band * 3 + g] + 1;
    }
    return band * 3 + g;
}

/* Where the next piece of band*3+g goes (and moves on): a pointer to 7 words. */
ZP_INLINE unsigned short* zpl_take(int idx)
{
    unsigned short* wptr;
    unsigned short at;
    wptr = (unsigned short*)ZPL_MEM(ZPL_O_WPTR);
    at = wptr[idx];
    wptr[idx] = (unsigned short)(at + 14);
    return (unsigned short*)(ZPL_MEM(0) + at);
}

/* One piece record, in the order of the queue port: src, count, x, y, clip0, clip1, flags. */
ZP_INLINE void zpl_put_piece(unsigned short* rec, struct zpl_spr* s, int srow, int top, int c0, int c1)
{
    int fl;
    fl = ZPL_TILE_MODE_FLAGS | ZPL_F_TRANSPARENT | ZPL_F_CLIP;
    if (s->flags & ZPL_SPR_HFLIP) fl = fl | ZPL_FLIP_X_FLAG;
    if (s->flags & ZPL_SPR_VFLIP) fl = fl | ZPL_FLIP_Y_FLAG;
    rec[0] = (unsigned short)(ZPL_SPRCELLS + 4 * (int)s->cells + 4 * (int)s->w * srow);
    rec[1] = s->w;
    rec[2] = (unsigned short)s->x;
    rec[3] = (unsigned short)top;                    /* the top of the tile row */
    rec[4] = (unsigned short)c0;
    rec[5] = (unsigned short)c1;
    rec[6] = (unsigned short)fl;
}

/* The pieces of tile row ty of sprite s: one per 8-line band the row touches, clipped to the band. Returns how many were dropped. */
ZP_INLINE int zpl_tile_row(struct zpl_spr* s, int g, int ty, int fill)
{
    int top, srow, first, last, band, c0, c1, idx, dropped;
    dropped = 0;
    top = (int)s->y + 8 * ty;
    if (top + 8 <= 0 || top >= 256) return 0;                     /* this tile row is off the top or bottom */
    srow = ty;
    if (s->flags & ZPL_SPR_VFLIP) srow = (int)s->h - 1 - ty;
    first = 0;
    if (top > 0) first = top >> 3;
    last = 0;
    if (top + 7 > 0) last = (top + 7) >> 3;
    if (last > ZPL_BANDS - 1) last = ZPL_BANDS - 1;
    for (band = first; band <= last; band = band + 1) {
        c0 = 8 * band;
        if (top > c0) c0 = top;
        c1 = 8 * band + 8;
        if (top + 8 < c1) c1 = top + 8;
        idx = zpl_admit(band, g, fill);
        if (idx < 0) { dropped = dropped + 1; continue; }
        if (fill) zpl_put_piece(zpl_take(idx), s, srow, top, c0, c1);
    }
    return dropped;
}

/* All the pieces of one sprite. Returns how many were dropped. */
ZP_INLINE int zpl_sprite_pieces(struct zpl_spr* s, int fill)
{
    int g, ty, dropped;
    if (s->w == 0 || s->h == 0) return 0;
    if ((int)s->x + 8 * (int)s->w <= 0 || (int)s->x >= 256) return 0;       /* wholly off to the left or right */
    g = 0;
    if ((s->flags & ZPL_SPR_PRIO_MASK) == ZPL_SPR_PRIO_MID) g = 1;
    if ((s->flags & ZPL_SPR_PRIO_MASK) == ZPL_SPR_PRIO_BACK) g = 2;
    dropped = 0;
    for (ty = 0; ty < (int)s->h; ty = ty + 1) dropped = dropped + zpl_tile_row(s, g, ty, fill);
    return dropped;
}

/* One pass over the sprites in array order. fill = 0 counts pieces per band and group; fill = 1 writes each piece where the
 * layout put it. Both passes admit exactly the same pieces. Returns the pieces dropped. */
ZP_INLINE int zpl_pass(int fill)
{
    struct zpl_spr* sprs;
    struct zpl_ctx* c;
    unsigned short* total;
    int i, hi, dropped;
    total = (unsigned short*)ZPL_MEM(ZPL_O_TOTAL);
    for (i = 0; i < ZPL_BANDS; i = i + 1) total[i] = 0;
    c = zpl_ctxp();
    c->pieces = 0;
    sprs = zpl_sprp(0);
    hi = (int)c->spr_hi;
    dropped = 0;
    for (i = 0; i < hi; i = i + 1) dropped = dropped + zpl_sprite_pieces(sprs + i, fill);
    return dropped;
}

/* Lays the stream out from the counts: per band a header word (the pieces above both layers, plus ZPL_EXT_FLAG when there are
 * also pieces behind BG0 or between the layers), then for an extended band [behind count][behind pieces][between count][between
 * pieces], then the above pieces; and sets where each group is written. Returns the length of the stream in bytes. */
ZP_INLINE int zpl_layout(void)
{
    unsigned short* cnt;
    unsigned short* wptr;
    unsigned short* w;
    unsigned char* start;
    unsigned char* pos;
    int b, nb, nm, na;
    cnt = (unsigned short*)ZPL_MEM(ZPL_O_CNT);
    wptr = (unsigned short*)ZPL_MEM(ZPL_O_WPTR);
    start = ZPL_MEM(ZPL_O_BUF) + ZPL_STREAM_OFF;
    pos = start;
    for (b = 0; b < ZPL_BANDS; b = b + 1) {
        na = (int)cnt[b * 3];
        nm = (int)cnt[b * 3 + 1];
        nb = (int)cnt[b * 3 + 2];
        w = (unsigned short*)pos;
        pos = pos + 2;
        if (nb + nm > 0) {
            w[0] = (unsigned short)(na | ZPL_EXT_FLAG);
            *(unsigned short*)pos = (unsigned short)nb;
            pos = pos + 2;
            wptr[b * 3 + 2] = ZPL_OFS(pos);
            pos = pos + 14 * nb;
            *(unsigned short*)pos = (unsigned short)nm;
            pos = pos + 2;
            wptr[b * 3 + 1] = ZPL_OFS(pos);
            pos = pos + 14 * nm;
        } else {
            w[0] = (unsigned short)na;
        }
        wptr[b * 3] = ZPL_OFS(pos);
        pos = pos + 14 * na;
    }
    return (int)(pos - start);
}

/* Builds the frame block in RAM: the four scroll words, then the packed sprite stream (docs/ppu/layers.md "Sprite priority").
 * Returns the number of pieces dropped (also in zpl_dropped()); the length of the stream is in the state (stream_bytes). The stream is only
 * rebuilt when a sprite changed since the last call; the scroll words are refreshed every time. */
ZP_INLINE int zpl_build_block(ZP_VOID)
{
    struct zpl_ctx* c;
    unsigned short* w;
    int dropped, k;

    c = zpl_ctxp();
    w = (unsigned short*)ZPL_MEM(ZPL_O_BUF);
    for (k = 0; k < 4; k = k + 1) w[k] = c->scroll[k];
    if (c->built && !c->sdirty) return (int)c->dropped;      /* no sprite changed: the stream in RAM is still right, only the scroll moved */
    zpl_zero(ZPL_MEM(ZPL_O_CNT), ZPL_BANDS * 3 * 2);
    dropped = zpl_pass(0);
    c->stream_bytes = (unsigned short)zpl_layout();
    zpl_pass(1);
    c->dropped = (unsigned short)dropped;
    c->sdirty = 0;
    c->built = 1;
    return dropped;
}

/* The width a box spans on the 32-cell grid (clipped at the right edge). */
ZP_INLINE int zpl_box_width(struct zpl_box* x)
{
    if ((int)x->col + (int)x->w > ZPL_WIN_COLS) return ZPL_WIN_COLS - (int)x->col;
    return (int)x->w;
}

/* One band's record in the window table: ink count, ink column, ink cells, ink flags, glass count, glass column (12 bytes).
 * The first ink box and the first glass box (array order) that cover the band win. */
ZP_INLINE void zpl_window_band(unsigned char* t, int b)
{
    struct zpl_ctx* c;
    struct zpl_box* x;
    unsigned short* rec;
    int i, ink, glass;

    c = zpl_ctxp();
    rec = (unsigned short*)(t + ZPL_WIN_HDR_BYTES + b * ZPL_WIN_REC_BYTES);
    ink = -1;
    glass = -1;
    for (i = 0; i < (int)c->nbox; i = i + 1) {
        x = c->box + i;
        if (x->w == 0 || x->h == 0 || x->col >= ZPL_WIN_COLS || b < (int)x->row || b >= (int)x->row + (int)x->h) continue;
        if (x->kind == ZPL_BOX_GLASS) { if (glass < 0) glass = i; }
        else { if (ink < 0) ink = i; }
    }
    if (ink >= 0) {
        x = c->box + ink;
        rec[0] = (unsigned short)zpl_box_width(x);
        rec[1] = x->col;
        rec[2] = (unsigned short)((int)x->cells + 4 * (int)x->w * (b - (int)x->row));
        rec[3] = x->kind == ZPL_BOX_OVERLAY ? ZPL_F_TRANSPARENT : 0;
    }
    if (glass >= 0) {
        x = c->box + glass;
        rec[4] = (unsigned short)zpl_box_width(x);
        rec[5] = x->col;
    }
}

/* Builds the window table (a header of the glass mode and a reserved word, then one record per band) at `t`. */
ZP_INLINE void zpl_build_window(unsigned char* t)
{
    struct zpl_ctx* c;
    int b;
    c = zpl_ctxp();
    zpl_zero(t, ZPL_WIN_TABLE_BYTES);
    *(unsigned short*)t = c->glass_mode;
    for (b = 0; b < ZPL_BANDS; b = b + 1) zpl_window_band(t, b);
}

/* ---- state setters that touch only RAM (usable before and after the hardware is up) ---------------------------------- */

/* Sprite slot i (0 .. ZPL_MAX_SPRITES-1): top-left x, y on screen (signed), w x h in 8x8 tiles, the index of its first
 * sprite cell (cells are row-major, w*h of them), flags = ZPL_SPR_HFLIP | ZPL_SPR_VFLIP | one ZPL_SPR_PRIO_*.
 * Later slots draw over earlier ones within a priority. */
ZP_INLINE void zpl_sprite_set(int i, int x, int y, int w, int h, int cells, int flags)
{
    struct zpl_spr* s;
    struct zpl_ctx* c;
    if (i < 0 || i >= ZPL_MAX_SPRITES) return;
    s = zpl_sprp(i);
    s->x = (short)x; s->y = (short)y;
    s->w = (unsigned short)w; s->h = (unsigned short)h;
    s->cells = (unsigned short)cells; s->flags = (unsigned short)flags;
    c = zpl_ctxp();
    c->sdirty = 1;
    if (i >= (int)c->spr_hi) c->spr_hi = (unsigned short)(i + 1);
}
ZP_INLINE void zpl_sprite_move(int i, int x, int y)
{
    struct zpl_spr* s;
    struct zpl_ctx* c;
    if (i < 0 || i >= ZPL_MAX_SPRITES) return;
    s = zpl_sprp(i);
    c = zpl_ctxp();
    c->sdirty = 1;
    s->x = (short)x;
    s->y = (short)y;
}
ZP_INLINE void zpl_sprite_flags(int i, int flags)
{
    struct zpl_spr* s;
    struct zpl_ctx* c;
    if (i < 0 || i >= ZPL_MAX_SPRITES) return;
    s = zpl_sprp(i);
    c = zpl_ctxp();
    c->sdirty = 1;
    s->flags = (unsigned short)flags;
}
ZP_INLINE void zpl_sprite_hide(int i)
{
    struct zpl_spr* s;
    struct zpl_ctx* c;
    if (i < 0 || i >= ZPL_MAX_SPRITES) return;
    s = zpl_sprp(i);
    c = zpl_ctxp();
    c->sdirty = 1;
    s->w = 0;
}
ZP_INLINE void zpl_sprites_clear(ZP_VOID)
{
    struct zpl_ctx* c;
    zpl_zero(ZPL_MEM(ZPL_O_SPR), ZPL_MAX_SPRITES * 12);
    c = zpl_ctxp();
    c->spr_hi = 0;
    c->sdirty = 1;
}

/* Layer scroll (layer 0 = BG0, 1 = BG1), any value, positive or negative: the maps are 64x64 cells = 512x512 pixels and wrap, and the
 * microcode wants 0..511, so it is wrapped here. */
ZP_INLINE void zpl_scroll(int layer, int x, int y)
{
    struct zpl_ctx* c;
    c = zpl_ctxp();
    c->scroll[layer * 2] = (unsigned short)(x & 511);
    c->scroll[layer * 2 + 1] = (unsigned short)(y & 511);
}

/* Whether each BG layer lets what is below show through its alpha-0 pixels (BG0 opaque and BG1 transparent by default).
 * BG0 must be transparent for a ZPL_SPR_PRIO_BACK sprite to be visible. Takes effect at the next frame. */
ZP_INLINE void zpl_layer_transparent(int bg0, int bg1)
{
    struct zpl_ctx* c;
    c = zpl_ctxp();
    c->l0flags = bg0 ? ZPL_F_TRANSPARENT : 0;
    c->l1flags = bg1 ? ZPL_F_TRANSPARENT : 0;
}
ZP_INLINE void zpl_sprites_enable(int on)
{
    struct zpl_ctx* c;
    c = zpl_ctxp();
    c->spren = on ? 1 : 0;
}

/* Window layer boxes. kind = ZPL_BOX_SOLID / ZPL_BOX_OVERLAY / ZPL_BOX_GLASS, col/row/w/h on the 32 x 32 cell grid,
 * `cells` = PPU address of w*h cells (from zpl_window_cells_load(); ignored for glass). One ink and one glass box per band. */
ZP_INLINE int zpl_window_box(int kind, int col, int row, int w, int h, int cells)
{
    struct zpl_ctx* c;
    struct zpl_box* x;
    c = zpl_ctxp();
    if ((int)c->nbox >= ZPL_MAX_BOXES) return -1;
    x = c->box + c->nbox;
    x->kind = (unsigned short)kind; x->col = (unsigned short)col; x->row = (unsigned short)row;
    x->w = (unsigned short)w; x->h = (unsigned short)h; x->cells = (unsigned short)cells;
    c->nbox = c->nbox + 1;
    c->wdirty = 1;
    return (int)c->nbox - 1;
}
ZP_INLINE void zpl_window_clear(ZP_VOID)
{
    struct zpl_ctx* c;
    c = zpl_ctxp();
    c->nbox = 0;
    c->wdirty = 1;
}
ZP_INLINE void zpl_window_glass_mode(int mode)    /* ZPL_GLASS_AVERAGE / SUBTRACT / ADD */
{
    struct zpl_ctx* c;
    c = zpl_ctxp();
    c->glass_mode = (unsigned short)mode;
    c->wdirty = 1;
}

/* Frames the library has dropped pieces from: how many pieces the last zpl_frame() left out (0 = everything drawn). */
ZP_INLINE int zpl_dropped(ZP_VOID)
{
    struct zpl_ctx* c;
    c = zpl_ctxp();
    return (int)c->dropped;
}

#ifndef ZPL_HOST_TEST
/* ================================================================================================================= *
 * Hardware: loading the microcode and assets, and the per-frame handshake with the PPU
 * ================================================================================================================= */

/* CPU bus address of a PPU memory offset. The offset goes through unsigned int: an int is 16 bits and signed, so $8000 and up would
 * sign-extend to a bank below $B0 (this is where the second BG map lives). */
#define ZPL_PPU(off) (ZP_PPU_WIN_BASE + (zpaddr)(unsigned int)(off))

ZP_INLINE void zpl_pw(unsigned int off, int v) { zp_poke16(ZPL_PPU(off), (zpu16)v); }
ZP_INLINE int zpl_pr(unsigned int off) { return (int)zp_peek16(ZPL_PPU(off)); }

/* Copies len bytes from ROM or RAM `src` to PPU memory `off`. A multiple of 256 bytes to a 256-aligned offset goes by DMA straight
 * through. Anything else is copied in 256-byte chunks: the chunk is DMAed into RAM first (zp_peek8 can only read ROM bank 0, and DMA moves
 * whole 256-byte units so it cannot be pointed at a smaller PPU target without spilling over what follows), then written byte by byte. */
ZP_INLINE void zpl_copy_to_ppu(zpaddr src, unsigned int off, int len)
{
    int i, n, done;
    zpaddr stage;
    if ((len & 255) == 0 && (off & 255) == 0) {
        zp_dma_load(src, ZPL_PPU(off), (zpu32)len, ZPL_DMA_CHAN);
        return;
    }
    stage = ZP_RAM_BASE + (zpaddr)(ZPL_RAM + ZPL_O_SCR);
    done = 0;
    while (done < len) {
        n = len - done;
        if (n > 256) n = 256;
        zp_dma_load(src + (zpaddr)done, stage, 256UL, ZPL_DMA_CHAN);
        for (i = 0; i < n; i = i + 1) zp_poke8(ZPL_PPU(off + done + i), zp_peek8(stage + (zpaddr)i));
        done = done + n;
    }
}

/* Boots the layers microcode and resets the library. `pages` * 256 bytes of microcode (ucode/layers/build/layers.bin, padded to a
 * multiple of 256) must be in ROM at bank:offset; zplink places it where you say. Needs the PPU loader of the Boot ROM (COP #$F7),
 * which the native boot and the real Boot ROM both have. Defaults: BG0 opaque, BG1 transparent, sprites on, no window. */
ZP_INLINE void zpl_init(int bank, int offset, int pages)
{
    struct zpl_ctx* c;
    zpl_zero(ZPL_MEM(0), ZPL_RAM_SIZE);
    c = zpl_ctxp();
    c->l0flags = 0;
    c->l1flags = ZPL_F_TRANSPARENT;
    c->spren = 1;
    c->active = ZPL_LISTS;
    c->glass_mode = ZPL_GLASS_AVERAGE;
    c->pool_next = ZPL_WINCELLS;
    zp_boot_ppu_load((zpu8)bank, (zpu16)offset, (zpu16)pages);
    zp_dma_fill(ZPL_PPU(ZPL_LISTS), 0, 4096UL, ZPL_DMA_CHAN);     /* both frame blocks empty: every band header 0, scroll 0 */
    zp_dma_fill(ZPL_PPU(ZPL_LISTS2), 0, 4096UL, ZPL_DMA_CHAN);
    zpl_pw(ZPL_CTRL_NCOLS, 33);
    zpl_pw(ZPL_CTRL_SPREN, 1);
    zpl_pw(ZPL_CTRL_L0FLAGS, 0);
    zpl_pw(ZPL_CTRL_L1FLAGS, ZPL_F_TRANSPARENT);
    zpl_pw(ZPL_CTRL_LISTBASE, ZPL_LISTS);
    zpl_pw(ZPL_CTRL_WINBASE, 0);
}

/* 256 colours, 16-bit BBBBBGGGGGRRRRR1 (bit 0 = alpha: palette entries 0, 16, 32.. conventionally clear for see-through). */
ZP_INLINE void zpl_palette_load(zpaddr src) { zpl_copy_to_ppu(src, ZPL_PAL, 512); }
ZP_INLINE void zpl_palette_set(int index, int color16) { zpl_pw(ZPL_PAL + 2 * index, color16); }

/* Tile pixels: 8 x 8, one byte per pixel (a palette index), 64 bytes. There are 4 banks of 256 tile ids. Before zpl_start() only. */
ZP_INLINE void zpl_tile_load(int bank, int id, zpaddr src)
{
    int i;
    struct zpl_ctx* c;
    c = zpl_ctxp();
    if (c->started) return;
    zpl_pw(ZPL_CTRL_TILEBANK, bank);
    zpl_pw(ZPL_CTRL_TILEID, id);
    zp_dma_load(src, ZP_RAM_BASE + (zpaddr)(ZPL_RAM + ZPL_O_SCR), 256UL, ZPL_DMA_CHAN);     /* the 64 bytes, staged in RAM */
    zpl_pw(ZPL_CTRL_TILEREQ, 1);
    while (zpl_pr(ZPL_CTRL_TILEREQ) != 2) { }
    for (i = 0; i < 64; i = i + 1) zp_poke8(ZPL_PPU(0x0300 + i), zp_peek8(ZP_RAM_BASE + (zpaddr)(ZPL_RAM + ZPL_O_SCR + i)));
    zpl_pw(ZPL_CTRL_TILEREQ, 3);
    while (zpl_pr(ZPL_CTRL_TILEREQ) != 0) { }
}
/* `count` tiles, 64 bytes each from src, as ids first_id, first_id+1, ... of one bank. */
ZP_INLINE void zpl_tiles_load(int bank, int first_id, zpaddr src, int count)
{
    int i;
    for (i = 0; i < count; i = i + 1) zpl_tile_load(bank, first_id + i, src + (zpaddr)(64 * i));
}

/* Map layer 0 / 1: 64 x 64 cells of 4 bytes (a tile-id word, then ZPL_ATTR(...)), row-major, 16 KiB from src. */
ZP_INLINE void zpl_map_load(int layer, zpaddr src) { zpl_copy_to_ppu(src, layer ? ZPL_MAP1 : ZPL_MAP0, 16384); }
ZP_INLINE void zpl_map_set(int layer, int x, int y, int tile, int attr)
{
    unsigned int off;
    off = (unsigned int)(layer ? ZPL_MAP1 : ZPL_MAP0) + 4u * (unsigned int)(y * 64 + x);
    zpl_pw(off, tile);
    zpl_pw(off + 2, attr);
}

/* Sprite cells (4 bytes each like the cells of a map, room for 256): `count` cells from src into cell slots first.. */
ZP_INLINE void zpl_cells_load(int first, zpaddr src, int count) { zpl_copy_to_ppu(src, ZPL_SPRCELLS + 4 * first, 4 * count); }
ZP_INLINE void zpl_cell_set(int index, int tile, int attr)
{
    zpl_pw(ZPL_SPRCELLS + 4 * index, tile);
    zpl_pw(ZPL_SPRCELLS + 4 * index + 2, attr);
}

/* Window cells: `count` cells from src into the window cell pool (408 cells in all); returns their PPU address for
 * zpl_window_box(), or -1 if the pool is full. Glass boxes need no cells but a fill: zpl_window_glass_fill(). */
ZP_INLINE int zpl_window_cells_load(zpaddr src, int count)
{
    int at;
    struct zpl_ctx* c;
    c = zpl_ctxp();
    at = (int)c->pool_next;
    if (at + 4 * count > ZPL_WINCELLS_END) return -1;
    zpl_copy_to_ppu(src, at, 4 * count);
    c->pool_next = (unsigned short)(at + 4 * count);
    return at;
}
/* The one cell every glass span is filled with (usually a solid tile in the colour to blend). */
ZP_INLINE void zpl_window_glass_fill(int tile, int attr)
{
    int i;
    for (i = 0; i < ZPL_WIN_COLS; i = i + 1) {
        zpl_pw(ZPL_GLFILL + 4 * i, tile);
        zpl_pw(ZPL_GLFILL + 4 * i + 2, attr);
    }
}

/* Starts the PPU producing frames, and the 60 Hz timer zpl_frame() paces on. Load everything else first. */
ZP_INLINE void zpl_start(ZP_VOID)
{
    struct zpl_ctx* c;
    c = zpl_ctxp();
    zpl_pw(ZPL_CTRL_RUN, 1);
    c->started = 1;
    zp_timer_enable(ZP_TMR_VBLANK);
}

/* Builds the sprite stream of this frame, sends it and the scroll words to the idle frame block of the PPU, and flips to it (and the
 * window table, if the boxes changed). Waits first if the PPU has not yet taken the previous flip. Returns the pieces dropped. */
ZP_INLINE int zpl_present(ZP_VOID)
{
    struct zpl_ctx* c;
    int idle, widle, dropped;

    c = zpl_ctxp();
    if (c->pending) {
        while (zpl_pr(ZPL_CTRL_FRAMES) == 0) { }                  /* the PPU counts a frame when it snapshots LISTBASE */
        c->pending = 0;
    }
    dropped = zpl_build_block();
    idle = c->active == ZPL_LISTS ? ZPL_LISTS2 : ZPL_LISTS;
    zp_dma_load(ZP_RAM_BASE + (zpaddr)(ZPL_RAM + ZPL_O_BUF), ZPL_PPU(idle), (zpu32)(ZPL_STREAM_OFF + c->stream_bytes), ZPL_DMA_CHAN);
    if (c->wdirty) {
        zpl_build_window(ZPL_MEM(ZPL_O_WTAB));
        widle = c->winactive == ZPL_WIN0 ? ZPL_WIN1 : ZPL_WIN0;
        zpl_copy_to_ppu(ZP_RAM_BASE + (zpaddr)(ZPL_RAM + ZPL_O_WTAB), widle, ZPL_WIN_TABLE_BYTES);
        c->winactive = c->nbox ? (unsigned short)widle : 0;
        zpl_pw(ZPL_CTRL_WINBASE, c->winactive);
        c->wdirty = 0;
    }
    zpl_pw(ZPL_CTRL_NCOLS, 33);
    zpl_pw(ZPL_CTRL_SPREN, c->spren);
    zpl_pw(ZPL_CTRL_L0FLAGS, c->l0flags);
    zpl_pw(ZPL_CTRL_L1FLAGS, c->l1flags);
    /* Flip first, then clear the counter. The PPU adds one to FRAMES when it snapshots LISTBASE: cleared before the flip, a
     * snapshot in between would count the OLD block and we would wrongly think the new one taken; cleared after, the worst case
     * is one extra frame of waiting. */
    zpl_pw(ZPL_CTRL_LISTBASE, idle);
    zpl_pw(ZPL_CTRL_FRAMES, 0);
    c->active = (unsigned short)idle;
    c->pending = 1;
    return dropped;
}

/* Waits for the 60 Hz tick (zpl_start() enabled the timer). */
ZP_INLINE void zpl_vsync(ZP_VOID) { zp_timer_wait(ZP_TMR_VBLANK); }

/* The usual end of a game loop: wait for the 60 Hz tick, then present. One call per frame gives a steady 60 fps. */
ZP_INLINE int zpl_frame(ZP_VOID)
{
    zpl_vsync();
    return zpl_present();
}
#endif /* !ZPL_HOST_TEST */

#endif /* ZEROPOINT_ZP_LAYERS_H */

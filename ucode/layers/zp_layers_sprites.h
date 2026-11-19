/* zp_layers_sprites.h - CPU side of the sprite half of the two-layer microcode (docs/ppu/layers.md). C89, header only.
 *
 * The PPU microcode never scans an OAM: it replays one short list of "pieces" per 8-row band. This turns sprites into
 * those lists. A sprite is w x h tiles whose cells (4 bytes each: tile word, bank|attrs|$1000 word, row-major) the caller
 * has put in PPU RAM at `cells`. Each tile row of a sprite is cut into one piece per band its 8 rows touch, clipped to
 * that band, so a sprite's pieces sit in the same band as the BG rows they must go over (queue order is z-order, and BG0,
 * BG1 and then the pieces are launched band by band). Sprites are emitted in array order: later = on top.
 *
 * `lists` is the bytes to store at ZPL_LISTS in PPU RAM: ZPL_BANDS x ZPL_LIST_STRIDE. Per band: a word count, then up to
 * ZPL_MAXP records of 7 words in the queue port's order (src, count, x, y, clip0, clip1, flags). A band that is full drops
 * further pieces (the admission control: the blitter's time per band is fixed, so what does not fit is not drawn). Full means
 * ZPL_MAXP pieces, or ZPL_BAND_BUDGET channel-cycles of blit time (the real limit: a 4-cell piece costs 4x a 1-cell one);
 * `dropped` counts what was left out. x/y/clip words are little-endian; y and the clips are frame rows (the microcode adds 0 or 256 to put them in
 * the frame the hardware is on).
 */
#ifndef ZP_LAYERS_SPRITES_H
#define ZP_LAYERS_SPRITES_H

#include "zp_layers_layout.h"

#if defined(__GNUC__)
#define ZPL_UNUSED __attribute__((unused))
#else
#define ZPL_UNUSED
#endif

#define ZPL_SPR_HFLIP 0x01
#define ZPL_SPR_VFLIP 0x02
#define ZPL_FLAG_FLIPX 0x02   /* queue port flag bits */
#define ZPL_FLAG_FLIPY 0x04

typedef struct {
    short x, y;               /* top-left of the sprite on screen, signed */
    unsigned short w, h;      /* in 8x8 tiles */
    unsigned short cells;     /* PPU address of the w*h cells */
    unsigned short flags;     /* ZPL_SPR_* */
} ZPL_Sprite;

static ZPL_UNUSED void zpl_put16(unsigned char* p, unsigned v) { p[0] = (unsigned char)(v & 0xFF); p[1] = (unsigned char)((v >> 8) & 0xFF); }

/* returns the number of pieces dropped for lack of room; budget[b] is the blitter time band b may spend on sprites (the window layer takes its own
 * out of it, zp_layers_window.h) */
static ZPL_UNUSED unsigned zpl_build_lists_bands(const ZPL_Sprite* spr, unsigned n, unsigned char lists[ZPL_BANDS][ZPL_LIST_STRIDE], const unsigned long budget[ZPL_BANDS]) {
    unsigned dropped = 0, i, ty, b, k;
    unsigned counts[ZPL_BANDS];
    unsigned long costs[ZPL_BANDS];   /* blitter channel-cycles queued per band: a clipped tile costs 8 + 32 per row it writes */
    for (b = 0; b < ZPL_BANDS; b++) {
        counts[b] = 0;
        costs[b] = 0;
        for (k = 0; k < ZPL_LIST_STRIDE; k++) lists[b][k] = 0;
    }
    for (i = 0; i < n; i++) {
        const ZPL_Sprite* s = &spr[i];
        if (s->w == 0 || s->h == 0) continue;
        if ((long)s->x + 8L * s->w <= 0 || (long)s->x >= 256) continue;       /* wholly off to the left or right */
        for (ty = 0; ty < s->h; ty++) {
            long top = (long)s->y + 8L * (long)ty;
            long first, last, band;
            unsigned srcRow = (s->flags & ZPL_SPR_VFLIP) ? s->h - 1 - ty : ty;
            if (top + 8 <= 0 || top >= 256) continue;                             /* this tile row is off the top or bottom */
            first = top < 0 ? 0 : top / 8;
            last = (top + 7) / 8;
            if (last > ZPL_BANDS - 1) last = ZPL_BANDS - 1;
            for (band = first; band <= last; band++) {
                long c0 = top > 8 * band ? top : 8 * band;
                long c1 = top + 8 < 8 * band + 8 ? top + 8 : 8 * band + 8;
                unsigned char* rec;
                unsigned long cost = (unsigned long)s->w * (8UL + 32UL * (unsigned long)(c1 - c0));
                if (counts[band] >= ZPL_MAXP || costs[band] + cost > budget[band]) { dropped++; continue; }
                costs[band] += cost;
                rec = &lists[band][2 + counts[band] * 2 * ZPL_LIST_RECORD_WORDS];
                zpl_put16(rec + 0, s->cells + 4u * s->w * srcRow);               /* src */
                zpl_put16(rec + 2, s->w);                                        /* count */
                zpl_put16(rec + 4, (unsigned)s->x);                              /* x */
                zpl_put16(rec + 6, (unsigned)top);                               /* y: the tile's top row */
                zpl_put16(rec + 8, (unsigned)c0);                                /* clip0 */
                zpl_put16(rec + 10, (unsigned)c1);                               /* clip1 */
                zpl_put16(rec + 12, ZPL_TILE_MODE_FLAGS | ZPL_F_TRANSPARENT | ZPL_F_CLIP |
                                    ((s->flags & ZPL_SPR_HFLIP) ? ZPL_FLAG_FLIPX : 0) |
                                    ((s->flags & ZPL_SPR_VFLIP) ? ZPL_FLAG_FLIPY : 0));
                counts[band]++;
            }
        }
    }
    for (b = 0; b < ZPL_BANDS; b++) zpl_put16(&lists[b][0], counts[b]);
    return dropped;
}

/* The PPU reads a packed stream, not the dense array: per band a word count, then count records of 14 bytes, bands back to back, so an
 * empty band costs two bytes of ROM. Writes it to out[0..cap) and returns its length, or 0 if it does not fit. A frame block is the four
 * scroll words (ZPL_SCROLL_OFF) then this stream at ZPL_STREAM_OFF. */
static ZPL_UNUSED unsigned zpl_pack_stream(unsigned char lists[ZPL_BANDS][ZPL_LIST_STRIDE], unsigned char* out, unsigned cap) {
    unsigned b, n = 0, i, count, bytes;
    for (b = 0; b < ZPL_BANDS; b++) {
        count = lists[b][0] | ((unsigned)lists[b][1] << 8);
        bytes = 2 + count * 2 * ZPL_LIST_RECORD_WORDS;
        if (n + bytes > cap) return 0;
        for (i = 0; i < bytes; i++) out[n + i] = lists[b][i];
        n += bytes;
    }
    return n;
}

static ZPL_UNUSED unsigned zpl_build_lists_budget(const ZPL_Sprite* spr, unsigned n, unsigned char lists[ZPL_BANDS][ZPL_LIST_STRIDE], unsigned long budget) {
    unsigned long perBand[ZPL_BANDS];
    unsigned b;
    for (b = 0; b < ZPL_BANDS; b++) perBand[b] = budget;
    return zpl_build_lists_bands(spr, n, lists, perBand);
}

static ZPL_UNUSED unsigned zpl_build_lists(const ZPL_Sprite* spr, unsigned n, unsigned char lists[ZPL_BANDS][ZPL_LIST_STRIDE]) {
    return zpl_build_lists_budget(spr, n, lists, ZPL_BAND_BUDGET);
}

#endif

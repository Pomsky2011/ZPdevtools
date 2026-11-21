/* zp_layers_sprites.h - CPU side of the sprite half of the two-layer microcode (docs/ppu/layers.md). C89, header only.
 *
 * The PPU microcode never scans an OAM: it replays one short list of "pieces" per 8-row band. This turns sprites into
 * those lists. A sprite is w x h tiles whose cells (4 bytes each: tile word, bank|attrs|$1000 word, row-major) the caller
 * has put in PPU RAM at `cells`. Each tile row of a sprite is cut into one piece per band its 8 rows touch, clipped to
 * that band, so a sprite's pieces sit in the same band as the BG rows they must go over (queue order is z-order, and the
 * pieces and the two BG layers are launched band by band). Sprites are emitted in array order: later = on top, within one priority.
 * Priority (flags bits 2-3): 0 above both layers (the default), ZPL_SPR_PRIO_MID between BG0 and BG1, ZPL_SPR_PRIO_BACK behind BG0
 * (it only shows where BG0 is transparent: ZPL_CTRL_L0FLAGS = ZPL_F_TRANSPARENT). The window layer stays above all of them.
 *
 * `lists` is the bytes to store at ZPL_LISTS in PPU RAM: ZPL_BANDS x ZPL_LIST_STRIDE. Per band: a word count, then up to
 * ZPL_MAXP records of 7 words in the queue port's order (src, count, x, y, clip0, clip1, flags), behind-BG0 pieces first, then between, then above.
 * The header word is the number of pieces above BG1, plus ZPL_EXT_FLAG when the band has pieces behind BG0 or between the layers; their two counts
 * sit at ZPL_EXT_OFF in the band's row (behind, between). A band that is full drops
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
#define ZPL_SPR_PRIO_MASK 0x0C   /* 0 = above BG1 (default), then: */
#define ZPL_SPR_PRIO_MID  0x04   /* between BG0 and BG1 */
#define ZPL_SPR_PRIO_BACK 0x08   /* behind BG0 */
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
    unsigned char prio[ZPL_BANDS][ZPL_MAXP];   /* per piece: 0 above, 1 between, 2 behind (the header nibble it counts in) */
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
                prio[band][counts[band]] = (s->flags & ZPL_SPR_PRIO_MASK) == ZPL_SPR_PRIO_BACK ? 2 : (s->flags & ZPL_SPR_PRIO_MASK) == ZPL_SPR_PRIO_MID ? 1 : 0;
                counts[band]++;
            }
        }
    }
    for (b = 0; b < ZPL_BANDS; b++) {   /* behind BG0 first, then between, then above: a stable sort of the band's records into launch order */
        unsigned char tmp[2 * ZPL_LIST_RECORD_WORDS * ZPL_MAXP];
        unsigned group, j, out = 0, n[3];
        n[0] = n[1] = n[2] = 0;
        for (j = 0; j < counts[b]; j++) n[prio[b][j]]++;
        for (k = 0; k < counts[b] * 2 * ZPL_LIST_RECORD_WORDS; k++) tmp[k] = lists[b][2 + k];
        for (group = 3; group-- > 0;)
            for (j = 0; j < counts[b]; j++)
                if (prio[b][j] == group) {
                    for (k = 0; k < 2 * ZPL_LIST_RECORD_WORDS; k++) lists[b][2 + out * 2 * ZPL_LIST_RECORD_WORDS + k] = tmp[j * 2 * ZPL_LIST_RECORD_WORDS + k];
                    out++;
                }
        zpl_put16(&lists[b][0], n[0] | ((n[1] + n[2]) ? ZPL_EXT_FLAG : 0));
        zpl_put16(&lists[b][ZPL_EXT_OFF], n[2]);
        zpl_put16(&lists[b][ZPL_EXT_OFF + 2], n[1]);
    }
    return dropped;
}

/* The PPU reads a packed stream, not the dense array: per band the header word, then (EXT bands only) the behind count and the behind
 * records, the between count and the between records, then the above records, 14 bytes each; bands back to back, so a band with nothing
 * costs two bytes of ROM. Writes it to out[0..cap) and returns its length, or 0 if it does not fit. A frame block is the four scroll words
 * (ZPL_SCROLL_OFF) then this stream at ZPL_STREAM_OFF. */
static ZPL_UNUSED unsigned zpl_pack_stream(unsigned char lists[ZPL_BANDS][ZPL_LIST_STRIDE], unsigned char* out, unsigned cap) {
    unsigned b, n = 0, i, hdr, nb, nm, na, rec = 2 * ZPL_LIST_RECORD_WORDS;
    for (b = 0; b < ZPL_BANDS; b++) {
        const unsigned char* row = lists[b];
        hdr = row[0] | ((unsigned)row[1] << 8);
        na = hdr & (ZPL_EXT_FLAG - 1);
        nb = nm = 0;
        if (hdr & ZPL_EXT_FLAG) {
            nb = row[ZPL_EXT_OFF] | ((unsigned)row[ZPL_EXT_OFF + 1] << 8);
            nm = row[ZPL_EXT_OFF + 2] | ((unsigned)row[ZPL_EXT_OFF + 3] << 8);
        }
        if (n + 2 + ((hdr & ZPL_EXT_FLAG) ? 4 : 0) + (nb + nm + na) * rec > cap) return 0;
        for (i = 0; i < 2; i++) out[n++] = row[i];
        if (hdr & ZPL_EXT_FLAG) {
            for (i = 0; i < 2; i++) out[n++] = row[ZPL_EXT_OFF + i];
            for (i = 0; i < nb * rec; i++) out[n++] = row[2 + i];
            for (i = 0; i < 2; i++) out[n++] = row[ZPL_EXT_OFF + 2 + i];
            for (i = 0; i < nm * rec; i++) out[n++] = row[2 + nb * rec + i];
            for (i = 0; i < na * rec; i++) out[n++] = row[2 + (nb + nm) * rec + i];
        } else {
            for (i = 0; i < na * rec; i++) out[n++] = row[2 + i];
        }
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

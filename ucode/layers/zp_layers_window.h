/* zp_layers_window.h - CPU side of the window layer of the layers microcode (docs/ppu/layers.md "Window layer"). C89, header only.
 *
 * The window is one standstill layer above everything (BG0, BG1, sprites): textboxes, HUD. It is a 32 x 32 grid of 8x8 cells that never scrolls,
 * described to the PPU as spans, per 8-row band: one *ink* span (cells you supply, 4 bytes each like a map's) and one *glass* span (a blended fill).
 * This builds the PPU's table from a list of boxes and says how much blitter time each band has left for sprites.
 *
 *   ZPL_BOX_SOLID    ink, opaque. Every pixel is drawn, so the PPU leaves out the BG under it (cells must not use the T attribute / alpha-0 colours)
 *   ZPL_BOX_OVERLAY  ink drawn with T: alpha-0 pixels show what is below (text over glass, a border). Hides nothing, so it costs full time
 *   ZPL_BOX_GLASS    a blended fill from the fill row at ZPL_GLFILL (you fill its 32 cells); `cells` unused. Drawn under the ink of the same band
 *
 * A band holds at most one ink span and one glass span: of boxes that share a band, the first in array order wins and the rest are dropped there.
 * Costs are blitter channel-cycles per band (a tile row costs 8 + 32 x 8 per cell, 392 blended): the same unit as ZPL_BAND_BUDGET, which is how
 * much sprite work a band takes over the worst background. A solid box *saves* time (the BG it hides costs 2 x 264 per column, it costs 264), an
 * overlay or glass box adds its cost, so over a worst-case background only a few of those cells fit; pass a larger budget when the background is lighter.
 */
#ifndef ZP_LAYERS_WINDOW_H
#define ZP_LAYERS_WINDOW_H

#include "zp_layers_sprites.h"

#define ZPL_BOX_SOLID 1
#define ZPL_BOX_OVERLAY 2
#define ZPL_BOX_GLASS 3

#define ZPL_GLASS_AVERAGE 1   /* 50% of the box colour over what is below */
#define ZPL_GLASS_SUBTRACT 2  /* darkens by the box colour */
#define ZPL_GLASS_ADD 3       /* lightens by the box colour */

#define ZPL_WIN_COLS 32
#define ZPL_COST_CELL 264UL
#define ZPL_COST_GLASS_CELL 392UL
#define ZPL_COST_BG_COLUMN 528UL   /* what a hidden column saves: two layers x 264, the cheapest a cell can cost */

typedef struct {
    unsigned short col, row;   /* top-left cell on the 32 x 32 grid */
    unsigned short w, h;       /* in cells */
    unsigned short cells;      /* PPU address of the w*h cells, row-major (not used by ZPL_BOX_GLASS) */
    unsigned short kind;       /* ZPL_BOX_* */
} ZPL_Box;

/* Fills `table` (ZPL_WIN_TABLE_BYTES, to be stored at ZPL_WIN0 or ZPL_WIN1 and flipped in with ZPL_CTRL_WINBASE; WINBASE 0 = no window) and
 * spriteBudget[b], the blitter time left in band b for sprites out of `budget` (pass that to zpl_build_lists_bands). `glassMode` is a ZPL_GLASS_*.
 * Returns how many box-bands were dropped (a second ink or glass span in a band, or one that does not fit in the budget). */
static ZPL_UNUSED unsigned zpl_build_window(const ZPL_Box* box, unsigned n, unsigned glassMode, unsigned long budget,
                                            unsigned char table[ZPL_WIN_TABLE_BYTES], unsigned long spriteBudget[ZPL_BANDS]) {
    unsigned dropped = 0, b, i, k;
    for (k = 0; k < ZPL_WIN_TABLE_BYTES; k++) table[k] = 0;
    zpl_put16(table, glassMode);
    for (b = 0; b < ZPL_BANDS; b++) {
        unsigned char* rec = table + ZPL_WIN_HDR_BYTES + b * ZPL_WIN_REC_BYTES;
        long net = 0;                     /* blitter time the window adds to the band, less what it saves: may be negative */
        int ink = -1, glass = -1;
        for (i = 0; i < n; i++) {
            const ZPL_Box* x = &box[i];
            if (x->w == 0 || x->h == 0 || x->col >= ZPL_WIN_COLS || b < x->row || b >= (unsigned)x->row + x->h) continue;
            if (x->kind == ZPL_BOX_GLASS) { if (glass < 0) glass = (int)i; else dropped++; }
            else { if (ink < 0) ink = (int)i; else dropped++; }
        }
        if (ink >= 0) {
            const ZPL_Box* x = &box[ink];
            unsigned w = x->col + x->w > ZPL_WIN_COLS ? ZPL_WIN_COLS - x->col : x->w;
            long delta = (long)(w * ZPL_COST_CELL);
            if (x->kind == ZPL_BOX_SOLID && w > 1) delta -= (long)((w - 1) * ZPL_COST_BG_COLUMN);   /* the edge cell stays: a fine X leaves part of it showing */
            if (net + delta <= (long)budget) {
                net += delta;
                zpl_put16(rec + 0, w);
                zpl_put16(rec + 2, x->col);
                zpl_put16(rec + 4, x->cells + 4u * x->w * (b - x->row));
                zpl_put16(rec + 6, x->kind == ZPL_BOX_OVERLAY ? ZPL_F_TRANSPARENT : 0);
            } else dropped++;
        }
        if (glass >= 0) {
            const ZPL_Box* x = &box[glass];
            unsigned w = x->col + x->w > ZPL_WIN_COLS ? ZPL_WIN_COLS - x->col : x->w;
            long delta = (long)(w * ZPL_COST_GLASS_CELL);
            if (net + delta <= (long)budget) {
                net += delta;
                zpl_put16(rec + 8, w);
                zpl_put16(rec + 10, x->col);
            } else dropped++;
        }
        spriteBudget[b] = net <= 0 ? budget : (net >= (long)budget ? 0UL : budget - (unsigned long)net);
    }
    return dropped;
}

#endif

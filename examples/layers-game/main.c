/*
 * layers-game: a small game loop on the layers library (include/zeropoint/zp_layers.h).
 *
 * Two scrolling layers (a sky and a ground on BG0, clouds, bushes and trees on BG1, at a different speed), a hero you move with the
 * pad that draws over everything, a bird that flies BETWEEN the two layers so the clouds pass in front of it, and a text box in the
 * window layer, which never scrolls.
 *
 * Builds with both compilers (see the Makefile):   make          (zpcc)       make CC=dos      (def88186cc)
 * Runs on the emulator:                            zeropoint_sdl build/game.zpb
 */
#include <zeropoint/zeropoint.h>
#include <zeropoint/zp_layers.h>
#include "assets.h"

#define HERO_W 16
#define HERO_H 16

/* def88186cc has no '(void)' parameter list, so write main as main(). */
int main()
{
    int x, y, t, bx;

    /* The microcode sits in ROM at $10:0000 (the Makefile places it there), LAYERS_PAGES pages of 256 bytes. */
    zpl_init(0x10, 0x0000, LAYERS_PAGES);

    /* Everything the PPU draws from goes in before zpl_start(). Tiles are 4 banks of 256 ids; here bank 0 is the opaque ground
     * for BG0, bank 2 the see-through tiles of BG1, bank 1 the sprites, bank 3 the text box. */
    zpl_palette_load(ASSET_PAL);
    zpl_tiles_load(0, 0, ASSET_TILES0, ASSET_TILES0_N);
    zpl_tiles_load(2, 0, ASSET_TILES2, ASSET_TILES2_N);
    zpl_tiles_load(1, 0, ASSET_TILES1, ASSET_TILES1_N);
    zpl_tiles_load(3, 0, ASSET_TILES3, ASSET_TILES3_N);
    zpl_cells_load(0, ASSET_CELLS, ASSET_CELLS_N);          /* sprite cells 0-3: the hero, 4-5: the bird */
    zpl_map_load(0, ASSET_MAP0);
    zpl_map_load(1, ASSET_MAP1);

    /* The text box: 28 x 5 cells at column 2, row 26 of the 32 x 32 window grid. Solid: the PPU leaves out the background under it. */
    zpl_window_box(ZPL_BOX_SOLID, 2, 26, 28, 5, zpl_window_cells_load(ASSET_WINCELLS, ASSET_WINCELLS_N));

    /* Sprite slots draw in order, later over earlier. The bird is PRIO_MID: in front of BG0, behind BG1. */
    x = 120;
    y = 136;
    zpl_sprite_set(1, 0, 36, 2, 1, 4, ZPL_SPR_PRIO_MID);
    zpl_sprite_set(0, x, y, 2, 2, 0, 0);

    zpl_start();

    t = 0;
    for (;;) {
        x = x + 2 * zp_pad_dx(0);
        y = y + 2 * zp_pad_dy(0);
        if (x < 0) x = 0;
        if (x > 256 - HERO_W) x = 256 - HERO_W;
        if (y < 8) y = 8;
        if (y > 208 - HERO_H) y = 208 - HERO_H;
        zpl_sprite_move(0, x, y);

        bx = (t * 2) % 288 - 16;                             /* the bird crosses the screen and wraps */
        zpl_sprite_move(1, bx, 36 + ((t >> 3) & 7));

        zpl_scroll(0, t >> 1, 0);                            /* BG0 at half speed: the ground... */
        zpl_scroll(1, t, 0);                                 /* ...BG1 at full: clouds and bushes in front of it */
        t = t + 1;

        zpl_frame();                                         /* wait for the 60 Hz tick, then send the frame */
    }
}

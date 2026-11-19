/* gen_frames.c - the demo cart's animation: 256 frames (one per 1/60 s: the CPU waits on the V-blank timer), each one a 256-byte frame block (the four scroll words at ZPL_SCROLL_OFF, then the
 * sprite lists packed by zpl_pack_stream at ZPL_STREAM_OFF, built with the same zp_layers_sprites.h the harness uses). The CPU DMAs one block per frame
 * into the idle list buffer and flips ZPL_CTRL_LISTBASE, so scroll and sprites change together.
 *
 *   gen_frames <out.bin> [<window.bin>]
 *
 * Everything is periodic in 256 frames so the loop is seamless: BG0 scrolls 2 px/frame (512 px = its whole width), BG1 4 px/frame.
 * Sprite cells (in the PPU at ZPL_SPRCELLS): 0 = ball (1x1), 1-4 = box (2x2), 5-12 = ship (4x2), all bank 2.
 *
 * The window layer is static (a textbox over the bottom rows, a frosted-glass lens over the ground); the same box list builds its table,
 * written to <window.bin>, and leaves the sprite builder the blitter time that is left in each band (none of the sprites goes through those rows).
 */
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "../zp_layers_window.h"

#define FRAMES 256
#define BLOCK_BYTES 256   /* a packed frame is at most ~212 bytes; small blocks keep the cart's data to 4 verified chunks */

int main(int argc, char** argv) {
    static unsigned char lists[ZPL_BANDS][ZPL_LIST_STRIDE];
    FILE* out;
    int k;
    unsigned totalDropped = 0;
    ZPL_Box boxes[2];
    unsigned char table[ZPL_WIN_TABLE_BYTES];
    unsigned long budgets[ZPL_BANDS];
    if (argc < 2) { fprintf(stderr, "usage: %s <out.bin> [<window.bin>]\n", argv[0]); return 2; }
    /* textbox: 30 x 5 cells at (1, 26), opaque (the BG under it is not drawn); lens: 3 x 3 at (3, 22), averaged with a near-white fill */
    boxes[0].col = 1; boxes[0].row = 26; boxes[0].w = 30; boxes[0].h = 5; boxes[0].cells = ZPL_WINCELLS; boxes[0].kind = ZPL_BOX_SOLID;
    boxes[1].col = 3; boxes[1].row = 22; boxes[1].w = 3; boxes[1].h = 3; boxes[1].cells = 0; boxes[1].kind = ZPL_BOX_GLASS;
    if (zpl_build_window(boxes, 2, ZPL_GLASS_AVERAGE, ZPL_BAND_BUDGET, table, budgets)) { fprintf(stderr, "the window does not fit its budget\n"); return 1; }
    if (argc > 2) {
        FILE* w = fopen(argv[2], "wb");
        if (!w) { perror(argv[2]); return 1; }
        fwrite(table, 1, sizeof table, w);
        fclose(w);
    }
    out = fopen(argv[1], "wb");
    if (!out) { perror(argv[1]); return 1; }
    for (k = 0; k < FRAMES; k++) {
        const double t = 2.0 * 3.14159265358979 * k / FRAMES;
        ZPL_Sprite spr[3];
        unsigned dropped;
        int scroll[4];
        unsigned char block[BLOCK_BYTES];
        int i;

        /* ship: sweeps across the sky, turning round at each end */
        spr[0].w = 4; spr[0].h = 2; spr[0].cells = ZPL_SPRCELLS + 4 * 5;
        spr[0].x = (short)(112 + (int)(104 * sin(t)));
        spr[0].y = (short)(22 + (int)(12 * sin(3 * t)));
        spr[0].flags = cos(t) < 0 ? ZPL_SPR_HFLIP : 0;
        /* box: a loop in the middle of the screen */
        spr[1].w = 2; spr[1].h = 2; spr[1].cells = ZPL_SPRCELLS + 4 * 1;
        spr[1].x = (short)(120 + (int)(80 * cos(2 * t)));
        spr[1].y = (short)(72 + (int)(18 * sin(4 * t)));
        spr[1].flags = 0;
        /* ball: bounces along the ground */
        spr[2].w = 1; spr[2].h = 1; spr[2].cells = ZPL_SPRCELLS + 4 * 0;
        spr[2].x = (short)(124 + (int)(104 * sin(t + 1.0)));
        spr[2].y = (short)(156 - (int)fabs(34 * sin(8 * t)));
        spr[2].flags = 0;

        dropped = zpl_build_lists_bands(spr, 3, lists, budgets);
        totalDropped += dropped;
        memset(block, 0, sizeof block);
        if (!zpl_pack_stream(lists, block + ZPL_STREAM_OFF, BLOCK_BYTES - ZPL_STREAM_OFF)) {   /* scroll words, then the packed stream */
            fprintf(stderr, "frame %d: the packed sprite stream does not fit a %d-byte frame block\n", k, BLOCK_BYTES);
            return 1;
        }
        /* scroll: BG0 (sky + ground) 2 px/frame, BG1 (clouds + foreground) 4 px/frame, both wobbling in Y */
        scroll[0] = (2 * k) % 512;
        scroll[1] = 8 + (int)floor(6 * sin(t) + 0.5);
        scroll[2] = (4 * k) % 512;
        scroll[3] = 4 + (int)floor(3 * sin(2 * t) + 0.5);
        for (i = 0; i < 4; i++) zpl_put16(&block[ZPL_SCROLL_OFF + 2 * i], (unsigned)scroll[i]);
        fwrite(block, 1, sizeof block, out);
    }
    fclose(out);
    fprintf(stderr, "%d frames, %u sprite pieces dropped by the budget\n", FRAMES, totalDropped);
    return totalDropped ? 1 : 0;
}

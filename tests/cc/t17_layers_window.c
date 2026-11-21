/* The C layers library's window table builder against the host reference (ucode/layers/zp_layers_window.h): the same arrangement as
 * t16_layers_stream.c. Each scene is a few random boxes of every kind (some hanging off the right edge, several sharing bands so the
 * "first ink, first glass wins" rule matters); both sides build the table and record every word of it. */
#include "tcc.h"

#ifdef HOST
#include "../../ucode/layers/zp_layers_window.h"
#else
#define ZPL_RAM 0x6000
#include <zeropoint/zp_layers.h>
#endif

#define NSCENES 4

int rnd(int scene, int i, int k)
{
    U16 s;
    int j;
    s = 3 + scene * 6007 + i * 197;
    for (j = 0; j <= k; j = j + 1) s = (s * 25173 + 13849) & 0xFFFF;
    return (int)(s >> 4);
}

int box_kind(int scene, int i) { return 1 + rnd(scene, i, 0) % 3; }
int box_col(int scene, int i) { return rnd(scene, i, 1) % 32; }
int box_row(int scene, int i) { return rnd(scene, i, 2) % 32; }
int box_w(int scene, int i) { return 1 + rnd(scene, i, 3) % 12; }
int box_h(int scene, int i) { return 1 + rnd(scene, i, 4) % 10; }
int box_cells(int scene, int i) { return 0x11A0 + 4 * (rnd(scene, i, 5) % 40); }

int main()
{
    int scene, n, i, k, mode;

    for (scene = 0; scene < NSCENES; scene = scene + 1) {
        n = 2 + scene * 2;
        mode = 1 + scene % 3;
#ifdef HOST
        {
            ZPL_Box box[8];
            unsigned char table[ZPL_WIN_TABLE_BYTES];
            unsigned long budgets[ZPL_BANDS];
            for (i = 0; i < n; i = i + 1) {
                box[i].kind = (unsigned short)box_kind(scene, i); box[i].col = (unsigned short)box_col(scene, i);
                box[i].row = (unsigned short)box_row(scene, i); box[i].w = (unsigned short)box_w(scene, i);
                box[i].h = (unsigned short)box_h(scene, i); box[i].cells = (unsigned short)box_cells(scene, i);
            }
            zpl_build_window(box, (unsigned)n, (unsigned)mode, ZPL_BAND_BUDGET, table, budgets);
            for (k = 0; k < ZPL_WIN_TABLE_BYTES / 2; k = k + 1) rec((S16)(table[2 * k] | (table[2 * k + 1] << 8)));
        }
#else
        {
            unsigned short* w;
            zpl_zero(ZPL_MEM(0), ZPL_RAM_SIZE);
            zpl_window_glass_mode(mode);
            for (i = 0; i < n; i = i + 1)
                zpl_window_box(box_kind(scene, i), box_col(scene, i), box_row(scene, i), box_w(scene, i), box_h(scene, i), box_cells(scene, i));
            zpl_build_window(ZPL_MEM(ZPL_O_WTAB));
            w = (unsigned short*)ZPL_MEM(ZPL_O_WTAB);
            for (k = 0; k < ZPL_WIN_TABLE_BYTES / 2; k = k + 1) rec((S16)w[k]);
        }
#endif
    }
    return 0;
}

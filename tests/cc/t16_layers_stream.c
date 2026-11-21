/* The C layers library's frame block builder against the host reference builder (ucode/layers/zp_layers_sprites.h).
 *
 * tests/cc/run.sh runs this on the host (-DHOST) as the reference and under zpcc and def88186cc in the emulator, and compares what
 * they record. Here the host side is NOT the library: it is the reference builder (zpl_build_lists + zpl_pack_stream, the code
 * the layers test and the demo use), and the console side is the library (zpl_build_block): so equal output means the console builder
 * writes byte for byte the stream the reference does, for the same sprites: every priority, flips, partly off-screen sprites, bands
 * that overflow ZPL_MAXP and pieces that are dropped. Each scene records: pieces dropped, the stream's length in bytes, the four
 * scroll words and then every word of the stream. */
#include "tcc.h"

#ifdef HOST
#include "../../ucode/layers/zp_layers_sprites.h"
#else
#define ZPL_RAM 0x6000          /* the test image sits low in Work RAM, the recorder at $8000: keep the library's state clear of both */
#include <zeropoint/zp_layers.h>
#endif

#define NSCENES 3

/* parameter k of sprite i in a scene: a 16-bit LCG (the same arithmetic on the host and the console: every step is masked to 16 bits) */
int rnd(int scene, int i, int k)
{
    U16 s;
    int j;
    s = 1 + scene * 7919 + i * 131;
    for (j = 0; j <= k; j = j + 1) s = (s * 25173 + 13849) & 0xFFFF;
    return (int)(s >> 4);
}

int spr_x(int scene, int i) { return rnd(scene, i, 0) % 300 - 30; }
int spr_y(int scene, int i) { return rnd(scene, i, 1) % 290 - 30; }
int spr_w(int scene, int i) { return 1 + rnd(scene, i, 2) % 4; }
int spr_h(int scene, int i) { return scene == 2 ? 1 + rnd(scene, i, 3) % 2 : 1 + rnd(scene, i, 3) % 4; }
int spr_c(int scene, int i) { return rnd(scene, i, 4) % 100; }
int spr_f(int scene, int i)
{
    int f, p;
    f = rnd(scene, i, 5) & 3;                    /* hflip, vflip */
    p = rnd(scene, i, 6) % 3;                    /* above both layers (0), between (4), behind BG0 (8) */
    if (p == 1) f = f | 4;
    if (p == 2) f = f | 8;
    return f;
}

int main()
{
    int scene, n, i, k, dropped, bytes;

    for (scene = 0; scene < NSCENES; scene = scene + 1) {
        n = scene == 0 ? 6 : (scene == 1 ? 24 : 56);
#ifdef HOST
        {
            static unsigned char lists[ZPL_BANDS][ZPL_LIST_STRIDE];
            static unsigned char block[4096];
            ZPL_Sprite spr[64];
            int sc[4];
            for (i = 0; i < n; i = i + 1) {
                spr[i].x = (short)spr_x(scene, i); spr[i].y = (short)spr_y(scene, i);
                spr[i].w = (unsigned short)spr_w(scene, i); spr[i].h = (unsigned short)spr_h(scene, i);
                spr[i].cells = (unsigned short)(ZPL_SPRCELLS + 4 * spr_c(scene, i));   /* the reference takes a PPU address, the library a cell index */ spr[i].flags = (unsigned short)spr_f(scene, i);
            }
            sc[0] = scene * 11; sc[1] = scene * 5 + 1; sc[2] = 400 + scene; sc[3] = 3 * scene;
            dropped = (int)zpl_build_lists(spr, (unsigned)n, lists);
            bytes = (int)zpl_pack_stream(lists, block + ZPL_STREAM_OFF, sizeof(block) - ZPL_STREAM_OFF);
            for (k = 0; k < 4; k = k + 1) { block[2 * k] = (unsigned char)(sc[k] & 255); block[2 * k + 1] = (unsigned char)((sc[k] >> 8) & 255); }
            rec(dropped); rec(bytes);
            for (k = 0; k < 4 + bytes / 2; k = k + 1) rec((S16)(block[2 * k] | (block[2 * k + 1] << 8)));
        }
#else
        {
            unsigned short* w;
            struct zpl_ctx* c;
            zpl_zero(ZPL_MEM(0), ZPL_RAM_SIZE);
            for (i = 0; i < n; i = i + 1)
                zpl_sprite_set(i, spr_x(scene, i), spr_y(scene, i), spr_w(scene, i), spr_h(scene, i), spr_c(scene, i), spr_f(scene, i));
            zpl_scroll(0, scene * 11, scene * 5 + 1);
            zpl_scroll(1, 400 + scene, 3 * scene);
            dropped = zpl_build_block();
            c = zpl_ctxp();
            bytes = (int)c->stream_bytes;
            w = (unsigned short*)ZPL_MEM(ZPL_O_BUF);
            rec(dropped); rec(bytes);
            for (k = 0; k < 4 + bytes / 2; k = k + 1) rec((S16)w[k]);
        }
#endif
    }
    return 0;
}

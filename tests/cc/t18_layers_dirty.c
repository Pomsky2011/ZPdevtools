/* EXPECT: 5 7 9 11 1 1 1 */
/* The C layers library rebuilds the sprite stream only when a sprite changed; the scroll words are refreshed on every build.
 * Records: the scroll words of a first build (SCX0, SCX1), the same two after only the scroll changed, whether the stream was left
 * untouched by that (1), whether moving a sprite changed it (1), and whether the stream bytes then equal a from-scratch build (1). */
#include "tcc.h"
#define ZPL_RAM 0x6000
#include <zeropoint/zp_layers.h>

int same(int a_off, int b_off, int bytes)
{
    unsigned char* a;
    unsigned char* b;
    int i;
    a = ZPL_MEM(a_off);
    b = ZPL_MEM(b_off);
    for (i = 0; i < bytes; i = i + 1) if (a[i] != b[i]) return 0;
    return 1;
}

/* copies the stream (after the 8 scroll bytes) of the current block to the spare area at +$1A00 */
void save(int bytes)
{
    unsigned char* a;
    unsigned char* b;
    int i;
    a = ZPL_MEM(ZPL_O_BUF + 8);
    b = ZPL_MEM(0x1A00);
    for (i = 0; i < bytes; i = i + 1) b[i] = a[i];
}

int main()
{
    struct zpl_ctx* c;
    unsigned short* w;
    int bytes, stale;

    zpl_zero(ZPL_MEM(0), ZPL_RAM_SIZE);
    zpl_zero(ZPL_MEM(0x1A00), 0x400);
    zpl_sprite_set(0, 40, 20, 2, 2, 0, 0);
    zpl_sprite_set(1, 120, 100, 3, 1, 4, ZPL_SPR_PRIO_MID);
    zpl_scroll(0, 5, 6);
    zpl_scroll(1, 7, 8);
    zpl_build_block();
    c = zpl_ctxp();
    bytes = (int)c->stream_bytes;
    w = (unsigned short*)ZPL_MEM(ZPL_O_BUF);
    rec(w[0]); rec(w[2]);                          /* 5 7 */
    save(bytes);

    zpl_scroll(0, 9, 6);
    zpl_scroll(1, 11, 8);
    zpl_build_block();                             /* nothing but the scroll changed */
    rec(w[0]); rec(w[2]);                          /* 9 11 */
    rec(same(ZPL_O_BUF + 8, 0x1A00, bytes));       /* the stream is the same bytes: 1 */

    zpl_sprite_move(1, 121, 100);                  /* a sprite moved: the stream must be rebuilt */
    zpl_build_block();
    stale = same(ZPL_O_BUF + 8, 0x1A00, bytes);
    rec(stale ? 0 : 1);                            /* it changed: 1 */
    c = zpl_ctxp();
    save((int)c->stream_bytes);
    c->built = 0;                                  /* force a from-scratch build of the same sprites... */
    zpl_build_block();
    rec(same(ZPL_O_BUF + 8, 0x1A00, (int)c->stream_bytes));   /* ...which equals the incremental one: 1 */
    return 0;
}

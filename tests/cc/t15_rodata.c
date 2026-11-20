/* ONLY: zpcc */
#include "tcc.h"

/* zpcc only (def88186cc has no globals): constant data lives in ROM and can be read by name - indexed in place, in
   loops, through struct members - and copied into a local array; its address is never used as a pointer value. */

static const S16 table[6] = {100, 200, 300, 400, 500, 600};
static const unsigned char bytes[8] = {1, 2, 3, 250, 251, 252, 253, 254};
static const S32 big[3] = {70000L, -5L, 123456L};
struct P { S16 x; S16 y; unsigned char c; };
static const struct P pts[3] = {{1, 2, 3}, {40, 50, 60}, {7, 8, 9}};

S16 sum_table(void)
{
    S16 i; S16 s;
    s = 0;
    for (i = 0; i < 6; i = i + 1) s = s + table[i] / 100;
    return s;
}

S16 byte_values(void)
{
    S16 i; S16 s;
    s = 0;
    for (i = 3; i < 8; i = i + 1) s = s + bytes[i];
    return s + bytes[0];
}

S16 longs(void) { return (S16)(big[0] / 1000) + (S16)(big[1] * 10) + (S16)(big[2] % 1000); }

S16 structs(void)
{
    S16 i; S16 s;
    s = 0;
    for (i = 0; i < 3; i = i + 1) s = s + pts[i].x + pts[i].y * 2 + pts[i].c;
    return s;
}

S16 copied(void)
{
    S16 local[6];
    S16 i;
    for (i = 0; i < 6; i = i + 1) local[i] = table[i];
    local[2] = local[2] + 1;
    return local[2] + table[2] + local[5];
}

S16 whole_array_init(void)
{
    S16 a[4] = {7, 8, 9, 10};
    unsigned char b[3] = {250, 251, 252};
    return a[3] + b[2];
}

int main()
{
    rec(sum_table());
    rec(byte_values());
    rec(longs());
    rec(structs());
    rec(copied());
    rec(whole_array_init());
    return 0;
}

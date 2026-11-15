#include "tcc.h"

S16 g_sum(S16 n) { S16 i; S16 s; s = 0; for (i = 1; i <= n; i = i + 1) s = s + i; return s; }
S16 w_count(S16 n) { S16 c; c = 0; while (n > 0) { c = c + 1; n = n - 2; } return c; }
S16 dw(S16 n) { S16 c; c = 0; do { c = c + 3; n = n - 1; } while (n > 0); return c; }
S16 ter(S16 a, S16 b) { return a > b ? a - b : b - a; }
S16 land(S16 a, S16 b) { return a && b; }
S16 lor(S16 a, S16 b) { return a || b; }
S16 pick(S16 n) { if (n < 0) return 1; else if (n == 0) return 2; else if (n < 10) return 3; return 4; }

int main()
{
    rec(7 + 5);
    rec(7 - 12);
    rec(6 * 7);
    rec(100 / 7);
    rec(100 % 7);
    rec(0x0F0F & 0x00FF);
    rec(0x0F00 | 0x00F0);
    rec(0x0FF0 ^ 0x00FF);
    rec(3 << 4);
    rec(0x4000 >> 3);
    rec(g_sum(10));
    rec(w_count(9));
    rec(dw(4));
    rec(ter(3, 10));
    rec(ter(10, 3));
    rec(land(1, 0));
    rec(land(2, 3));
    rec(lor(0, 0));
    rec(lor(0, 5));
    rec(pick(-5));
    rec(pick(0));
    rec(pick(7));
    rec(pick(70));
    rec(3 < 4);
    rec(4 < 3);
    rec(-3 < 2);
    rec(2 <= 2);
    rec(2 >= 3);
    rec(5 == 5);
    rec(5 != 5);
    rec(-8 / 2);
    rec(-7 % 3);
    return 0;
}

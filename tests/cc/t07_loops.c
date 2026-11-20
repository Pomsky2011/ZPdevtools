#include "tcc.h"

/* Loop semantics.  The hardware LOOP/LPEND counter is a single register pair, so counted loops that nest, call,
   continue, or touch their own counter must behave exactly like ordinary loops (they used to run wrong). */

S16 inner_sum(S16 n) { S16 j; S16 s; s = 0; for (j = 0; j < 4; j = j + 1) s = s + n + j; return s; }

S16 zero_trip(void) { S16 i; S16 c; c = 0; for (i = 0; i < 0; i = i + 1) c = c + 1; return c; }
S16 neg_trip(void) { S16 i; S16 c; c = 0; for (i = 0; i < -3; i = i + 1) c = c + 1; return c; }
S16 one_trip(void) { S16 i; S16 c; c = 0; for (i = 0; i < 1; i = i + 1) c = c + 1; return c; }
S16 after_loop(void) { S16 i; for (i = 0; i < 5; i = i + 1) { } return i; }

S16 continue_in_for(void)
{
    S16 i; S16 s; s = 0;
    for (i = 0; i < 8; i = i + 1) { if (i % 2) continue; s = s + i; }
    return s;
}

S16 break_in_for(void)
{
    S16 i; S16 s; s = 0;
    for (i = 0; i < 8; i = i + 1) { if (i == 5) break; s = s + i; }
    return s * 10 + i;
}

S16 body_changes_counter(void)
{
    S16 i; S16 s; s = 0;
    for (i = 0; i < 10; i = i + 1) { s = s + 1; if (i == 2) i = i + 4; }
    return s * 100 + i;
}

S16 nested(void)
{
    S16 i; S16 j; S16 s; s = 0;
    for (i = 0; i < 3; i = i + 1) {
        for (j = 0; j < 4; j = j + 1) s = s + i * 10 + j;
        s = s + 1000;
    }
    return s;
}

S16 nested3(void)
{
    S16 i; S16 j; S16 k; S16 s; s = 0;
    for (i = 0; i < 2; i = i + 1)
        for (j = 0; j < 3; j = j + 1)
            for (k = 0; k < 2; k = k + 1) s = s + 1 + i + j + k;
    return s;
}

S16 call_in_loop(void)
{
    S16 i; S16 s; s = 0;
    for (i = 0; i < 3; i = i + 1) s = s + inner_sum(i);
    return s;
}

S16 early_return(void)
{
    S16 i;
    for (i = 0; i < 10; i = i + 1) { if (i * i > 20) return i; }
    return -1;
}

S16 while_loops(void)
{
    S16 i; S16 s; s = 0; i = 0;
    while (i < 10) { i = i + 1; if (i == 3) continue; if (i == 8) break; s = s + i; }
    return s * 100 + i;
}

S16 do_loops(void)
{
    S16 i; S16 s; s = 0; i = 0;
    do { i = i + 1; if (i == 2) continue; s = s + i; } while (i < 5);
    return s * 10 + i;
}

S16 switch_in_loop(void)
{
    S16 i; S16 s; s = 0;
    for (i = 0; i < 6; i = i + 1) {
        switch (i) {
        case 1: s = s + 10; break;
        case 3: s = s + 100; break;
        default: s = s + 1; break;
        }
    }
    return s;
}

S16 two_sequential(void)
{
    S16 i; S16 s; s = 0;
    for (i = 0; i < 3; i = i + 1) s = s + 1;
    for (i = 0; i < 4; i = i + 1) s = s + 10;
    return s;
}

int main()
{
    rec(zero_trip());
    rec(neg_trip());
    rec(one_trip());
    rec(after_loop());
    rec(continue_in_for());
    rec(break_in_for());
    rec(body_changes_counter());
    rec(nested());
    rec(nested3());
    rec(call_in_loop());
    rec(early_return());
    rec(while_loops());
    rec(do_loops());
    rec(switch_in_loop());
    rec(two_sequential());
    return 0;
}

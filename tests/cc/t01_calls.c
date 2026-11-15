#include "tcc.h"

S16 add3(S16 a, S16 b, S16 c) { return a + b + c; }
S16 mix(S16 a, S16 b) { S16 t; t = a + a; return t + b; }
S16 fib(S16 n) { if (n < 2) return n; return fib(n - 1) + fib(n - 2); }
S16 four(S16 a, S16 b, S16 c, S16 d) { return a + b + b + c + c + c + d + d + d + d; }
S16 six(S16 a, S16 b, S16 c, S16 d, S16 e, S16 f) { return a - b + c - d + e - f; }
S16 locals() { S16 a; S16 b; a = 3; b = 4; return a + b + b; }
S16 sub3(S16 a, S16 b, S16 c) { return a - b - c; }
S16 twice(S16 x) { return add3(x, x, x) + add3(x, 1, 2); }
S16 callarg(S16 a) { return add3(add3(a, 1, 1), add3(a, 2, 2), add3(a, 3, 3)); }
S16 modparam(S16 a, S16 b) { a = a + 1; b = b + a; return b; }
S16 clobber(S16 a, S16 b, S16 c) { S16 x; x = a + b; x = x + c; return x + a + b + c; }
S16 weights(S16 a, S16 b, S16 c, S16 d, S16 e, S16 f, S16 g, S16 h)
{
    return a + (b << 1) + (c << 2) + (d << 3) + (e << 4) + (f << 5) + (g << 6) + (h << 7);
}
S16 deep(S16 n) { S16 x; x = n + 1; if (n == 0) return 100; return deep(n - 1) + x; }

int main()
{
    rec(add3(1, 2, 3));
    rec(mix(5, 7));
    rec(fib(10));
    rec(four(1, 2, 3, 4));
    rec(six(10, 1, 9, 2, 8, 3));
    rec(locals());
    rec(sub3(10, 3, 2));
    rec(twice(5));
    rec(callarg(10));
    rec(modparam(4, 6));
    rec(clobber(1, 2, 3));
    rec(weights(1, 1, 1, 1, 1, 1, 1, 1));
    rec(weights(1, 0, 1, 0, 1, 0, 1, 0));
    rec(deep(5));
    return 0;
}

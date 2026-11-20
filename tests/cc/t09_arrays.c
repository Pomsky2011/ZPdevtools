#include "tcc.h"

/* local arrays: indexing, initializers, 2-D, char and long elements, arrays as parameters */

S16 sum(S16 *a, S16 n) { S16 i; S16 s; s = 0; for (i = 0; i < n; i = i + 1) s = s + a[i]; return s; }
S16 sum_brackets(S16 a[], S16 n) { S16 i; S16 s; s = 0; for (i = 0; i < n; i = i + 1) s = s + a[i]; return s; }
void fill(S16 a[4], S16 v) { S16 i; for (i = 0; i < 4; i = i + 1) a[i] = v + i; }
S16 first_nonzero(unsigned char *p, S16 n) { S16 i; for (i = 0; i < n; i = i + 1) if (p[i]) return i; return -1; }

S16 basic(void)
{
    S16 a[5];
    S16 i;
    for (i = 0; i < 5; i = i + 1) a[i] = i * i;
    return a[0] + a[1] * 10 + a[2] * 100 + a[3] + a[4];
}

S16 init_full(void) { S16 a[4] = {10, 20, 30, 40}; return a[0] + a[3]; }
S16 init_partial(void) { S16 a[6] = {1, 2}; return a[1] * 100 + a[2] + a[5]; }
S16 init_unsized(void) { S16 a[] = {5, 6, 7}; return sizeof(a) / sizeof(a[0]) * 100 + a[2]; }
S16 init_trailing_comma(void) { S16 a[3] = {1, 2, 3,}; return a[2]; }
S16 init_exprs(S16 x) { S16 a[3] = {x, x + 1, x * 2}; return a[0] + a[1] * 10 + a[2] * 100; }

S16 two_d(void)
{
    S16 m[3][4];
    S16 i; S16 j; S16 s;
    s = 0;
    for (i = 0; i < 3; i = i + 1)
        for (j = 0; j < 4; j = j + 1)
            m[i][j] = i * 10 + j;
    for (i = 0; i < 3; i = i + 1)
        for (j = 0; j < 4; j = j + 1)
            s = s + m[i][j] * (i + 1);
    return s + m[2][3];
}

S16 two_d_init(void)
{
    S16 m[2][3] = {{1, 2, 3}, {4, 5, 6}};
    S16 n[2][3] = {1, 2, 3, 4, 5, 6};
    S16 p[2][3] = {{1}, {4, 5}};
    return m[1][2] * 1000 + n[1][0] * 100 + p[0][2] * 10 + p[1][1];
}

S16 row_as_pointer(void)
{
    S16 m[3][3] = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    return sum(m[1], 3) * 10 + sum(m[2], 3) % 10;
}

S16 chars(void)
{
    unsigned char c[6];
    signed char s[4];
    S16 i;
    for (i = 0; i < 6; i = i + 1) c[i] = 250 + i;
    s[0] = 100; s[1] = 200; s[2] = -5; s[3] = 127 + 2;
    return c[0] + c[5] * 2 + s[1] + s[2] * 3 + s[3];
}

S16 char_string(void)
{
    char s[] = "hello";
    char t[8] = "ab";
    S16 n; n = 0;
    while (s[n]) n = n + 1;
    return n * 100 + s[1] + t[1] + t[2] + t[7];
}

S16 string_escapes(void)
{
    char s[] = "a\n\t\\\"\x41\101";
    return s[1] * 1000 + s[2] * 100 + s[3] + s[4] + s[5] + s[6] + sizeof(s);
}

S16 longs(void)
{
    S32 a[3];
    S32 t;
    a[0] = 100000L; a[1] = -3L; a[2] = a[0] / 7;
    t = a[0] + a[1] + a[2];
    return (S16)(t % 1000) + (S16)(a[2] >> 8);
}

S16 compound(void)
{
    S16 a[4] = {1, 2, 3, 4};
    a[1] += 10; a[2] -= 1; a[3] *= 5; a[0] <<= 3;
    a[1] |= 1; a[2] ^= 7; a[3] %= 7; a[0] &= 12;
    return a[0] * 1000 + a[1] * 100 + a[2] * 10 + a[3];
}

S16 incdec(void)
{
    S16 a[3] = {5, 6, 7};
    S16 r;
    r = a[0]++;
    r = r * 10 + ++a[1];
    r = r * 10 + a[2]--;
    r = r % 1000;
    return r + a[0] + a[1] + a[2];
}

S16 index_exprs(void)
{
    S16 a[8];
    S16 i;
    for (i = 0; i < 8; i = i + 1) a[i] = 100 + i;
    i = 2;
    return a[i + 1] + a[i * 2] * 2 + a[7 - i] + a[(i & 1)] + a[a[0] - 98];
}

S16 pass_arrays(void)
{
    S16 a[4];
    S16 b[3] = {1, 2, 3};
    fill(a, 10);
    return sum(a, 4) * 10 + sum_brackets(b, 3);
}

S16 scan(void)
{
    unsigned char b[5] = {0, 0, 7, 0, 9};
    return first_nonzero(b, 5) * 10 + first_nonzero(b + 3, 2);
}

S16 nested_index(void)
{
    S16 idx[4] = {3, 2, 1, 0};
    S16 v[4] = {10, 20, 30, 40};
    return v[idx[0]] + v[idx[idx[1]]] * 2 + v[idx[idx[idx[3]]]];
}

S16 sizeofs(void)
{
    S16 a[7];
    S16 m[3][5];
    char c[11];
    S32 l[4];
    return sizeof(a) * 1000 + sizeof(m) + sizeof(c) * 3 + sizeof(l[0]) + sizeof(m[0]) + sizeof(a[0]);
}

S16 bubble(void)
{
    S16 a[6] = {5, 2, 9, 1, 7, 3};
    S16 i; S16 j; S16 t;
    for (i = 0; i < 5; i = i + 1)
        for (j = 0; j < 5 - i; j = j + 1)
            if (a[j] > a[j + 1]) { t = a[j]; a[j] = a[j + 1]; a[j + 1] = t; }
    return a[0] * 100000 / 1000 + a[1] * 10 + a[2] + a[3] + a[4] + a[5] * 3;
}

int main()
{
    rec(basic());
    rec(init_full());
    rec(init_partial());
    rec(init_unsized());
    rec(init_trailing_comma());
    rec(init_exprs(3));
    rec(two_d());
    rec(two_d_init());
    rec(row_as_pointer());
    rec(chars());
    rec(char_string());
    rec(string_escapes());
    rec(longs());
    rec(compound());
    rec(incdec());
    rec(index_exprs());
    rec(pass_arrays());
    rec(scan());
    rec(nested_index());
    rec(sizeofs());
    rec(bubble());
    return 0;
}

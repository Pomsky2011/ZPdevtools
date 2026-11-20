#include "tcc.h"

/* pointers to locals: &, *, p[i], arithmetic, comparison, pointers to pointers, pointers as parameters and results */

void swap(S16 *a, S16 *b) { S16 t; t = *a; *a = *b; *b = t; }
void inc_all(S16 *p, S16 n) { while (n > 0) { (*p)++; p++; n = n - 1; } }
S16 *max_of(S16 *a, S16 *b) { return *a > *b ? a : b; }
S16 *find(S16 *a, S16 n, S16 v) { S16 i; for (i = 0; i < n; i = i + 1) if (a[i] == v) return &a[i]; return 0; }
void set_long(S32 *p, S32 v) { *p = v; }
void bump(S16 **pp) { (**pp)++; *pp = *pp + 1; }
S16 str_len(char *s) { S16 n; n = 0; while (*s) { s++; n++; } return n; }
void str_copy(char *d, char *s) { while ((*d++ = *s++) != 0) { } }
S16 str_cmp(char *a, char *b) { while (*a && *a == *b) { a++; b++; } return *a - *b; }
void zero(char *p, S16 n) { S16 i; for (i = 0; i < n; i = i + 1) p[i] = 0; }

S16 basic(void)
{
    S16 x; S16 y; S16 *p;
    x = 5; y = 7;
    p = &x;
    *p = *p + 10;
    p = &y;
    *p += 3;
    return x * 100 + y;
}

S16 swapped(void) { S16 a; S16 b; a = 3; b = 9; swap(&a, &b); return a * 10 + b; }

S16 walk(void)
{
    S16 a[5] = {1, 2, 3, 4, 5};
    S16 *p; S16 s;
    s = 0;
    for (p = a; p < a + 5; p++) s = s * 3 + *p;
    return s;
}

S16 pointer_math(void)
{
    S16 a[8] = {10, 11, 12, 13, 14, 15, 16, 17};
    S16 *p; S16 *q;
    p = a + 2;
    q = &a[6];
    return (q - p) * 1000 + *(p + 1) * 100 + p[3] + *(q - 2) + (p < q) * 7 + (q <= p) * 5 + (p == a + 2) * 3;
}

S16 incdec_ptr(void)
{
    S16 a[6] = {1, 2, 3, 4, 5, 6};
    S16 *p; S16 r;
    p = a;
    r = *p++;
    r = r * 10 + *p++;
    r = r * 10 + *++p;
    r = r % 1000;
    p--;
    return r * 10 + *p + (p - a);
}

S16 deref_assign_chain(void)
{
    S16 a[3]; S16 *p;
    p = a;
    *p++ = 4; *p++ = 5; *p = 6;
    return a[0] * 100 + a[1] * 10 + a[2];
}

S16 pp(void)
{
    S16 x; S16 y; S16 *p; S16 **q;
    x = 1; y = 2;
    p = &x; q = &p;
    **q = **q + 40;
    *q = &y;
    **q += 7;
    return x * 100 + y + (*q == &y);
}

S16 bumper(void)
{
    S16 a[3] = {1, 2, 3};
    S16 *p;
    p = a;
    bump(&p); bump(&p);
    return a[0] * 100 + a[1] * 10 + a[2] + (p - a) * 1000;
}

S16 arrays_of_pointers(void)
{
    S16 a; S16 b; S16 c;
    S16 *v[3];
    a = 10; b = 20; c = 30;
    v[0] = &a; v[1] = &b; v[2] = &c;
    *v[1] = *v[0] + *v[2];
    return a + b * 10 + *v[2];
}

S16 longs(void)
{
    S32 x; S32 y; S32 *p;
    x = 70000L; y = 0;
    p = &x;
    set_long(&y, *p / 7);
    *p = *p + y;
    return (S16)(x / 1000) + (S16)(y % 1000);
}

S16 chars(void)
{
    char a[16]; char b[16];
    char hello[] = "hello";
    char help[] = "help";
    str_copy(a, hello);
    str_copy(b, help);
    return str_len(a) * 1000 + str_len(b) * 100 + (str_cmp(a, b) > 0) * 10 + (str_cmp(a, a) == 0);
}

S16 find_ptr(void)
{
    S16 a[5] = {4, 8, 15, 16, 23};
    S16 *p;
    p = find(a, 5, 16);
    if (p == 0) return -1;
    *p = 99;
    return (p - a) * 1000 + a[3] + (find(a, 5, 42) == 0) * 7;
}

S16 max_ptr(void)
{
    S16 a; S16 b; S16 *m;
    a = 3; b = 8;
    m = max_of(&a, &b);
    *m = 0;
    return a * 10 + b;
}

S16 add_to_all(void)
{
    S16 a[4] = {1, 2, 3, 4};
    inc_all(a, 4);
    inc_all(a + 2, 2);
    return a[0] * 1000 + a[1] * 100 + a[2] * 10 + a[3];
}

S16 null_checks(void)
{
    S16 x; S16 *p; S16 *q;
    x = 1; p = &x; q = 0;
    return (p != 0) * 10 + (q == 0) + (!q) * 100 + (p && !q) * 1000;
}

S16 cond(void)
{
    S16 a[2] = {5, 6};
    S16 *p;
    p = a[0] > 3 ? &a[1] : &a[0];
    return *p;
}

S16 zeroing(void)
{
    char buf[6];
    S16 i;
    for (i = 0; i < 6; i = i + 1) buf[i] = i + 1;
    zero(buf + 1, 3);
    return buf[0] * 1000 + buf[1] * 100 + buf[3] * 10 + buf[4] + buf[5];
}

S16 casts(void)
{
    S16 a[2] = {0x1234, 0x5678};
    unsigned char *b;
    b = (unsigned char *)a;
    return b[0] + b[1] * 2 + b[2] * 3 + b[3] * 4;
}

S16 int_ptr(void)
{
    S16 a[4] = {1, 2, 3, 4};
    S16 *p;
    unsigned u;
    p = &a[1];
    u = (unsigned)p - (unsigned)a;
    return u * 10 + (S16)(&a[3] - p);
}

int main()
{
    rec(basic());
    rec(swapped());
    rec(walk());
    rec(pointer_math());
    rec(incdec_ptr());
    rec(deref_assign_chain());
    rec(pp());
    rec(bumper());
    rec(arrays_of_pointers());
    rec(longs());
    rec(chars());
    rec(find_ptr());
    rec(max_ptr());
    rec(add_to_all());
    rec(null_checks());
    rec(cond());
    rec(zeroing());
    rec(casts());
    rec(int_ptr());
    return 0;
}

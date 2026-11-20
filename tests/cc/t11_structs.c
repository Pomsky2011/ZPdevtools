#include "tcc.h"

/* structs and unions: members, nesting, arrays of structs, pointers to structs, member arrays, initializers */

struct P { S16 x; S16 y; };
struct Q { unsigned char tag; S16 v; unsigned char flags; };
struct L { S32 big; S16 small; };
struct N { S16 id; struct P pos; S16 hist[3]; };
struct Node { S16 val; struct Node *next; };
struct Buf { char name[6]; S16 len; };
struct Grid { S16 cell[2][3]; };
union U { S16 w; unsigned char b[2]; };

S16 dist2(struct P *a, struct P *b) { S16 dx; S16 dy; dx = a->x - b->x; dy = a->y - b->y; return dx * dx + dy * dy; }
void move(struct P *p, S16 dx, S16 dy) { p->x += dx; p->y += dy; }
S16 sum_list(struct Node *n) { S16 s; s = 0; while (n) { s = s + n->val; n = n->next; } return s; }
void set_name(struct Buf *b, char *src) { S16 i; i = 0; while (src[i]) { b->name[i] = src[i]; i = i + 1; } b->name[i] = 0; b->len = i; }

S16 basic(void)
{
    struct P p;
    p.x = 3; p.y = 4;
    p.x = p.x * 10 + p.y;
    return p.x * 10 + p.y;
}

S16 mixed_members(void)
{
    struct Q q;
    q.tag = 200; q.v = -300; q.flags = 255;
    q.flags = q.flags + 2;
    q.tag++;
    return q.tag + q.v + q.flags * 7;
}

S16 longs(void)
{
    struct L l;
    l.big = 123456L; l.small = 7;
    l.big = l.big + l.small;
    l.big += 100000L;
    return (S16)(l.big % 1000) + (S16)(l.big / 10000);
}

S16 nested(void)
{
    struct N n;
    S16 i;
    n.id = 1; n.pos.x = 2; n.pos.y = 3;
    for (i = 0; i < 3; i = i + 1) n.hist[i] = n.id + n.pos.x * i + n.pos.y;
    n.pos.x += n.hist[2];
    return n.id + n.pos.x * 10 + n.pos.y * 100 + n.hist[0] * 1000 + n.hist[1] + n.hist[2];
}

S16 pointers(void)
{
    struct P a; struct P b; struct P *p;
    a.x = 1; a.y = 2; b.x = 4; b.y = 6;
    move(&a, 3, 4);
    p = &b;
    p->x *= 2;
    (*p).y -= 1;
    return dist2(&a, &b) * 10 + (p->x == b.x) + a.x + a.y;
}

S16 arrays_of_structs(void)
{
    struct P v[4];
    struct P *p;
    S16 i; S16 s;
    for (i = 0; i < 4; i = i + 1) { v[i].x = i; v[i].y = i * i; }
    s = 0;
    for (p = v; p < v + 4; p++) s = s + p->x * 10 + p->y;
    return s + (v + 3 - v);
}

S16 linked(void)
{
    struct Node a; struct Node b; struct Node c;
    a.val = 1; b.val = 20; c.val = 300;
    a.next = &b; b.next = &c; c.next = 0;
    return sum_list(&a) + a.next->next->val + b.next->val * 2;
}

S16 init_lists(void)
{
    struct P p = {5, 6};
    struct Q q = {65, -2, 3};
    struct N n = {9, {1, 2}, {7, 8}};
    struct P ps[2] = {{1, 2}, {3, 4}};
    struct P pe[2] = {1, 2, 3, 4};
    return p.x + p.y * 10 + q.tag + q.v + n.id * 100 + n.pos.y + n.hist[1] + n.hist[2] + ps[1].x + pe[1].y * 1000;
}

S16 byte_buffer(void)
{
    struct Buf b;
    char hello[] = "hello";
    set_name(&b, hello);
    return b.len * 1000 + b.name[0] + b.name[4] + b.name[5];
}

S16 grid(void)
{
    struct Grid g;
    S16 i; S16 j;
    for (i = 0; i < 2; i = i + 1) for (j = 0; j < 3; j = j + 1) g.cell[i][j] = i * 3 + j;
    return g.cell[1][2] * 100 + g.cell[0][1] * 10 + g.cell[1][0];
}

S16 unions(void)
{
    union U u;
    u.w = 0x1234;
    return u.b[0] * 100 + u.b[1] + (u.w == 0x1234);
}

S16 member_addr(void)
{
    struct P p; S16 *q;
    p.x = 1; p.y = 2;
    q = &p.y;
    *q = 40;
    q = &p.x;
    q[1] += 2;
    return p.x * 100 + p.y;
}

S16 sizes(void)
{
    struct P p;
    struct P a[5];
    return sizeof(p) * 100 + sizeof(a) + sizeof(struct P) + sizeof(p.x);
}

S16 incdec_members(void)
{
    struct P p; S16 r;
    p.x = 5; p.y = 9;
    r = p.x++;
    r = r * 10 + --p.y;
    r = r * 10 + p.x;
    return r;
}

int main()
{
    rec(basic());
    rec(mixed_members());
    rec(longs());
    rec(nested());
    rec(pointers());
    rec(arrays_of_structs());
    rec(linked());
    rec(init_lists());
    rec(byte_buffer());
    rec(grid());
    rec(unions());
    rec(member_addr());
    rec(sizes());
    rec(incdec_members());
    return 0;
}

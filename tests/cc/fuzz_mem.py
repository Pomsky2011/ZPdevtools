#!/usr/bin/env python3
"""Differential fuzzer for def88186cc's arrays, pointers and structs.

Generates random programs over 16-bit and 8-bit arrays, pointer variables (arithmetic, comparison, difference, &),
helper functions that take pointers, and structs reached with '.', '->' and indexing; evaluates them with a Python
model that tracks which object every pointer points into (anything that would be undefined behaviour in C - an
out-of-range access or pointer, a comparison between different objects, an overflow - throws the program away) and
compares what the compiled program records in the emulator.

  tests/cc/fuzz_mem.py [N [seed]]        (default: 100 programs, seed 1;  FUZZ_CC=zpcc fuzzes the clang backend, with FUZZ_SMALL=1)
Needs what tests/cc/run.sh needs.  A failing program is kept in $FUZZ_KEEP (default /tmp/fuzz_mem_fail_<seed>_<n>.c).
"""
import os, random, subprocess, sys, tempfile

here = os.path.dirname(os.path.abspath(__file__))
root = os.path.abspath(os.path.join(here, '..', '..'))
zp_build = os.environ.get('ZP_BUILD', os.path.join(root, '..', 'ZeroPoint', 'build'))
RUN = os.path.join(zp_build, 'bin', 'zp_run_c')
DCC = os.environ.get('DEF88186CC', os.path.join(root, 'c_compiler', 'def88186cc'))
CPUASM = os.path.join(root, 'executables', 'cpuasm')

LO, HI = -32768, 32767
LEN = {'a': 1, 'b': 1, 'c': 1, 'd': 1, 'ar': 8, 'br': 8, 'cr': 8, 's': 1, 't': 1, 'sa': 4}
MASK = {'ar': 7, 'br': 7, 'cr': 7, 'sa': 3}
SCALARS = ['a', 'b', 'c', 'd']


class Skip(Exception):
    """undefined behaviour or out of range: throw this program away"""


class BreakEx(Exception):
    pass


class ContEx(Exception):
    pass


def chk(v):
    if v < LO or v > HI: raise Skip()
    return v


def cdiv(a, b):
    if b == 0: raise Skip()
    q = abs(a) // abs(b)
    return chk(q if (a >= 0) == (b >= 0) else -q)


class Env:
    def __init__(self):
        self.s = {n: 0 for n in SCALARS}                      # S16 scalars
        self.arr = {'ar': [0] * 8, 'br': [0] * 8, 'cr': [0] * 8}
        self.st = {'s': [{'x': 0, 'y': 0, 'c': 0}], 't': [{'x': 0, 'y': 0, 'c': 0}],
                   'sa': [{'x': 0, 'y': 0, 'c': 0} for _ in range(4)]}
        self.ptr = {'p': ('ar', 0), 'q': ('br', 0), 'pc': ('cr', 0), 'ps': ('sa', 0)}
        self.rec = []

    def get(self, obj, idx):
        if not (0 <= idx < LEN[obj]): raise Skip()
        if obj in self.s: return self.s[obj]
        return self.arr[obj][idx]

    def put(self, obj, idx, v):
        if not (0 <= idx < LEN[obj]): raise Skip()
        if obj in self.s: self.s[obj] = v
        else: self.arr[obj][idx] = (v & 0xFF) if obj == 'cr' else v


def evp(P, env, f):
    """pointer expression -> (object, index); one past the end is allowed, nothing beyond"""
    k = P[0]
    if k == 'pvar': return env.ptr[P[1]]
    if k == 'aref': return (P[1], 0)
    if k == 'addrvar': return (P[1], 0)
    if k == 'addr':
        i = ev(P[2], env, f) & MASK[P[1]]
        return (P[1], i)
    if k in ('padd', 'psub'):
        o, i = evp(P[1], env, f)
        i = i + P[2] if k == 'padd' else i - P[2]
        if not (0 <= i <= LEN[o]): raise Skip()
        return (o, i)
    if k == 'pcond':
        return evp(P[2], env, f) if ev(P[1], env, f) != 0 else evp(P[3], env, f)
    raise AssertionError(k)


def psrc(P):
    k = P[0]
    if k == 'pvar': return P[1]
    if k == 'aref': return P[1]
    if k == 'addrvar': return '(&%s)' % P[1]
    if k == 'addr': return '(&%s[(%s) & %d])' % (P[1], src(P[2]), MASK[P[1]])
    if k == 'padd': return '(%s + %d)' % (psrc(P[1]), P[2])
    if k == 'psub': return '(%s - %d)' % (psrc(P[1]), P[2])
    if k == 'pcond': return '(%s ? %s : %s)' % (src(P[1]), psrc(P[2]), psrc(P[3]))
    raise AssertionError(k)


def fld_get(env, SR, fname, f):
    o, i = evsr(SR, env, f)
    return env.st[o][i][fname]


def evsr(SR, env, f):
    k = SR[0]
    if k == 'svar': return (SR[1], 0)
    if k == 'selem': return ('sa', ev(SR[1], env, f) & 3)
    if k == 'sptr':
        o, i = evp(SR[1], env, f)
        if not (0 <= i < LEN[o]): raise Skip()
        return (o, i)
    raise AssertionError(k)


def srsrc(SR, fname):
    k = SR[0]
    if k == 'svar': return '%s.%s' % (SR[1], fname)
    if k == 'selem': return 'sa[(%s) & 3].%s' % (src(SR[1]), fname)
    if k == 'sptr': return '(%s)->%s' % (psrc(SR[1]), fname)
    raise AssertionError(k)


def ev(n, env, f):
    k = n[0]
    if k == 'num': return n[1]
    if k == 'var': return env.s[n[1]]
    if k == 'un':
        v = ev(n[2], env, f)
        return {'-': lambda: chk(-v), '~': lambda: ~v, '!': lambda: int(v == 0)}[n[1]]()
    if k == 'bin':
        op = n[1]
        a = ev(n[2], env, f)
        if op == '&&': return int(a != 0 and ev(n[3], env, f) != 0)
        if op == '||': return int(a != 0 or ev(n[3], env, f) != 0)
        b = ev(n[3], env, f)
        if op == '+': return chk(a + b)
        if op == '-': return chk(a - b)
        if op == '*': return chk(a * b)
        if op == '/': return cdiv(a, b)
        if op == '&': return a & b
        if op == '|': return a | b
        if op == '^': return a ^ b
        return int({'==': a == b, '!=': a != b, '<': a < b, '<=': a <= b, '>': a > b, '>=': a >= b}[op])
    if k == 'tern':
        return ev(n[2], env, f) if ev(n[1], env, f) != 0 else ev(n[3], env, f)
    if k == 'aidx':
        return env.get(n[1], ev(n[2], env, f) & MASK[n[1]])
    if k == 'deref':
        o, i = evp(n[1], env, f)
        return env.get(o, i)
    if k == 'pidx':
        o, i = env.ptr[n[1]]
        j = i + (ev(n[2], env, f) & 3)
        return env.get(o, j)
    if k == 'pcderef':
        o, i = env.ptr['pc']
        return env.get(o, i)
    if k == 'fld':
        return fld_get(env, n[1], n[2], f)
    if k == 'pdiff':
        o1, i1 = evp(n[1], env, f)
        o2, i2 = evp(n[2], env, f)
        if o1 != o2: raise Skip()
        return i1 - i2
    if k == 'pcmp':
        o1, i1 = evp(n[2], env, f)
        o2, i2 = evp(n[3], env, f)
        if o1 != o2: raise Skip()
        return int({'==': i1 == i2, '!=': i1 != i2, '<': i1 < i2, '<=': i1 <= i2, '>': i1 > i2, '>=': i1 >= i2}[n[1]])
    if k == 'sumarr':
        o, i = evp(n[1], env, f)
        s = 0
        for j in range(n[2]):
            s = chk(s + env.get(o, i + j))
        return s
    raise AssertionError(k)



def src(n):
    k = n[0]
    if k == 'num': return str(n[1])
    if k == 'var': return n[1]
    if k == 'un': return '(%s%s)' % (n[1], src(n[2]))
    if k == 'bin': return '(%s %s %s)' % (src(n[2]), n[1], src(n[3]))
    if k == 'tern': return '(%s ? %s : %s)' % (src(n[1]), src(n[2]), src(n[3]))
    if k == 'aidx': return '%s[(%s) & %d]' % (n[1], src(n[2]), MASK[n[1]])
    if k == 'deref': return '(*%s)' % psrc(n[1])
    if k == 'pidx': return '%s[(%s) & 3]' % (n[1], src(n[2]))
    if k == 'pcderef': return '(*pc)'
    if k == 'fld': return srsrc(n[1], n[2])
    if k == 'pdiff': return '(%s - %s)' % (psrc(n[1]), psrc(n[2]))
    if k == 'pcmp': return '(%s %s %s)' % (psrc(n[2]), n[1], psrc(n[3]))
    if k == 'sumarr': return 'sumn(%s, %d)' % (psrc(n[1]), n[2])
    raise AssertionError(k)


class Gen:
    def __init__(self, rng):
        self.r = rng

    # ---- pointer expressions ----
    def P(self, kind, depth=2):
        r = self.r
        if kind == 'S':
            leaves = [('pvar', r.choice(['p', 'q'])), ('aref', r.choice(['ar', 'br'])),
                      ('addr', r.choice(['ar', 'br']), self.E(1, False)), ('addrvar', r.choice(SCALARS))]
        elif kind == 'C':
            leaves = [('pvar', 'pc'), ('aref', 'cr'), ('addr', 'cr', self.E(1, False))]
        else:
            leaves = [('pvar', 'ps'), ('aref', 'sa'), ('addr', 'sa', self.E(1, False)),
                      ('addrvar', r.choice(['s', 't']))]
        if depth <= 0 or r.random() < 0.55: return r.choice(leaves)
        c = r.random()
        if c < 0.4: return ('padd', self.P(kind, depth - 1), r.randint(0, 3))
        if c < 0.7: return ('psub', self.P(kind, depth - 1), r.randint(0, 3))
        return ('pcond', self.E(1, False), self.P(kind, depth - 1), self.P(kind, depth - 1))

    def SR(self):
        r = self.r
        c = r.random()
        if c < 0.35: return ('svar', r.choice(['s', 't']))
        if c < 0.65: return ('selem', self.E(1, False))
        return ('sptr', self.P('T', 1))

    # ---- integer expressions ----
    def E(self, depth, calls=True):
        r = self.r
        if depth <= 0 or r.random() < 0.22:
            c = r.random()
            if c < 0.45: return ('var', r.choice(SCALARS))
            return ('num', r.choice([0, 1, 2, 3, 5, 7, 10, 16, 100, 255]))
        c = r.random()
        if c < 0.12: return ('un', r.choice('-~!'), self.E(depth - 1, calls))
        if c < 0.18: return ('tern', self.E(depth - 1, calls), self.E(depth - 1, calls), self.E(depth - 1, calls))
        if c < 0.34: return ('aidx', r.choice(['ar', 'br', 'cr']), self.E(depth - 1, False))
        if c < 0.44: return ('deref', self.P('S'))
        if c < 0.50: return ('pidx', r.choice(['p', 'q']), self.E(depth - 1, False))
        if c < 0.54: return ('pcderef',)
        if c < 0.64: return ('fld', self.SR(), r.choice(['x', 'y', 'c']))
        if c < 0.68: return ('pdiff', self.P('S'), self.P('S'))
        if c < 0.74: return ('pcmp', r.choice(['==', '!=', '<', '<=', '>', '>=']), self.P('S'), self.P('S'))
        if c < 0.78 and calls: return ('sumarr', self.P('S'), r.randint(1, 3))
        op = r.choice(['+', '-', '*', '/', '&', '|', '^', '==', '!=', '<', '<=', '>', '>=', '&&', '||'])
        return ('bin', op, self.E(depth - 1, calls), self.E(depth - 1, calls))

    # ---- statements ----
    def block(self, depth, n, in_loop=False):
        r = self.r
        out = []
        for _ in range(n):
            c = r.random()
            if c < 0.12: out.append(('assign', r.choice(SCALARS), self.E(3)))
            elif c < 0.20: out.append(('astore', r.choice(['ar', 'br']), self.E(1, False), self.E(3)))
            elif c < 0.25: out.append(('cstore', self.E(1, False), self.E(3)))
            elif c < 0.33: out.append(('dstore', self.P('S'), self.E(3)))
            elif c < 0.37: out.append(('pistore', r.choice(['p', 'q']), self.E(1, False), self.E(3)))
            elif c < 0.45: out.append(('passign', r.choice(['p', 'q']), self.P('S')))
            elif c < 0.50: out.append(('pstep', r.choice(['p', 'q', 'pc', 'ps']), r.choice([1, -1]), r.random() < 0.5))
            elif c < 0.54: out.append(('pcassign', self.P('C')))
            elif c < 0.58: out.append(('psassign', self.P('T')))
            elif c < 0.64: out.append(('dcomp', self.P('S'), r.choice(['+', '-', '*', '^', '|', '&']), self.E(2)))
            elif c < 0.68: out.append(('acomp', r.choice(['ar', 'br']), self.E(1, False), r.choice(['+', '-', '^']), self.E(2)))
            elif c < 0.72: out.append(('dinc', self.P('S'), r.choice([1, -1]), r.random() < 0.5))
            elif c < 0.75: out.append(('ainc', r.choice(['ar', 'br']), self.E(1, False), r.choice([1, -1]), r.random() < 0.5))
            elif c < 0.79: out.append(('fstore', self.SR(), r.choice(['x', 'y', 'c']), self.E(3)))
            elif c < 0.82: out.append(('fcomp', self.SR(), r.choice(['x', 'y']), r.choice(['+', '-', '^']), self.E(2)))
            elif c < 0.84: out.append(('finc', self.SR(), r.choice(['x', 'y', 'c']), r.choice([1, -1]), r.random() < 0.5))
            elif c < 0.87: out.append(('swap', self.P('S'), self.P('S')))
            elif c < 0.89: out.append(('bump', self.P('S')))
            elif c < 0.95: out.append(('rec', self.E(3)))
            elif c < 0.98 and depth > 0:
                out.append(('if', self.E(2), self.block(depth - 1, r.randint(1, 3)), self.block(depth - 1, r.randint(0, 2))))
            elif depth > 0:
                out.append(('for', 'i%d' % depth, r.randint(1, 4), self.block(depth - 1, r.randint(1, 3))))
            else:
                out.append(('rec', ('var', r.choice(SCALARS))))
        return out


def arith(op, a, b):
    if op == '+': return chk(a + b)
    if op == '-': return chk(a - b)
    if op == '*': return chk(a * b)
    if op == '^': return a ^ b
    if op == '|': return a | b
    if op == '&': return a & b
    raise AssertionError(op)


def pstep(env, name, d, f):
    o, i = env.ptr[name]
    i += d
    if not (0 <= i <= LEN[o]): raise Skip()
    env.ptr[name] = (o, i)


def deref_cell(env, P, f):
    o, i = evp(P, env, f)
    if not (0 <= i < LEN[o]): raise Skip()
    return o, i


def run(stmts, env, f):
    for s in stmts:
        k = s[0]
        if k == 'assign': env.s[s[1]] = ev(s[2], env, f)
        elif k == 'astore':
            i = ev(s[2], env, f) & MASK[s[1]]
            v = ev(s[3], env, f)
            env.put(s[1], i, v)
        elif k == 'cstore':
            i = ev(s[1], env, f) & 7
            v = ev(s[2], env, f)
            env.put('cr', i, v)
        elif k == 'dstore':
            v = ev(s[2], env, f)
            o, i = deref_cell(env, s[1], f)
            env.put(o, i, v)
        elif k == 'pistore':
            o, i = env.ptr[s[1]]
            j = i + (ev(s[2], env, f) & 3)
            v = ev(s[3], env, f)
            env.put(o, j, v)
        elif k == 'passign': env.ptr[s[1]] = evp(s[2], env, f)
        elif k == 'pcassign': env.ptr['pc'] = evp(s[1], env, f)
        elif k == 'psassign': env.ptr['ps'] = evp(s[1], env, f)
        elif k == 'pstep': pstep(env, s[1], s[2], f)
        elif k == 'dcomp':
            o, i = deref_cell(env, s[1], f)
            v = ev(s[3], env, f)
            env.put(o, i, arith(s[2], env.get(o, i), v))
        elif k == 'acomp':
            i = ev(s[2], env, f) & MASK[s[1]]
            v = ev(s[4], env, f)
            env.put(s[1], i, arith(s[3], env.get(s[1], i), v))
        elif k == 'dinc':
            o, i = deref_cell(env, s[1], f)
            env.put(o, i, chk(env.get(o, i) + s[2]))
        elif k == 'ainc':
            i = ev(s[2], env, f) & MASK[s[1]]
            env.put(s[1], i, chk(env.get(s[1], i) + s[3]))
        elif k == 'fstore':
            v = ev(s[3], env, f)
            o, i = evsr(s[1], env, f)
            env.st[o][i][s[2]] = (v & 0xFF) if s[2] == 'c' else v
        elif k == 'fcomp':
            o, i = evsr(s[1], env, f)
            v = ev(s[4], env, f)
            env.st[o][i][s[2]] = arith(s[3], env.st[o][i][s[2]], v)
        elif k == 'finc':
            o, i = evsr(s[1], env, f)
            cur = env.st[o][i][s[2]]
            env.st[o][i][s[2]] = ((cur + s[3]) & 0xFF) if s[2] == 'c' else chk(cur + s[3])
        elif k == 'swap':
            o1, i1 = deref_cell(env, s[1], f)
            o2, i2 = deref_cell(env, s[2], f)
            v1, v2 = env.get(o1, i1), env.get(o2, i2)
            env.put(o1, i1, v2)
            env.put(o2, i2, v1)
        elif k == 'bump':
            o, i = deref_cell(env, s[1], f)
            env.put(o, i, chk(env.get(o, i) + 1))
        elif k == 'rec': env.rec.append(ev(s[1], env, f))
        elif k == 'if': run(s[2] if ev(s[1], env, f) != 0 else s[3], env, f)
        elif k == 'for':
            for _ in range(s[2]): run(s[3], env, f)
        else: raise AssertionError(k)
        if len(env.rec) > 300: raise Skip()


FLD_T = {'x': 'S16', 'y': 'S16', 'c': 'unsigned char'}


def block_src(stmts, ind):
    pad = '    ' * ind
    o = []
    for s in stmts:
        k = s[0]
        if k == 'assign': o.append('%s%s = %s;' % (pad, s[1], src(s[2])))
        elif k == 'astore': o.append('%s%s[(%s) & %d] = %s;' % (pad, s[1], src(s[2]), MASK[s[1]], src(s[3])))
        elif k == 'cstore': o.append('%scr[(%s) & 7] = %s;' % (pad, src(s[1]), src(s[2])))
        elif k == 'dstore': o.append('%s*%s = %s;' % (pad, psrc(s[1]), src(s[2])))
        elif k == 'pistore': o.append('%s%s[(%s) & 3] = %s;' % (pad, s[1], src(s[2]), src(s[3])))
        elif k == 'passign': o.append('%s%s = %s;' % (pad, s[1], psrc(s[2])))
        elif k == 'pcassign': o.append('%spc = %s;' % (pad, psrc(s[1])))
        elif k == 'psassign': o.append('%sps = %s;' % (pad, psrc(s[1])))
        elif k == 'pstep':
            op = '++' if s[2] > 0 else '--'
            o.append('%s%s%s;' % (pad, (s[1] + op) if s[3] else (op + s[1]), ''))
        elif k == 'dcomp': o.append('%s*%s %s= %s;' % (pad, psrc(s[1]), s[2], src(s[3])))
        elif k == 'acomp': o.append('%s%s[(%s) & %d] %s= %s;' % (pad, s[1], src(s[2]), MASK[s[1]], s[3], src(s[4])))
        elif k == 'dinc':
            op = '++' if s[2] > 0 else '--'
            o.append('%s(*%s)%s;' % (pad, psrc(s[1]), op) if s[3] else '%s%s(*%s);' % (pad, op, psrc(s[1])))
        elif k == 'ainc':
            op = '++' if s[3] > 0 else '--'
            e = '%s[(%s) & %d]' % (s[1], src(s[2]), MASK[s[1]])
            o.append('%s%s%s;' % (pad, e, op) if s[4] else '%s%s%s;' % (pad, op, e))
        elif k == 'fstore': o.append('%s%s = %s;' % (pad, srsrc(s[1], s[2]), src(s[3])))
        elif k == 'fcomp': o.append('%s%s %s= %s;' % (pad, srsrc(s[1], s[2]), s[3], src(s[4])))
        elif k == 'finc':
            op = '++' if s[3] > 0 else '--'
            e = srsrc(s[1], s[2])
            o.append('%s%s%s;' % (pad, e, op) if s[4] else '%s%s%s;' % (pad, op, e))
        elif k == 'swap': o.append('%sswp(%s, %s);' % (pad, psrc(s[1]), psrc(s[2])))
        elif k == 'bump': o.append('%sbump(%s);' % (pad, psrc(s[1])))
        elif k == 'rec': o.append('%srec(%s);' % (pad, src(s[1])))
        elif k == 'if':
            o.append('%sif (%s) {' % (pad, src(s[1])))
            o += block_src(s[2], ind + 1)
            if s[3]:
                o.append('%s} else {' % pad)
                o += block_src(s[3], ind + 1)
            o.append('%s}' % pad)
        elif k == 'for':
            o.append('%sfor (%s = 0; %s < %d; %s = %s + 1) {' % (pad, s[1], s[1], s[2], s[1], s[1]))
            o += block_src(s[3], ind + 1)
            o.append('%s}' % pad)
    return o


def loopvars(stmts, acc):
    for s in stmts:
        if s[0] == 'if': loopvars(s[2], acc); loopvars(s[3], acc)
        elif s[0] == 'for': acc.add(s[1]); loopvars(s[3], acc)
    return acc


def make_program(rng):
    g = Gen(rng)
    if os.environ.get('FUZZ_SMALL'):                  # zpcc gives every value a stack slot: keep the programs short
        body = g.block(1, rng.randint(2, 4))
    else:
        body = g.block(3, rng.randint(6, 16))
    env = Env()
    for n in SCALARS: env.s[n] = rng.choice([0, 1, 2, 3, 5, 9, -1, -7, 100])
    init_vals = dict(env.s)
    run(body, env, None)
    L = ['#include "tcc.h"', '',
         'struct P { S16 x; S16 y; unsigned char c; };', '',
         'S16 sumn(S16 *a, S16 n) { S16 i; S16 s; s = 0; for (i = 0; i < n; i = i + 1) s = s + a[i]; return s; }',
         'void swp(S16 *a, S16 *b) { S16 t; t = *a; *a = *b; *b = t; }',
         'void bump(S16 *a) { *a = *a + 1; }', '',
         'S16 body(S16 a, S16 b, S16 c, S16 d)', '{']
    lv = sorted(loopvars(body, set()))
    if lv: L.append('    S16 %s;' % ', '.join(lv))
    L += ['    S16 ar[8]; S16 br[8]; unsigned char cr[8];',
          '    struct P s; struct P t; struct P sa[4];',
          '    S16 *p; S16 *q; unsigned char *pc; struct P *ps;',
          '    ar[0] = 0; ar[1] = 0; ar[2] = 0; ar[3] = 0; ar[4] = 0; ar[5] = 0; ar[6] = 0; ar[7] = 0;',
          '    br[0] = 0; br[1] = 0; br[2] = 0; br[3] = 0; br[4] = 0; br[5] = 0; br[6] = 0; br[7] = 0;',
          '    cr[0] = 0; cr[1] = 0; cr[2] = 0; cr[3] = 0; cr[4] = 0; cr[5] = 0; cr[6] = 0; cr[7] = 0;',
          '    s.x = 0; s.y = 0; s.c = 0; t.x = 0; t.y = 0; t.c = 0;',
          '    sa[0].x = 0; sa[0].y = 0; sa[0].c = 0; sa[1].x = 0; sa[1].y = 0; sa[1].c = 0;',
          '    sa[2].x = 0; sa[2].y = 0; sa[2].c = 0; sa[3].x = 0; sa[3].y = 0; sa[3].c = 0;',
          '    p = ar; q = br; pc = cr; ps = sa;']
    L += block_src(body, 1)
    L += ['    return 0;', '}', '', 'int main()', '{',
          '    body(%s);' % ', '.join(str(init_vals[n]) for n in SCALARS), '    return 0;', '}']
    return '\n'.join(L) + '\n', env.rec


def run_one(text, work, name):
    c = os.path.join(work, name + '.c'); asm = os.path.join(work, name + '.asm')
    img = os.path.join(work, name + '.img.asm'); binf = os.path.join(work, name + '.bin')
    open(c, 'w').write(text)
    if os.environ.get('FUZZ_CC') == 'zpcc':      # fuzz the clang backend instead
        r = subprocess.run([os.path.join(root, 'llvm_compiler', 'zpcc'), '-I', here, '-I', os.path.join(root, 'include'), c, '-o', asm],
                           capture_output=True, text=True, timeout=120)
    else:
        r = subprocess.run([DCC, '-DZP_CC_DOS', '-I', here, c, '-o', asm], capture_output=True, text=True, timeout=60)
    if r.returncode != 0 or not os.path.exists(asm):
        if 'bytes of stack frame' in (r.stdout + r.stderr): return None, 'TOOBIG'
        return None, 'compile failed: ' + (r.stdout + r.stderr)[-500:]
    with open(img, 'w') as f:
        f.write('.org $0200\n.bank $80\n')
        f.write('.include "%s"\n' % os.path.join(root, 'lib/crt0_entry.asm'))
        f.write('.include "%s"\n.include "%s"\n' % (asm, os.path.join(here, 'rec.asm')))
        f.write('.include "%s"\n.include "%s"\n' % (os.path.join(root, 'lib/zp_mmio.asm'), os.path.join(root, 'lib/zp_cop.asm')))
        if os.path.exists(os.path.join(root, 'lib/zp_long.asm')): f.write('.include "%s"\n' % os.path.join(root, 'lib/zp_long.asm'))
    r = subprocess.run([CPUASM, img, binf], capture_output=True, text=True, timeout=60)
    if r.returncode != 0: return None, 'assemble failed: ' + (r.stdout + r.stderr)[-300:]
    r = subprocess.run([RUN, 'none', binf], capture_output=True, text=True, timeout=120)
    lines = [l for l in r.stdout.strip().split('\n') if l]
    return (lines[-1].strip() if lines else ''), None


def main():
    n = int(sys.argv[1]) if len(sys.argv) > 1 else 100
    seed = int(sys.argv[2]) if len(sys.argv) > 2 else 1
    rng = random.Random(seed)
    work = tempfile.mkdtemp()
    done = fails = tries = toobig = 0
    while done < n and tries < n * 200:
        tries += 1
        try:
            text, rec = make_program(rng)
        except Skip:
            continue
        if not rec: continue
        done += 1
        want = ' '.join(str(v) for v in rec)
        got, err = run_one(text, work, 'p')
        if err == 'TOOBIG':
            toobig += 1
            done -= 1
            continue
        if err or got != want:
            fails += 1
            keep = os.environ.get('FUZZ_KEEP', '/tmp/fuzz_mem_fail_%d_%d.c' % (seed, done))
            open(keep, 'w').write(text)
            print('FAIL #%d (%s): %s\n  want: %s\n  got : %s' % (done, keep, err or 'wrong result', want[:200], (got or '')[:200]))
    print('%d programs, %d failed (seed %d, %d tried)%s' % (done, fails, seed, tries, (', %d skipped: frame over 255 bytes' % toobig) if toobig else ''))
    sys.exit(1 if fails else 0)


if __name__ == '__main__':
    main()

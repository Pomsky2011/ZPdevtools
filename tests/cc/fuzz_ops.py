#!/usr/bin/env python3
"""Differential fuzzer for def88186cc's code generator.

Generates random programs in the supported subset (16-bit int arithmetic, comparisons, logic, ternary, assignment,
if/else, for, calls), evaluates them with a reference interpreter that follows C semantics, runs the compiled
program in the emulator and compares what rec() recorded.  Every intermediate value is kept inside the 16-bit
range, so the answer is the same on any C implementation and a mismatch is a miscompile.

  tests/cc/fuzz_ops.py [N [seed]]        (default: 100 programs, seed 1)
Needs the same things as tests/cc/run.sh (zp_run_c, def88186cc, cpuasm).  A failing program is kept in
$FUZZ_KEEP (default /tmp/fuzz_ops_fail_<seed>_<n>.c).
"""
import os, random, subprocess, sys, tempfile

here = os.path.dirname(os.path.abspath(__file__))
root = os.path.abspath(os.path.join(here, '..', '..'))
zp_build = os.environ.get('ZP_BUILD', os.path.join(root, '..', 'ZeroPoint', 'build'))
RUN = os.path.join(zp_build, 'bin', 'zp_run_c')
DCC = os.environ.get('DEF88186CC', os.path.join(root, 'c_compiler', 'def88186cc'))
CPUASM = os.path.join(root, 'executables', 'cpuasm')

LO, HI = -32768, 32767
MODE = {'uns': False}        # an unsigned 16-bit program: values 0..65535, rec() sees them as (S16)

def to_s16(v):
    v &= 0xFFFF
    return v - 65536 if v >= 32768 else v

class BreakEx(Exception):
    pass

class ContEx(Exception):
    pass

class Skip(Exception):
    """this program leaves the safe range (overflow, divide by zero, ...): throw it away"""

def chk(v):
    lo, hi = (0, 65535) if MODE['uns'] else (LO, HI)
    if v < lo or v > hi: raise Skip()
    return v

def cdiv(a, b):
    if b == 0: raise Skip()
    q = abs(a) // abs(b)
    return chk(q if (a >= 0) == (b >= 0) else -q)

def cmod(a, b):
    if b == 0: raise Skip()
    return a - cdiv(a, b) * b

BIN = ['+', '-', '*', '/', '%', '&', '|', '^', '<<', '>>', '==', '!=', '<', '<=', '>', '>=', '&&', '||']

def ev(n, env, funcs):
    k = n[0]
    if k == 'num': return n[1]
    if k == 'var': return env[n[1]]
    if k == 'un':
        v = ev(n[2], env, funcs)
        return {'-': lambda: chk(-v), '~': lambda: ~v, '!': lambda: int(v == 0)}[n[1]]()
    if k == 'bin':
        op = n[1]
        a = ev(n[2], env, funcs)
        if op == '&&': return int(a != 0 and ev(n[3], env, funcs) != 0)
        if op == '||': return int(a != 0 or ev(n[3], env, funcs) != 0)
        b = ev(n[3], env, funcs)
        if op == '+': return chk(a + b)
        if op == '-': return chk(a - b)
        if op == '*': return chk(a * b)
        if op == '/': return cdiv(a, b)
        if op == '%': return cmod(a, b)
        if op == '&': return a & b
        if op == '|': return a | b
        if op == '^': return a ^ b
        if op == '<<':
            if a < 0 or b < 0 or b > 14: raise Skip()
            return chk(a << b)
        if op == '>>':
            if a < 0 or b < 0 or b > 15: raise Skip()
            return a >> b
        return int({'==': a == b, '!=': a != b, '<': a < b, '<=': a <= b, '>': a > b, '>=': a >= b}[op])
    if k == 'tern':
        return ev(n[2], env, funcs) if ev(n[1], env, funcs) != 0 else ev(n[3], env, funcs)
    if k == 'idx':
        return env['ar'][ev(n[1], env, funcs) & 3]
    if k == 'call':
        args = [ev(a, env, funcs) for a in n[2]]
        return run_func(funcs[n[1]], args, funcs)
    raise AssertionError(k)

def src(n):
    k = n[0]
    if k == 'num': return str(n[1])
    if k == 'var': return n[1]
    if k == 'un': return '(%s%s)' % (n[1], src(n[2]))
    if k == 'bin': return '(%s %s %s)' % (src(n[2]), n[1], src(n[3]))
    if k == 'tern': return '(%s ? %s : %s)' % (src(n[1]), src(n[2]), src(n[3]))
    if k == 'idx': return 'ar[(%s) & 3]' % src(n[1])
    if k == 'call': return '%s(%s)' % (n[1], ', '.join(src(a) for a in n[2]))
    raise AssertionError(k)

class Gen:
    def __init__(self, rng):
        self.r = rng
        self.arrays = False

    def expr(self, vars_, depth, funcs=()):
        r = self.r
        if depth <= 0 or r.random() < 0.25:
            if self.arrays and vars_ and r.random() < 0.15: return ('idx', ('var', r.choice(vars_)))
            if vars_ and r.random() < 0.6: return ('var', r.choice(vars_))
            if MODE['uns']: return ('num', r.choice([0, 1, 2, 3, 5, 7, 10, 16, 100, 255, 1000, 32767, 32768, 40000, 65535]))
            return ('num', r.choice([0, 1, 2, 3, 5, 7, 10, 16, 100, 255, 1000]))
        c = r.random()
        if c < 0.10: return ('un', '!' if MODE['uns'] else r.choice('-~!'), self.expr(vars_, depth - 1, funcs))
        if c < 0.17: return ('tern', self.expr(vars_, depth - 1, funcs), self.expr(vars_, depth - 1, funcs), self.expr(vars_, depth - 1, funcs))
        if c < 0.24 and funcs:
            f = r.choice(funcs)
            return ('call', f[0], [self.expr(vars_, depth - 1, funcs) for _ in range(f[1])])
        return ('bin', r.choice(BIN), self.expr(vars_, depth - 1, funcs), self.expr(vars_, depth - 1, funcs))

    def block(self, vars_, depth, funcs, nstmt, in_loop=False):
        r = self.r
        out = []
        assignable = [v for v in vars_ if v[0] not in 'iw']
        for _ in range(nstmt):
            c = r.random()
            if c < 0.30:
                out.append(('assign', r.choice(assignable), self.expr(vars_, 3, funcs)))
            elif c < 0.40 and self.arrays:
                out.append(('aassign', self.expr(vars_, 2, funcs), self.expr(vars_, 3, funcs)))
            elif c < 0.52:
                out.append(('rec', self.expr(vars_, 3, funcs)))
            elif c < 0.62 and depth > 0:
                out.append(('if', self.expr(vars_, 2, funcs), self.block(vars_, depth - 1, funcs, r.randint(1, 3), in_loop),
                            self.block(vars_, depth - 1, funcs, r.randint(0, 2), in_loop)))
            elif c < 0.70 and depth > 0:
                iv = 'i%d' % depth
                out.append(('for', iv, r.randint(0, 5), self.block(vars_ + [iv], depth - 1, funcs, r.randint(1, 3), True)))
            elif c < 0.77 and depth > 0:
                wv = 'w%d' % depth
                out.append(('while', wv, r.randint(1, 5), self.block(vars_ + [wv], depth - 1, funcs, r.randint(1, 3), True)))
            elif c < 0.82 and depth > 0:
                dv = 'w%d' % depth
                out.append(('dowhile', dv, r.randint(1, 4), self.block(vars_ + [dv], depth - 1, funcs, r.randint(1, 3), True)))
            elif c < 0.88 and depth > 0:
                ncase = r.randint(1, 3)
                cases = [(k, self.block(vars_, depth - 1, funcs, r.randint(1, 2), False)) for k in r.sample(range(4), ncase)]
                out.append(('switch', self.expr(vars_, 2, funcs), cases, self.block(vars_, depth - 1, funcs, r.randint(0, 2), False)))
            elif c < 0.94 and in_loop:
                out.append((r.choice(['brk', 'cont']), self.expr(vars_, 2, funcs)))
            else:
                out.append(('rec', ('var', r.choice(vars_))))
        return out

def run_loop_body(body, env, funcs, rec):
    """one iteration; returns True if the loop must stop (break)"""
    try:
        exec_block(body, env, funcs, rec)
    except ContEx:
        pass
    except BreakEx:
        return True
    return False

def exec_block(stmts, env, funcs, rec):
    for s in stmts:
        k = s[0]
        if k == 'assign': env[s[1]] = ev(s[2], env, funcs)
        elif k == 'aassign':
            i = ev(s[1], env, funcs) & 3
            env['ar'][i] = ev(s[2], env, funcs)
        elif k == 'rec': rec.append(to_s16(ev(s[1], env, funcs)) if MODE['uns'] else ev(s[1], env, funcs))
        elif k == 'if':
            exec_block(s[2] if ev(s[1], env, funcs) != 0 else s[3], env, funcs, rec)
        elif k == 'for':
            env[s[1]] = 0
            while env[s[1]] < s[2]:
                if run_loop_body(s[3], env, funcs, rec): break
                env[s[1]] += 1
        elif k == 'while':
            env[s[1]] = 0
            while env[s[1]] < s[2]:
                env[s[1]] += 1
                if run_loop_body(s[3], env, funcs, rec): break
        elif k == 'dowhile':
            env[s[1]] = 0
            while True:
                env[s[1]] += 1
                if run_loop_body(s[3], env, funcs, rec): break
                if not env[s[1]] < s[2]: break
        elif k == 'switch':
            v = ev(s[1], env, funcs) & 3
            body = s[3]
            for cv, cb in s[2]:
                if cv == v: body = cb; break
            exec_block(body, env, funcs, rec)
        elif k == 'brk':
            if ev(s[1], env, funcs) != 0: raise BreakEx()
        elif k == 'cont':
            if ev(s[1], env, funcs) != 0: raise ContEx()
        if len(rec) > 400: raise Skip()

def run_func(f, args, funcs):
    name, nparams, params, locals_, body, ret = f
    env = dict(zip(params, args))
    for l in locals_: env[l] = 0
    rec = []
    exec_block(body, env, funcs, rec)         # recs inside helpers are not allowed (helpers only compute)
    return ev(ret, env, funcs)

def block_src(stmts, ind):
    pad = '    ' * ind
    out = []
    for s in stmts:
        k = s[0]
        if k == 'assign': out.append('%s%s = %s;' % (pad, s[1], src(s[2])))
        elif k == 'aassign': out.append('%sar[(%s) & 3] = %s;' % (pad, src(s[1]), src(s[2])))
        elif k == 'rec': out.append('%srec(%s);' % (pad, ('(S16)(%s)' % src(s[1])) if MODE['uns'] else src(s[1])))
        elif k == 'if':
            out.append('%sif (%s) {' % (pad, src(s[1])))
            out += block_src(s[2], ind + 1)
            if s[3]:
                out.append('%s} else {' % pad)
                out += block_src(s[3], ind + 1)
            out.append('%s}' % pad)
        elif k == 'for':
            out.append('%sfor (%s = 0; %s < %d; %s = %s + 1) {' % (pad, s[1], s[1], s[2], s[1], s[1]))
            out += block_src(s[3], ind + 1)
            out.append('%s}' % pad)
        elif k == 'while':
            out.append('%s%s = 0;' % (pad, s[1]))
            out.append('%swhile (%s < %d) {' % (pad, s[1], s[2]))
            out.append('%s    %s = %s + 1;' % (pad, s[1], s[1]))
            out += block_src(s[3], ind + 1)
            out.append('%s}' % pad)
        elif k == 'dowhile':
            out.append('%s%s = 0;' % (pad, s[1]))
            out.append('%sdo {' % pad)
            out.append('%s    %s = %s + 1;' % (pad, s[1], s[1]))
            out += block_src(s[3], ind + 1)
            out.append('%s} while (%s < %d);' % (pad, s[1], s[2]))
        elif k == 'switch':
            out.append('%sswitch ((%s) & 3) {' % (pad, src(s[1])))
            for cv, cb in s[2]:
                out.append('%scase %d:' % (pad, cv))
                out += block_src(cb, ind + 1)
                out.append('%s    break;' % pad)
            out.append('%sdefault:' % pad)
            out += block_src(s[3], ind + 1)
            out.append('%s    break;' % pad)
            out.append('%s}' % pad)
        elif k == 'brk': out.append('%sif (%s) break;' % (pad, src(s[1])))
        elif k == 'cont': out.append('%sif (%s) continue;' % (pad, src(s[1])))
    return out

def loopvars(stmts, acc):
    for s in stmts:
        if s[0] == 'if': loopvars(s[2], acc); loopvars(s[3], acc)
        elif s[0] in ('for', 'while', 'dowhile'): acc.add(s[1]); loopvars(s[3], acc)
        elif s[0] == 'switch':
            for cv, cb in s[2]: loopvars(cb, acc)
            loopvars(s[3], acc)
    return acc

def make_program(rng):
    MODE['uns'] = rng.random() < 0.35
    TY = 'U16' if MODE['uns'] else 'S16'
    g = Gen(rng)
    funcs = {}
    flist = []
    for fi in range(rng.randint(0, 3)):
        np_ = rng.randint(1, 3)
        params = ['p%d' % i for i in range(np_)]
        locals_ = ['t0', 't1']
        # helpers: straight-line, no loops/recs (keeps run_func simple), may call earlier helpers
        body = [('assign', rng.choice(locals_), g.expr(params + locals_, 3, tuple(flist))) for _ in range(rng.randint(1, 3))]
        ret = g.expr(params + locals_, 3, tuple(flist))
        name = 'h%d' % fi
        funcs[name] = (name, np_, params, locals_, body, ret)
        flist.append((name, np_))
    mvars = ['a', 'b', 'c', 'd']
    g.arrays = bool(os.environ.get('FUZZ_ARRAYS')) and rng.random() < 0.5   # arrays are refused by def88186cc for now
    body = g.block(mvars, 3, tuple(flist), rng.randint(4, 12))
    init = [rng.choice([0, 1, 2, 3, 5, 9, 17, 100, 32767, 32768, 40000, 65535] if MODE['uns'] else [0, 1, 2, 3, 5, 9, 17, 100, -1, -7, -100]) for _ in mvars]
    env = dict(zip(mvars, init))
    for lv in loopvars(body, set()): env[lv] = 0
    env['ar'] = [0, 0, 0, 0]
    rec = []
    exec_block(body, env, funcs, rec)
    lines = ['#include "tcc.h"', '']
    for name, np_, params, locals_, hbody, ret in funcs.values():
        lines.append('%s %s(%s)' % (TY, name, ', '.join(TY + ' ' + p for p in params)))
        lines.append('{')
        lines.append('    %s %s;' % (TY, ', '.join(locals_)))
        lines.append('    ' + ' '.join('%s = 0;' % l for l in locals_))
        lines += block_src(hbody, 1)
        lines.append('    return %s;' % src(ret))
        lines.append('}')
        lines.append('')
    lines.append('S16 body(%s a, %s b, %s c, %s d)' % (TY, TY, TY, TY))
    lines.append('{')
    lv = sorted(loopvars(body, set()))
    if lv: lines.append('    S16 %s;' % ', '.join(lv))
    if g.arrays:
        lines.append('    S16 ar[4];')
        lines.append('    ar[0] = 0; ar[1] = 0; ar[2] = 0; ar[3] = 0;')
    lines += block_src(body, 1)
    lines.append('    return 0;')
    lines.append('}')
    lines.append('')
    lines.append('int main()')
    lines.append('{')
    lines.append('    body(%s);' % ', '.join(str(v) for v in init))
    lines.append('    return 0;')
    lines.append('}')
    return '\n'.join(lines) + '\n', rec

def run_one(text, work, name):
    c = os.path.join(work, name + '.c'); asm = os.path.join(work, name + '.asm')
    img = os.path.join(work, name + '.img.asm'); binf = os.path.join(work, name + '.bin')
    open(c, 'w').write(text)
    r = subprocess.run([DCC, '-DZP_CC_DOS', '-I', here, c, '-o', asm], capture_output=True, text=True, timeout=60)
    if r.returncode != 0 or not os.path.exists(asm):
        return None, 'compile failed: ' + (r.stdout + r.stderr)[-400:]
    with open(img, 'w') as f:
        f.write('.org $0200\n.bank $80\n')
        for inc in ['lib/crt0_entry.asm']: f.write('.include "%s"\n' % os.path.join(root, inc))
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
    done = fails = 0
    tries = 0
    while done < n and tries < n * 50:
        tries += 1
        try:
            text, rec = make_program(rng)
        except Skip:
            continue
        done += 1
        if not rec: done -= 1; continue
        want = ''.join('%d ' % v for v in rec).strip()
        got, err = run_one(text, work, 'p')
        if err or got != want:
            fails += 1
            keep = os.environ.get('FUZZ_KEEP', '/tmp/fuzz_ops_fail_%d_%d.c' % (seed, done))
            open(keep, 'w').write(text)
            print('FAIL #%d (%s): %s\n  want: %s\n  got : %s' % (done, keep, err or 'wrong result', want[:160], (got or '')[:160]))
    print('%d programs, %d failed (seed %d)' % (done, fails, seed))
    sys.exit(1 if fails else 0)

if __name__ == '__main__':
    main()

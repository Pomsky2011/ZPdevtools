#!/usr/bin/env python3
"""Preprocessor tests for def88186cc (-E).  Each positive case gives the exact expected output (blank lines and
trailing blanks ignored); each negative case must exit non-zero with a diagnostic and must not crash.

  tests/cc/pp_tests.py [path/to/def88186cc]      (default: ../c_compiler/def88186cc, or $DEF88186CC)
Run it against a sanitizer build to check memory safety too:
  gcc -g -fsanitize=address,undefined -I c_compiler -o /tmp/dcc_asan c_compiler/{main,ast,codegen,preprocessor,lexer,parser.tab}.c
  ASAN_OPTIONS=detect_leaks=0 tests/cc/pp_tests.py /tmp/dcc_asan
"""
import os, subprocess, sys, tempfile

here = os.path.dirname(os.path.abspath(__file__))
dcc = sys.argv[1] if len(sys.argv) > 1 else os.environ.get('DEF88186CC', os.path.join(here, '..', '..', 'c_compiler', 'def88186cc'))

def run(src, args=(), files=None):
    with tempfile.TemporaryDirectory() as d:
        for name, text in (files or {}).items():
            with open(os.path.join(d, name), 'w') as f: f.write(text)
        p = os.path.join(d, 'in.c')
        with open(p, 'w') as f: f.write(src)
        r = subprocess.run([dcc, '-E'] + list(args) + [p], capture_output=True, text=True, timeout=30, cwd=d)
        return r

def norm(s):
    return [' '.join(l.split()) for l in s.split('\n') if l.strip()]

OK = []      # (name, source, expected output lines, extra args, extra files)
FAIL = []    # (name, source, message fragment, extra files)

def ok(name, src, expect, args=(), files=None): OK.append((name, src, expect, args, files))
def bad(name, src, frag='', files=None): FAIL.append((name, src, frag, files))

ok('object macro', '#define X 5\nint y = X;\n', ['int y = 5;'])
ok('no expansion in strings', '#define X 5\nchar *s = "X"; char c = \'X\'; int y = X;\n', ['char *s = "X"; char c = \'X\'; int y = 5;'])
ok('no expansion in comments', '#define X 5\n/* X */ int y = X; /* X\nX */\n', ['int y = 5;'])
ok('escaped quote in string', '#define X 5\nchar *s = "a\\"X\\"b"; int y = X;\n', ['char *s = "a\\"X\\"b"; int y = 5;'])
ok('self reference stops', '#define foo foo\nint foo;\n', ['int foo;'])
ok('mutual recursion stops', '#define A B\n#define B A\nA B\n', ['A B'])
ok('function-like', '#define ADD(a,b) ((a)+(b))\nint y = ADD(1, ADD(2,3));\n', ['int y = ((1)+(((2)+(3))));'])
ok('commas in parens/strings', '#define F(x) [x]\nF((1,2)) F("a,b")\n', ['[(1,2)] ["a,b"]'])
ok('name without parens is plain', '#define F(x) x\nint F;\n', ['int F;'])
ok('zero-arg macro', '#define Z() 7\nint y = Z();\n', ['int y = 7;'])
ok('macro in arg expanded once', '#define N 3\n#define S(x) x+x\nS(N)\n', ['3+3'])
ok('number not an identifier', '#define xFF 9\nint a = 0xFF; int b = 1e5;\n', ['int a = 0xFF; int b = 1e5;'])
ok('undef', '#define X 1\n#undef X\nX\n', ['X'])
ok('redefine same is silent', '#define X 1\n#define X 1\nX\n', ['1'])
ok('cli define', 'X\n', ['3'], args=['-DX=3'])
ok('cli define default 1', 'X\n', ['1'], args=['-DX'])
ok('line splice', '#define X 1 + \\\n 2\nint y = X;\n', ['int y = 1 + 2;'])
ok('comments become space', 'a/**/b // gone\nc\n', ['a b', 'c'])
ok('__LINE__', '\n\nint l = __LINE__;\n', ['int l = 3;'])
ok('__FILE__ is quoted', 'char *f = __FILE__;\n', None)   # checked below
ok('__STDC__', 'int s = __STDC__;\n', ['int s = 1;'])
ok('#line', '#line 100\nint l = __LINE__;\n', ['int l = 100;'])
ok('elif chain takes first true', '#if 1\nA\n#elif 1\nB\n#else\nC\n#endif\n', ['A'])
ok('elif after false', '#if 0\nA\n#elif 1\nB\n#else\nC\n#endif\n', ['B'])
ok('else', '#if 0\nA\n#elif 0\nB\n#else\nC\n#endif\n', ['C'])
ok('nested skipped', '#if 0\n#if 1\nx\n#else\ny\n#endif\n#else\nz\n#endif\n', ['z'])
ok('nested in taken', '#if 1\n#if 0\nx\n#else\ny\n#endif\n#endif\n', ['y'])
ok('ifdef/ifndef', '#define A\n#ifdef A\na\n#endif\n#ifndef A\nb\n#endif\n#ifdef B\nc\n#else\nd\n#endif\n', ['a', 'd'])
ok('#if uses macro values', '#define V 3\n#if V > 2 && (V*2 == 6) && !defined(NOPE)\nyes\n#else\nno\n#endif\n', ['yes'])
ok('#if arithmetic/precedence', '#if 2+3*4 == 14 && (1<<4) == 16 && 7%4 == 3 && -1 < 0 && ~0 == -1\nok\n#endif\n', ['ok'])
ok('#if ternary and char', "#if (1 ? 2 : 3) == 2 && 'A' == 65 && '\\n' == 10 && 0x1F == 31 && 017 == 15\nok\n#endif\n", ['ok'])
ok('#if undefined identifier is 0', '#if FOO\nno\n#else\nyes\n#endif\n', ['yes'])
ok('#if defined forms', '#define A 0\n#if defined A && defined(A) && !defined B\nok\n#endif\n', ['ok'])
ok('#if short circuit skips div by zero', '#if 0 && (1/0)\nno\n#else\nyes\n#endif\n', ['yes'])
ok('#if suffixes', '#if 10UL == 10 && 3L == 3\nok\n#endif\n', ['ok'])
ok('directives in skipped region ignored', '#if 0\n#error nope\n#bogus\n#define X 1\n#endif\nX\n', ['X'])
ok('pragma ignored', '#pragma once\nint x;\n', ['int x;'])
ok('include quoted', '#include "h.h"\nint y = H;\n', ['int y = 4;'], files={'h.h': '#define H 4\n'})
ok('include guard', '#include "g.h"\n#include "g.h"\nint y = G;\n', ['int g_once;', 'int y = 8;'], files={'g.h': '#ifndef G_H\n#define G_H\n#define G 8\nint g_once;\n#endif\n'})
ok('computed include', '#define HDR "h.h"\n#include HDR\nH\n', ['4'], files={'h.h': '#define H 4\n'})
ok('long but legal', '#define X 1\n' + ('X ' * 3000) + '\n', [' '.join(['1'] * 3000)])
ok('deep macro chain', ''.join('#define M%d M%d\n' % (i, i + 1) for i in range(40)) + '#define M40 ok\nM0\n', ['ok'])

bad('#error', '#error stop here\nint x;\n', 'stop here')
bad('unterminated #if', '#if 1\nint x;\n', 'unterminated')
bad('unterminated #ifdef in include', '#include "u.h"\nint x;\n', 'unterminated', files={'u.h': '#ifdef X\n'})
bad('#endif without #if', '#endif\n', '#endif without')
bad('#else without #if', '#else\n', '#else without')
bad('#else twice', '#if 1\n#else\n#else\n#endif\n', '#else after')
bad('#elif after #else', '#if 0\n#else\n#elif 1\n#endif\n', '#elif after')
bad('unknown directive', '#bogus\n', 'invalid preprocessing directive')
bad('missing include', '#include "no_such_file.h"\n', 'cannot open')
bad('bad include syntax', '#include foo\n', '#include expects')
bad('self include does not crash', '#include "in.c"\n', 'nested')
bad('self include fan-out terminates', '#include "in.c"\n#include "in.c"\n#include "in.c"\n', 'more than')
bad('wrong arg count', '#define F(a,b) a\nF(1)\n', 'requires 2')
bad('too many args', '#define F(a) a\nF(1,2)\n', 'requires 1')
bad('unterminated arg list', '#define F(a) a\nF(1\n', 'unterminated')
bad('stringize', '#define S(x) #x\nS(a)\n', 'not supported')
bad('paste', '#define P(a,b) a##b\nP(x,y)\n', 'not supported')
bad('variadic', '#define V(...) 1\n', 'variadic')
bad('duplicate param', '#define F(a,a) a\n', 'duplicate')
bad('bad param list', '#define F(a b) a\n', 'expected')
bad('define no name', '#define\n', 'macro name missing')
bad('define defined', '#define defined 1\n', 'cannot be defined')
bad('#if empty', '#if\n#endif\n', 'no expression')
bad('#if division by zero', '#if 1/0\n#endif\n', 'division by zero')
bad('#if trailing operator', '#if 1 +\n#endif\n', 'in #if expression')
bad('#if garbage', '#if 1 2\n#endif\n', 'extra tokens')
bad('#if unbalanced paren', '#if (1\n#endif\n', "missing ')'")
bad('#if function call', '#if foo(1)\n#endif\n', 'function call')
bad('#if bad number', '#if 12abc\n#endif\n', 'invalid')
bad('#if deeply nested', '#if ' + '(' * 5000 + '1' + ')' * 5000 + '\n#endif\n', 'nested too deeply')
bad('#if unary chain', '#if ' + '!' * 5000 + '1\n#endif\n', 'nested too deeply')
bad('#if nested too deep', '#if 1\n' * 300 + '#endif\n' * 300, 'nested more than')
bad('line too long', 'x' * 40000 + '\n', 'too long')
bad('expansion blows up', '#define A B B B B B B B B\n#define B C C C C C C C C\n#define C D D D D D D D D\n#define D E E E E E E E E\n#define E F F F F F F F F\n#define F 1 1 1 1 1 1 1 1\nA\n', 'too long')
bad('unterminated comment', '/* never ends\nint x;\n', 'unterminated comment')
bad('bad #line', '#line abc\n', '#line needs')

fails = 0
for name, src, expect, args, files in OK:
    if expect is None and name == '__FILE__ is quoted':
        r = run(src, args, files)
        good = r.returncode == 0 and r.stdout.strip().startswith('char *f = "') and r.stdout.strip().endswith('in.c";')
    elif expect is None:
        continue
    else:
        r = run(src, args, files)
        good = r.returncode == 0 and norm(r.stdout) == [' '.join(l.split()) for l in expect]
    if not good:
        fails += 1
        print('FAIL %s\n  rc=%d\n  out=%r\n  err=%r' % (name, r.returncode, r.stdout[:300], r.stderr[-300:]))
for name, src, frag, files in FAIL:
    r = run(src, (), files)
    crashed = r.returncode < 0 or 'AddressSanitizer' in r.stderr or 'runtime error' in r.stderr
    good = r.returncode != 0 and not crashed and frag in r.stderr
    if not good:
        fails += 1
        print('FAIL(neg) %s\n  rc=%d crashed=%s\n  err=%r' % (name, r.returncode, crashed, r.stderr[-300:]))
total = len([o for o in OK if o[2] is not None or o[0] == '__FILE__ is quoted']) + len(FAIL)
print('%d passed, %d failed' % (total - fails, fails))
sys.exit(1 if fails else 0)

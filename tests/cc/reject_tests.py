#!/usr/bin/env python3
"""def88186cc must never exit 0 (or leave an .asm behind) for a program it cannot compile correctly.
Every case here is outside the supported subset (c_compiler/README.md, "Supported subset") or plain wrong; each
must be refused with a non-zero exit, a diagnostic, no crash and no output file.  When a feature is implemented,
move its case to the positive checks in tests/cc/run.sh.

  tests/cc/reject_tests.py [path/to/def88186cc]      (default: ../c_compiler/def88186cc, or $DEF88186CC)
"""
import os, subprocess, sys, tempfile

here = os.path.dirname(os.path.abspath(__file__))
dcc = sys.argv[1] if len(sys.argv) > 1 else os.environ.get('DEF88186CC', os.path.join(here, '..', '..', 'c_compiler', 'def88186cc'))

def compile_(src):
    with tempfile.TemporaryDirectory() as d:
        p = os.path.join(d, 'in.c'); out = os.path.join(d, 'out.asm')
        with open(p, 'w') as f: f.write(src)
        r = subprocess.run([dcc, p, '-o', out], capture_output=True, text=True, timeout=30, cwd=d)
        asm = open(out).read() if os.path.exists(out) else None
        return r, asm

REJECT = [
  # (name, source, fragment expected in stderr)
  ('float type',            'int main(){ float x; return 0; }', 'floating-point'),
  ('double type',           'int main(){ double x; return 0; }', 'floating-point'),
  ('float constant',        'int main(){ return 1.5; }', 'floating-point'),
  ('float exponent',        'int main(){ return 1e3; }', 'floating-point'),
  ('invalid octal',         'int main(){ return 089; }', 'octal'),
  ('integer too large',     'int main(){ return 4294967296; }', 'too large'),
  ('undefined variable',    'int main(){ return nope; }', 'Undefined variable'),
  ('assign undefined',      'int main(){ nope = 1; return 0; }', 'Undefined variable'),
  ('subscript of a scalar',  'int main(){ int x; return x[1]; }', 'neither an array nor a pointer'),
  ('deref of a scalar',      'int main(){ int x; return *x; }', 'not a pointer'),
  ('void deref',             'int main(){ void *p; return *p; }', 'void pointer'),
  ('string literal pointer', 'int main(){ char *s; s = "hi"; return 0; }', 'string literal'),
  ('string literal arg',     'int f(char *s){ return 0; } int main(){ return f("hi"); }', 'string literal'),
  ('string init non-char',   'int main(){ int a[3] = "ab"; return 0; }', 'char array'),
  ('array assignment',       'int main(){ int a[2]; int b[2]; a = b; return 0; }', 'array'),
  ('struct assignment',      'struct P { int x; }; int main(){ struct P a; struct P b; a = b; return 0; }', 'struct'),
  ('struct value param',     'struct P { int x; }; int f(struct P p){ return 0; } int main(){ return 0; }', 'by value'),
  ('struct value return',    'struct P { int x; }; struct P f(){ struct P p; return p; } int main(){ return 0; }', 'by value'),
  ('struct as value',        'struct P { int x; }; int main(){ struct P a; return a; }', 'struct'),
  ('unknown struct member',  'struct P { int x; }; int main(){ struct P a; return a.z; }', 'no member'),
  ('member of a scalar',     'int main(){ int a; return a.x; }', 'not a struct'),
  ('arrow on a scalar',      'struct P { int x; }; int main(){ int a; return a->x; }', 'pointer'),
  ('incomplete struct var',  'int main(){ struct Nope a; return 0; }', 'incomplete'),
  ('address of an array',    'int main(){ int a[3]; int *p; p = &a; return 0; }', 'whole array'),
  ('2-D pointer arithmetic', 'int main(){ int m[2][3]; int *p; p = m + 1; return 0; }', 'multi-dimensional'),
  ('add two pointers',       'int main(){ int a[2]; int *p; int *q; p = a; q = a; return p + q; }', 'two pointers'),
  ('excess initializer',     'int main(){ int a[2] = {1, 2, 3}; return 0; }', 'excess'),
  ('missing array size',     'int main(){ int a[]; return 0; }', 'size'),
  ('missing inner size',     'int main(){ int a[][3]; return 0; }', 'size'),
  ('zero array size',        'int main(){ int a[0]; return 0; }', 'array size'),
  ('too many dimensions',    'int main(){ int a[1][1][1][1][1]; return 0; }', 'dimensions'),
  ('array too large',        'int main(){ int a[20000]; return 0; }', 'too large'),
  ('redeclaration',          'int main(){ int a; int a; return 0; }', 'redeclaration'),
  ('inner scope is gone',    'int main(){ { int a; a = 1; } return a; }', 'Undefined variable'),
  ('void variable',          'int main(){ void v; return 0; }', 'void'),
  ('side effect in op=',     'int f(){ return 0; } int main(){ int a[2]; a[f()] += 1; return 0; }', 'side effects'),
  ('side effect in ++',      'int main(){ int a[3]; int i; i = 0; a[i++]++; return 0; }', 'side effects'),
  ('string too long',        'int main(){ char s[2] = "abc"; return 0; }', 'too long'),
  ('subtract int - ptr',     'int main(){ int a[2]; int *p; p = a; return 1 - p; }', 'integer minus a pointer'),
  # supported-subset boundary: these are real C89 but not implemented, so they must fail loudly, not miscompile
  ('global variable',       'int g = 5; int main(){ return g; }', ''),
  ('static function',       'static int f(int x){ return x; } int main(){ return f(1); }', ''),
  ('static local',          'int main(){ static int n; return n; }', ''),
  ('function pointer',      'int f(int x){ return x; } int main(){ int (*p)(int) = f; return p(1); }', ''),
  ('string concatenation',  'char *s = "a" "b"; int main(){ return 0; }', ''),
  ('const qualifier',       'int main(){ const int a = 1; return a; }', ''),
  ('register',              'int main(){ register int a = 1; return a; }', ''),
  ('bit-field',             'struct s { unsigned a:3; }; int main(){ return 0; }', ''),
  ('K&R definition',        'int f(a) int a; { return a; } int main(){ return f(1); }', ''),
  ('varargs',               'int f(int n, ...){ return n; } int main(){ return f(1,2); }', ''),
  ('stringizing',           '#define S(x) #x\nint main(){ return 0; S(a) }', 'not supported'),
  ('token pasting',         '#define P(a,b) a##b\nint main(){ return P(1,2); }', 'not supported'),
]

fails = 0
for name, src, frag in REJECT:
    r, asm = compile_(src)
    crashed = r.returncode < 0 or r.returncode >= 128 or 'AddressSanitizer' in r.stderr or 'runtime error' in r.stderr
    problems = []
    if r.returncode == 0: problems.append('exit 0')
    if crashed: problems.append('crashed')
    if asm is not None: problems.append('left an output file')
    if frag and frag not in r.stderr and frag not in r.stdout: problems.append('no "%s" in the diagnostic' % frag)
    if not (r.stderr.strip() or r.stdout.strip()): problems.append('no diagnostic')
    if problems:
        fails += 1
        print('FAIL %s: %s\n  stderr=%r' % (name, ', '.join(problems), r.stderr[-200:]))

# a few things that must keep working, with the exact value checked in the generated code
r, asm = compile_('int main(){ return 010; }')
if r.returncode != 0 or '#$0008' not in asm: fails += 1; print('FAIL octal 010 must be 8')
r, asm = compile_('int main(){ return 0X1F + 0x1f; }')
if r.returncode != 0 or '$001F' not in asm.upper(): fails += 1; print('FAIL hex constants')

r, asm = compile_('int f(void){ return 1; } int g(void); int main(void){ return f(); }')
if r.returncode != 0: fails += 1; print('FAIL f(void) parameter lists must compile: %s' % r.stderr[-200:])

total = len(REJECT) + 3
print('%d passed, %d failed' % (total - fails, fails))
sys.exit(1 if fails else 0)

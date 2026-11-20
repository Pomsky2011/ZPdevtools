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
  ('array subscript',       'int main(){ int x; return x[1]; }', 'array indexing'),
  ('array variable',        'int main(){ int a[3]; return 0; }', 'array variables'),
  ('array initializer',     'int main(){ int a[3] = {1,2,3}; return 0; }', 'array variables'),
  ('address-of',            'int main(){ int x; int *p; p = &x; return 0; }', 'address'),
  ('pointer dereference',   'int main(){ int *p; return *p; }', 'dereference'),
  ('struct member read',    'struct P { int x; int y; }; int main(){ struct P a; return a.y; }', ''),
  ('break outside loop',    'int main(){ break; return 0; }', 'break'),
  ('continue outside loop', 'int main(){ continue; return 0; }', 'continue'),
  ('#error',                '#error nope\nint main(){ return 0; }', '#error'),
  ('lexical garbage',       'int main(){ return 1 @ 2; }', 'Unexpected character'),
  ('missing semicolon',     'int main(){ return 1 }', 'syntax error'),
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

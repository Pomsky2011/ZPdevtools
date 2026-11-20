#!/usr/bin/env python3
"""zpcc must refuse, cleanly, what it cannot compile correctly: a non-zero exit, a diagnostic, no crash and no
output file.  Globals live in ROM (bank $00) and pointers address bank $80, so a global's address may not be used as
a pointer value and a global may not be written; a function whose frame exceeds what 'n,S' can address is an error.

  tests/cc/zpcc_reject_tests.py [path/to/zpcc]      (default: ../llvm_compiler/zpcc, or $ZPCC_BIN)
"""
import os, subprocess, sys, tempfile

here = os.path.dirname(os.path.abspath(__file__))
root = os.path.abspath(os.path.join(here, '..', '..'))
zpcc = sys.argv[1] if len(sys.argv) > 1 else os.environ.get('ZPCC_BIN', os.path.join(root, 'llvm_compiler', 'zpcc'))

def compile_(src):
    with tempfile.TemporaryDirectory() as d:
        p = os.path.join(d, 'in.c'); out = os.path.join(d, 'out.asm')
        open(p, 'w').write(src)
        r = subprocess.run([zpcc, '-I', os.path.join(root, 'include'), p, '-o', out], capture_output=True, text=True, timeout=120, cwd=d)
        return r, os.path.exists(out)

REJECT = [
    ('store to a global',       'int g; int main(void){ g = 1; return 0; }', 'read-only'),
    ('store to a global array', 'int t[4]; int main(void){ t[1] = 2; return 0; }', 'read-only'),
    ('address of a global',     'int g; int f(int *p){ return 0; } int main(void){ return f(&g); }', 'address of global'),
    ('global array decays',     'int t[4]; int main(void){ int *p; p = t; return *p; }', 'address of global'),
    ('string literal pointer',  'int f(char *s){ return 0; } int main(void){ return f("hi"); }', 'address of global'),
    ('global address as integer','int t[4]; int f(int i){ return (int)&t[i]; } int main(void){ return f(1); }', 'address of global'),
    ('copy into a global',      'int t[4]; int main(void){ int a[4] = {1,2,3,4}; __builtin_memcpy(t, a, 8); return 0; }', 'read-only'),
    ('frame past 255 bytes',    'int main(void){ int a[150]; a[0] = 1; return a[0]; }', 'stack frame'),
    ('memmove',                 'int main(void){ char a[8]; __builtin_memmove(a + 1, a, 4); return 0; }', 'memmove'),
    ('wide integer',            'long long f(long long a){ return a + 1; } int main(void){ return (int)f(1); }', 'wider than 32'),
]

fails = 0
for name, src, frag in REJECT:
    r, left = compile_(src)
    crashed = r.returncode < 0 or r.returncode >= 128
    problems = []
    if r.returncode == 0: problems.append('exit 0')
    if crashed: problems.append('crashed')
    if left: problems.append('left an output file')
    if frag not in r.stderr: problems.append('no "%s" in the diagnostic' % frag)
    if problems:
        fails += 1
        print('FAIL %s: %s\n  stderr=%r' % (name, ', '.join(problems), r.stderr[-250:]))
print('%d passed, %d failed' % (len(REJECT) - fails, fails))
sys.exit(1 if fails else 0)

#!/usr/bin/env python3
"""Mutation-fuzz the assemblers and disassemblers under ASan/UBSan; any sanitizer report or signal is a failure.

  tests/asm/fuzz_asm.py [n-per-tool=300] [seed=1234]

Seeds are the files under examples/ (.asm for the assemblers, .bin for the disassemblers). Crashing inputs are
kept in a temp directory whose path is printed."""
import collections, glob, os, random, re, subprocess, sys, tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.join(HERE, '..', '..')
N = int(sys.argv[1]) if len(sys.argv) > 1 else 300
random.seed(int(sys.argv[2]) if len(sys.argv) > 2 else 1234)
W = tempfile.mkdtemp(prefix='fuzz_asm_')
ENV = dict(os.environ, ASAN_OPTIONS='detect_leaks=0', UBSAN_OPTIONS='print_stacktrace=1')
ASM = {'cpuasm': 'cpu', 'apuasm': 'apu', 'ppuasm': 'ppu'}
DIS = ['cpudisasm', 'apudisasm', 'ppudisasm']
JUNK = [b'A' * 300, b'A' * 5000, b'$' + b'F' * 200, b'(' * 100, b'"' * 50, b'-99999999999', b'\n' * 50]


def build(tool):
    out = os.path.join(W, tool)
    subprocess.check_call(['gcc', '-O1', '-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-I' + os.path.join(ROOT, 'src'),
                           os.path.join(ROOT, 'src', tool + '.c'), '-o', out, '-lm'], stderr=subprocess.DEVNULL)
    return out


def seeds(pattern):
    out = []
    for f in glob.glob(os.path.join(ROOT, 'examples', pattern), recursive=True):
        b = open(f, 'rb').read()
        if 0 < len(b) < 20000:
            out.append(b)
    return out


def mutate(b, text):
    b = bytearray(b)
    for _ in range(random.randint(1, 6)):
        k, i = random.random(), (random.randrange(len(b)) if b else 0)
        if k < .25 and b:
            b[i] = random.randrange(256)
        elif k < .4 and b:
            del b[i:i + random.randint(1, 30)]
        elif k < .6:
            b[i:i] = random.choice(JUNK)
        elif k < .75 and text:
            ls = b.split(b'\n')
            ls.insert(random.randrange(len(ls)), random.choice(ls))
            b = bytearray(b'\n'.join(ls))
        elif b:
            b[i:i] = bytes(random.randrange(256) for _ in range(random.randint(1, 8)))
    return bytes(b)


found = collections.defaultdict(collections.Counter)
for tool in list(ASM) + DIS:
    text = tool in ASM
    exe = build(tool)
    pool = seeds(ASM[tool] + '/*.asm') if text else seeds('**/*.bin')
    pool = pool or seeds('**/*.asm' if text else '**/*.bin')
    inp, outp = os.path.join(W, 'in'), os.path.join(W, 'out')
    for n in range(N):
        data = mutate(random.choice(pool), text)
        open(inp, 'wb').write(data)
        try:
            p = subprocess.run([exe, inp] + ([outp] if text else []), capture_output=True, timeout=10, env=ENV, cwd=W)
        except subprocess.TimeoutExpired:
            key = 'timeout'
        else:
            err = p.stderr.decode('latin1')
            m = re.search(r'ERROR: AddressSanitizer: (\S+)', err) or re.search(r'runtime error: (.*)', err)
            if not m and p.returncode >= 0:
                continue
            fr = re.search(r'#\d+ 0x\w+ in \w+ \S*/(\w+\.c:\d+)', err)
            key = (m.group(1)[:60] if m else 'signal %d' % -p.returncode) + ' @ ' + (fr.group(1) if fr else '?')
        if key not in found[tool]:
            open(os.path.join(W, '%s_%d.crash' % (tool, len(found[tool]))), 'wb').write(data)
        found[tool][key] += 1

for tool, c in found.items():
    print('FAIL', tool)
    for k, v in c.most_common():
        print('  %4d  %s' % (v, k))
if found:
    print('crashing inputs in', W)
    sys.exit(1)
print('no crashes in %d inputs x %d tools' % (N, len(ASM) + len(DIS)))

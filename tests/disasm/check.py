#!/usr/bin/env python3
"""cpudisasm / apudisasm / ppudisasm tests.

 - golden output for a snippet per CPU, checked line by line against the source it was assembled from
 - round trip: assemble -> disassemble -> strip the address column -> reassemble must give the same bytes
   (all of examples/apu and examples/ppu that assemble, and the cpu examples that stand alone)
 - every opcode decodes (nothing "unknown"; the APU's undefined encodings must say "reserved")
 - -s/-e/-b/-c/-o, an empty file, a trailing odd byte, a missing file

  tests/disasm/check.py [bindir]        (default: executables/)

The cpu round trip is exhaustive: every opcode with a spread of operand values (either side of the $FF / $FFFF
boundaries cpuasm picks direct page / absolute / long by) must reassemble to the same bytes, which is why cpudisasm
prints '!' (force absolute) and '>' (force long) where the bare text would be read as another instruction. Opcodes
$6E and $76, the 6-cycle duplicates of INC/DEC absolute ($6A, $72), are spelled INC.L / DEC.L so that they survive too.
"""
import glob, os, re, subprocess, sys, tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, '..', '..'))
BIN = sys.argv[1] if len(sys.argv) > 1 else os.path.join(ROOT, 'executables')
W = tempfile.mkdtemp(prefix='disasm_test_')
failures = []
ASM = {'cpu': ('cpuasm', lambda s, o: [s, o]), 'apu': ('apuasm', lambda s, o: [s, o]), 'ppu': ('ppuasm', lambda s, o: [s, '-o', o])}


def check(cond, what):
    if not cond:
        failures.append(what)
        print('FAIL:', what)


def run(name, *args, cwd=W):
    p = subprocess.run([os.path.join(BIN, name)] + list(args), cwd=cwd, capture_output=True, timeout=60)
    return p.returncode, p.stdout.decode('latin1'), p.stderr.decode('latin1')


def path(n):
    return os.path.join(W, n)


def write(n, data):
    open(path(n), 'wb' if isinstance(data, bytes) else 'w').write(data)
    return n


def assemble(cpu, src, out, cwd=W):
    tool, args = ASM[cpu]
    return run(tool, *args(src, out), cwd=cwd)[0] == 0


def instr_lines(text):
    """The `ADDR:  text` lines with the address and any trailing comment removed."""
    out = []
    for ln in text.splitlines():
        m = re.match(r'^[0-9A-Fa-f]+:\s+(.*)$', ln)
        if m:
            out.append(re.sub(r'\s*;.*$', '', m.group(1)).strip())
    return out


def disasm(cpu, binfile, *opts):
    rc, out, err = run(cpu + 'disasm', binfile, *opts)
    return rc, out


# ---- golden snippets -----------------------------------------------------------------------------------------
GOLD = {
    'cpu': ('.org $0200\nSEI\nCLD\nREP #$30\nLDA #$1234\nLDX #$0010\nSTA $2000\nLDA $8035,X\nLDA $123456\nLDA $123456,X\n'
            'INX\nDEX\nPHA\nPLA\nNOP\n',
            ['SEI', 'CLD', 'REP #$30', 'LDA #$1234', 'LDX #$0010', 'STA $2000', 'LDA $8035,X', 'LDA $123456',
             'LDA $123456,X', 'INX', 'DEX', 'PHA', 'PLA', 'NOP']),
    'apu': ('zor r0\nlst 0, 10\nscr y, $01\nadd x, y, r0\nlfn 0\ninc x\ndec y\nhlt\n',
            ['ZOR R0', 'LST 0, 10', 'SCR Y, $01', 'ADD X, Y, R0', 'LFN 0', 'INC X', 'DEC Y', 'HLT']),
    'ppu': ('TARREG 2, LSB, R63\nSETBYTE 2, $E0\nADD R63, R1\nINC R5\nCLR R3\n',
            ['TARREG       2, LSB, R63', 'SETBYTE      2, $E0', 'ADD          R63, R1', 'INC          R5', 'CLR          R3']),
}
for cpu, (src, want) in GOLD.items():
    write(cpu + '.asm', src)
    check(assemble(cpu, cpu + '.asm', cpu + '.bin'), '%s golden: snippet assembles' % cpu)
    rc, out = disasm(cpu, cpu + '.bin')
    got = [re.sub(r'\s+', ' ', l) for l in instr_lines(out)]
    want = [re.sub(r'\s+', ' ', l) for l in want]
    check(rc == 0 and got == want, '%s golden: disassembly matches the source\n   got  %s\n   want %s' % (cpu, got, want))
    write(cpu + '_rt.asm', '\n'.join(instr_lines(out)) + '\n')
    check(assemble(cpu, cpu + '_rt.asm', cpu + '_rt.bin') and open(path(cpu + '_rt.bin'), 'rb').read() == open(path(cpu + '.bin'), 'rb').read(),
          '%s golden: the disassembly reassembles to the same bytes' % cpu)

FORMS = ['LDA !$0005', 'LDA !$0005,X', 'LDA !$0005,Y', 'JMP !($0005)', 'JMP !($0005,X)', 'JMP ![$0005]', 'JMP [$1234]',
         'JMP $1234', 'JMP >$001234', 'LDA >$000005', 'LDA >$000005,X', 'LDA $05', 'LDA $0105', 'LDA $010000', 'CPX !$0005',
         'BRA !$0005', 'BEQ >$000005', 'CALL >$000005', 'MVN $01,$02', 'MVP $7E,$7F', 'DIV X,Y', 'DIV $123456,X', 'SHL X', 'SHR Y', 'INC $1234', 'INC.L $1234', 'DEC $1234', 'DEC.L $1234',
         'INC.L !$0005', 'DEC.L !$0010']
write('forms.asm', '.org $0200\n' + '\n'.join(FORMS) + '\n')
check(assemble('cpu', 'forms.asm', 'forms.bin'), 'cpu forms: the snippet assembles')
rc, out = disasm('cpu', 'forms.bin')
check(rc == 0 and instr_lines(out) == FORMS,
      'cpu forms: !/> prefixes, block moves and DIV X,Y are printed as written\n   got  %s' % instr_lines(out))

# ---- round trip over the example sources ----------------------------------------------------------------------
EXAMPLES = os.path.join(ROOT, 'examples')
passed = {'cpu': 0, 'apu': 0, 'ppu': 0}
for cpu in ('cpu', 'apu', 'ppu'):
    for src in sorted(glob.glob(os.path.join(EXAMPLES, cpu, '*.asm'))):
        d, b = os.path.dirname(src), os.path.basename(src)
        if 'no-roundtrip' in open(src).read():
            continue   # marked as holding data (a string, a table): a disassembler cannot tell data from code
        if not assemble(cpu, b, path('x.bin'), cwd=d) or os.path.getsize(path('x.bin')) > 8192:
            continue   # some PPU examples no longer assemble (stale), others are huge padded images
        rc, out = disasm(cpu, 'x.bin')
        write('x_rt.asm', '\n'.join(instr_lines(out)) + '\n')
        ok = rc == 0 and assemble(cpu, 'x_rt.asm', 'x_rt.bin') and open(path('x.bin'), 'rb').read() == open(path('x_rt.bin'), 'rb').read()
        check(ok, 'round trip: %s/%s' % (cpu, b))
        passed[cpu] += ok
check(passed['cpu'] >= 5, 'round trip: at least 5 cpu examples ran (%d)' % passed['cpu'])
check(passed['apu'] >= 5, 'round trip: at least 5 apu examples ran (%d)' % passed['apu'])
check(passed['ppu'] >= 10, 'round trip: at least 10 ppu examples ran (%d)' % passed['ppu'])

# ---- every cpu opcode, round trip -----------------------------------------------------------------------------
TAILS = [(0x34, 0x12, 0x00, 0x56), (0x05, 0x00, 0x00, 0x00), (0xFF, 0x00, 0x00, 0x00), (0x00, 0x01, 0x00, 0x00),
         (0xFF, 0xFF, 0x00, 0x00), (0x00, 0x00, 0x01, 0x00), (0xF0, 0x80, 0x00, 0x00), (0xFE, 0xFF, 0x80, 0x00)]
broken = []
for op in range(256):
    for tail in TAILS:
        data = bytes([op]) + bytes(tail)
        write('op.bin', data)
        rc, out = disasm('cpu', 'op.bin')
        starts = [int(m.group(1), 16) for m in re.finditer(r'^([0-9A-Fa-f]+):', out, re.M)]
        first, size = instr_lines(out)[0], (starts[1] if len(starts) > 1 else len(data))
        write('op.asm', first + '\n')
        if os.path.exists(path('op_rt.bin')):
            os.remove(path('op_rt.bin'))
        ok = assemble('cpu', 'op.asm', 'op_rt.bin') and open(path('op_rt.bin'), 'rb').read() == data[:size]
        if not ok:
            broken.append('%02X %s (%s)' % (op, first, data[:size].hex()))
            break
check(not broken, 'cpu: every opcode reassembles to itself from its own disassembly (%d fail, first: %s)' % (len(broken), broken[:6]))

# ---- opcode coverage -------------------------------------------------------------------------------------------
BAD = re.compile(r'unknown|illegal|invalid|\?\?\?|\.byte|\.db', re.I)
bad_ops = []
for op in range(256):
    write('op.bin', bytes([op, 0x34, 0x12, 0x00]))
    rc, out = disasm('cpu', 'op.bin')
    lines = instr_lines(out)
    if rc != 0 or not lines or BAD.search(lines[0]) or not re.match(r'^[A-Z][A-Z0-9]*\b', lines[0]):
        bad_ops.append('%02X' % op)
check(not bad_ops, 'cpudisasm decodes every one of the 256 opcodes (failed: %s)' % ' '.join(bad_ops[:20]))

write('allapu.bin', b''.join(bytes([h, l]) for h in range(256) for l in (0x00, 0x5A, 0xFF)))
rc, out = disasm('apu', 'allapu.bin')
lines = instr_lines(out)
check(rc == 0 and len(lines) == 768, 'apudisasm: 768 words in, 768 instructions out (%d)' % len(lines))
stray = [l for l in out.splitlines() if re.search(r'\.WORD', l) and 'reserved' not in l]
check(not stray and not BAD.search('\n'.join(lines)), 'apudisasm: undefined encodings are .WORD ... reserved, nothing is "unknown"')

words = []
for op in range(16):
    for x in (0, 0x155, 0xFFF):
        words.append(bytes([(op << 4) | (x >> 8), x & 0xFF]))
for sub in range(4):                       # Preset E sub-operations
    words.append(bytes([0xE0 | (sub << 2), 0x00]))
for sub in range(16):                      # Preset F sub-operations
    words.append(bytes([0xF0 | sub, 0x00]))
write('allppu.bin', b''.join(words))
rc, out = disasm('ppu', 'allppu.bin')
lines = instr_lines(out)
check(rc == 0 and len(lines) == len(words) and not BAD.search('\n'.join(lines)),
      'ppudisasm: every basic opcode and Preset E/F sub-operation decodes (%d of %d)' % (len(lines), len(words)))

# ---- options and odd inputs -------------------------------------------------------------------------------------
for cpu in ('cpu', 'apu', 'ppu'):
    full = disasm(cpu, cpu + '.bin')[1]
    rc = run(cpu + 'disasm', cpu + '.bin', '-o', 'o_' + cpu + '.txt')[0]
    check(rc == 0 and open(path('o_' + cpu + '.txt')).read() == full, '%sdisasm: -o writes exactly what stdout shows' % cpu)
    check(re.search(r'^; .*Disassembly of %s\.bin' % cpu, full, re.M) and re.search(r'^; Size: \d+ bytes \(0x[0-9A-F]+\)$', full, re.M),
          '%sdisasm: header names the file and its size' % cpu)
    end = {'cpu': '2', 'apu': '4', 'ppu': '4'}[cpu]       # exactly the end of the second instruction
    check(instr_lines(disasm(cpu, cpu + '.bin', '-e', end)[1]) == instr_lines(full)[:2],
          '%sdisasm: -e stops after the second instruction' % cpu)

rc, out = disasm('cpu', 'cpu.bin', '-s', '4', '-e', 'A')   # addresses are hex
check(instr_lines(out) == ['LDA #$1234', 'LDX #$0010'] and re.search(r'Range: \$000004 - \$00000A', out),
      'cpudisasm: -s/-e select a range')
rc, out = disasm('cpu', 'cpu.bin', '-b')
check(re.search(r'^000004:\s+37 34 12\s+LDA #\$1234', out, re.M), 'cpudisasm: -b shows the instruction bytes')
rc, out = disasm('cpu', 'cpu.bin', '-c')
check(re.search(r'LDA #\$1234\s+; opcode \$37', out), 'cpudisasm: -c shows the opcode comment')
rc, out = disasm('apu', 'apu.bin', '-b')
check(re.search(r'^0002:\s+68 0A\s+LST 0, 10', out, re.M), 'apudisasm: -b shows the instruction bytes')
rc, out = disasm('cpu', 'cpu.bin', '-s', '99999', '-e', '99999')
check(rc == 0 and instr_lines(out) == [], 'cpudisasm: a range past the end of the file is empty, not a crash')

write('empty.bin', b'')
for cpu in ('cpu', 'apu', 'ppu'):
    rc, out = disasm(cpu, 'empty.bin')
    check(rc == 0 and instr_lines(out) == [], '%sdisasm: an empty file gives an empty listing' % cpu)
    rc, out, err = run(cpu + 'disasm', 'no_such_file.bin')
    check(rc == 1 and 'Cannot open' in (out + err), '%sdisasm: a missing file is an error (exit 1)' % cpu)
    rc, out, err = run(cpu + 'disasm')
    check(rc != 0 and 'Usage' in (out + err), '%sdisasm: no argument prints usage and fails' % cpu)
write('odd.bin', open(path('ppu.bin'), 'rb').read() + b'\x7f')
rc, out = disasm('ppu', 'odd.bin')
check(rc == 0 and instr_lines(out) == instr_lines(disasm('ppu', 'ppu.bin')[1]), 'ppudisasm: a trailing odd byte is ignored, not read past')

if failures:
    print('%d failure(s); files kept in %s' % (len(failures), W))
    sys.exit(1)
print('all disassembler checks passed')

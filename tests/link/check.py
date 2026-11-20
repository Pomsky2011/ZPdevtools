#!/usr/bin/env python3
"""zplink / zpbuild tests. Both tools are checked against an independent reader of the formats: the ELF is parsed with
struct, and a signed .zpb has its header, payload, trailer digests (hashlib) and RSA-2048 PKCS#1 v1.5 signature (the
dev public key from src/zpkey.h, pure Python) recomputed from the spec in ZeroPoint/docs/zpb-format.md.

  tests/link/check.py [bindir]        (default: executables/)
"""
import hashlib, os, re, struct, subprocess, sys, tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, '..', '..'))
BIN = sys.argv[1] if len(sys.argv) > 1 else os.path.join(ROOT, 'executables')
W = tempfile.mkdtemp(prefix='zplink_test_')
CHUNK = 16384
failures = []


def check(cond, what):
    if not cond:
        failures.append(what)
        print('FAIL:', what)


def tool(name, *args):
    p = subprocess.run([os.path.join(BIN, name)] + [str(a) for a in args], cwd=W, capture_output=True, timeout=60)
    return p.returncode, (p.stdout + p.stderr).decode('latin1')


def write(name, data):
    open(os.path.join(W, name), 'wb').write(data)
    return name


def read(name):
    return open(os.path.join(W, name), 'rb').read()


def pattern(seed, n):
    return bytes((seed * 31 + i * 7 + (i >> 8)) & 0xFF for i in range(n))


# ---- ELF reader -----------------------------------------------------------------------------------------------
def parse_elf(b):
    e = {'ident': b[:16]}
    (e['type'], e['machine'], _, e['entry'], e['phoff'], e['shoff'], _, _, e['phentsize'], e['phnum'], e['shentsize'],
     e['shnum'], e['shstrndx']) = struct.unpack_from('<HHIIIIIHHHHHH', b, 16)
    e['phdrs'] = []
    for i in range(e['phnum']):
        t, off, va, pa, fs, ms, fl, al = struct.unpack_from('<IIIIIIII', b, e['phoff'] + i * e['phentsize'])
        e['phdrs'].append(dict(type=t, offset=off, vaddr=va, filesz=fs, memsz=ms, flags=fl, data=b[off:off + fs]))
    e['sections'] = {}
    if e['shnum']:
        sh = [struct.unpack_from('<IIIIIIIIII', b, e['shoff'] + i * e['shentsize']) for i in range(e['shnum'])]
        stroff = sh[e['shstrndx']][4]
        for name, typ, flags, addr, off, size, *_ in sh:
            nm = b[stroff + name:b.index(b'\0', stroff + name)].decode()
            e['sections'][nm] = dict(addr=addr, offset=off, size=size)
    return e


# ---- ZPB reader + independent signature verification ----------------------------------------------------------
KEY = open(os.path.join(ROOT, 'src', 'zpkey.h')).read()
LIMBS = [int(x, 16) for x in re.search(r'ZP_RSA_N\[128\] = \{(.*?)\};', KEY, re.S).group(1).replace('\n', ' ').replace(',', ' ').split()]
RSA_N = sum(l << (16 * i) for i, l in enumerate(LIMBS))
RSA_E = int(re.search(r'ZP_RSA_E = (\d+)', KEY).group(1))
SHA256_PREFIX = bytes.fromhex('3031300d060960864801650304020105000420')


def blake2s(b):
    return hashlib.blake2s(b).digest()


def parse_zpb(b):
    z = {'magic': b[:4], 'version': b[4], 'flags': b[5], 'hdrsize': struct.unpack_from('<H', b, 6)[0],
         'romsize': struct.unpack_from('<I', b, 8)[0], 'entry': struct.unpack_from('<I', b, 12)[0],
         'title': b[16:48], 'dev': b[48:64], 'header': b[:64]}
    z['payload'] = b[64:64 + z['romsize']]
    t = b[64 + z['romsize']:]
    z['tmagic'], z['tver'], z['keywords'], z['siglen'] = t[:4], t[4], t[5], struct.unpack_from('<H', t, 6)[0]
    z['digest'], z['sig'] = t[8:40], t[40:296]
    z['codesize'] = struct.unpack_from('<I', t, 296)[0] if z['tver'] in (2, 3) else None
    z['manifest'] = t[300:] if z['tver'] == 3 else b''
    z['trailer_len'] = len(t)
    return z


def expected_digest(z):
    h, p = z['header'], z['payload']
    if z['tver'] == 1:
        return hashlib.sha256(h + p).digest()
    cs = z['codesize']
    if z['tver'] == 2:
        return hashlib.sha256(h + blake2s(p[:cs]) + blake2s(p[cs:])).digest()
    return hashlib.sha256(h + blake2s(p[:cs]) + blake2s(z['manifest'])).digest()   # the stored manifest, as the boot ROM sees it


def manifest_of(z):
    data = z['payload'][z['codesize']:]
    return b''.join(blake2s(data[i:i + CHUNK]) for i in range(0, len(data), CHUNK))


def sig_ok(z, digest):
    em = pow(int.from_bytes(z['sig'], 'big'), RSA_E, RSA_N).to_bytes(256, 'big')
    want = b'\x00\x01' + b'\xff' * (256 - 3 - len(SHA256_PREFIX) - 32) + b'\x00' + SHA256_PREFIX + digest
    return em == want


def verify_zpb(b):
    """True if the whole file is internally consistent and correctly signed."""
    z = parse_zpb(b)
    if z['magic'] != b'ZPB\0' or z['version'] != 2 or z['hdrsize'] != 64 or z['tmagic'] != b'ZPSG':
        return False
    d = expected_digest(z)
    if z['tver'] == 3 and z['manifest'] != manifest_of(z):
        return False
    return d == z['digest'] and sig_ok(z, d)


def build(elf, out, *opts):
    rc, log = tool('zpbuild', elf, '-o', out, *opts)
    return rc, log, (read(out) if rc == 0 else None)


def link(out, *args):
    return tool('zplink', '-o', out, *args)


# ================================================================== zplink
cpu, ppu, apu, dat = pattern(1, 100), pattern(2, 64), pattern(3, 32), pattern(4, 16)
for n, d in (('c.bin', cpu), ('p.bin', ppu), ('a.bin', apu), ('d.bin', dat)):
    write(n, d)

rc, log = link('one.elf', '--cpu', 'c.bin')
check(rc == 0, 'zplink --cpu: exits 0')
e = parse_elf(read('one.elf'))
check(e['ident'][:4] == b'\x7fELF' and e['ident'][4] == 1 and e['ident'][5] == 1, 'ELF: magic, 32-bit, little-endian')
check(e['type'] == 2 and e['machine'] == 0x5A50, 'ELF: ET_EXEC, machine "ZP"')
check(e['entry'] == 0x8000, 'ELF: default entry is the cpu base 0x8000')
check(len(e['phdrs']) == 1, 'ELF: one PT_LOAD for one segment')
ph = e['phdrs'][0]
check(ph['type'] == 1 and ph['vaddr'] == 0x8000 and ph['filesz'] == 100 and ph['data'] == cpu,
      'ELF: cpu segment at 0x8000 carries the file bytes')
check(ph['flags'] == 5, 'ELF: cpu segment is R+X')
check({'.text', '.shstrtab'} <= set(e['sections']), 'ELF: .text and .shstrtab section headers')

rc, log = link('all.elf', '--cpu', 'c.bin', '--ppu', 'p.bin', '--apu', 'a.bin', '--data', 'd.bin@0x20000')
e = parse_elf(read('all.elf'))
segs = {p['vaddr']: p for p in e['phdrs']}
check(rc == 0 and set(segs) == {0x8000, 0x100000, 0x200000, 0x20000}, 'zplink: cpu/ppu/apu default bases and --data@addr')
check(segs.get(0x100000, {}).get('data') == ppu and segs.get(0x200000, {}).get('data') == apu
      and segs.get(0x20000, {}).get('data') == dat, 'zplink: every segment carries its own bytes')
check(all(segs[a]['flags'] == 6 for a in (0x100000, 0x200000, 0x20000)), 'ELF: ppu/apu/data segments are R+W')
check({'.text', '.ppu', '.apu', '.data'} <= set(e['sections']), 'ELF: a section per role')

for spec, addr in (('c.bin@0x9000', 0x9000), ('c.bin@$A000', 0xA000), ('c.bin@36864', 0x9000), ('c.bin@0x7FFF00', 0x7FFF00)):
    rc, log = link('a.elf', '--cpu', spec)
    check(rc == 0 and parse_elf(read('a.elf'))['phdrs'][0]['vaddr'] == addr, 'zplink: address form %s' % spec)
    check(parse_elf(read('a.elf'))['entry'] == addr, 'zplink: default entry follows the cpu address (%s)' % spec)
link('e.elf', '--cpu', 'c.bin', '--entry', '0x8004')
check(parse_elf(read('e.elf'))['entry'] == 0x8004, 'zplink: --entry overrides the entry point')

rc, log = link('edge.elf', '--cpu', 'c.bin@0x8000', '--data', 'd.bin@0xFFFFF0')
check(rc == 0, 'zplink: a segment ending exactly at the 16 MiB limit is accepted')
link('x1.elf', '--cpu', 'c.bin', '--ppu', 'p.bin')
link('x2.elf', '--cpu', 'c.bin', '--ppu', 'p.bin')
check(read('x1.elf') == read('x2.elf'), 'zplink: output is deterministic')

# refusals: each must exit 1 and must not leave an ELF behind
refuse = [
    ('overlapping segments', ['--cpu', 'c.bin@0x8000', '--data', 'd.bin@0x8010']),
    ('adjacent-but-overlapping by one byte', ['--cpu', 'c.bin@0x8000', '--data', 'd.bin@0x8063']),
    ('missing input file', ['--cpu', 'nonexistent.bin']),
    ('non-numeric address', ['--cpu', 'c.bin@zzz']),
    ('empty address', ['--cpu', 'c.bin@']),
    ('address past 24 bits', ['--cpu', 'c.bin@0x1000000']),
    ('segment running past 16 MiB', ['--cpu', 'c.bin@0xFFFFFF']),
    ('bad entry', ['--cpu', 'c.bin', '--entry', '0x1000000']),
    ('non-numeric entry', ['--cpu', 'c.bin', '--entry', 'main']),
    ('--data without an address', ['--cpu', 'c.bin', '--data', 'd.bin']),
    ('unknown option', ['--cpu', 'c.bin', '--bogus']),
    ('no segments', []),
]
for what, args in refuse:
    if os.path.exists(os.path.join(W, 'r.elf')):
        os.remove(os.path.join(W, 'r.elf'))
    rc, log = link('r.elf', *args)
    check(rc == 1, 'zplink refuses %s (exit 1, was %d)' % (what, rc))
    check(not os.path.exists(os.path.join(W, 'r.elf')), 'zplink leaves no ELF behind for %s' % what)
rc, log = tool('zplink', '--cpu', 'c.bin')
check(rc == 1, 'zplink refuses a missing -o')

# ================================================================== zpbuild
rc, log = tool('zpbuild', '--selftest')
check(rc == 0 and 'OK' in log, 'zpbuild --selftest')

link('rom.elf', '--cpu', 'c.bin@0x8000', '--data', 'd.bin@0x9000')
rc, log, v1 = build('rom.elf', 'v1.zpb', '--title', 'Test Game', '--dev', 'Tester')
check(rc == 0, 'zpbuild: builds a ROM')
z = parse_zpb(v1)
expect_payload = bytearray(0x9000 + 16)
expect_payload[0x8000:0x8000 + 100] = cpu
expect_payload[0x9000:0x9000 + 16] = dat
check(z['magic'] == b'ZPB\0' and z['version'] == 2 and z['flags'] == 1 and z['hdrsize'] == 64, 'ZPB header: magic, v2, signed flag, size')
check(z['romsize'] == len(expect_payload) and z['payload'] == bytes(expect_payload),
      'ZPB: payload is the zero-filled flat image of the ELF segments')
check(z['entry'] == 0x8000, 'ZPB: entry comes from the ELF')
check(z['title'].rstrip(b'\0') == b'Test Game' and z['dev'].rstrip(b'\0') == b'Tester', 'ZPB: --title / --dev land in the header')
check(z['tmagic'] == b'ZPSG' and z['tver'] == 1 and z['keywords'] == 32 and z['siglen'] == 256, 'ZPB: v1 trailer fields')
check(z['trailer_len'] == 296 and len(v1) == 64 + z['romsize'] + 296, 'ZPB: v1 trailer is 296 bytes')
check(z['digest'] == hashlib.sha256(z['header'] + z['payload']).digest(), 'ZPB v1: digest = SHA-256(header || payload)')
check(sig_ok(z, z['digest']), 'ZPB v1: RSA-2048 PKCS#1 v1.5 signature verifies against the dev public key')
check(verify_zpb(v1), 'ZPB v1: whole file verifies independently')

rc, log, d = build('rom.elf', 'defaults.zpb')
z = parse_zpb(d)
check(z['title'].rstrip(b'\0') == b'ZeroPoint Game' and z['dev'].rstrip(b'\0') == b'unknown', 'ZPB: default title and developer')

rc, log, d = build('rom.elf', 'long.zpb', '--title', 'T' * 100, '--dev', 'D' * 100)
z = parse_zpb(d)
check(rc == 0 and z['title'] == b'T' * 31 + b'\0' and z['dev'] == b'D' * 15 + b'\0', 'ZPB: long title/dev are cut and stay NUL-terminated')
check(verify_zpb(d), 'ZPB: still verifies with cut title/dev')

rc, log, d = build('rom.elf', 'ent.zpb', '--entry', '0x8010')
check(rc == 0 and parse_zpb(d)['entry'] == 0x8010 and verify_zpb(d), 'zpbuild: --entry overrides the ELF entry and is signed')

# v3: code + chunked data manifest (data spans several chunks, last one short)
big = pattern(5, 2 * CHUNK + 1234)
write('big.bin', big)
link('v3.elf', '--cpu', 'c.bin@0x8000', '--data', 'big.bin@0x20000')
rc, log, v3 = build('v3.elf', 'v3.zpb', '--codesize', 0x20000)
z = parse_zpb(v3)
check(rc == 0 and z['tver'] == 3 and z['codesize'] == 0x20000, 'ZPB v3: trailer version and codeSize')
check(len(z['manifest']) == 32 * 3, 'ZPB v3: one manifest entry per 16384-byte chunk (3 for 2 full chunks + a short one)')
check(z['manifest'] == manifest_of(z), 'ZPB v3: manifest entries are BLAKE2s of each chunk')
check(verify_zpb(v3), 'ZPB v3: digest and signature verify independently')
check(z['trailer_len'] == 300 + 96, 'ZPB v3: trailer length = 300 + 32 * chunks')
rc, log, v3b = build('v3.elf', 'v3b.zpb', '--codesize', 0x20000 + 2 * CHUNK + 1234)
check(rc == 0 and len(parse_zpb(v3b)['manifest']) == 0 and verify_zpb(v3b), 'ZPB v3: codeSize covering everything leaves an empty manifest')

# v2: whole-payload split, no manifest
rc, log, v2 = build('v3.elf', 'v2.zpb', '--codesize', 0x20000, '--trailer', 2)
z = parse_zpb(v2)
check(rc == 0 and z['tver'] == 2 and z['codesize'] == 0x20000 and z['trailer_len'] == 300, 'ZPB v2: trailer has codeSize and no manifest')
check(verify_zpb(v2), 'ZPB v2: digest and signature verify independently')
rc, log, d = build('v3.elf', 'v3t.zpb', '--codesize', 0x20000, '--trailer', 3)
check(rc == 0 and d == v3, 'zpbuild: --trailer 3 is the default')

# determinism: PKCS#1 v1.5 has no randomness, so the same input must give the same bytes
rc, log, again = build('rom.elf', 'v1b.zpb', '--title', 'Test Game', '--dev', 'Tester')
check(again == v1, 'zpbuild: output is deterministic')

# the verifier has to be able to say no
SIG = 64 + parse_zpb(v1)['romsize'] + 40
for what, mutate in (('a payload byte', lambda b: b[:100] + bytes([b[100] ^ 1]) + b[101:]),
                     ('the entry point', lambda b: b[:12] + bytes([b[12] ^ 1]) + b[13:]),
                     ('the title', lambda b: b[:16] + bytes([b[16] ^ 1]) + b[17:]),
                     ('the signature', lambda b: b[:SIG + 10] + bytes([b[SIG + 10] ^ 1]) + b[SIG + 11:])):
    check(not verify_zpb(mutate(v1)), 'verifier rejects a ROM with %s changed' % what)
check(not verify_zpb(v3[:-1] + bytes([v3[-1] ^ 1])), 'verifier rejects a v3 ROM whose manifest was changed')

# refusals: each exits 1 and writes no ROM
write('notelf.elf', cpu)
write('trunc.elf', read('rom.elf')[:30])
elf = bytearray(read('rom.elf'))
phoff = struct.unpack_from('<I', elf, 28)[0]


def evil(name, off, fmt, val):
    b = bytearray(elf)
    struct.pack_into(fmt, b, off, val)
    write(name, bytes(b))


evil('evil_filesz.elf', phoff + 16, '<I', 0x7FFFFFFF)
evil('evil_vaddr.elf', phoff + 8, '<I', 0xFFFFFF00)
evil('evil_phnum.elf', 44, '<H', 0xFFFF)
evil('evil_phoff.elf', 28, '<I', 0xFFFFFFF0)
evil('evil_offset.elf', phoff + 4, '<I', 0xFFFFFFF0)
big8 = pattern(6, 100)
write('b8.bin', big8)
link('over8.elf', '--cpu', 'c.bin@0x8000', '--data', 'b8.bin@0x7FFFFF')   # ends at 0x800063, past the 8 MiB ROM window
for what, args in [
    ('a missing input', ['nonexistent.elf']),
    ('a file that is not an ELF', ['notelf.elf']),
    ('a truncated ELF', ['trunc.elf']),
    ('a segment size far past the file', ['evil_filesz.elf']),
    ('a segment address near 4 GiB', ['evil_vaddr.elf']),
    ('an absurd program-header count', ['evil_phnum.elf']),
    ('a program-header table outside the file', ['evil_phoff.elf']),
    ('a segment offset outside the file', ['evil_offset.elf']),
    ('a payload over 8 MiB', ['over8.elf']),
    ('--codesize larger than the payload', ['rom.elf', '--codesize', 0x100000]),
    ('--trailer without --codesize', ['rom.elf', '--trailer', 2]),
    ('--trailer 4', ['rom.elf', '--codesize', 10, '--trailer', 4]),
    ('an unknown option', ['rom.elf', '--bogus']),
]:
    if os.path.exists(os.path.join(W, 'r.zpb')):
        os.remove(os.path.join(W, 'r.zpb'))
    rc, log = tool('zpbuild', *(args + ['-o', 'r.zpb']))
    check(rc == 1, 'zpbuild refuses %s (exit 1, was %d)' % (what, rc))
    check(not os.path.exists(os.path.join(W, 'r.zpb')), 'zpbuild writes no ROM for %s' % what)
rc, log = tool('zpbuild', 'rom.elf')
check(rc == 1, 'zpbuild refuses a missing -o')

# the whole chain on real assembler output
write('prog.asm', b'.org $8000\n.bank $00\nstart:\n    SEI\n    CLD\n    REP #$30\n    LDA #$1234\n    STA $2000\nloop:\n    NOP\n')
rc, log = tool('cpuasm', 'prog.asm', 'prog.bin')
check(rc == 0, 'chain: cpuasm assembles')
rc, log = link('prog.elf', '--cpu', 'prog.bin')
rc2, log2, rom = build('prog.elf', 'prog.zpb', '--title', 'Chain')
check(rc == 0 and rc2 == 0 and verify_zpb(rom), 'chain: cpuasm -> zplink -> zpbuild gives a verifying ROM')
check(parse_zpb(rom)['payload'][0x8000:0x8000 + len(read('prog.bin'))] == read('prog.bin'),
      'chain: the assembled bytes are in the ROM at 0x8000')

if failures:
    print('%d failure(s); files kept in %s' % (len(failures), W))
    sys.exit(1)
print('all zplink/zpbuild checks passed')

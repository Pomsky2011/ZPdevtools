# zplink / zpbuild — ZeroPoint linker & ROM signer

Turning a game's flat binaries into a shippable ROM is a **two-stage** pipeline:

```
cpuasm/ppuasm/apuasm/def88186cc   ->  zplink   ->  ELF32   ->  zpbuild  ->  signed .zpb
        (flat segments)              (dev side)              (HQ / mastering)
```

- **`zplink`** is the *locating linker*. It places each flat segment at its
  declared 24-bit address, checks for overlap, resolves the entry point, and
  emits an **ELF32 executable**. It holds **no signing key** — it is safe to
  ship to every developer.
- **`zpbuild`** is the *mastering / signing* step. It reads the ELF, rebuilds
  the flat payload, and wraps it in a signed `.zpb` ROM. The private key lives
  here, not in `zplink`, so it never leaves the trusted build machine.

Splitting the two keeps the signing key out of developers' hands: if it shipped
inside the linker everyone runs, one leak would let anyone mint ROMs that pass
the console's signature check. Instead developers produce and test ELFs, and a
real vendor signs them at HQ (historically the sort of job that ran on a big
UNIX/IRIX workstation on the mastering bench, not the dev's DOS box).

---

## zplink — locating linker (emits ELF)

The ZeroPoint assemblers (`cpuasm`/`ppuasm`/`apuasm`) and C compilers
(`zpcc`/`def88186cc`) emit *flat* binaries already assembled for a fixed load
address — there is no relocatable object format.

```
zplink -o OUT.elf
       --cpu FILE[@ADDR] [--ppu FILE[@ADDR]] [--apu FILE[@ADDR]]
       [--data FILE@ADDR ...] [--entry ADDR]
```

| Option | Meaning |
|--------|---------|
| `-o OUT.elf` | The ELF32 output (the developer-side deliverable). |
| `--cpu/--ppu/--apu FILE[@ADDR]` | A flat segment and where it loads. Defaults: cpu `@0x008000`, ppu `@0x100000`, apu `@0x200000`. |
| `--data FILE@ADDR` | Extra data segment (tiles, samples, tables). Repeatable. |
| `--entry ADDR` | Entry point. Default: base of the first `--cpu` segment. |

Put the entrypoint init stub at the front of your program by `.include`-ing
`lib/crt0_entry.asm` at the top (see [SDK.md](SDK.md)); it sets 16-bit CPU mode
and slides straight into `main`. `zplink` records that address as the ELF entry.

### ELF details

`ELFCLASS32`, little-endian, `ET_EXEC`, custom `e_machine = 0x5A50` ("ZP"). One
`PT_LOAD` per segment (`R E` for CPU code, `R W` for data), plus section headers
(`.text`, `.ppu`, `.apu`, `.data`, `.shstrtab`) so `readelf`/`objdump` show the
full layout:

```
$ readelf -l game.elf
  LOAD  0x000094 0x00008000 0x00008000 ... R E   # CPU
  LOAD  ...      0x00100000 0x00100000 ... RW    # PPU blob
  LOAD  ...      0x00200000 0x00200000 ... RW    # APU blob
```

---

## zpbuild — ROM signer (ELF → signed .zpb)

```
zpbuild IN.elf -o OUT.zpb [--title "NAME"] [--dev "AUTHOR"] [--entry ADDR]
        [--codesize N] [--selftest] [-v]
```

| Option | Meaning |
|--------|---------|
| `IN.elf` | A `zplink` ELF. `zpbuild` reconstructs the flat payload from its `PT_LOAD` segments. |
| `-o OUT.zpb` | The signed ROM. |
| `--title` / `--dev` | ROM metadata stamped into the header at signing time. |
| `--entry ADDR` | Override the entry point (default: the ELF's entry). |
| `--codesize N` | Opt into code + chunked-data-manifest signing (trailer version 3, see below): the first `N` bytes of the payload are "code," the rest is bulk "data." Omit for plain single-region signing (trailer version 1). |
| `--selftest` | Run the SHA-256 + RSA sign/verify/tamper self-test and exit. |

The payload is wrapped in the emulator's 64-byte `ZPBHeader` (see
`ZeroPoint/include/rom.h`) and then **signed**:

```
+------------------+  offset 0
| ZPBHeader (64)   |  magic "ZPB", version 2, flags, romSize, entry, title, dev
+------------------+  offset 64
| payload (romSize)|  the located image (CPU@base, PPU, APU, data...)
+------------------+  offset 64 + romSize
| "ZPSG" trailer   |  see below - shape depends on trailer version (1 or 3)
+------------------+
```

The stock emulator reads exactly `romSize` payload bytes, so the signed trailer
is transparent to it — an unsigned-aware loader still works, and a
signature-checking loader (the `ZPbootROM` project's `rsa_verify`, on real
hardware or the emulator with `--boot`) finds the trailer at `64 + romSize`.

### Trailer versions

`zpbuild` produces one of two trailer versions depending on `--codesize`
(full byte layout: `ZeroPoint/docs/zpb-format.md`):

- **Version 1** (default, no `--codesize`): `"ZPSG"` tag + trailer version +
  key size + siglen + a single SHA-256 digest of `ZPBHeader ‖ payload` (32
  bytes) + RSA-2048 signature (256 bytes). Verifying it means hashing the
  *entire* payload, which for a large cartridge (up to 8 MB) takes minutes
  at boot on the target 16-bit CPU — fine for small ROMs, not for large ones.
- **Version 3** (`--codesize N`): splits the payload into `code`
  (`payload[0..N)`) and `data` (`payload[N..romSize)`). `data` is chopped
  into 16384-byte chunks, each independently hashed with BLAKE2s into a
  manifest; `code_digest = BLAKE2s(code)`, `manifest_digest =
  BLAKE2s(concatenated per-chunk hashes)`, and the signed digest is
  `SHA256(header ‖ code_digest ‖ manifest_digest)`. The boot ROM only hashes
  `code` and the (small) manifest synchronously at boot — verifying any one
  data chunk is deferred until cartridge code actually loads it (via a
  `COP #$FF` call into the boot ROM, chunk index in `X`, vectoring to
  `$E0:0004`; see `ZeroPoint/CLAUDE.md`'s CPU Memory Map and
  `ZeroPoint/docs/zpb-format.md`'s "Runtime chunk verification" section).
  This is what makes an 8 MB cartridge boot in seconds instead of minutes
  while still making it impossible to consume unverified data undetected.
  (Trailer version 2, code+whole-data-BLAKE2s with no chunk manifest, exists
  in the format spec and the verifier still accepts it, but `zpbuild` no
  longer produces it — superseded by version 3.)

At runtime, the raw header + trailer (magic/version/digest/signature, plus
`codeSize` and the manifest for version 3) are mapped read-only into CPU
memory at bank `$E2` once the cartridge bus connects — `$E1` is reserved for
Boot ROM growth past 64 KiB, kept separate from `$E2` so a large Boot ROM
spanning `$E0-$E1` can't collide with the metadata region. `rsa_verify`
reads it from there, not through the cartridge's own read/write window.

### Signing & keys

The signature is **RSA-2048, PKCS#1 v1.5 over SHA-256**, computed over the
digest described above (`ZPBHeader ‖ payload` for version 1, or the
composite `header ‖ code_digest ‖ manifest_digest` for version 3 — either
way the entry point, title, size, and payload contents are all
authenticated). The key compiled into `zpbuild` from `src/zpkey.h` is a
**development** key (Montgomery constants precomputed; *not* secret-grade)
— good enough for a locally-loadable ROM the emulator accepts. A real vendor
builds `zpbuild` against their own private key kept on the mastering
machine.

Because `zpbuild` reproduces the old (pre-split) `zplink` ROM output exactly,
existing `.zpb` files are byte-for-byte unchanged. The implementation is
self-contained C89 (`src/zpsha256.h`, `src/zprsa.h`) and produces signatures
**byte-identical to OpenSSL** for the same key. Verify a built ROM out-of-band:

```sh
# split off header+payload and the trailing signature, then:
openssl pkeyutl -verify -pubin -inkey devpub.pem \
    -in rom_digest.bin -sigfile rom_sig.bin -pkeyopt digest:sha256
```

### Local testing vs shipping

- **Local:** `zpbuild game.elf -o game.zpb` signs with the bundled dev key; the
  emulator loads it (it ignores the trailer). This is the fast edit-run loop.
- **Shipping:** hand `game.elf` to HQ. Their `zpbuild` (built against the
  production key) makes the ROM that real hardware will trust. Do **not** ship a
  locally dev-key-signed `.zpb`.

---

## Notes / known toolchain quirks

- `rominspect` auto-detects and reports on all three artifact formats: the ELF
  from `zplink` (lists load segments and `.text`/`.ppu`/`.apu`/… sections),
  the signed `.zpb` from `zpbuild` (header + payload + `ZPSG` trailer, with
  `-c` recomputing the SHA-256 digest), and raw/headerless blobs (a rombuilder
  flat image or a bare assembler object). It follows the authoritative
  `include/rom.h` header layout.
- `cpuasm`'s opcode table and the emulator's opcode map currently disagree on
  some encodings. The linker/signer are byte-agnostic (they never interpret
  instructions), so they are unaffected — but hand-written asm may need that
  reconciled before it runs on the emulator.

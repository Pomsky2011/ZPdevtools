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
        [--selftest] [-v]
```

| Option | Meaning |
|--------|---------|
| `IN.elf` | A `zplink` ELF. `zpbuild` reconstructs the flat payload from its `PT_LOAD` segments. |
| `-o OUT.zpb` | The signed ROM. |
| `--title` / `--dev` | ROM metadata stamped into the header at signing time. |
| `--entry ADDR` | Override the entry point (default: the ELF's entry). |
| `--selftest` | Run the SHA-256 + RSA sign/verify/tamper self-test and exit. |

The payload is wrapped in the emulator's 64-byte `ZPBHeader` (see
`ZeroPoint/include/rom.h`) and then **signed**:

```
+------------------+  offset 0
| ZPBHeader (64)   |  magic "ZPB", version 2, flags, romSize, entry, title, dev
+------------------+  offset 64
| payload (romSize)|  the located image (CPU@base, PPU, APU, data...)
+------------------+  offset 64 + romSize
| "ZPSG" trailer   |  8-byte tag + SHA-256 digest (32) + RSA-2048 signature (256)
+------------------+
```

The stock emulator reads exactly `romSize` payload bytes, so the signed trailer
is transparent to it — an unsigned-aware loader still works, and a
signature-checking loader (real hardware) finds the trailer at `64 + romSize`.

### Signing & keys

The signature is **RSA-2048, PKCS#1 v1.5 over SHA-256**, computed over
`ZPBHeader ‖ payload` (so the entry point, title, and size are authenticated).
The key compiled into `zpbuild` from `src/zpkey.h` is a **development** key
(Montgomery constants precomputed; *not* secret-grade) — good enough for a
locally-loadable ROM the emulator accepts. A real vendor builds `zpbuild`
against their own private key kept on the mastering machine.

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

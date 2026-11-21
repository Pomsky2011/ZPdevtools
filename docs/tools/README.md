# Development tools

Everything here is plain C89 built by `make` into `executables/`, and each tool also builds for MS-DOS ([../dos/](../dos/README.md)).
Usage below is what each tool prints and does today.

| Tool | Does | Usage |
|------|------|-------|
| `cpuasm` | assemble DEF88186 code | `cpuasm input.asm [output.bin]` |
| `ppuasm` | assemble PPU microcode | `ppuasm input.asm [-o output.bin]` |
| `apuasm` | assemble APU programs | `apuasm input.asm [output.bin]` |
| `cpudisasm` `ppudisasm` `apudisasm` | disassemble | `<tool> input.bin [-o file] [-s hex] [-e hex] [-c] [-b]` |
| `def88186cc` | C compiler, strict C89 subset | `def88186cc input.c -o output.asm` |
| `zpcc` | C compiler, clang/LLVM backend | `zpcc [-o out.asm] [-I dir] [-D macro] [-O<n>] input.c` |
| `zplink` | locate flat segments into an ELF | see [../sdk/linker.md](../sdk/linker.md) |
| `zpbuild` | sign an ELF into a `.zpb` ROM | see [../sdk/linker.md](../sdk/linker.md) |
| `rominspect` | inspect an ELF, `.zpb` or raw file | `rominspect file [-v] [-x] [-c]` |
| `hexview` | hex dump | `hexview file [-s off] [-n count] [-w width] [-g group] [-a] [-o file] [--analyze]` |

## The build pipeline

Assemble each part, link the parts into an ELF (no signing key involved), sign the ELF into a ROM, and check it:

```sh
cpuasm main.asm main.bin
ppuasm gfx.asm gfx.bin
apuasm snd.asm snd.bin
zplink -o game.elf --cpu main.bin@0x8000 --ppu gfx.bin@0x100000 --apu snd.bin@0x200000 --entry 0x8000
zpbuild game.elf -o game.zpb --title "My Game" --dev "Me"
rominspect game.zpb -c
```

The emulator loads the `.zpb`. `examples/link/` is this pipeline as a Makefile (`make elf`, `make rom`, `make clean`). A C program
adds one step in front: `zpcc main.c -o main.asm` (or `def88186cc`), then `cpuasm`, with `lib/crt0_entry.asm` included first
and the SDK headers on the include path ([../sdk/README.md](../sdk/README.md)).

## Assemblers

Each assembler is documented with the processor it targets: [cpuasm](../cpu/programming-guide.md),
[apuasm](../apu/programming-guide.md), [ppuasm](../ppu/assembler.md). The traps worth knowing up front:

- **`cpuasm`** looks for an `.include` next to the including file first, then in the working directory. A missing include
  prints `Cannot open input file: name` and then fails on every label that file would have defined; a label defined twice
  (a file included twice) is an error, and so is a file that includes itself. Directives: `.org` `.bank` `.equ` `.byte` `.word`
  `.long` `.include` (and `.data`/`.code`, accepted and ignored); `.byte`/`.word`/`.long` take numbers, `.equ` names and labels,
  and refuse a number that does not fit. `BNE`, `BPL`, `BVC` and `BCC` are pseudo-instructions. See
  [multifile-assembly.md](multifile-assembly.md).
- **`apuasm`** resolves `.include` relative to the including file. Directives: `.org` `.db` `.dbh` `.include` `.rp`. Its RP lint
  rejects a jump to a label in another page.
- **`ppuasm`** has no directives at all, and builds 16-bit constants only through `TARREG`/`SETBYTE` (`SETBYTE` refuses a plain number over 255).

## Disassemblers

```sh
cpudisasm game.bin                       # whole file
cpudisasm game.bin -s 8000 -e 8100 -b    # a range, with the bytes
apudisasm snd.bin -o snd.txt             # to a file
```

| Option | |
|--------|---|
| `-o file` | write to a file instead of the terminal |
| `-s addr`, `-e addr` | start and end address, in hex |
| `-b` | show the bytes of each instruction |
| `-c` | show comments |

The output is the assembler's own syntax, so it can be assembled again; it starts with a header giving the file's size and
address range, then one `ADDR:  instruction` line per instruction. The PPU and APU disassemblers print the canonical form
of each instruction, which is how [../ppu/encodings.md](../ppu/encodings.md) and [../apu/encodings.md](../apu/encodings.md) are generated.

## rominspect

`rominspect` recognises three kinds of file and reports on each:

| Input | Report |
|-------|--------|
| ELF from `zplink` | header, load segments with their addresses and sizes, sections |
| `.zpb` from `zpbuild` | header, signature, and with `-c` an integrity check |
| anything else | treated as a raw blob |

`-v` is verbose. `-x` extracts the parts: from an ELF it writes `NAME_text.bin` (the CPU segment), `NAME_ppu.bin` and
`NAME_apu.bin`; from a `.zpb` it writes `NAME_payload.bin`, the whole located image.

## hexview

```sh
hexview game.elf -n 32            # the first 32 bytes
hexview game.bin -s 0x100 -w 8    # from offset 0x100, 8 bytes a line
hexview game.bin --analyze        # file statistics
```

`-g 1|2|4` groups bytes, `-a` prints addresses in hex only, `-o` writes to a file.

## Scripts

| Script | Does |
|--------|------|
| `scripts/zpsnd.py` | bake samples and a score into a ZPA sound pack ([../apu/zpsnd.md](../apu/zpsnd.md)) |
| `scripts/build_adpcm_rom.py` | audio file to an ADPCM-playing ROM ([../apu/adpcm.md](../apu/adpcm.md)) |
| `scripts/gen_cpu_docs.py`, `gen_apu_docs.py`, `gen_ppu_docs.py` | regenerate the generated pages under `docs/` |
| `scripts/check_doc_asm.py` | assemble every `cpuasm`, `apuasm` and `ppuasm` block in the Markdown docs |

## Building the tools

```sh
make                 # everything into executables/
make assemblers      # also: disassemblers, rom-tools, linkers, utilities
make examples        # assemble the PPU examples
make check           # build, then the fast tool tests: assemblers, C89 conformance, linker, disassemblers
make clean
make install
make help
```

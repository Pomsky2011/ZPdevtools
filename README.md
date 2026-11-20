# ZeroPoint Development Tools

The toolchain for the ZeroPoint fantasy console: assemblers, two C compilers, a linker and ROM signer,
disassemblers and a few utilities. The emulator lives in the sibling `ZeroPoint` repo.

Every tool except `zpcc` (C++, needs clang) is strict C89, so the rest can also be built on MS-DOS (see `docs/dos/DOS_BUILD.md`).

## Tools

| Group | Tool | What it does |
|---|---|---|
| Assemblers | `cpuasm` | DEF88186 assembly, `.include`, operand range checks |
| | `ppuasm` | PPU microcode with shorthand macros (`-o out.bin`; a positional output name is ignored) |
| | `apuasm` | APU programs (MMP/SST) |
| Compilers | `def88186cc` (`c_compiler/`) | A subset of C89, DOS-portable; refuses what it cannot compile |
| | `zpcc` (`llvm_compiler/`) | Full C through clang/LLVM; needs clang |
| Linking | `zplink` | Locates flat CPU/PPU/APU segments into an ELF32 executable |
| | `zpbuild` | Signs a zplink ELF into a `.zpb` ROM (`--selftest` checks the signing path) |
| | `rombuilder` | Older, unsigned CPU+PPU+APU ROM packer |
| | `rominspect` | Inspects ELF / ZPB / raw files (`-v` verbose, `-c` check, `-x` extract) |
| Disassemblers | `cpudisasm`, `ppudisasm`, `apudisasm` | `-b` bytes, `-c` comments, `-s`/`-e` range |
| Utilities | `hexview` | Hex viewer with `--analyze` statistics |
| | `wav2mmp` | WAV to an APU MMP sample include |

Python tools in `scripts/`: the ADPCM player builder (`build_adpcm_rom.py`, `docs/apu/adpcm.md`) and the ZPA sound pack
baker `zpsnd.py` (`docs/apu/zpsnd.txt`).

## Building

```bash
make -j$(nproc)          # every tool into executables/
make examples            # assemble examples/ppu/*.asm to .bin
make -C c_compiler       # def88186cc (needs flex and bison)
make -C llvm_compiler    # zpcc
```

Nothing built is tracked: `executables/`, `*.bin`, `*.zpb` and `*.elf` are all gitignored.

## Quick start

```bash
executables/ppuasm examples/ppu/simple_white_pixel.asm -o pixel.bin
executables/apuasm examples/apu/hello.asm hello.bin
executables/cpuasm examples/cpu/test.asm test.bin
make -C examples/link                      # assemble, link with zplink, sign with zpbuild -> game.zpb
```

Run a PPU program in the emulator: from `ZeroPoint/build`, `./bin/run_demo /path/to/ZPdevtools/pixel.bin`.

## Testing

```bash
make check               # assembler, robustness, C89, zplink/zpbuild and disassembler suites
tests/cc/run.sh          # compiler suite: gcc vs both compilers in the emulator (see tests/cc/README.md)
tests/cc/examples_check.sh   # c_compiler/examples compile; examples/unsupported are refused
python3 tests/asm/fuzz_asm.py    # assembler fuzzer (not part of make check)
```

## Documentation

| | |
|---|---|
| `docs/DEV-TOOLS.md` | Usage of every tool |
| `docs/LINKER.md`, `docs/SDK.md` | zplink/zpbuild and the C SDK (`include/zeropoint/`, `lib/`) |
| `docs/ROMBUILDER_GUIDE.md`, `docs/ASM_MULTIFILE_GUIDE.md` | rombuilder, multi-file assembly |
| `docs/cpu/` | DEF88186: architecture, addressing modes, instructions, opcode table, memory map, interrupts, timing, programming guide |
| `spec/` | Machine-read specs: the CPU opcode map and the measured per-opcode table (see `spec/README.md`) |
| `docs/ppu/` | `ucode.txt` (instruction reference), `preset-e-and-shorthands.txt`, `layers.md` (BG layers + sprites microcode, in `ucode/layers/`) |
| `docs/apu/` | APU: `README.txt` is the index; also MMP, SST, ZPA sequencer, ADPCM |
| `docs/dos/` | Building on MS-DOS, C89 porting rules |
| `c_compiler/README.md` | What `def88186cc` accepts and refuses |
| `llvm_compiler/README.md` | `zpcc` |

`game-template/` is a git submodule (`ZPgameTemplate`): a buildable skeleton for a ZeroPoint game.

### PPU assembler notes

Assembler shorthands (label jumps such as `JMR loop`, `PUSH`/`POP`/`JSR`/`RET`, `HLT`) use target registers 0 and 1
internally. Use target registers 2 and 3 for manual `TARREG`/`SETBYTE`/`BUILD` work. Full reference:
`docs/ppu/preset-e-and-shorthands.txt` and `docs/ppu/ucode.txt`.

## License

Licensed under the Mozilla Public License v2.0 (MPL-2.0) — see [LICENSE](LICENSE).

MPL-2.0 is file-level copyleft: modifications to these tools' own source files must stay under MPL and have their source made available if distributed. Programs, ROMs, or games you build *with* these tools (via `cpuasm`, `ppuasm`, `apuasm`, `def88186cc`, `rombuilder`, etc.) are not Covered Software and are not subject to this license — you may license your own output however you like.

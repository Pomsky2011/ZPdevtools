# ZeroPoint development documentation

Reference and guides for the ZeroPoint fantasy console's toolchain. Everything here describes the emulator at revision 10 (which
is frozen) and was checked against it: see "How these docs are kept honest" below.

## The machine

| | |
|---|---|
| [cpu/](cpu/README.md) | The DEF88186 main CPU: architecture, memory map, addressing modes, instructions, the 256-opcode table, interrupts, timing, programming guide |
| [apu/](apu/README.md) | The audio processor and its mixer: architecture, instruction set, sample storage, programming guide, the ZPA sequencer, the ADPCM player |
| [ppu/](ppu/README.md) | The graphics processor: architecture, instruction set, tiles and blitters, `ppuasm`, programming guide, the layers microcode |

## The tools

| | |
|---|---|
| [tools/](tools/README.md) | Every tool's usage and traps, multi-file assembly, and the legacy `rombuilder` |
| [sdk/](sdk/README.md) | The C SDK, the [C calling convention](sdk/abi.md), and [`zplink` / `zpbuild`](sdk/linker.md) |
| [dos/](dos/README.md) | Building on MS-DOS and the C89 rules |

The two C compilers have their own references: [`c_compiler/README.md`](../c_compiler/README.md) (what `def88186cc` accepts and
refuses) and [`llvm_compiler/README.md`](../llvm_compiler/README.md) (`zpcc`).

## Where to start

- **Writing assembly for the CPU:** [cpu/architecture.md](cpu/architecture.md), then [cpu/programming-guide.md](cpu/programming-guide.md).
- **Making sound:** [apu/architecture.md](apu/architecture.md), then [apu/programming-guide.md](apu/programming-guide.md); for a
  jingle or music, [apu/zpsnd.md](apu/zpsnd.md).
- **Drawing:** [ppu/architecture.md](ppu/architecture.md), then [ppu/graphics.md](ppu/graphics.md).
- **Writing in C:** [sdk/README.md](sdk/README.md), then [sdk/abi.md](sdk/abi.md).
- **Building a ROM:** the pipeline at the top of [tools/README.md](tools/README.md).

## How these docs are kept honest

- Reference tables are **generated**, not typed: [cpu/opcode-table.md](cpu/opcode-table.md) from a run of every opcode on the
  emulator, and [apu/encodings.md](apu/encodings.md) and [ppu/encodings.md](ppu/encodings.md) from the assemblers and
  disassemblers. Regenerate with `scripts/gen_cpu_docs.py`, `gen_apu_docs.py` and `gen_ppu_docs.py`.
- Every `cpuasm`, `apuasm` and `ppuasm` code block is assembled by `python3 scripts/check_doc_asm.py`, and the programs that state
  a result were run on the emulator.
- Where the docs record a surprising behaviour (a jump that goes to the wrong bank, an interrupt that wedges a loop), it was
  reproduced rather than inferred from the source. The tool bugs that turned up along the way are in [../TODO.md](../TODO.md).
- The opcode map and the measured per-opcode data live in [../spec/](../spec/README.md).

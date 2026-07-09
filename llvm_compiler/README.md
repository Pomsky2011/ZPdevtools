# zpcc — clang/LLVM C compiler for the DEF88186

`zpcc` is the modern companion to `def88186cc`. Where `def88186cc` is a small,
self-contained, DOS-portable compiler, `zpcc` leans on a full **clang/LLVM**
front end for fast, standards-accurate parsing, then lowers the resulting LLVM
IR to DEF88186 assembly that `cpuasm` accepts.

```
input.c ──clang(--target=msp430 -emit-llvm -O0)──▶ input.ll
input.ll ──zpcc backend──▶ input.asm ──cpuasm──▶ input.bin
```

## Why MSP430?

DEF88186 is not a real LLVM target, so `zpcc` never uses LLVM's code generator.
It only uses clang's **front end** and the **MSP430 data layout**, which happens
to be an exact model of the DEF88186 programming model:

- 16-bit `int`
- 16-bit pointers
- little-endian

`zpcc` walks the IR itself and emits DEF88186 mnemonics.

## Building

Requires an LLVM/clang development install (`llvm-config` on `PATH`). Tested with
LLVM 22.

```bash
make            # produces ./zpcc
```

## Usage

```bash
./zpcc program.c -o program.asm      # C  → DEF88186 assembly
./zpcc program.ll -o program.asm     # LLVM IR → assembly (skips clang)
./zpcc program.c --emit-llvm --keep-ll   # inspect the intermediate IR
../executables/cpuasm program.asm program.bin
```

| Option | Meaning |
|--------|---------|
| `-o <file>` | Output assembly file (default `<input>.asm`) |
| `-O<n>` | clang front-end optimization level (default `0`) |
| `--emit-llvm` | Emit the intermediate `.ll` and stop |
| `--keep-ll` | Keep the intermediate `.ll` |

> The backend consumes `-O0`-style IR (explicit `alloca`/`load`/`store`). Higher
> `-O` levels may introduce constructs the backend does not lower yet (notably
> PHI nodes from `mem2reg`); stay at `-O0` unless you know the IR stays simple.

## ABI

Identical to `def88186cc`, so the two toolchains interoperate and share `cpuasm`:

- 16-bit mode (`REP #$30`)
- first three arguments in `A`, `X`, `Y`; remaining args pushed right-to-left
- return value in `A`
- each function allocates a fixed-size stack frame in its prologue and leaves
  `S` untouched for the body, so locals stay addressable as `off,S`

## Supported IR subset

`alloca`, `load`, `store`, `getelementptr`, `add`/`sub`/`mul`/`[us]div`/`[us]rem`,
`and`/`or`/`xor`, `shl`/`lshr`/`ashr`, `icmp`, `br`, `ret`, `call`,
`zext`/`sext`/`trunc`/`bitcast`/`ptrtoint`/`inttoptr`, `select`, and
integer/pointer/byte-array (string) globals. Anything else is reported with a
clear diagnostic instead of producing silently-wrong code.

## Known limitations

- **Not machine-optimized.** Every SSA value gets its own stack slot, so `-O0`
  IR produces verbose (but correct) code. The value here is clang's correctness,
  not code size.
- **Unsigned compares are lowered as signed** (`BMI`/`BPL`); the ISA exposes no
  carry-based conditional branch in the classic compiler's mnemonic set.
- **`ashr` uses `LSR`** (logical shift) — no arithmetic-right yet.
- **Relative branches** (`BRA`/`BEQ`/…) limit the size of a single function;
  very large functions can exceed branch range.
- **Semantic validation is at the assembler level so far** (output assembles
  cleanly via `cpuasm`, structure mirrors the known-good `def88186cc` ABI). End-
  to-end execution should be validated through the full ROM/system integration
  path, since the DEF88186 CPU runs interconnected with the PPU/APU/DMA over the
  24-bit banked memory map rather than in isolation.

## Examples

See `examples/`:

- `factorial.c` — recursion, `if`, `for` loop
- `boolean.c` — `bool`, pointers, array indexing (`arr[i]`)

```bash
./zpcc examples/factorial.c && ../executables/cpuasm examples/factorial.asm factorial.bin
```

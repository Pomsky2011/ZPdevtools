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
`and`/`or`/`xor`, `shl`/`lshr`/`ashr`, `icmp`, `br`, `switch`, `ret`, `call` (including `llvm.memcpy` and
`llvm.memset`), `zext`/`sext`/`trunc`/`bitcast`/`ptrtoint`/`inttoptr`, `select`, and read-only
integer/array/struct globals. Anything else is reported with a clear diagnostic, the exit status is non-zero and no
output file is left behind.

## Memory model

A pointer is a 16-bit offset into **bank $80**, the bank that holds the stack and all data. Every load and store through
one is `LDA`/`STA $800000,X` (bytes in 8-bit mode, a `long` as two words), which does not depend on the data bank
register. Locals, arrays and structs live in the stack frame; `def88186cc` uses the same model and the same struct layout
(clang's 16-bit data layout), so the two toolchains agree on both.

Globals are **ROM** (bank $00 in a cartridge, wherever the code was assembled):

- a global can be **read by name**: indexed in place (`table[i]`), through struct members, or copied into a local array
  (`memcpy` from a constant initialiser is how `int a[4] = {1,2,3,4}` works);
- its **address cannot be used as a pointer value** (passing it, storing it, comparing it, `"text"` as a `char *`): it
  would be mistaken for a bank-$80 address. That is an error, not wrong code;
- it **cannot be written**: there is no writable data segment yet. A store is an error.

## Known limitations

- **Not machine-optimized.** Every SSA value gets its own stack slot, so `-O0` IR produces verbose (but correct) code.
  A function whose frame, with its temporaries, passes **255 bytes** is an error (`n,S` has a one-byte operand; the
  assembler now rejects what it cannot encode instead of truncating it): split it up or use smaller arrays.
- No `memmove`, no integers wider than 32 bits, no floating point, no varargs, no struct values (by-value copy, passing
  or returning); an indirect call through a function pointer carries at most one argument.
- Plain `char` follows clang's MSP430 default; use `signed char` / `unsigned char` when it matters.

## Testing

`tests/cc/run.sh zpcc` compares it with gcc in the emulator, `FUZZ_CC=zpcc python3 tests/cc/fuzz_ops.py N SEED` and
`FUZZ_CC=zpcc FUZZ_SMALL=1 python3 tests/cc/fuzz_mem.py N SEED` fuzz it against reference models, and
`python3 tests/cc/zpcc_reject_tests.py` checks every refusal above.

## Examples

See `examples/`:

- `factorial.c` — recursion, `if`, `for` loop
- `boolean.c` — `bool`, pointers, array indexing (`arr[i]`)

```bash
./zpcc examples/factorial.c && ../executables/cpuasm examples/factorial.asm factorial.bin
```

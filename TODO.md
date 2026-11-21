# ZPdevtools TODO

Open items only. History is in `git log`; the BUILD encoder/decoder bug write-up is in the history of `docs/ppu/ucode.txt` §15.4 (removed in the PPU docs rewrite).

## Known limitations

- **`HLT` is a loop, not a CPU halt.** ppuasm expands it to a short infinite loop (it uses target registers 0 and 1).
- **Shorthands overwrite target registers 0 and 1** (label jumps, `PUSH`/`POP`/`JSR`/`RET`, `HLT`). Use 2 and 3 for manual
  `TARREG`/`SETBYTE`/`BUILD`. Documented in `docs/ppu/assembler.md`.
- **No immediate `MOV` in ppuasm.** The hardware `MOV` is DP-addressed only; use `SETBYTE`, which takes full expressions.
- **`JMR label` main loops can be wedged for good** by an interrupt whose handler ends in `RET` (it leaves `R62` pointing at
  the `JMR`). That is how the PPU behaves (revision 13), not an assembler bug; the safe idle pattern is in
  `docs/ppu/architecture.md`.
- **The DOS build is unverified.** `Makefile.dos` / `scripts/build-dos.bat` are not exercised in CI, only the C89 conformance
  (`tests/c89/check.sh`) is. See `docs/dos/README.md`.

## Bugs found writing the layers C library (`include/zeropoint/zp_layers.h`)

The library works around each of these; they are real and should be fixed (and given a regression test in `tests/cc/` when they are).

- **`def88186cc` crashes (segfault) on a call result or an integer constant cast straight to a struct pointer**: `c = (struct C*)f();`,
  `c = (struct C*)0x4000;`. Casting a *variable* works, and `(struct C*)expr` inside a larger expression is refused with a clean error (so
  is `f()->m`). A crash should be an error, like the other refusals.
- **`def88186cc` mis-scales `(struct S*)b + i`**: the offset is `2 * i` bytes, not `i * sizeof(struct S)` (a miscompile, which the
  compiler promises not to do). `s = (struct S*)b; s + i` is right.
- **`def88186cc` rejects several declarators in one struct member declaration** (`short x, y;`: parse error), though locals accept them.
- **`zpcc` cannot turn an integer constant into a pointer** (`(unsigned char*)0x4000`: "unsupported constant expression"); from a variable
  or a function's result it can. That is why the library reaches its RAM through `zpl_mem(int off)`.
- **`zp_peek8` cannot read ROM above bank 0**: any bank but `$80`/`$A0`/`$B0`/`$D8` is read from bank `$00` at the same low 16 bits, so
  `zp_peek8(0x110C00UL)` returns a byte of bank 0. DMA reads ROM fine; the library stages ROM data through RAM. `lib/zp_mmio.asm` could dispatch ROM
  banks (or copy through a DMA of its own).
- **The layers library is slow** (~10-12k CPU cycles per sprite per frame, the compilers emit unoptimised code): see `docs/sdk/README.md`. Hand-written
  assembly for `zpl_pass` and `zpl_layout`, or letting the host precompute stream blocks, are the two ways out.

## Open

- Verbose/debug mode for the assemblers.
- Symbol table export for debugging.
- `def88186cc`: optimisation passes (constant folding, dead code, peephole); switch jump tables. Subset limits are in
  `c_compiler/README.md`.
- Docs: troubleshooting and loop-debugging guides.

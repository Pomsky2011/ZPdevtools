# ZPdevtools TODO

Open items only. History is in `git log`; the BUILD encoder/decoder bug write-up is in the history of `docs/ppu/ucode.txt` §15.4 (removed in the PPU docs rewrite).

## Known limitations

- **`HLT` is a loop, not a CPU halt.** ppuasm expands it to a short infinite loop (it uses target registers 0 and 1).
- **Shorthands overwrite target registers 0 and 1** (label jumps, `PUSH`/`POP`/`JSR`/`RET`, `HLT`). Use 2 and 3 for manual
  `TARREG`/`SETBYTE`/`BUILD`. Documented in `docs/ppu/assembler.md`.
- **No immediate `MOV` in ppuasm.** The hardware `MOV` is DP-addressed only; use `SETBYTE`, which takes full expressions.
- **`JMR label` main loops can be wedged for good** by an interrupt whose handler ends in `RET` (it leaves `R62` pointing at
  the `JMR`). That is how the frozen rev-10 PPU behaves, not an assembler bug; the safe idle pattern is in
  `docs/ppu/architecture.md`.
- **The DOS build is unverified.** `Makefile.dos` / `scripts/build-dos.bat` are not exercised in CI, only the C89 conformance
  (`tests/c89/check.sh`) is. See `docs/dos/README.md`.

## Open

- Verbose/debug mode for the assemblers.
- Symbol table export for debugging.
- `def88186cc`: optimisation passes (constant folding, dead code, peephole); switch jump tables. Subset limits are in
  `c_compiler/README.md`.
- Docs: troubleshooting and loop-debugging guides.

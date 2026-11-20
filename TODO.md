# ZPdevtools TODO

Open items only. History is in `git log`; the BUILD encoder/decoder bug write-up is `docs/ppu/ucode.txt` §15.4.

## Known limitations

- **`HLT` is a loop, not a CPU halt.** ppuasm expands it to a short infinite loop (it uses target registers 0 and 1).
- **Shorthands overwrite target registers 0 and 1** (label jumps, `PUSH`/`POP`/`JSR`/`RET`, `HLT`). Use 2 and 3 for manual
  `TARREG`/`SETBYTE`/`BUILD`. Documented in `docs/ppu/preset-e-and-shorthands.txt`.
- **No immediate `MOV` in ppuasm.** The hardware `MOV` is DP-addressed only; use `SETBYTE`, which takes full expressions.
- **The DOS build is unverified.** `Makefile.dos` / `scripts/build-dos.bat` are not exercised in CI, only the C89 conformance
  (`tests/c89/check.sh`) is. See `docs/dos/DOS_BUILD.md`.

## Open

- Verbose/debug mode for the assemblers.
- Symbol table export for debugging.
- `def88186cc`: optimisation passes (constant folding, dead code, peephole); switch jump tables. Subset limits are in
  `c_compiler/README.md`.
- Docs: troubleshooting and loop-debugging guides.
- `cpuasm`: `.word label` and `.byte label` emit **nothing**, with no error (only numeric operands work), so
  `examples/cpu/test.asm`'s `.word reset` never wrote its vector. Needs forward-reference fixups.
- `cpuasm`: aliases the opcode CSV lists that it rejects: `LP` (`LOOP`), `JML [addr]`, `PUSHF`, `PPP`, `INA`, `DEA`.
- `cpuasm`: its internal cycle numbers disagree with the emulator for 23 opcodes (indexed stores and read-modify-writes
  are 1 too low; `MUL` is listed at 9-13, the emulator charges a flat 8). Lengths all agree.
- `include/zeropoint/zp_mmio.h` puts the Boot ROM at `$FF0000` (it is `$E0`) and RAM at 2 MB (the console has 128 KB).

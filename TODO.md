# ZPdevtools TODO

Open items only. History is in `git log`; the BUILD encoder/decoder bug write-up is in the history of `docs/ppu/ucode.txt` §15.4 (removed in the PPU docs rewrite).

## Known limitations

- **`HLT` is a loop, not a CPU halt.** ppuasm expands it to a short infinite loop (it uses target registers 0 and 1).
- **Shorthands overwrite target registers 0 and 1** (label jumps, `PUSH`/`POP`/`JSR`/`RET`, `HLT`). Use 2 and 3 for manual
  `TARREG`/`SETBYTE`/`BUILD`. Documented in `docs/ppu/assembler.md`.
- **No immediate `MOV` in ppuasm.** The hardware `MOV` is DP-addressed only; use `SETBYTE`, which takes full expressions.
- **ppuasm reads `0b101` as 0 with no error**, and `SETBYTE` truncates a number above 255 to its low byte (only labels are
  split by the target's LSB/MSB). `JMR label` main loops can be wedged for good by an interrupt whose handler ends in `RET`
  (it leaves `R62` pointing at the `JMR`); the safe idle pattern is in `docs/ppu/architecture.md`.
- **The DOS build is unverified.** `Makefile.dos` / `scripts/build-dos.bat` are not exercised in CI, only the C89 conformance
  (`tests/c89/check.sh`) is. See `docs/dos/README.md`.

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
- `wav2mmp` (the C tool and `scripts/wav2mmp.py`) writes APU assembly that does not assemble: it emits the removed `SBF` instruction.
  `zpsnd.py` is the working way to make sample data.
- `rombuilder` writes a headerless flat image that the emulator rejects ("bad magic number") yet reports as ready to use. It is
  superseded by `zplink` + `zpbuild`; either make it emit a ZPB or remove it.
- `cpuasm` does not diagnose a duplicate label (a file included twice assembles twice) and resolves `.include` relative to the
  working directory, where `apuasm` uses the including file.

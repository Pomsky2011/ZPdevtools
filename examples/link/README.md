# Link example

End-to-end demonstration of the dev-side to mastering pipeline: `cpuasm` assembles the program, `zplink` locates
the CPU/PPU/APU segments into an ELF, and `zpbuild` signs that ELF into a `.zpb` ROM.

```sh
make            # = make rom: game.bin, gfx.bin, snd.bin -> game.elf -> game.zpb
make elf        # dev-side link only -> game.elf
make rom        # sign the ELF -> game.zpb (uses the bundled dev key)
make clean
```

Files:

- `game.asm` — a minimal CPU program that `.include`s `lib/crt0_entry.asm`
  (the init stub that slides into `main`), DMA-loads a PPU blob, and starts the
  PPU. Assembles with `cpuasm` alone.
- `main.c` — the same idea written against the C SDK; compiles with both `zpcc`
  and `def88186cc -DZP_CC_DOS` (see `../../docs/sdk/README.md`). The Makefile does not build it.
- `gfx.bin` / `snd.bin` — placeholder PPU/APU asset blobs the Makefile generates (replace with
  `ppuasm`/`apuasm`/`wav2mmp` output; keep them 256-byte aligned).

Inspect the results:

```sh
readelf -l game.elf         # program headers: CPU@0x8000, PPU@0x100000, APU@0x200000
../../executables/zpbuild --selftest  # verify the SHA-256 + RSA-2048 signing path
```

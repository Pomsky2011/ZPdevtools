# Link example

End-to-end demonstration of `zplink`'s two build modes.

```sh
make            # builds both:
                #   entry mode -> game.img      + game.img.elf
                #   rom   mode -> game.zpb (signed) + game.zpb.elf
make entry      # entry-mode image + ELF only
make rom        # signed ROM + ELF only
make clean
```

Files:

- `game.asm` — a minimal CPU program that `.include`s `lib/crt0_entry.asm`
  (the init stub that slides into `main`), DMA-loads a PPU blob, and starts the
  PPU. Assembles with `cpuasm` alone.
- `main.c` — the same idea written against the C SDK; compiles with both `zpcc`
  and `def88186cc -DZP_CC_DOS` (see `../../docs/SDK.md`).
- `gfx.bin` / `snd.bin` — placeholder PPU/APU asset blobs (replace with
  `ppuasm`/`apuasm`/`wav2mmp` output; keep them 256-byte aligned).

Inspect the results:

```sh
readelf -l game.zpb.elf     # program headers: CPU@0x8000, PPU@0x100000, APU@0x200000
zplink --selftest           # verify the SHA-256 + RSA-2048 signing path
```

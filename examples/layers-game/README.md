# layers-game

A small game loop on the C layers library (`include/zeropoint/zp_layers.h`, docs in `docs/sdk/README.md`):

- two scrolling layers: a sky and a ground on BG0, clouds, bushes and trees on BG1 at twice the speed;
- a hero you move with the pad (`zp_pad.h`) that draws over everything;
- a bird that flies **between** the two layers (`ZPL_SPR_PRIO_MID`), so the clouds pass in front of it;
- a text box in the window layer, which never scrolls.

Everything is drawn by `gen_assets.py` (palette, tiles, maps, a 12-letter font), so there are no binary files to carry.

```sh
make              # build/game.zpb with zpcc (the clang compiler)
make CC=dos       # the same with def88186cc (use BUILD=build-dos to keep both)
make run          # zeropoint_sdl build/game.zpb
```

Needs the toolchain built first (`make` in `ZPdevtools`, in `llvm_compiler/` and/or `c_compiler/`) and python3. `tests/layers/run.sh` builds it with each
compiler, runs it headless and compares every pixel of four frames with an independent reconstruction, and checks the game advances exactly one step per frame.

How it is put together: `main.c` calls `zpl_init` (which boots `ucode/layers`' microcode from ROM at `$10:0000`), loads the assets (the Makefile places them
at `$11:0000`, `gen_assets.py` writes where each piece sits into `build/assets.h`), `zpl_start`, then each frame moves sprites and scrolls and calls `zpl_frame`.
`build/game.asm` is crt0, the compiled program and the runtime (`zp_mmio.asm`, `zp_cop.asm`, `zp_long.asm`) stuck together.

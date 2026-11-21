# layers-game

This example is the **game template** now: `../../game-template` (the [ZPgameTemplate](https://github.com/Pomsky2011/ZPgameTemplate) repo, a git
submodule). It is a small game on the C layers library (`include/zeropoint/zp_layers.h`, docs in `docs/sdk/README.md`): two scrolling layers,
a hero you move with the pad, a bird that flies between the layers, a text box in the window layer, all drawn from PNGs in the template's `assets/`
through `scripts/png2zp.py` (`docs/tools/png2zp.md`).

```sh
make              # build/game.zpb with zpcc (the clang compiler)
make CC=dos       # the same with def88186cc (use BUILD=build-dos to keep both)
make run          # zeropoint_sdl build/game.zpb
```

This Makefile just runs the template's from here. For a game of your own, copy the template out instead (its README says how).
`tests/layers/run.sh` builds the template with each compiler, runs it on the emulator (native boot and the real Boot ROM) and compares every pixel of
several frames with an independent reconstruction, checks that the game advances exactly one step per frame, and builds it again with other art
(`tests/layers/swap_assets.py`) to prove that swapping PNGs changes the picture.

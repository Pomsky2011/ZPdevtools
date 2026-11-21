# Two background layers + sprites + window layer microcode

Status: built and tested (2026-11-16; window layer added the same day). Two scrolling 64x64 background layers with pixel scroll on both
axes, sprites of any size in 8x8 tiles, and one standstill window layer above everything (textboxes, HUD), running on the PPU's tile-row
queue (ZeroPoint `docs/ppu.md` "Tile-Row Queue").
Generator `ZPdevtools/ucode/layers/gen_layers.py` (ppuasm has no data directives and a 16-bit immediate costs 8 cycles, so
the code is generated), CPU-side sprite list builder `zp_layers_sprites.h` and window table builder `zp_layers_window.h` (C89), layout
constants `zp_layers_layout.h` (generated), test `ZeroPoint/bin/test_layers <layers.bin> [seed]`. `gen_layers_v1.py` is the first version, which issued
every tile itself; it is kept because its measurements are why the hardware queue exists (see History).

## What the user asked for, and how it landed

| Asked | Result |
|-------|--------|
| 2 background layers, 64x64 maps, pixel scroll X and Y | Done. BG0 behind BG1, each layer scrolls independently, wrapping at 512 px |
| Sprites of configurable size | Done: any w x h in 8x8 tiles, hflip/vflip, array order within a priority |
| Sprite priority | Done, see "Sprite priority": behind BG0, between the layers, or above both (the default) |
| The PPU must never lag (a late band flickers the frame) | Guaranteed by construction and tested, see "Never lag" |
| Hardware: write-ahead queue, 128 tiles, framebuffer access given up | Done as a tile-**row** queue (a map row is one launch), 128 cells, `$E000-$FFFF` and `$C000-$DFFF` open bus in write-ahead mode |
| Late entries dropped with a sticky flag | Done (`$0364` bit 0); the tests assert it stays clear |
| RGBA16 only | Yes |
| 1 standstill layer above all the others, for textboxes | Done, see "Window layer": per band one ink span and one glass span, opaque ink hides the BG under it |
| A translucent box | Done as a blended fill (`glass`): any width (a full 32-cell row is ~12.5k of the band's ~278k channel-cycles) |

## CPU interface

PPU memory (write-ahead mode makes `$C000-$FFFF` open bus, so everything lives below it; the PPU has its own 64 KiB, the
CPU's 128 KB of WRAM is unaffected):

| Range | Use |
|-------|-----|
| `$0400-$0DFF` | code (about 1.8 KiB of it today; the generator refuses to run past `$0E00`) |
| `$0E00-$0F83`, `$0F90-$1113` | window tables, two buffers (`WIN0`, `WIN1`): 4-byte header + 32 bands x 12 B |
| `$1120-$119F` | window glass fill: one row of 32 identical cells (`GLFILL`) |
| `$11A0-$17FF` | window cell pool, 408 cells: where the cells of window boxes live (nothing else reads it) |
| `$1800-$1BFF` | sprite tile cells, 4 bytes each, room for 256 |
| `$1C00-$1DFF` | palette, 256 x BBGR16, loaded once at start with `PALETTE256` |
| `$1E00-$1E15` | control block, words: `NCOLS SPREN FRAMES L1FLAGS TILEREQ TILEBANK TILEID RUN LISTBASE WINBASE L0FLAGS` |
| `$1F00-$1FFF` | stack (grows up) |
| `$2000-$3FFF` | two frame blocks of 4 KiB (`$2000`, `$3000`): the four scroll words (`+0`), then the packed sprite stream from `+8` (a header word per band, then its 14-byte pieces; see "Sprite priority"; ~290 pieces fit) |
| `$4000-$7FFF` | BG0 map, 64x64 cells x 4 B |
| `$8000-$BFFF` | BG1 map |

- **Cells** (both maps and sprites): word0 = tile ID in the low byte, word1 = `bank | attrs | $1000` (bank bits 0-1, T = `$04`,
  H = `$08`, V = `$10`). Tile data is 8bpp (the 256-entry palette), so each layer can own an index range. A cell with T is
  see-through where its palette entry has alpha 0.
- **Frame block** (double-buffered): the sprite lists *and* the four scroll words `SCX0 SCY0 SCX1 SCY1` (0-511) live in one 4 KiB
  block at `LISTBASE`; the scroll words are at `+0` and the sprite stream starts at `+8`. The PPU snapshots `LISTBASE` (and `NCOLS`, `SPREN`, `L1FLAGS`) once per
  frame (and `WINBASE`), so the CPU fills the *idle* block (`$2000` or `$3000`) at its leisure, then writes `LISTBASE` to flip: scroll and sprites
  change together and nothing tears. Because `FRAMES` increments when the PPU takes the snapshot, the CPU can zero it, wait for
  it to turn non-zero, and then know the other block is free. `L1FLAGS` is OR'd into BG1's queue flags (`$40` makes the whole
  layer transparent where its tiles are alpha 0); `L0FLAGS` does the same for BG0 and is 0 (opaque) unless the CPU sets it, which is what
  lets a sprite *behind* BG0 show (see "Sprite priority"). Both are snapshotted once per frame.
- **Maps are read when drawn**: a queued row's cells are read at dispatch, not at launch. Update a map (or the sprite cells)
  only where the PPU is not about to draw, or double buffer it.
- **Sprites**: fill `ZPL_Sprite {x, y, w, h, cells, flags}` entries and call `zpl_build_lists()`, then `zpl_pack_stream()` to
  squeeze its dense rows into the frame block at `ZPL_STREAM_OFF` (after the scroll words). Each tile row of a sprite becomes one *piece* per 8-row band its rows touch, clipped to that band,
  so a sprite always sits in the same band as the BG rows it must go over. Later sprites are on top. All sprite pieces are
  transparent (alpha-0 pixels skipped). Set `SPREN` non-zero to enable sprites. `flags` also picks the sprite's priority.
- **Starting up and tiles**: the PPU idles until `RUN` is non-zero, serving tile uploads (tile pixels can only be written through the
  `$0300` window, which is keyed on the PPU's live bank/ID). To upload a tile the CPU writes `TILEBANK`, `TILEID`, then
  `TILEREQ = 1`; the PPU selects the tile and sets `TILEREQ = 2`; the CPU writes the tile's 64 bytes to `$0300-$033F` and sets
  `TILEREQ = 3`; the PPU clears it to 0. Upload the palette (`$1C00`), maps and sprite cells first, then set `RUN`: the PPU loads
  the palette and starts its frame counter level with the hardware's *then* (not at power-on: a cart can spend many frames
  uploading, and a counter initialised at power-on starts far behind the hardware's and every row is in the past).

## Window layer

One layer above BG0, BG1 *and* sprites that never scrolls: a 32 x 32 grid of 8x8 cells, described to the PPU as **spans** per 8-row band. The
CPU states it as a list of boxes (`zp_layers_window.h`, `ZPL_Box {col, row, w, h, cells, kind}`) and `zpl_build_window()` turns that into the
table. `WINBASE` (control word, snapshotted per frame like `LISTBASE`) points at the table; **0 = no window**, which costs nothing and is what
every cart that does not know about the layer has. Tables are double-buffered like the frame blocks: fill the idle one (`ZPL_WIN0` /
`ZPL_WIN1`), then write `WINBASE`.

| Kind | What it is | Effect on the layers below |
|------|------------|----------------------------|
| `ZPL_BOX_SOLID` | ink: your cells (4 B, same format as a map's), opaque | the BG under it is **not drawn** (the producer launches only the columns it does not hide) |
| `ZPL_BOX_OVERLAY` | ink drawn with T: alpha-0 pixels show what is below (text on glass, a border) | none: costs its full blit time |
| `ZPL_BOX_GLASS` | a blended fill from the fill row `ZPL_GLFILL` (you fill its 32 cells), drawn under the ink of the same band | none |

- **Spans.** A band holds at most one ink span and one glass span, each a run of cells in one row: of boxes that share a band, the first in array
  order wins there. Table: header `GLMODE` (1 average, 2 subtract, 3 add; the second header word is reserved for the alpha, see below), then per
  band six words `INKCNT INKCOL INKSRC INKFLG GLCNT GLCOL` (`INKFLG` 0 = solid, `ZPL_F_TRANSPARENT` = overlay).
- **Cells.** A box's cells live anywhere in PPU RAM (the pool at `ZPL_WINCELLS`, 408 cells, is what is free); they are read when drawn, like a
  map's. Cells of a SOLID box must be opaque: no T attribute and no alpha-0 colours, because the BG under them is not there to show through.
  This is a pool of boxes and not a flat 32 x 32 map (4 KiB) because PPU RAM has no 4 KiB left once the maps, the frame blocks, the code and
  the sprite cells are in; a 28 x 12 textbox is 336 cells.
- **Culling.** Under a solid span the producer skips every BG cell the span hides. With a fine X scroll the cells at the span's two ends still
  show part of themselves, so it draws `[0, ICOL + (FX != 0))` and `[ICOL + ICNT, 33)` of each layer, two launches per half instead of one.
  `test_layers` checks it pixel for pixel at the screen edges, 1-cell and full-width spans, every scroll phase.
- **Glass.** The blit snapshots the blend settings when the row is launched, so the producer pushes the blend mode just before the glass launch
  and puts blend mode 0 back right after it; nothing else is ever blended. **Alpha is fixed at 15**, so the strength is the mode's: average is
  50% of the box colour, subtract darkens by it, add lightens by it. Why not a variable alpha: `applyTranslucency` reads the *live*
  translucency when a blit retires, not the blit's snapshot, so a lower alpha would leak into every blit that retires between the push and the
  restore (it changes a pixel's alpha bit, and the glass test would need timing-dependent expectations). Using the snapshot there is a core
  change (`CORE_REVISION`), not done. A blended pixel over a destination that is exactly `$0000` is written unblended (the blit skips the blend
  when the destination is all zero).
- **Budget.** Per band, in the unit of `ZPL_BAND_BUDGET` (the sprite time over the worst background): a solid span *saves* time (the BG it
  hides costs 2 x 264 per column, it costs 264), an overlay costs 264 per cell, glass 392. `zpl_build_window(boxes, n, mode, budget, table,
  spriteBudget)` admits ink first, then glass, while the net cost stays within `budget`, drops what does not fit (returns the count) and fills
  `spriteBudget[b]`, what is left of the band for sprites: pass it to `zpl_build_lists_bands()`. At the shipped 150000 every box fits (a full
  row of glass is ~12.5k, of overlay ~8.4k) with ~125k left for sprites, which is about what `ZPL_MAXP` full-height 32-tile pieces ask for. (On
  the old 340-cycle line the budget was 1300 and only ~3 cells of glass fitted per band.)
- **Order.** Sprites behind BG0, BG0, sprites between, BG1, sprites above both, then the window per band (glass, then ink): queue order is z-order, and the hardware retires a blit only
  after the older ones it overlaps, so the glass reads a finished background and a sprite never shows through a box.
- **Producer.** A window band costs the PPU more than a plain one (the BG launches double under a culling span, plus the window's own), so
  the producer had to get cheaper first, see "History: producer cost".

## Sprite priority

A sprite's `flags` bits 2-3 pick where it sits in the band's launch order (`ZPL_SPR_PRIO_MASK`):

| Flags | Constant | Sits |
|-------|----------|------|
| `$00` | (none) | above BG0 and BG1 (the default, and the only choice before this existed) |
| `$04` | `ZPL_SPR_PRIO_MID` | between BG0 and BG1: BG1's opaque pixels cover it |
| `$08` | `ZPL_SPR_PRIO_BACK` | behind BG0 |

- **Behind BG0 needs a transparent BG0.** BG0 is opaque by default, so a sprite behind it is completely covered and never shows. Write
  `ZPL_F_TRANSPARENT` (`$40`) to `L0FLAGS` and BG0's alpha-0 pixels let it through; what is behind everything is black.
- **Within one priority** later sprites are on top, as before; the builder keeps array order and the admission budget (`ZPL_BAND_BUDGET`,
  `ZPL_MAXP`) is shared by all three priorities of a band, so a band never holds more than `ZPL_MAXP` pieces in total.
- **Stream format.** Per band the PPU reads a header word: bits 0-3 are the number of pieces above both layers, and `ZPL_EXT_FLAG` (`$10`)
  says the band also has pieces behind BG0 or between the layers. A plain band is the header and its pieces (14 B each), exactly the
  old format, so a stream built for the old microcode still runs. An extended band reads, after the header, `[count][behind pieces]
  [count][between pieces]` and then the above pieces, so it costs 4 B more. `zpl_build_lists_bands()` and `zpl_pack_stream()` do all of
  this; in the dense form the two extra counts sit at `ZPL_EXT_OFF`.
- **Cost.** The plain band is the old straight line (BG0, BG1, sprites, window) with one compare of overhead, and the saturation tests
  have the same edge as before (see "Never lag"). An extended band takes a separate path that costs about 40 PPU cycles more. A mixed-priority saturation
  case is in `test_layers`.
- **Demo:** the ship flies between the layers, so the clouds (BG1) pass in front of it.

## Never lag

The requirement was that the PPU can never lag, because one late band flickers the frame. Four things make that hold:

1. **Rows name their frame.** Every launch carries the frame number it belongs to (`$035E`) and the hardware places it. An
   earlier version computed a frame offset once per band and then blocked on queue space; by the time it launched, the
   hardware had crossed scanline 257 and the piece landed a whole frame late. The queue is in order, so that one held entry
   blocked everything behind it for a frame: exactly the cascade to avoid. With the frame in the launch there is no race.
2. **Late work cannot block.** A frame the beam has passed is dispatched at once and flagged late instead of waiting.
3. **Admission control is by blitter time and by count** (and a window takes its share out of the band, see above). The hardware has a fixed amount of blit time per band (8 channels,
   8 lines x 4343 cycles = 34,744 cycles each, ~278k channel-cycles; it was 2720 cycles each on the old 340-cycle line), the two layers take ~20k of it under a
   fine scroll, and what is left is what sprites may use. The list builder admits sprite pieces per band up to `ZPL_BAND_BUDGET` channel-cycles
   (a tile costs `8 + 32 x rows written`), and at most `ZPL_MAXP` = 15 pieces in total (the most the header's count field takes); anything
   beyond that is dropped deterministically (the builder returns the count). Since the raster was stretched the count is what binds: the widest
   possible load, 15 full-height pieces of 32-tile sprites, is ~127k channel-cycles, under half of the band's capacity. On the old line it was the other
   way round (a 4-tile piece cost four times a 1-tile one, so the blitter time was the limit, and `ZPL_MAXP` was 8).
   The frame blocks bound a frame too: a block is 4 KiB, so ~290 pieces per frame whatever their bands.
4. **A worst-case test.** `test_layers` saturates every band with far more sprites than the budget admits (up to 16
   four-tile sprites per band, and 15 sprites of 16 and of 32 tiles, sharing one cell array so nothing but the budget and the piece cap limit them), under fine scroll on both
   layers, and asserts the late and overflow bits stay clear. With `LAYERS_ALLPHASES=1` it does so for all 64 combinations of
   the two layers' fine-Y phase, the only thing the background's cost depends on.

Measured limit: there is none below what the builder can ask for. With all 64 fine-Y phase pairs (`LAYERS_ALLPHASES=1`) and saturated
sprites, up to 15 full-height 32-tile pieces per band (the exact worst case, 126,720 channel-cycles), with and without a window (7 shapes x 5 scrolls x
every sprite load, from a 1-wide full-height box to a 28 x 12 textbox, overlay and glass), no band goes late at the shipped `ZPL_BAND_BUDGET` of
**150000** or at 260000. On the old 340-cycle line the same test had an edge: clean through 1800 and first late at 2100 (2400 and up with a window),
so the shipped budget was 1300, about 40% under it. The producer is a similar story: it used to be ~90% busy at 8 one-cell pieces
under a window and now has 12.8x the time (see "History: producer cost" for what it took then).

Also verified: per-frame production keeps pace with the beam (the frame counter advances once per beam frame), and every
case is compared against an independent per-pixel reference over the whole frame (BG0, BG1, sprites), on several seeds.

## How it works

- A **producer loop** on the PPU walks 32 bands per frame. For each band and layer it computes the world row and column, then
  launches one queue row per half of the tile row (a fine Y scroll splits every tile row across two bands, clipped to the
  band, so z-order between layers stays exact), with the hardware wrapping the 64-cell map row. The band's sprite pieces behind BG0,
  BG0, the pieces between, BG1, then the pieces above both, because queue order is z-order. The hardware holds each row until its screen rows are safe to write.
- The producer runs ahead of the beam by however much the 128-cell queue holds, and blocks (`wait_space`) when it is full.
- The first frame after start is a transient (the producer starts a frame ahead); the tests judge frames after it.
- One band costs about 50 PPU cycles per launch, so the PPU is mostly idle: the blitter is the bottleneck, as designed. The frame number is written once per frame
  (`$035E` keeps its word), and `launch` is inlined at its call sites.
- A band with a window reads its record first (`ICNT`, `ICOL`), draws each BG layer's visible columns (`do_range`), then the sprites, then
  `do_window` (glass pass, ink pass).

## Demo cartridge

`demo/build_demo.py` builds `demo/build/demo.zpb` (dev-signed): sky bands, parallax clouds and a foreground strip on BG1, a
stone-and-dirt ground on BG0, and a ball, a 2x2 box and a 4x2 ship moving over them, looping every 256 frames; above it all a static window: a
30 x 5 textbox (solid, so the BG under it is not drawn) with its own 5x7 font, and a 3 x 3 frosted-glass lens over the ground. Run it with
`zeropoint_sdl demo/build/demo.zpb`, or capture frames headlessly with `dump_splash none demo/build/demo.zpb <prefix> <frames...>`
(`none` boots through the native HLE boot ROM). The CPU program is generated DEF88186 assembly: `COP #$F7` loads the PPU program,
DMA copies the palette, maps, sprite cells and the window (table, glass fill row, textbox cells), the tile mailbox uploads the 57 tiles, then each frame it DMAs the next
precomputed 256-byte frame block (scroll words + packed sprite stream, `demo/gen_frames.c`, built with the same
`zp_layers_sprites.h`) into the idle buffer and flips `LISTBASE`, then waits for the V-blank timer (`$D80050` bit 7) so the loop
steps once per 1/60 s of console time (256 steps, 4.3 s) instead of once per PPU frame (~766/s). The ROM carries 64 KiB of frame blocks in its PPU segment
(bank `$10` up), which `zplink` accepts as one blob.

The cart is signed v3 (`--codesize`, only the CPU program is hashed at boot) and the CPU asks the Boot ROM to verify each 16 KiB
chunk (`COP #$FF`) before reading it. It used to be v1: the real Boot ROM hashes a v1 payload in software and `sha256_hash` is
single-bank, so a 2 MB v1 demo ran the hash for minutes and then halted on a failed verify. Through the real Boot ROM the demo
now reaches the cartridge in about 12 s of console time (`dump_splash <bootrom.bin> demo.zpb ...`, `PB=$00` by frame 9000).

Traps hit while building it, so the next cart does not repeat them: `zp_dma_load` rounds every length up to 256 bytes and spills zero padding past the end,
so a small asset right below another one in PPU RAM zeroes its start (the window's three DMAs are ordered so the cells, which sit after the fill row, go last); `cpuasm` only recognises `,X` in upper case (`,x` silently
assembles a plain long address, so a copy loop reads and writes one byte); immediates are always one byte; there is no `CPX #`
or `CMP` with a long operand (compare through A, or zero a counter and test `BEQ`); a PPU image copied by `COP #$F7` goes
through the PPU window, so its padding must be zero bytes (a `NOP` word writes `$10` into the live ports below `$0400`).

## Limits and what is not done

- Sprite priority is per sprite and has three levels; the window is always above them. There is no per-tile BG priority (a BG tile over a
  sprite): BG rows are launched whole.
- No per-sprite blend. The queue snapshots the global blend settings at launch, so a blended sprite is a launch with different
  settings pushed first; not wired up.
- Sprite cell storage is 256 cells; sprites may share cell arrays (the saturation test does).
- A band whose sprites exceed the budget silently loses its overflow pieces (the builder reports how many).
- The C library for the console side is `include/zeropoint/zp_layers.h` (docs/sdk/README.md, example `examples/layers-game/`). Its frame builder is an
  unoptimised C port of the host builder in `zp_layers_sprites.h` and costs ~10-12k CPU cycles per sprite, so a game with many moving sprites still
  wants the host to precompute them (as the demo does) or hand-written assembly for `zpl_pass`/`zpl_layout`.
- `NCOLS` is fixed at 33 (full width at any fine X).
- Window: one ink span and one glass span per band (two boxes side by side in the same rows do not both fit; the later is dropped there), a
  408-cell pool and no flat 32 x 32 map, boxes are cell-aligned (no pixel offset), glass alpha is fixed at 15 (see "Glass"), and glass over a
  fine-scrolled background is a few cells wide because the blitter is nearly full with the two layers.
- A box's cells are read when drawn, so rewrite them (a typewriter text) in place only where the PPU is not about to draw, like a map.

## History: producer cost

(Written for the old 340-cycle line, core revision 12 and before. With revision 13's 4343-cycle line the producer has 12.8x the time per band and none of
these margins binds any more; the rule about label jumps below still holds, they cost ~10 cycles.)

Adding the window made the saturated tests fail at the shipped budget. Measured with a throwaway harness that samples the PPU's execution pointer:
the producer's wait-for-queue-space loop was idle only 9.9% of the time at 8 one-cell sprite pieces per band, and a window band (BG launches
doubled by culling, plus the window's two) pushed it past 100%. What it spent: every launch was a `JSR launch` that did a `JSR wait_space` (a
call and return are ~26 cycles each), a 16-bit immediate for `DP` (8) and nine word stores. Now `launch` is inline, `DP` is set from a register
that already holds the port address, the frame word is written once per frame, the guard against an impossible count is only in the sprite path,
and a sprite piece is read straight into the launch registers (one `MOV` + `ADD DP` per word) instead of copied word by word through a
temporary. Idle at 8 one-cell pieces rose from 9.9% to 38.5% without a window, and the worst windowed band (a full-height 1-wide box, which
saves almost nothing but still splits every BG row) is 10.7% idle at 8 pieces. Those are the numbers to watch if the producer grows again.

Sprite priority hit the same wall. **A label jump is ~10 cycles, taken or not**: `ppuasm` expands `JMZ label` (and `JMG`, `JNG`, `JNZ`, `JMR`) into
four `TARREG`/`SETBYTE` instructions that load the target into `R62` (8 cycles) and then the jump, so the one-line `CMP` + `JMG` that
looked like 2 cycles cost ~10. The first version (header decode and two more tests on every band) failed the windowed saturation test at
a budget of 1700, where the old microcode held to 2400-2500: the same instruction count as a few "free" compares, ~35 cycles per band. The
fix was to count jumps, not instructions: the plain band has one test and no jump back (the extended band is out of line, it falls straight
through to the window), the header read is unconditional (`MUL` by a 0/1 `SPREN` instead of a compare and a jump), and `do_layer`'s "world
row mod 512" no longer divides (scroll Y is reduced once per frame, then one conditional subtract: ~9 cycles saved per layer per band). The
edge is the same as before the change (fine at 2400-2500, as measured by `LAYERS_BUDGET=...`). A delay-injection experiment (extra
`INC`s at the top of every band) is a good probe: the old microcode tolerated ~24 cycles per band there.

## History: why the hardware changed

The first version (`gen_layers_v1.py`) issued every tile with `SETTILE`/`SETPOS`/`TILEDRAW`. Measured with `test_layers` v1:
33 cycles per cell, which is both its issue cost and the blitter's capacity, with 829 (tile-aligned) to 1232 (fine scroll)
cycles of fixed overhead per band, and 5546 cycles per band for two fine-scrolled layers against an 8-scanline (2720 cycle)
band. Two full layers did not fit even tile-aligned. A late row left stale pixels from earlier frames, which is the flicker.
The write-ahead tile-row queue, the signed/absolute rows, the overlap-only retire order and `Display::rowIsClear` came out of
that. Display behaviour the work established (from `src/display.cpp`): at scanline S the window is `[S-1, S+15)` and row y is
latched at the end of scanline y+1; in RGBA16 slots clear in pairs, so a row is only safe once the slot's previous
occupant is cleared (`rowIsClear`), which for an even row is a scanline later than "inside the window".

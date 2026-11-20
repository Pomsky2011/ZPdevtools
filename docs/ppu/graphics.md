# Graphics: tiles, palettes, blitters and the framebuffer

How pixels get onto the screen. The instructions that drive this are in [instruction-set.md](instruction-set.md); the
registers are in the memory map in [architecture.md](architecture.md).

## The framebuffer

The display is 256 pixels wide and 256 lines tall. There is no full-screen framebuffer. The PPU writes into a **rolling
buffer**: 8 KiB at `$E000-$FFFF` made of eight 1 KiB banks, which holds a short window of scanlines that slides down the
screen as the beam moves and is output as it goes. In 16-bit mode that is 16 scanlines (two per bank); in 32-bit mode, 8.
A row can only be written while it is inside the window; a write to a row that has already left it is lost. This is why
the layers microcode and the tile-row queue exist: they hold work until its rows are writable ([layers.md](layers.md)).
The emulator's `docs/display.md` describes the window mechanics in full.

The colour format is chosen by the **render mode**:

| Mode | Pixel | Palette entries |
|------|-------|-----------------|
| 16-bit (default) | RGBA5551, 2 bytes | `BBBBBGGGGGRRRRRA`: blue in bits 15-11, green 10-6, red 5-1, alpha bit 0 |
| 32-bit | RGBA8888, 4 bytes | four bytes in memory: `RR GG BB AA` |

Select it with `SETRENDMOD 0`/`1` or bit 7 of `$00F0`. An alpha of 0 means transparent where something honours it.

## VOC registers, `$00F0-$00FF`

| Address | Name | |
|---------|------|---|
| `$00F0` | render mode | bit 7 colour depth (0 = 16-bit, 1 = 32-bit); bit 6 rolling mode (0 = block, 1 = single); bit 5 H-blank interrupt enable; bit 4 V-blank interrupt enable; bit 3 palette mode for the pixel port (0 = 16 colours, 1 = 256); bit 0 reset (self-clearing) |
| `$00F1-F2` | palette address | stored and read back |
| `$00F3-FA` | bank order | eight bytes; writable only when auto-roll is off |
| `$00FB` | auto-roll | 0 turns auto-roll off, so the bank order can be written |
| `$00FC` | blend and attributes | bits 1-0 blend mode; bits 5-2 per-blit attributes (below) |
| `$00FD` | push / halt | bit 2 halt the PPU; bit 3 **push** the blend mode and translucency; bit 4 push the attributes |
| `$00FE-FF` | translucency | four 4-bit opacity values |

Blend mode, translucency and halt are writable in 16-bit render mode only; the attribute bits work in either.

## Tiles

A tile is 8 x 8 pixels in 64 bytes of tile storage. There are four banks of 256 tiles, so 1,024 tiles; a tile ID is 8 bits and
the bank is chosen separately with `SETTILEBANK`.

`SETTILE RX, mode` picks the tile ID and a mode 0-3. Only bit 1 of the mode matters to the blitter:

| Mode | Tile data | Palette |
|------|-----------|---------|
| 0 or 1 | **4 bits per pixel**: 32 bytes, 4 per row, the left pixel in the high nibble | the 16-entry palette |
| 2 or 3 | **8 bits per pixel**: 64 bytes, one byte per pixel, row by row | the 256-entry palette |

The colour format of what lands in the framebuffer follows the render mode, not the tile mode.

**To define a tile:** select its bank and ID (`SETTILEBANK`, `SETTILE`), then write its bytes to `$0300-$033F`. That window
always addresses the currently selected tile, which is the only way to write tile pixels.

## Drawing a tile: TILEDRAW

Set the position with `SETPOS` (or by writing `$0200`/`$0202`), select the tile, and issue `TILEDRAW`. The instruction takes a
snapshot of the position, tile, mode, blend settings and attributes into a free **blit channel** and returns after 8 cycles; the
channel does the drawing later, in the background.

- There are **8 channels**. If all are busy, `TILEDRAW` waits for one.
- Blits are drawn **in issue order**, whatever they cost, so program order is z-order.
- X is signed: a tile can hang off the left edge and is clipped there. Pixels outside the framebuffer window are dropped.
- A full opaque tile takes 8 + 8 x 8 x 4 = **264 cycles** to draw; a blended one 8 + 8 x 8 x 6 = **392**. A clipped tile pays only for
  the rows it draws.

### Attributes

Written through `$00FC` and made live by setting bit 4 of `$00FD`:

| `$00FC` bit | Attribute |
|-------------|-----------|
| 2 | **transparent**: palette entries with alpha 0 leave the framebuffer alone |
| 3 | **hflip**: mirror left-right |
| 4 | **vflip**: mirror top-bottom |
| 5 | **row clip**: draw only the rows in `[$0204, $0206)` (screen rows) |

### Blend and translucency

Blend mode (`$00FC` bits 1-0) 0 draws normally. Modes 1, 2 and 3 combine the tile with what is already there: **average**,
**subtract** and **add**. A blended pixel reads the framebuffer back and costs 6 cycles instead of 4. Both the blend mode and
the four translucency values are latched when `$00FD` bit 3 is written.

The translucency values are the four nibbles of `$00FE-$00FF`. A tile uses the one selected by its **tile ID modulo 4**:
15 leaves it opaque, 0 makes its pixels transparent, anything between scales the colour's alpha.

## Palettes

`PALETTE16` and `PALETTE256` load from the address in `DP`; `CLRPALETTE` resets both to grayscale (the default after reset).
The 16-entry palette serves 4bpp tiles, line blits, and the pixel port in its default mode. The 256-entry palette serves 8bpp
tiles and the pixel port in palette mode 1.

## The pixel port

To draw a single pixel: write X to `$0100`, Y to `$0102` and a palette index to `$0104`, then write anything to `$010A`; that
last write draws. The index selects from the 16-entry palette (low 4 bits) or the 256-entry one, per `$00F0` bit 3. A full
program that draws a white pixel is in [programming-guide.md](programming-guide.md).

## Line blitter, `$0340-$034A`

For horizontal runs: draws `COUNT` 4bpp pixels (high nibble first) along one row, through the 16-entry palette; an entry with
alpha 0 is transparent. X is signed, so a run can start off the left edge.

| Address | Register |
|---------|----------|
| `$0340` | source address of the packed pixels |
| `$0342` | X (signed) |
| `$0344` | Y |
| `$0346` | pixel count |
| `$0348` | write to launch |
| `$034A` | status: bits 0-3 channel busy; bit 7 **overrun** (sticky; write to clear) |

There are four line channels, separate from the tile ones. A launch that finds all four busy is dropped and sets the overrun
bit; it cannot stall the program, so pace your launches. A run costs 8 + `COUNT` cycles.

## Tile-row queue, `$0350-$0366`

Where `TILEDRAW` issues one tile, the queue takes a whole row of cells in one launch and **holds each tile until its rows are
writable**, so the program never has to time its drawing against the beam. It is the hardware under the layers microcode.

| Address | Register | |
|---------|----------|---|
| `$0350` | `SRC` | address of the cell array: 4 bytes per cell (word 0 = tile ID in the low byte; word 1 = bank in bits 0-1, then attributes: bit 2 transparent, bit 3 hflip, bit 4 vflip) |
| `$0352` | `COUNT` | cells in the row |
| `$0354` | `X` | screen X of the first cell (signed); each cell is 8 pixels on |
| `$0356` | `Y` | row Y (signed) |
| `$0358` `$035A` | `CLIP0` `CLIP1` | clip rows, used with the clip flag |
| `$035C` | `FLAGS` | bit 0 clip, bit 1 flip X (also reverses the cell order), bit 2 flip Y, bit 3 wrap (the source wraps inside its 256-byte page), bits 5-4 tile mode, bit 6 transparent |
| `$035E` | `FRAME` | the 16-bit frame number the row belongs to |
| `$0360` | `GO` | write launches; read gives the cells still pending |
| `$0362` | `MODE` | bit 0 write-ahead mode: `$C000-$FFFF` becomes open bus |
| `$0364` | `STATUS` | bit 0 **late** (a row missed its frame), bit 1 **overflow**; sticky, a write clears |
| `$0366` | `HWFRAME` | read: the frame the hardware is on |

The queue holds 128 cells in total. A row that would overflow it is dropped whole and sets the overflow bit. A cell is
dispatched at most every 8 cycles; queue order is z-order, as with `TILEDRAW`. A row names its frame (`FRAME`) in absolute terms,
so a launch that blocks for a long time still lands in the frame it was meant for; a row whose frame the beam has passed is
late, and the late bit is set instead of tearing the picture. The emulator's `docs/ppu.md` has the full programming model, and
[layers.md](layers.md) shows a complete user of it.

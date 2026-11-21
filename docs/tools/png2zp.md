# png2zp: art as PNGs

`scripts/png2zp.py` turns indexed PNGs (and Tiled maps) into what the layers C library loads: one palette, tile banks, 64x64 maps,
sprite cells and window boxes. `build/assets.bin` holds the data (every piece 256-aligned, placed in ROM with `zplink --data
build/assets.bin@0x110000`), `build/assets.h` says where each piece sits and how big it is. The game template
([../../game-template](../../game-template)) runs it from its Makefile; changing a PNG changes the picture with no code change.

```sh
python3 scripts/png2zp.py assets/assets.json -o build [--rom-base 0x110000] [--quantize[=N]]
```

Needs python3 and Pillow. Tests: `python3 scripts/test_png2zp.py` (conversion, palette rules, flips, Tiled, errors) and
`tests/layers/run.sh` (the template built with its sample art and with other art, frames compared pixel for pixel on the emulator,
native boot and real Boot ROM).

## The rules it works to

- **One palette, 256 entries (RGB555), shared by every image.** Entries 0, 16, 32.. have alpha 0 on this PPU, so an opaque colour never
  gets one of those indices and a transparent pixel is always index 0. That leaves **240 opaque colours** for the whole game; the
  converter says so if you go over. An indexed PNG keeps the order of its own palette (used colours only), so you steer the layout.
  Two colours that differ only in the low 3 bits of a channel are the same colour on the console and share an entry.
- **Four tile banks of 256 tiles, with fixed jobs:** bank 0 is BG0 (opaque unless the image has transparent pixels), 1 the sprites,
  2 BG1 (see-through), 3 the window (font and frame). Identical tiles are stored once. `"flips": true` on a layer also folds tiles that are
  mirror images (the map cell carries the flip bits).
- **Maps are 64x64 cells (512x512 pixels) and wrap.** An image smaller than that is repeated over the map (`"fill": "repeat"`, the
  default) or leaves the rest empty (`"fill": "clear"`: tile of index 0, i.e. see-through / palette entry 0).
- **Everything is tiled in 8x8:** image sizes must be multiples of 8.

## The manifest (`assets.json`)

Paths are relative to the manifest.

```json
{
  "rom_base": "0x110000",
  "quantize": 48,
  "bg0": {"image": "ground.png"},
  "bg1": {"tiled": "level.tmj", "layer": "foreground", "flips": true},
  "sprites": [{"name": "hero", "image": "hero.png", "frame_w": 2, "frame_h": 2}],
  "font": {"image": "font.png", "first": 32},
  "frame": "frame.png",
  "boxes": [{"name": "msg", "w": 28, "h": 5, "border": true, "pad": 2, "lines": ["HELLO", "", "WORLD"]}]
}
```

| Key | Meaning |
|---|---|
| `rom_base` | where `assets.bin` sits (`--rom-base` wins; default `0x110000`) |
| `quantize` | accept RGB(A) PNGs and reduce each to at most N colours (also `--quantize[=N]`, default 48, max 240). Without it only indexed PNGs are taken |
| `bg0`, `bg1` | a layer: `"image"` (a PNG, each 8x8 block becomes a cell) or `"tiled"` (a Tiled map, below). Also `fill`, `flips`, `transparent` |
| `sprites` | in order, they share the cell table (256 cells). `frame_w`/`frame_h` are in tiles; the default is the whole image as one frame. Frames run left to right, top to bottom |
| `font` | a sheet of 8x8 glyphs, 16 to a row, glyph *i* is ASCII `first + i` (default 32); it must be opaque, so paint the background in |
| `frame` | one 8x8 opaque tile, the border of a box |
| `boxes` | text baked into window cells. `lines` start on the first row inside the border; `pad` is the column text starts at (default 1 with a border) |

Per image, `"transparent"` is `"alpha"` (PNG transparency: a `tRNS` entry or an alpha channel; the default when the PNG has any),
`"index0"` (palette index 0 of an indexed PNG), `"#rrggbb"` (a key colour) or `"none"`.

## What lands in `assets.h`

`ASSET_PAL`, `ASSET_TILES0..3` (+ `_N` tile counts), `ASSET_MAP0`/`MAP1`, `ASSET_CELLS` (+ `ASSET_CELLS_N`), and per piece
`ASSET_BOX_<NAME>` (+ `_N`, `_W`, `_H`), `ASSET_SPR_<NAME>_CELL` (first cell), `_W`, `_H`, `_FRAMES`. Maps and banks only appear if something was
built for them. `ASSET_BG0_HAS_ALPHA` / `ASSET_BG1_HAS_ALPHA` say a layer has transparent pixels: BG1 is see-through by default, BG0 is
not, so a transparent BG0 needs `zpl_layer_transparent(1, 1)`. Frame *f* of a sprite starts at cell `ASSET_SPR_X_CELL + f * W * H`.

## Tiled

Export the map as JSON (`.tmj`). Supported: orthogonal, finite, tile size 8x8, one tileset image per tileset (embedded, or an external
`.tsj`; margin and spacing honoured), CSV data or base64 (plain, zlib or gzip), the horizontal and vertical flip flags. Refused with a message:
infinite maps, other orientations, rotated (diagonal-flip) tiles, image-collection tilesets, zstd. The map must fit 64x64 cells; a
layer is picked with `"layer": "name"` (default: the first tile layer). Tile 0 (empty) becomes the clear tile.

## Text

Boxes are baked at convert time. To change text while the game runs, write the glyph tile ids yourself: the font's tiles are in bank 3
in the order of the sheet (a duplicate glyph is stored once, so look ids up rather than assuming `ch - first`).

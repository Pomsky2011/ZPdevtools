# rombuilder (legacy)

`rombuilder` places a CPU binary, a PPU binary and an APU binary at fixed addresses in one flat image and writes a small text
file describing the layout:

```sh
rombuilder -cpu main.bin -ppu gfx.bin -apu snd.bin -o game.rom
```

| Option | Default |
|--------|---------|
| `-cpu-base ADDR` | `0x008000` |
| `-ppu-base ADDR` | `0x100000` |
| `-apu-base ADDR` | `0x200000` |
| `-entry ADDR` | the CPU base |
| `-v` | verbose |

It writes `game.rom` and `game.txt`. With the defaults the image is 2,097,160 bytes (the APU blob sits at offset 2 MB), and
`game.txt` lists each component's file, base, size and entry point plus a memory-layout table.

## It is not what loads in the emulator

The image has **no ZPB header**. The emulator requires one: loading a `rombuilder` image fails with
`Invalid ROM file: bad magic number`, whereas a ROM from `zplink` and `zpbuild` loads. `rombuilder` still prints "ROM is ready to
use with the ZeroPoint emulator", which is wrong.

Use the pipeline in [README.md](README.md) instead: `zplink` does the same job of placing the three segments at their
addresses (with the same default bases) and `zpbuild` adds the header and the signature ([../sdk/linker.md](../sdk/linker.md)).
`rombuilder` is useful only if you want the flat image for some other purpose, such as a hex dump of the layout.

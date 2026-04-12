# Complete ROM Build Example

This example demonstrates the complete workflow from source code to a final ROM file.

## Project Structure

```
complete_rom_example/
├── README.md           - This file
├── cpu/
│   └── main.asm        - CPU main program
├── ppu/
│   └── graphics.asm    - PPU graphics code
├── apu/
│   └── audio.asm       - APU audio code
├── build.sh            - Automated build script
└── build/              - Output directory (created during build)
```

## Build Instructions

### Manual Build

```bash
# 1. Assemble CPU code
../../cpuasm cpu/main.asm build/cpu.bin

# 2. Assemble PPU code
../../ppuasm ppu/graphics.asm -o build/ppu.bin

# 3. Assemble APU code
../../apuasm apu/audio.asm build/apu.bin

# 4. Build ROM
../../rombuilder \
  -cpu build/cpu.bin \
  -ppu build/ppu.bin \
  -apu build/apu.bin \
  -entry 0x8000 \
  -o build/game.rom \
  -v
```

### Automated Build

```bash
chmod +x build.sh
./build.sh
```

## Output Files

After building, you'll have:

- `build/cpu.bin` - Assembled CPU code
- `build/ppu.bin` - Assembled PPU code
- `build/apu.bin` - Assembled APU code
- `build/game.rom` - Final ROM file
- `build/game.txt` - ROM metadata

## ROM Metadata

The `game.txt` file contains:
- CPU entry point (0x8000)
- Base addresses for each component
- Size of each component
- Memory layout map

## Loading in Emulator

```bash
# Load ROM in ZeroPoint emulator
../../ZeroPoint/build_qt/bin/zeropoint_sdl build/game.rom
```

The emulator reads `game.txt` to determine:
- Where to load CPU, PPU, and APU code
- Where CPU execution should begin

## Customization

Edit `build.sh` to:
- Change base addresses
- Modify entry point
- Add optimization flags
- Include additional assets

#!/bin/bash
# build.sh - Automated ROM build script for ZeroPoint

set -e  # Exit on error

echo "========================================"
echo "ZeroPoint ROM Build Script"
echo "========================================"
echo ""

# Create build directory
mkdir -p build

# Step 1: Assemble CPU code
echo "[1/4] Assembling CPU code..."
../../executables/cpuasm cpu/main.asm build/cpu.bin
if [ $? -ne 0 ]; then
    echo "Error: CPU assembly failed"
    exit 1
fi
echo "  ✓ CPU assembled"

# Step 2: Assemble PPU code
echo "[2/4] Assembling PPU code..."
../../executables/ppuasm ppu/graphics.asm -o build/ppu.bin
if [ $? -ne 0 ]; then
    echo "Error: PPU assembly failed"
    exit 1
fi
echo "  ✓ PPU assembled"

# Step 3: Assemble APU code
echo "[3/4] Assembling APU code..."
../../executables/apuasm apu/audio.asm build/apu.bin
if [ $? -ne 0 ]; then
    echo "Error: APU assembly failed"
    exit 1
fi
echo "  ✓ APU assembled"

# Step 4: Build ROM
echo "[4/4] Building ROM..."
../../executables/rombuilder \
    -cpu build/cpu.bin \
    -ppu build/ppu.bin \
    -apu build/apu.bin \
    -entry 0x8000 \
    -o build/game.rom \
    -v

if [ $? -ne 0 ]; then
    echo "Error: ROM build failed"
    exit 1
fi

echo ""
echo "========================================"
echo "Build Complete!"
echo "========================================"
echo ""
echo "Output files:"
echo "  - build/game.rom (ROM file)"
echo "  - build/game.txt (metadata)"
echo ""
echo "File sizes:"
ls -lh build/*.bin build/game.rom | awk '{print "  " $9 ": " $5}'
echo ""
echo "To view metadata:"
echo "  cat build/game.txt"
echo ""
echo "To run in emulator:"
echo "  zeropoint_sdl build/game.rom"
echo ""

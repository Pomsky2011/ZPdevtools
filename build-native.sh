#!/bin/bash
# ZPdevtools Build Script for Native Platforms
# Builds all development tools for current platform

set -e

echo "========================================"
echo "  ZPdevtools Native Build Script"
echo "========================================"
echo ""

# Detect OS and architecture
OS="$(uname -s)"
ARCH="$(uname -m)"

case "$OS" in
    Darwin*)
        PLATFORM="macOS"
        NCPU=$(sysctl -n hw.ncpu)
        ;;
    Linux*)
        PLATFORM="Linux"
        NCPU=$(nproc)
        ;;
    CYGWIN*|MINGW*|MSYS*)
        PLATFORM="Windows (MSYS/MinGW)"
        NCPU=$(nproc)
        ;;
    *)
        PLATFORM="Unknown"
        NCPU=4
        ;;
esac

echo "Platform:     $PLATFORM"
echo "Architecture: $ARCH"
echo "CPU cores:    $NCPU"
echo ""

# Check for GCC/Clang
if ! command -v gcc &> /dev/null && ! command -v clang &> /dev/null; then
    echo "Error: No C compiler found (gcc or clang required)"
    echo ""
    echo "Install:"
    if [ "$PLATFORM" = "macOS" ]; then
        echo "  xcode-select --install"
    elif [ "$PLATFORM" = "Linux" ]; then
        echo "  Debian/Ubuntu: sudo apt install build-essential"
        echo "  Fedora:        sudo dnf install gcc make"
        echo "  Arch:          sudo pacman -S base-devel"
    fi
    exit 1
fi

# Determine compiler
if command -v gcc &> /dev/null; then
    COMPILER="gcc"
    echo "Compiler: GCC $(gcc --version | head -n1)"
elif command -v clang &> /dev/null; then
    COMPILER="clang"
    echo "Compiler: Clang $(clang --version | head -n1)"
fi

echo ""
echo "Building all tools..."
echo ""

# Clean previous build
make clean 2>/dev/null || true

# Build all tools
make -j$NCPU

echo ""
echo "========================================"
echo "  Build Complete!"
echo "========================================"
echo ""
echo "Built tools:"
echo ""
echo "Assemblers:"
ls -1 ppuasm apuasm cpuasm 2>/dev/null | sed 's/^/  ✓ /' || echo "  (assemblers not built)"
echo ""
echo "Disassemblers:"
ls -1 ppudisasm apudisasm cpudisasm 2>/dev/null | sed 's/^/  ✓ /' || echo "  (disassemblers not built)"
echo ""
echo "ROM Tools:"
ls -1 rombuilder rominspect 2>/dev/null | sed 's/^/  ✓ /' || echo "  (ROM tools not built)"
echo ""
echo "Utilities:"
ls -1 wav2mmp hexview 2>/dev/null | sed 's/^/  ✓ /' || echo "  (utilities not built)"
echo ""
echo "Usage examples:"
echo "  ./ppuasm examples/ppu/simple_pixel_test.asm output.bin"
echo "  ./apuasm examples/apu/hello.asm"
echo "  ./cpuasm examples/cpu/test.asm output.bin"
echo "  ./rombuilder -cpu cpu.bin -ppu ppu.bin -apu apu.bin -o game.rom"
echo ""
echo "For DOS builds, see: build-dos.bat or Makefile.dos"
echo ""

# ZeroPoint Development Tools Guide

Complete reference for all ZeroPoint development utilities.

---

## Table of Contents

1. [Assemblers](#assemblers)
   - [CPU Assembler (cpuasm)](#cpu-assembler-cpuasm)
   - [PPU Assembler (ppuasm)](#ppu-assembler-ppuasm)
   - [APU Assembler (apuasm)](#apu-assembler-apuasm)

2. [Disassemblers](#disassemblers)
   - [CPU Disassembler (cpudisasm)](#cpu-disassembler-cpudisasm)
   - [PPU Disassembler (ppudisasm)](#ppu-disassembler-ppudisasm)
   - [APU Disassembler (apudisasm)](#apu-disassembler-apudisasm)

3. [ROM Tools](#rom-tools)
   - [ROM Builder (rombuilder)](#rom-builder-rombuilder)
   - [ROM Inspector (rominspect)](#rom-inspector-rominspect)

4. [Utilities](#utilities)
   - [Hex Viewer (hexview)](#hex-viewer-hexview)
   - [WAV to MMP Converter (wav2mmp)](#wav-to-mmp-converter-wav2mmp)

5. [C Compiler](#c-compiler)
   - [DEF88186 C Compiler (def88186cc)](#def88186-c-compiler-def88186cc)

---

## Assemblers

### CPU Assembler (cpuasm)

Assembles DEF88186 assembly language into binary code.

**Usage:**
```bash
cpuasm <input.asm> [output.bin]
```

**Features:**
- Full DEF88186 instruction set (256 opcodes)
- 65816 base + 8086 extensions
- `.include` directive for multi-file projects
- `.data` and `.code` section support
- Label support with forward references
- Comments with `;`

**Example:**
```asm
; hello.asm
.code
start:
    LDA #$42
    STA $8000
    JMP start
```

```bash
cpuasm hello.asm hello.bin
```

**See Also:**
- `docs/cpu/` - Complete CPU documentation
- `ASM_MULTIFILE_GUIDE.md` - Multi-file assembly guide
- `examples/cpu/` - Example programs

---

### PPU Assembler (ppuasm)

Assembles PPU microcode into binary format.

**Usage:**
```bash
ppuasm <input.asm> <output.bin>
```

**Features:**
- 15 basic instructions + Preset E/F extensions
- Shorthand macros (JMR, PUSH, POP, JSR, RET, HLT)
- Register aliases (`R10 = COUNTER`)
- Memory-mapped I/O constant definitions
- Target register operations

**Example:**
```asm
; draw_pixel.asm
start:
    SETPOS 100, 100
    SETTILE 0x42
    TILEDRAW
    HLT
```

```bash
ppuasm draw_pixel.asm draw_pixel.bin
```

**See Also:**
- `docs/ppu/` - PPU documentation
- `docs/ppu/preset-e-and-shorthands.txt` - Shorthand guide
- `examples/ppu/` - Example microcode

---

### APU Assembler (apuasm)

Assembles APU/DSP programs into binary format.

**Usage:**
```bash
apuasm <input.asm> <output.bin>
```

**Features:**
- 47 APU instructions
- Stack operations (PUX, POX, PUY, POY)
- Function calls (CFN, CCF, RET)
- MMP and SST support
- Bank switching (SETDB, SETBF)

**Example:**
```asm
; play_tone.asm
start:
    LDI R0, #$00
    LDI R1, #$FF
    MMP 0, R0    ; Channel 0
    SST 0, R1    ; Sample data
    HLT
```

```bash
apuasm play_tone.asm play_tone.bin
```

**See Also:**
- `docs/apu/` - Complete APU documentation
- `docs/apu/programming-guide.txt` - Programming examples
- `examples/apu/` - Example programs

---

## Disassemblers

### CPU Disassembler (cpudisasm)

Disassembles DEF88186 binary files into assembly language.

**Usage:**
```bash
cpudisasm <input.bin> [options]
```

**Options:**
- `-o <file>` - Output to file instead of stdout
- `-s <addr>` - Start address (hex)
- `-e <addr>` - End address (hex)
- `-c` - Show comments with instruction descriptions
- `-b` - Show bytes alongside assembly

**Example:**
```bash
# Disassemble entire file
cpudisasm program.bin

# Disassemble specific range with bytes
cpudisasm rom.bin -s 0x8000 -e 0x8100 -b -c

# Save to file
cpudisasm binary.bin -o output.asm
```

**Output Format:**
```
; Disassembly of program.bin
; Size: 1024 bytes (0x400)
; Range: $000000 - $000400

000000:  NOP
000001:  LDA $8000
000004:  STA $9000
000007:  JMP $0001
```

---

### PPU Disassembler (ppudisasm)

Disassembles PPU microcode binaries into assembly language.

**Usage:**
```bash
ppudisasm <input.bin> [options]
```

**Options:**
- `-o <file>` - Output to file
- `-s <addr>` - Start address (hex)
- `-e <addr>` - End address (hex)
- `-c` - Show comments
- `-b` - Show bytes

**Example:**
```bash
# Disassemble PPU microcode
ppudisasm microcode.bin

# With bytes and comments
ppudisasm ppu_program.bin -b -c
```

**Output Format:**
```
; PPU Disassembly of microcode.bin
; Size: 512 bytes (0x200)

0000:  30 0A   SWAPREG      R1, R2
0002:  80 00   INC          R0, R0
0004:  E4 00   TARREG       0, LSB, R0
```

---

### APU Disassembler (apudisasm)

Disassembles APU binaries into assembly language.

**Usage:**
```bash
apudisasm <input.bin> [options]
```

**Options:**
- `-o <file>` - Output to file
- `-s <addr>` - Start address (hex)
- `-e <addr>` - End address (hex)
- `-c` - Show comments
- `-b` - Show bytes

**Example:**
```bash
# Disassemble APU program
apudisasm audio.bin

# Specific range with formatting
apudisasm apu_code.bin -s 0x8000 -e 0x9000 -b
```

**Output Format:**
```
; APU Disassembly of audio.bin
; Size: 256 bytes (0x100)

0000:  00 00   MOV        R0, R0
0002:  08 20   ADD        R1, R0
0004:  5C 00   JMP        $0000
```

---

## ROM Tools

### ROM Builder (rombuilder)

Combines CPU, PPU, and APU binaries into a single ZPB ROM file.

**Usage:**
```bash
rombuilder -cpu <file> -ppu <file> -apu <file> -o <output.zpb> [options]
```

**Options:**
- `-cpu <file>` - CPU binary file
- `-ppu <file>` - PPU binary file
- `-apu <file>` - APU binary file
- `-o <file>` - Output ROM file (.zpb)
- `-entry <addr>` - CPU entry point (hex, default: 0x8000)
- `-title "Title"` - ROM title (max 64 chars)
- `-dev "Developer"` - Developer name (max 64 chars)
- `-v` - Verbose output

**Example:**
```bash
# Build complete ROM
rombuilder -cpu main.bin -ppu graphics.bin -apu audio.bin \
           -o game.zpb -title "My Game" -dev "Studio X" -entry 0x8000

# Minimal ROM
rombuilder -cpu program.bin -ppu empty.bin -apu empty.bin -o test.zpb
```

**Output:**
- `game.zpb` - Complete ROM file
- `game.txt` - Metadata file with memory layout

**See Also:**
- `ROMBUILDER_GUIDE.md` - Complete ROM builder documentation

---

### ROM Inspector (rominspect)

Analyzes and displays information about ZPB ROM files.

**Usage:**
```bash
rominspect <rom.zpb> [options]
```

**Options:**
- `-v` - Verbose mode (show checksums, memory layout)
- `-x` - Extract components to separate files
- `-c` - Verify checksums

**Example:**
```bash
# Inspect ROM
rominspect game.zpb

# Verbose inspection with checksum verification
rominspect game.zpb -v -c

# Extract components
rominspect game.zpb -x
# Creates: game_cpu.bin, game_ppu.bin, game_apu.bin
```

**Output:**
```
=== ZeroPoint ROM Information ===

File: game.zpb
Size: 131328 bytes (128.25 KB)

--- Header ---
Magic:        ZPB (valid)
Version:      1
Flags:        0x00
Title:        My Game
Developer:    Studio X
Entry Point:  $008000

--- Component Sizes ---
CPU Binary:   32768 bytes (32.00 KB)
PPU Binary:   65536 bytes (64.00 KB)
APU Binary:   32768 bytes (32.00 KB)
Total:        131072 bytes (128.00 KB)
```

---

## Utilities

### Hex Viewer (hexview)

Smart hex dump utility with ASCII display and analysis.

**Usage:**
```bash
hexview <file> [options]
```

**Options:**
- `-s <offset>` - Start at offset (hex or decimal)
- `-n <count>` - Display N bytes
- `-w <width>` - Bytes per line (1-64, default: 16)
- `-g <group>` - Group bytes (1, 2, or 4, default: 1)
- `-a` - Hex addresses only (no decimal)
- `-o <file>` - Write to file instead of stdout
- `--analyze` - Show file statistics

**Examples:**
```bash
# Basic hex dump
hexview program.bin

# View specific range
hexview rom.zpb -s 0x100 -n 256

# Custom formatting
hexview data.bin -w 8 -g 2

# Analyze file
hexview binary.bin --analyze

# Save to file
hexview input.bin -o dump.txt
```

**Output:**
```
File: program.bin
Size: 1024 bytes (0x400)

00000000 (       0): 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  |................|
00000010 (      16): 10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F  |................|
00000020 (      32): 20 21 22 23 24 25 26 27 28 29 2A 2B 2C 2D 2E 2F  | !"#$%&'()*+,-./|
```

**Analysis Output:**
```
=== File Statistics ===
Total bytes: 1024 (0x400)

Most common bytes:
  0x00 (  0): 512 times (50.00%)
  0xFF (255): 256 times (25.00%)
  0x42 ( 66): 128 times (12.50%)
```

---

### WAV to MMP Converter (wav2mmp)

Converts WAV audio files to MMP format for the APU.

**Usage:**
```bash
wav2mmp <input.wav> <output.inc> [options]
```

**Features:**
- Converts 8-bit/16-bit WAV files
- Resamples to APU sample rate (48 kHz)
- Generates SST (Sample Storage) headers
- Outputs assembly-ready `.inc` files

**Example:**
```bash
# Convert WAV to MMP
wav2mmp sound.wav sound.inc

# Use in APU assembly
.include "sound.inc"
```

**See Also:**
- `docs/apu/mmp.txt` - MMP documentation
- `docs/apu/sst.txt` - SST documentation

---

## C Compiler

### DEF88186 C Compiler (def88186cc)

Full C89-compliant compiler for DEF88186 CPU.

**Usage:**
```bash
def88186cc <input.c> [-o output.asm] [options]
```

**Options:**
- `-o <file>` - Output assembly file
- `-I <path>` - Add include directory
- `-D MACRO[=value]` - Define preprocessor macro
- `-E` - Preprocessor only (no compilation)
- `-bin` - Full pipeline: compile → assemble → link

**Features:**
- Full C89/ANSI C support
- Preprocessor (`#include`, `#define`, `#ifdef`, etc.)
- Inline assembly (`__asm__("instruction")`)
- Structs, unions, enums, typedef
- All operators (arithmetic, logical, bitwise, ternary, sizeof)
- Control flow (if/else, while, for, do-while, switch/case, break, continue)
- Functions with recursion
- Hardware optimizations (MUL/DIV, LOOP/LPEND)

**Example:**
```c
// program.c
#include <stdint.h>

int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

int main() {
    int result = factorial(5);
    __asm__("STA $8000");  // Store result
    return 0;
}
```

```bash
# Compile to assembly
def88186cc program.c -o program.asm

# Full build
def88186cc program.c -bin
# Creates: program.asm, program.bin
```

**See Also:**
- `c_compiler/README.md` - Complete compiler documentation
- `c_compiler/examples/` - 16+ example programs
- `ASM_MULTIFILE_GUIDE.md` - Multi-file projects

---

## Building the Tools

### Build All Tools
```bash
make
```

### Build Specific Categories
```bash
make assemblers      # Build ppuasm, apuasm, cpuasm
make disassemblers   # Build cpudisasm, ppudisasm, apudisasm
make rom-tools       # Build rombuilder, rominspect
make utilities       # Build hexview, wav2mmp
```

### Install System-Wide
```bash
sudo make install
```

### Clean Build Files
```bash
make clean
```

---

## Workflow Examples

### Complete Game Development Workflow

1. **Write C code for CPU:**
```bash
def88186cc src/main.c -I include -o build/main.asm
cpuasm build/main.asm build/cpu.bin
```

2. **Write PPU microcode:**
```bash
ppuasm src/graphics.asm build/ppu.bin
```

3. **Write APU audio:**
```bash
wav2mmp assets/music.wav build/music.inc
apuasm src/audio.asm build/apu.bin
```

4. **Build ROM:**
```bash
rombuilder -cpu build/cpu.bin -ppu build/ppu.bin -apu build/apu.bin \
           -o game.zpb -title "My Game" -dev "Your Name"
```

5. **Inspect and verify:**
```bash
rominspect game.zpb -v -c
```

### Debugging Workflow

1. **Disassemble binary to see what was generated:**
```bash
cpudisasm build/cpu.bin -b -c > debug/cpu.asm
ppudisasm build/ppu.bin -b > debug/ppu.asm
```

2. **Hex dump to inspect raw data:**
```bash
hexview build/cpu.bin --analyze
hexview game.zpb -s 0x100 -n 256
```

3. **Extract ROM components for analysis:**
```bash
rominspect game.zpb -x
# Creates individual component binaries
```

---

## Tips and Tricks

### Disassembly Tips

- Use `-b` flag to see actual byte values for debugging
- Use `-c` flag to get instruction descriptions
- Use `-s` and `-e` to focus on specific code sections
- Compare disassembly with source to verify compilation

### ROM Building Tips

- Always verify checksums with `rominspect -c`
- Use descriptive titles and developer names
- Set correct entry point for your program
- Extract components with `-x` to verify contents

### Hex Viewer Tips

- Use `--analyze` to find patterns in unknown files
- Use `-w 8 -g 2` for 16-bit data visualization
- Use `-w 4 -g 4` for 32-bit data visualization
- Pipe to `grep` or `less` for large files

### Build Automation

Create a `build.sh` script:
```bash
#!/bin/bash
set -e

# Build CPU
def88186cc src/main.c -o build/main.asm
cpuasm build/main.asm build/cpu.bin

# Build PPU
ppuasm src/graphics.asm build/ppu.bin

# Build APU
apuasm src/audio.asm build/apu.bin

# Build ROM
rombuilder -cpu build/cpu.bin -ppu build/ppu.bin -apu build/apu.bin \
           -o game.zpb -title "My Game" -dev "Studio"

# Verify
rominspect game.zpb -c

echo "Build complete: game.zpb"
```

---

## Troubleshooting

### Assembly Errors

**Problem:** Label not found
```
Solution: Check for typos, ensure label is defined before use
```

**Problem:** Invalid instruction
```
Solution: Check instruction name, verify operand format
```

### Disassembly Issues

**Problem:** Garbage output
```
Solution: File may not be code, try different start offset with -s
```

**Problem:** Wrong instruction length
```
Solution: May be data, not code. Use hexview to inspect raw bytes
```

### ROM Building Errors

**Problem:** Invalid binary size
```
Solution: Check that input files exist and are valid
```

**Problem:** Entry point outside ROM
```
Solution: Verify entry point address matches your code layout
```

---

## See Also

- `README.md` - Main toolchain documentation
- `TODO.md` - Known issues and planned features
- `ASM_MULTIFILE_GUIDE.md` - Multi-file assembly guide
- `ROMBUILDER_GUIDE.md` - ROM builder guide
- `docs/cpu/` - CPU architecture documentation
- `docs/ppu/` - PPU architecture documentation
- `docs/apu/` - APU architecture documentation
- `examples/` - Example programs for all components

---

**ZeroPoint Development Tools** - Professional toolchain for fantasy console development.

Built with [Claude Code](https://claude.com/claude-code)

Co-Authored-By: Claude <noreply@anthropic.com>

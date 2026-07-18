# ZeroPoint Development Tools

Complete development toolchain for the ZeroPoint fantasy console.

## Tools Overview

### Compilers & Assemblers
- **C Compiler** (def88186cc) - Self-contained, DOS-portable C compiler with preprocessor and inline assembly. Accepts `bool`/`true`/`false` and `//` comments (early-1990s C style) in addition to C89.
- **LLVM C Compiler** (zpcc, `llvm_compiler/`) - clang/LLVM-based compiler for fast, standards-accurate compilation on modern hosts. Uses clang as the front end and lowers LLVM IR to DEF88186 assembly. See `llvm_compiler/README.md`.
- **CPU Assembler** (cpuasm) - DEF88186 assembly with `.include` support
- **PPU Assembler** (ppuasm) - PPU microcode with shorthand macros
- **APU Assembler** (apuasm) - APU/DSP programs with MMP/SST support

### Disassemblers
- **CPU Disassembler** (cpudisasm) - Disassemble DEF88186 binaries
- **PPU Disassembler** (ppudisasm) - Disassemble PPU microcode
- **APU Disassembler** (apudisasm) - Disassemble APU programs

### ROM Tools
- **Linker** (zplink) - Locate flat CPU/PPU/APU segments and emit an ELF32 executable (dev-side; no signing key)
- **ROM Signer** (zpbuild) - Sign a zplink ELF into a `.zpb` ROM (the HQ/mastering step; holds the key)
- **ROM Builder** (rombuilder) - Combine CPU/PPU/APU binaries into ZPB ROM files (classic path)
- **ROM Inspector** (rominspect) - Analyze, verify, and extract ROM components

### Utilities
- **Hex Viewer** (hexview) - Smart binary file viewer with statistics
- **WAV Converter** (wav2mmp) - Convert WAV files to MMP audio format

**📖 For complete tool documentation, see [DEV-TOOLS.md](DEV-TOOLS.md)**

---

**⚠️ IMPORTANT: Assembler Shorthand Users**

If you use any of the following shorthands:
- Label-based jumps (`JMR loop`, `JMZ label`, etc.)
- Stack operations (`PUSH`, `POP`, `JSR`, `RET`)
- Halt shorthand (`HLT`)

**Target registers 0 and 1 will be corrupted/overwritten by these shorthands.**

You CAN use target registers 0 and 1 manually, but be aware they'll get stomped on whenever you use a shorthand. For reliable code, stick to target registers 2 and 3.

✅ Safe from shorthand interference: Target registers 2 and 3
⚠️ Will be overwritten by shorthands: Target registers 0 and 1

See `docs/ppu/preset-e-and-shorthands.txt` for complete documentation.

## Building

### Modern Systems (macOS, Linux, Windows)

Build all tools at once:
```bash
make
```

Or build individual tool categories:
```bash
make assemblers      # ppuasm, apuasm, cpuasm
make disassemblers   # cpudisasm, ppudisasm, apudisasm
make rom-tools       # rombuilder, rominspect
make utilities       # hexview, wav2mmp
```

### MS-DOS 4.01+ / Turbo C 2.01

All tools are fully compatible with MS-DOS 4.01+ and can be compiled with Turbo C 2.01!

See **[DOS_BUILD.md](DOS_BUILD.md)** for complete DOS build instructions.

Quick build on DOS:
1. Copy all `.c` files and `compat.h` to DOS
2. Run `BUILD.BAT`

**Note:** DOS versions use 8.3 filenames (e.g., `cpudasm.c` instead of `cpudisasm.c`). Modern builds support both names via symlinks.

## zpasm - PPU Assembler

Assembles PPU microcode assembly language into binary format.

### Usage

```bash
./zpasm <input.asm> [-o output.bin]
```

If no output file is specified, uses the input filename with `.bin` extension.

### Syntax

#### Comments

```asm
; This is a comment
INC R0  ; Increment register 0
```

#### Labels

```asm
loop:
    INC R0
    CMP R0, R1
    JNG     ; Jump to PC if not greater
```

#### Registers

Registers can be specified in multiple formats:
- `R0` to `R63` - Standard register notation
- `0` to `63` - Direct register numbers
- `R59` - V-Blank Interrupt Address (pushes return address and jumps on V-Blank if non-zero)
- `R60` - H-Blank Interrupt Address (pushes return address and jumps on H-Blank if non-zero)
- `SP` - Stack Pointer (alias for R61, used by interrupts and RET)
- `PC` - Program Counter (alias for R62)
- `DP` - Data Pointer (alias for R63)

**Interrupts**: When R59 or R60 is non-zero, the PPU automatically pushes the return address onto the stack before jumping to the interrupt handler. Use `RET` to return from interrupt handlers.

#### Immediate Values

- Decimal: `42`
- Hexadecimal: `0x2A` or `0x2a`
- Binary: `0b00101010`

### Instructions

#### Basic Instructions

| Instruction | Operands | Description | Example |
|------------|----------|-------------|---------|
| `DEFCALL` | X, Y | Define callable: address in RX, ID in RY.LSB | `DEFCALL R10, R11` |
| `ENDDEFCALL` | X | End call definition for ID in RX.LSB | `ENDDEFCALL R11` |
| `SWAPREG` | X, Y | Swap registers X and Y | `SWAPREG R0, R1` |
| `CLR` | X | Clear register X | `CLR R0` |
| `CMP` | X, Y | Compare X and Y (sets flags) | `CMP R0, R1` |
| `CLRF` | - | Clear comparison flags | `CLRF` |
| `JMZ` | - | Jump to (PC) if zero | `JMZ` |
| `JMG` | - | Jump to (PC) if greater | `JMG` |
| `JNZ` | - | Jump to (PC) if not zero | `JNZ` |
| `JNG` | - | Jump to (PC) if not greater | `JNG` |
| `JML` | - | Jump to (PC) if less (alias JNG) | `JML` |
| `INC` | X | Increment register X | `INC R0` |
| `DEC` | X | Decrement register X | `DEC R0` |
| `ADD` | X, Y | X = X + Y | `ADD R0, R1` |
| `SUB` | X, Y | X = X - Y | `SUB R0, R1` |
| `MUL` | X, Y | X = X × Y | `MUL R0, R1` |
| `INTDIV` | X, Y | X = X / Y (integer) | `INTDIV R0, R1` |

#### Preset E Instructions (Immediate/Byte Operations)

**⚠️ If using shorthands, target registers 0 and 1 will be overwritten! Use 2 and 3 for safety.**

| Instruction | Operands | Description | Example |
|------------|----------|-------------|---------|
| `TARREG` | T, Y, X | Set target register T to point to register X's byte Y | `TARREG 2, LSB, R5` |
| `SETBYTE` | T, imm8 | Set byte in target register to 8-bit immediate | `SETBYTE 2, 0x42` |
| `BUILD` | T1, T2, X | Build register X from target registers (T1=MSB, T2=LSB) | `BUILD 2, 3, R10` |
| `CPREG` | X, Y | Copy register X to Y (uses register banks) | `CPREG R5, R10` |

**Example - Load 0x1234 into R5:**
```asm
TARREG 2, LSB, R5    ; Target 2 → R5 low byte
TARREG 3, MSB, R5    ; Target 3 → R5 high byte
SETBYTE 2, 0x34      ; R5[7:0] = 0x34
SETBYTE 3, 0x12      ; R5[15:8] = 0x12
```

#### Preset F Instructions (Graphics/Control)

| Instruction | Operands | Description | Example |
|------------|----------|-------------|---------|
| `SETPOS` | X, Y | Set position (4-bit reg IDs) | `SETPOS R0, R1` |
| `SETTILE` | X, mode | Set tile from register X, mode 0-3 | `SETTILE R10, 3` |
| `SETDP` | X | Set DP from register X | `SETDP R5` |
| `MOVDP` | X | Move register X to (DP) | `MOVDP R1` |
| `SETRENDMOD` | mode | Set render mode (0=16bit, 1=32bit) | `SETRENDMOD 0` |
| `PALETTE16` | - | Load 16-color palette from (DP) | `PALETTE16` |
| `PALETTE256` | - | Load 256-color palette from (DP) | `PALETTE256` |
| `JMR` | - | Jump relative to (PC) | `JMR` |
| `MOV` | X | Move from (DP) to register X | `MOV R5` |
| `SETREGBANK` | X_bank, Y_bank | Set register banks | `SETREGBANK 0, 1` |
| `CLRTILE` | - | Clear all tile storage (all 4 banks) | `CLRTILE` |
| `CLRPALETTE` | - | Clear palette | `CLRPALETTE` |
| `TILEDRAW` | - | Draw tile at position (from 0x0200); dispatches to one of 8 async blit channels when one is free, else draws synchronously | `TILEDRAW` |
| `SETTILEBANK` | X | Select tile bank (0-3) from register X | `SETTILEBANK R5` |
| `CALL` | address | Call function | `CALL function_label` |
| `GBLS` | X | Get blank status into register X | `GBLS R0` |

### Assembler Shorthands

The assembler provides powerful shorthands that expand to instruction sequences. **These use target registers 0 and 1 internally.**

#### Label-Based Jumps

Instead of manually loading PC, you can jump directly to labels:

```asm
loop:
    INC R0
    CMP R0, R10
    JNG loop        ; Jumps if R0 <= R10
    HLT             ; Halt when done
```

Supported: `JMR label`, `JMZ label`, `JNZ label`, `JMG label`, `JNG label`

#### Stack Operations

```asm
PUSH R5         ; Push register onto stack
POP R5          ; Pop from stack into register
JSR function    ; Jump to subroutine (saves PC)
RET             ; Return from subroutine
```

#### HLT Shorthand

```asm
HLT             ; Infinite loop (replaces old HALT opcode)
```

#### Variable Aliases

Define meaningful names for registers and constants:

```asm
$0x0100 = IO_BASE
R10 = COUNTER
R20 = X_POS

; Then use them:
TARREG 2, LSB, COUNTER
SETBYTE 2, IO_BASE
```

### Examples

#### Simple Counter (0 to 10) - New Method

```asm
; Count from 0 to 10 using new features
R0 = COUNTER
R1 = TARGET

start:
    CLR COUNTER

    ; Load TARGET = 10 using Preset E
    TARREG 2, LSB, TARGET
    SETBYTE 2, 10

loop:
    INC COUNTER
    CMP COUNTER, TARGET
    JNG loop        ; Jump if COUNTER <= TARGET (uses shorthand)

    HLT             ; Halt (uses shorthand)
```

#### Addition - New Method

```asm
; Add two numbers: R2 = R0 + R1
R0 = A
R1 = B
R2 = RESULT

start:
    ; Load A = 5
    TARREG 2, LSB, A
    SETBYTE 2, 5

    ; Load B = 10
    TARREG 2, LSB, B
    SETBYTE 2, 10

    ; RESULT = A + B
    CLR RESULT
    ADD RESULT, A
    ADD RESULT, B

    HLT
```

#### Memory Operations - New Method

```asm
; Write value to memory and read it back
R10 = VALUE

start:
    ; Load DP = 0x1000
    TARREG 2, LSB, DP
    TARREG 3, MSB, DP
    SETBYTE 2, 0x00
    SETBYTE 3, 0x10     ; DP = 0x1000

    ; Load VALUE = 42
    TARREG 2, LSB, VALUE
    SETBYTE 2, 42

    ; Write VALUE to (DP)
    MOVDP VALUE

    ; Clear VALUE
    CLR VALUE

    ; Read from (DP) back to VALUE
    MOV VALUE

    HLT
```

#### Check Blank Status

```asm
; Get VBlank/HBlank status
    GBLS R0         ; R0 = 0 (VBlank), 1 (HBlank), or 65535 (active)
    HALT
```

## Binary Format

The assembler outputs raw 16-bit big-endian instructions:

```
[byte 0] [byte 1]  = instruction 0
[byte 2] [byte 3]  = instruction 1
...
```

Each instruction is encoded as:
```
[15:12] - 4-bit opcode
[11:0]  - 12-bit operand
```

For Preset F instructions (opcode 0xF):
```
[15:12] - 0xF
[11:8]  - 4-bit sub-opcode
[7:0]   - 8-bit sub-operand
```

## Testing Assembled Programs

### PPU Programs

You can test assembled PPU binaries with the test runner:

```bash
cd ZeroPoint/build
./bin/test_demo examples/your_program.bin
```

This loads the binary and runs it on the PPU, showing visual output.

### APU Programs

You can test assembled APU binaries with the APU test runner:

```bash
cd ZeroPoint/build
./bin/test_apu examples/apu/your_program.bin 10000
```

This loads the binary and runs it on the APU emulator for up to 10,000 cycles.

For audio output:
```bash
./bin/run_apu_demo examples/apu/your_program.bin
```

## apuasm - APU Assembler

Assembles APU assembly language into binary format.

### Usage

```bash
./apuasm <input.asm> [output.bin]
```

If no output file is specified, uses the input filename with `.bin` extension.

### Syntax

#### Comments
```asm
; This is a comment
SCR X, 42    ; Set X register to 42
```

#### Labels
```asm
loop:
    ADD X, Y, X
    JMP loop
```

#### Registers
- `X`, `Y` - Primary registers (aliases for R0, R1)
- `R0` to `R255` - General purpose registers
- Special registers: `PC` (Program Counter), `RP` (ROM Page), `DP` (Data Page), `DB` (Data Byte)

#### Immediate Values
- Decimal: `42`
- Hexadecimal: `0x2A` or `$2A`
- Binary: `0b00101010`

### APU Instructions

See `docs/apu/instruction-set.txt` for complete instruction reference.

#### Common Instructions

| Instruction | Example | Description |
|------------|---------|-------------|
| `NOP` | `NOP 100` | No operation (stall 100 cycles) |
| `SCR` | `SCR X, 42` | Set register to constant |
| `ADD` | `ADD X, Y, R2` | Add X + Y, store in R2 |
| `SUB` | `SUB X, Y, R2` | Subtract X - Y, store in R2 |
| `JMP` | `JMP $50` | Jump to address |
| `LST` | `LST 0, 10` | Loop start (ID 0, 10 iterations) |
| `LFN` | `LFN 0` | Loop finish (ID 0) |
| `SDP` | `SDP $30` | Set data page to $30 |
| `WRH` | `WRH 0xAB` | Write high byte to $DPDB |
| `WRL` | `WRL 0xCD` | Write low byte to $DPDB |
| `HLT` | `HLT` | Halt (infinite loop) |

### APU Example Programs

#### Simple Addition
```asm
; Add 42 + 100, result in R2
start:
    SCR X, 42       ; X = 42
    SCR Y, 100      ; Y = 100
    ADD X, Y, R2    ; R2 = X + Y = 142
    HLT
```

#### Hardware Loop
```asm
; Count from 0 to 10
start:
    ZOR X           ; X = 0
    LST 0, 10       ; Loop 10 times
loop:
    SCR Y, 1
    ADD X, Y, X     ; X++
    LFN 0           ; End loop
    HLT
```

See `examples/apu/` for more examples.

## Documentation

### PPU Documentation
- `README.md` - This file (quick reference)
- `docs/ppu/ucode.txt` - Full PPU microcode specification (Revision 3.0, ground-truth-verified against the emulator source), including SETTILEBANK, tile banking, and the concurrent TILEDRAW blitter
- `docs/ppu/preset-e-and-shorthands.txt` - Preset E, SETTILE/TILEDRAW/SETTILEBANK & assembler shorthands guide — pairs well with `ucode.txt`, focused more on idioms/examples than architecture

**⚠️ Read `preset-e-and-shorthands.txt` for shorthand/idiom examples; `ucode.txt` is the full architectural reference.**

### APU Documentation
- `docs/apu/README.txt` - APU documentation index
- `docs/apu/overview.txt` - Architecture overview (8-bit RISC, 4.2 MHz, MMP, SST)
- `docs/apu/instruction-set.txt` - Complete instruction reference (41 instructions)
- `docs/apu/memory-map.txt` - Memory layout ($0000-$FFFF, banking)
- `docs/apu/registers.txt` - Register reference (X, Y, PC, RP, DP, DB, BF)
- `docs/apu/sst.txt` - Sample Storage System format
- `docs/apu/mmp.txt` - Music Mixing Processor (16 stereo channels)
- `docs/apu/programming-guide.txt` - Examples and programming patterns

**🎵 Complete APU/DSP documentation now available!**

## Files

### Assemblers
- `zpasm.c` - PPU assembler (C source)
- `zpasm` - Compiled PPU assembler (created by gcc)
- `apuasm.c` - APU assembler (C source)
- `apuasm` - Compiled APU assembler (created by gcc)

### Examples
- `examples/ppu/` - PPU example programs
  - `test_preset_e.asm` - Demonstrates Preset E and shorthands
  - `fibonacci.asm` - Fibonacci sequence generator
  - Various pixel/tile demos
- `examples/apu/` - APU example programs
  - `hello.asm` - Simple register operations
  - `counter.asm` - Hardware loop counter
  - `tone_gen.asm` - Sample data generation

### Documentation
- `docs/ppu/` - PPU documentation
  - `ucode.txt` - Full PPU instruction reference (Revision 3.0, all opcodes)
  - `preset-e-and-shorthands.txt` - PPU idioms/shorthands guide (Preset E, SETTILE, TILEDRAW, SETTILEBANK)
- `docs/apu/` - APU documentation (8 files)
  - `README.txt` - APU documentation index
  - `overview.txt` - Architecture overview
  - `instruction-set.txt` - Complete APU instruction reference
  - `memory-map.txt` - Memory layout
  - `registers.txt` - Register reference
  - `sst.txt` - Sample Storage System
  - `mmp.txt` - Music Mixing Processor
  - `programming-guide.txt` - Examples and patterns
- `README.md` - This file
- `TODO.md` - Development tracking

## License

Licensed under the Mozilla Public License v2.0 (MPL-2.0) — see [LICENSE](LICENSE).

MPL-2.0 is file-level copyleft: modifications to these tools' own source files must stay under MPL and have their source made available if distributed. Programs, ROMs, or games you build *with* these tools (via `cpuasm`, `ppuasm`, `apuasm`, `def88186cc`, `rombuilder`, etc.) are not Covered Software and are not subject to this license — you may license your own output however you like.

# ZeroPoint Development Tools

Development tools for the ZeroPoint fantasy console PPU microarchitecture.

## Building

```bash
make
```

This creates the `zpasm` executable.

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
- `PC` - Program Counter (alias for R62)
- `DP` - Data Pointer (alias for R63)

#### Immediate Values

- Decimal: `42`
- Hexadecimal: `0x2A` or `0x2a`
- Binary: `0b00101010`

### Instructions

#### Basic Instructions

| Instruction | Operands | Description | Example |
|------------|----------|-------------|---------|
| `DEFCALL` | address | Define callable function | `DEFCALL 0x100` |
| `ENDDEFCALL` | - | End function definition | `ENDDEFCALL` |
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
| `HALT` | - | Stop execution | `HALT` |

#### Preset F Instructions (Graphics/Control)

| Instruction | Operands | Description | Example |
|------------|----------|-------------|---------|
| `SETPOS` | X, Y | Set position (4-bit reg IDs) | `SETPOS R0, R1` |
| `SETTILE` | tile_id | Set tile ID | `SETTILE 0x42` |
| `SETDP` | X | Set DP from register X | `SETDP R5` |
| `MOVDP` | X | Move register X to (DP) | `MOVDP R1` |
| `SETRENDMOD` | mode | Set render mode (0=16bit, 1=32bit) | `SETRENDMOD 0` |
| `PALETTE16` | - | Load 16-color palette from (DP) | `PALETTE16` |
| `PALETTE256` | - | Load 256-color palette from (DP) | `PALETTE256` |
| `JMR` | - | Jump relative to (PC) | `JMR` |
| `MOV` | X | Move from (DP) to register X | `MOV R5` |
| `SETREGBANK` | X_bank, Y_bank | Set register banks | `SETREGBANK 0, 1` |
| `CLRTILE` | - | Clear current tile | `CLRTILE` |
| `CLRPALETTE` | - | Clear palette | `CLRPALETTE` |
| `STRTDEFTILE` | - | Start defining tile | `STRTDEFTILE` |
| `ENDDEFTILE` | - | End defining tile | `ENDDEFTILE` |
| `CALL` | address | Call function | `CALL function_label` |
| `GBLS` | X | Get blank status into register X | `GBLS R0` |

### Examples

#### Simple Counter (0 to 10)

```asm
; Count from 0 to 10
    CLR R0          ; Counter
    CLR R1          ; Target

    ; Set R1 = 10
    INC R1
    INC R1
    INC R1
    INC R1
    INC R1
    INC R1
    INC R1
    INC R1
    INC R1
    INC R1

    ; Set PC to loop address (calculate manually or use second example)
    CLR PC
    ; ... set PC to loop start address

loop:
    INC R0
    CMP R0, R1
    JNG             ; Jump if R0 <= R1

    HALT
```

#### Addition

```asm
; Add two numbers: R2 = R0 + R1
    CLR R0
    CLR R1
    CLR R2

    ; R0 = 5
    INC R0
    INC R0
    INC R0
    INC R0
    INC R0

    ; R1 = 10
    INC R1
    INC R1
    INC R1
    INC R1
    INC R1
    INC R1
    INC R1
    INC R1
    INC R1
    INC R1

    ; R2 = R0 + R1
    ADD R2, R0
    ADD R2, R1

    HALT
```

#### Memory Operations

```asm
; Write value to memory and read it back
    CLR R0
    CLR R1
    CLR R2

    ; Set R0 = 0x1000 (memory address)
    ; ... (increment R0 to desired address)

    ; Set DP to R0
    SETDP R0

    ; Write R1 to (DP)
    MOVDP R1

    ; Read from (DP) to R2
    MOV R2

    HALT
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

You can test assembled binaries with the test runner:

```bash
make test_assembled
./test_assembled examples/add.bin
```

This loads the binary and runs it on the PPU, showing the final register states.

## Files

- `zpasm.c` - PPU assembler (C source)
- `zpasm` - Compiled assembler (created by make)
- `test_assembled.cpp` - Test runner for assembled binaries
- `examples/` - Example assembly programs
  - `add.asm` - Simple addition (5 + 10 = 15)
  - `counter.asm` - Loop counter (0 to 5)
  - `memory.asm` - Memory read/write operations
  - `vblank.asm` - Blank status check
- `Makefile` - Build configuration
- `README.md` - This file

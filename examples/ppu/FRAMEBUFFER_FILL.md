# Direct Framebuffer Fill - White Screen Demo

**IMPORTANT:** Pixel I/O ($0100-$010A) is DEPRECATED. Always write directly to the framebuffer.

## The Correct Method

### Framebuffer Memory Map
- **Address**: $E000 - $FFFF (8 KB)
- **Size**: 8192 bytes = 4096 pixels
- **Format**: 16-bit per pixel (RGBA16: 5-5-5-1)
- **White**: 0xFFFF (all bits set)

### Direct Write Pattern

```asm
; 1. Point DP to framebuffer start
TARREG 3, LSB, DP
SETBYTE 3, 0x00
TARREG 3, MSB, DP
SETBYTE 3, 0xE0         ; DP = $E000

; 2. Load pixel value
TARREG 2, LSB, R0
SETBYTE 2, 0xFF
TARREG 2, MSB, R0
SETBYTE 2, 0xFF         ; R0 = white

; 3. Write loop
fill:
    MOV R0, (DP)        ; Write to address in DP
    ADD DP, R1          ; Increment pointer
    CMP DP, R2          ; Check if done
    JNG fill            ; Loop if not
```

## Working Demo: `white_fill.asm`

Minimal 58-byte demo that fills screen with white:

```asm
start:
    TARREG 3, LSB, DP
    SETBYTE 3, 0x00
    TARREG 3, MSB, DP
    SETBYTE 3, 0xE0         ; DP = $E000 (framebuffer)

    TARREG 2, LSB, R0
    SETBYTE 2, 0xFF
    TARREG 2, MSB, R0
    SETBYTE 2, 0xFF         ; R0 = $FFFF (white)

    TARREG 2, LSB, R1
    SETBYTE 2, 0x02
    TARREG 2, MSB, R1
    SETBYTE 2, 0x00         ; R1 = 2 (step)

    TARREG 2, LSB, R2
    SETBYTE 2, 0xFE
    TARREG 2, MSB, R2
    SETBYTE 2, 0xFF         ; R2 = $FFFE (end)

fill:
    MOV R0, (DP)
    ADD DP, R1
    CMP DP, R2
    JNG fill

done:
    HLT
```

## Build & Run

```bash
cd /Users/alexanderwhite/Documents/Code/ZPdevtools
./ppuasm examples/ppu/white_fill.asm examples/ppu/white_fill.bin

cd /Users/alexanderwhite/Documents/Code/ZeroPoint/build_qt
./bin/run_demo ../../ZPdevtools/examples/ppu/white_fill.bin 2>&1 | tee white_fill_output.txt
```

## Color Values (16-bit RGBA)

| Color | Hex Value | Binary |
|-------|-----------|--------|
| White | $FFFF | 1111 1111 1111 1111 |
| Black | $0000 | 0000 0000 0000 0000 |
| Red | $F801 | 1111 1000 0000 0001 |
| Green | $07C1 | 0000 0111 1100 0001 |
| Blue | $003F | 0000 0000 0011 1111 |

Format: RRRRRGGGGGGBBBBA (5-6-5-1)

## Performance

- **Cycles**: ~8200 (fill 4096 pixels)
- **Instructions**: 29
- **Binary size**: 58 bytes

## Key Registers

- **DP (R63)**: Data Pointer - framebuffer address
- **R0**: Pixel value to write
- **R1**: Step (always 2 for 16-bit pixels)
- **R2**: End address
- **Targets 2-3**: Temp for TARREG/SETBYTE operations

**DO NOT use $0100-$010A pixel I/O - it's deprecated!**

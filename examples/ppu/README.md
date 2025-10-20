# PPU Gradient and Graphics Demos

This directory contains example programs demonstrating the ZeroPoint PPU's graphics capabilities.

## Demos

### color_bars.asm (176 bytes)
Draws 8 vertical color bars across the screen.
- **Features**: 32-bit color mode, nested loops, pixel I/O
- **Size**: 8 bars × 32 pixels wide × 64 pixels tall
- **Performance**: ~16,384 pixels × ~20 instructions = ~327K cycles

### gradient_demo_simple.asm (114 bytes)
Draws 64 scanlines with horizontal color gradient.
- **Features**: Per-pixel color calculation, R=Y, G=X, B=255-Y
- **Size**: 256×64 pixels = 16,384 pixels
- **Performance**: ~327K cycles to complete

### gradient_test.asm (156 bytes)
Quick 8×8 pixel grid test (scaled 32× for visibility).
- **Features**: Fast rendering test, 64 pixels only
- **Size**: 8×8 grid scaled to 256×256 window
- **Performance**: ~1,280 cycles to complete

### gradient_rolling_demo.asm (142 bytes) ⚠️ Advanced
V-Blank interrupt-driven gradient renderer.
- **Features**: Uses R59 V-Blank interrupt, automatic return address push, RET
- **Complexity**: Requires stack initialization and interrupt handling
- **Performance**: Draws incrementally during V-Blank periods
- **Note**: Runs indefinitely (infinite loop with interrupt handler)

## Running Demos

### Quick Test (headless)
```bash
cd /path/to/ZeroPoint/build
./bin/test_demo /path/to/demo.bin
```

### Interactive (with window)
```bash
cd /path/to/ZeroPoint/build
./bin/run_demo /path/to/demo.bin
```

The window will display the rendered output. Press ESC to close.

## How They Work

### Pixel I/O Method
Most demos use memory-mapped pixel I/O at `0x0100-0x010B`:

```asm
; Set DP to pixel I/O base
TARREG 2, LSB, DP
TARREG 3, MSB, DP
SETBYTE 2, 0x00
SETBYTE 3, 0x01

; Write X position (0x0100-0x0101)
MOVDP X_VAL
INC DP
INC DP

; Write Y position (0x0102-0x0103)
MOVDP Y_VAL
INC DP
INC DP

; Write R, G, B (0x0104-0x0109)
MOVDP RED
INC DP
INC DP
MOVDP GREEN
INC DP
INC DP
MOVDP BLUE
INC DP
INC DP

; Write Alpha (0x010A) - triggers pixel draw!
TARREG 2, LSB, TEMP
SETBYTE 2, 255
MOVDP TEMP
```

### V-Blank Interrupt Method
Advanced demos use R59 for V-Blank interrupts:

```asm
; Initialize stack (REQUIRED!)
TARREG 2, LSB, SP
TARREG 3, MSB, SP
SETBYTE 2, 0xFF
SETBYTE 3, 0xFF

; Set interrupt handler address
TARREG 2, LSB, R59
TARREG 3, MSB, R59
SETBYTE 2, handler
SETBYTE 3, handler

; Main loop
main:
    JMR main

; Interrupt handler
handler:
    ; Draw pixels here
    ; ...
    RET  ; Return address automatically pushed by hardware!
```

## Performance Notes

- **Pixel I/O**: ~20 instructions per pixel
- **256×256 screen**: 65,536 pixels × 20 = ~1.3M instructions
- **64 scanlines**: 16,384 pixels × 20 = ~327K instructions
- **Rolling Framebuffer**: Only 8-16 scanlines buffered at once
- **V-Blank Window**: ~1,100 cycles per frame @ 60Hz

For full-screen effects, use V-Blank interrupts to draw ahead into the rolling framebuffer!

## Bank-Based Framebuffer

The display uses a bank-based rolling buffer:
- **8 banks** × 1 KiB = 8 KiB total
- **16-bit mode**: 16 scanlines buffered (2 per bank)
- **32-bit mode**: 8 scanlines buffered (1 per bank)
- **Memory-mapped**: `0xE000-0xFFFF`

Each H-Blank, banks roll:
- 32-bit mode: 2 banks per H-Blank
- 16-bit mode: 1 bank per H-Blank (every 2 scanlines)

## Tips

1. **Start simple**: Use `gradient_test.asm` or `color_bars.asm` first
2. **Loop limits**: Use `CMP X, 7; JNG loop` for "while X < 8"
3. **Stack required**: Always initialize SP before using interrupts
4. **RET works**: Interrupts automatically push return address
5. **Be patient**: Pixel I/O is slow but works correctly

## Troubleshooting

**Demo doesn't halt?**
- Check loop conditions (JNG means ≤, not <)
- Use limit-1 for "while X < N": `CMP X, N-1; JNG loop`

**Nothing renders?**
- Ensure SETRENDMOD is called (0=16-bit, 1=32-bit)
- Check that Alpha is written last (triggers pixel draw)
- Verify run_demo is syncing Display with PPU

**Slow performance?**
- Normal! Pixel I/O takes ~20 instructions per pixel
- For full screen: use V-Blank interrupts
- Consider direct framebuffer access at 0xE000

Happy coding! 🎨

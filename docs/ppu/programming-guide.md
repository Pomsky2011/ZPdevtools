# PPU programming guide

Writing PPU programs with `ppuasm`. Every block here is assembled by `python3 scripts/check_doc_asm.py`, and each program
with a stated result was run on the emulator. Read [architecture.md](architecture.md) for the registers and ports and
[assembler.md](assembler.md) for the shorthands and their traps.

## Constants and registers

A constant comes in a byte at a time through a **target register**. Use targets 2 and 3: the assembler's jump shorthands
overwrite 0 and 1.

```ppuasm
    TARREG 2, LSB, R7
    SETBYTE 2, 0x00
    TARREG 3, MSB, R7
    SETBYTE 3, 0x01        ; R7 = $0100
```

A register starts at 0, so a constant under 256 needs only the low half. Keep anything you use in a loop in a register: a
16-bit constant costs 8 cycles each time.

## Subroutines

`JSR label` and `RET` use the stack at `SP` (`R61`, reset to `$8000`). This program calls a routine twice and ends with
`R5 = 20`, `SP` back at `$8000`:

```ppuasm
main:
    CLR R5
    JSR add_ten
    JSR add_ten
    HLT
add_ten:
    TARREG 2, LSB, R6
    SETBYTE 2, 10
    ADD R5, R6
    RET
```

`JSR` and `RET` are 26 and 10 bytes. For a hot call, `DEFCALL` a slot once and `CALL n` it: two bytes each time.

## Loops and comparisons

Only `CMP` sets flags, so a loop must compare. `JNG` jumps if *not greater*, so this loops while `R5 <= 5` and finishes with
`R5 = 6`:

```ppuasm
    CLR R5
    CLR R6
    TARREG 2, LSB, R6
    SETBYTE 2, 5           ; R6 = 5
loop:
    INC R5
    CMP R5, R6
    JNG loop
    HLT
```

A taken jump costs 3 cycles, so the loop is 1 + 1 + 3 = 5 cycles an iteration.

## Drawing a pixel

Write X, Y and a palette index to the pixel port, then write anything to `$010A`. After reset the 16-entry palette is
grayscale, so index 15 is white. This draws a white pixel at (10, 10):

```ppuasm
    TARREG 2, LSB, R5
    SETBYTE 2, 10          ; R5 = 10
    TARREG 2, LSB, R6
    SETBYTE 2, 15          ; R6 = 15, the palette index
    TARREG 2, LSB, R7
    SETBYTE 2, 0x00
    TARREG 3, MSB, R7
    SETBYTE 3, 0x01        ; R7 = $0100
    SETDP R7
    MOVDP R5               ; $0100 = X
    INC R7
    INC R7
    SETDP R7
    MOVDP R5               ; $0102 = Y
    INC R7
    INC R7
    SETDP R7
    MOVDP R6               ; $0104 = palette index
    TARREG 2, LSB, R7
    SETBYTE 2, 0x0A
    SETDP R7
    MOVDP R6               ; $010A: this write draws the pixel
    HLT
```

Run on the emulator, pixel (10, 10) reads back `FFFFFFFF` (opaque white) and its neighbour at (11, 10) is untouched. The pixel
port is for one-offs; anything bigger uses tiles ([graphics.md](graphics.md)).

## Waiting for the beam

`GBLS RX` tells you where the beam is: 0 in V-blank, 1 in H-blank, 65535 in the visible area. To wait for V-blank, loop on it:

```ppuasm
wait_vblank:
    GBLS R5
    CLR R6
    CMP R5, R6
    JNZ wait_vblank        ; R5 is 0 only in V-blank
    HLT
```

This is a polling loop and it uses `JNZ label`, which loads `R62` each time round. That is fine for a program that takes no
interrupts. A program that does use interrupts should not poll like this; see the next section.

## Interrupts

The PPU's V-blank interrupt needs three things: the handler's address in `R59`, bit 4 of `$00F0` set, and a way for the main
program to wait that an interrupt cannot wedge. This program counts V-blanks in `R8`; after 100,000, 300,000 and 800,000 PPU
cycles it reads 1, 3 and 9, with `SP` back at `$8000`:

```ppuasm
    ; R59 = address of the handler
    TARREG 2, LSB, R59
    SETBYTE 2, vblank
    TARREG 3, MSB, R59
    SETBYTE 3, vblank

    ; VOC $00F0 bit 4 = V-blank interrupt enable
    TARREG 2, LSB, R7
    SETBYTE 2, 0xF0
    TARREG 3, MSB, R7
    SETBYTE 3, 0x00        ; R7 = $00F0
    SETDP R7
    TARREG 2, LSB, R6
    SETBYTE 2, 0x10        ; R6 = $0010
    MOVDP R6

    ; wait the safe way: R62 = idle, then spin on a bare JMR
    TARREG 2, LSB, R62
    SETBYTE 2, idle
    TARREG 3, MSB, R62
    SETBYTE 3, idle
idle:
    JMR

vblank:
    INC R8                 ; count it
    RET
```

The bare `JMR` is deliberate. `RET` ends by loading the return address into `R62`, so after any interrupt `R62` holds the
address of the interrupted instruction. For a `JMR` that is its own address, which is exactly where the loop must go. A
`JMR label` loop would be wedged by an interrupt landing at the wrong cycle; see [architecture.md](architecture.md).

Handlers should be short (the H-blank window is 84 cycles), should save and restore `DP` if they change it, and must keep
the stack balanced: every interrupt pushes, and `RET` pops.

## Starting a program from the CPU

The CPU loads the program into the PPU window (bank `$B0`) at `$0000` and writes `PPUCTRL` (`$D80000`) bit 0. The Boot ROM's
`COP #$F7` does the load, and `lib/zp_cop.asm` wraps it. For a quick look, the emulator repository's `bin/run_demo program.bin`
runs a PPU program in a window. See also `examples/ppu/` for working programs.

## Debugging

1. Is the program at address 0, and was the PPU started? Check `PPUSTATUS` and `PPUPC`.
2. Is the loop wedged? Sample `PPUPC` over time: a PPU spinning on one address with `R62` equal to it is stuck on its own
   jump. This is what an interrupt does to a `JMR label` loop.
3. Did a `0b` literal silently become 0? `ppuasm` does not read binary numbers.
4. Did a constant above 255 get truncated? `SETBYTE` takes only a byte, and a plain number is not split.
5. For tiles: is the tile ID and bank the one you defined? The `$0300` window writes whichever tile is selected right now.
6. For layered drawing, did the late bit (`$0364` bit 0) get set? A row was queued too late for its frame.

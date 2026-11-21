# Timing

One CPU cycle is 4 master cycles: 58.8 ns at 17.002839 MHz. The CPU is deterministic, so a given program takes the same
number of cycles every time.

## The cost model

Every instruction has a fixed cost, listed in [opcode-table.md](opcode-table.md). Fetching the opcode and operands, and
reading or writing memory, add nothing on top: where the instruction's bytes are, and whether it touches ROM or RAM,
changes nothing. M and X do not change a cost either.

The only things that change a cost:

| Case | Extra |
|------|-------|
| A read through `addr,X`, `addr,Y` or `(dp),Y` whose index addition carries into the high address byte | +1 cycle |
| A *store* or read-modify-write through those same three forms | +1 cycle, always (already in the table) |
| A conditional branch (`BEQ` `BMI` `BCS` `BVS`) that is taken | +1 cycle |
| ... and whose target is in a different 256-byte page from the next instruction | +1 more |
| `BRA` to a different page | +1 |
| `MVN`, `MVP` | 1 cycle per byte moved |
| Any access to the APU window (bank `$A0`) while the APU is in fast mode | +1 cycle per byte accessed |
| `WAI` | 3 cycles, then **the cycle counter stands still** while the CPU waits |

Costs worth knowing:

| | Cycles |
|---|--------|
| `NOP`, register transfers, flag instructions, `INX`/`INY`/`INC A` | 2 |
| `LDA #`, `LDA addr`, `STA addr` | 2, 3, 3 |
| `LDA long`, `STA long` | 5 |
| `LDA addr,X` | 3, or 4 across a page |
| `JSR` and `RTS` | 4 and 6 (10 for the pair) |
| `CALL` and `RTL` | 16 and 6 |
| `BRK`, `COP`, an interrupt, `RTI` | 7, 7, 7, 6 |
| `MUL`, `DIV` | 8 |
| `LOOP`, and each `LPEND` | 2, and 1 |
| `HLT` | 0 |

`CALL` at 16 cycles is more than three times a `JSR`; prefer `JSR` within a bank.

## How many cycles you have

The display is 261 lines, each 4343 master cycles (core revision 13; before it, 340 cycles a line, one per pixel), so:

| | Master cycles | CPU cycles | Time |
|---|---------------|-----------|------|
| One scanline | 4,343 | **1,086** | 63.9 µs |
| Visible part of a line (pixels 0-255) | 3,271 | 818 | |
| H-blank part (pixels 256-339) | 1,072 | 268 | |
| One frame (261 lines) | 1,133,523 | **283,381** | 16.667 ms |
| V-blank (5 lines) | 21,715 | 5,429 | |

A frame is exactly 1/60 s, the period of the `V` timer, and a pixel lasts about 12.8 master cycles (the beam is at pixel
`floor(cycle in line x 340 / 4343)`, which is what the display status registers report). A frame gives the CPU about
283,000 cycles, roughly 70,000 instructions. (Before core revision 13 a pixel was one cycle: a frame was 88,740 cycles, 1.3 ms,
about 766 a second, and the CPU got only 22,185 cycles of it.) The `V` timer is not phase-locked to the raster, so for anything
that must change in the V-blank, use the display's own V-blank.

## Measuring

The emulator can print the speed it runs at (`zeropoint_sdl --speed`) and the headless `bin/debugger` shows the cycle
counter after each step. The cycle counter is the CPU's own count: it does not include the cycles spent in `WAI`.

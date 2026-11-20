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

The display is 261 lines of 340 pixels, drawn at the master clock rate, so:

| | Master cycles | CPU cycles | Time |
|---|---------------|-----------|------|
| One scanline | 340 | **85** | 5.0 µs |
| Visible part of a line (256 pixels) | 256 | 64 | |
| H-blank part (84 pixels) | 84 | 21 | |
| One frame (261 lines) | 88,740 | **22,185** | 1.30 ms |
| V-blank (5 lines) | 1,700 | 425 | |

That is about 766 frames a second, not 60. A "frame" in this machine is short; the CPU gets only 22,185 cycles in it,
roughly 5,500 instructions. Anything that must happen at 60 Hz is paced by a timer (the `V` timer is 16.667 ms) or by
counting frames, not by the display's own V-blank.

## Measuring

The emulator can print the speed it runs at (`zeropoint_sdl --speed`) and the headless `bin/debugger` shows the cycle
counter after each step. The cycle counter is the CPU's own count: it does not include the cycles spent in `WAI`.

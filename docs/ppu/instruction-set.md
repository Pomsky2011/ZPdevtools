# PPU instruction set

Thirty-five instructions: fourteen basic ones, four **Preset E** instructions for building constants, and sixteen **Preset F**
instructions for graphics and control. The word each form assembles to is in [encodings.md](encodings.md). `RX` and `RY` are
registers, `R62` is the jump target register, `DP` is `R63`. Registers are 16-bit and arithmetic wraps at 16 bits.

## Cycle costs

The effect of an instruction happens on the cycle it issues; the PPU is then busy for the rest of its cost.

| Instruction | Cycles |
|-------------|--------|
| a basic datapath instruction (`NOP`, `CLR`, `INC`, `ADD`, `CMP`, ...) | 1 |
| `MUL` | 4 |
| `INTDIV` | 16 |
| a conditional jump | 1 not taken, **3 taken** |
| `JMR` | 3 |
| Preset E instruction | 2 |
| Preset F instruction (default) | 2 |
| `MOV`, `MOVDP` | 3 |
| `CALL` | 4 |
| `PALETTE16` | 2 + 16 = 18 |
| `PALETTE256` | 2 + 256 = 258 |
| `CLRTILE`, `CLRPALETTE` | 8 |
| `TILEDRAW` | 8 to issue; the blit runs on in the background ([graphics.md](graphics.md)) |
| interrupt entry | 4 |

A 16-bit constant is four instructions at 2 cycles (two `TARREG`, two `SETBYTE`): 8 cycles. Keep a constant you use often in a
register.

## Basic instructions

| Instruction | Does |
|-------------|------|
| `DEFCALL RX, RY` | call table entry `RX & $FF` = `RY` (an entry-point address for `CALL`) |
| `MOVXP RY` | `RY` = the address of the instruction **after the next one** (the execution pointer plus 4). Used to build return addresses. |
| `NOP` | nothing |
| `SWAPREG RX, RY` | exchange the two registers |
| `CLR RY` | `RY` = 0 |
| `CMP RX, RY` | `zero` = (`RX == RY`), `greater` = (`RX > RY`, unsigned). Nothing else sets the flags. |
| `CLRF` | clear both flags |
| `JMZ` `JNZ` `JMG` `JNG` | jump to the address in `R62` if the condition holds (see [architecture.md](architecture.md)) |
| `INC RY`, `DEC RY` | add or subtract 1 |
| `ADD RX, RY` | `RX += RY` |
| `SUB RX, RY` | `RX -= RY` |
| `MUL RX, RY` | `RX` = the low 16 bits of `RX * RY` |
| `INTDIV RX, RY` | `RX /= RY`, unsigned. **Dividing by zero leaves `RX` alone.** |

The unconditional jump is `JMR` (Preset F). All the jumps take their target from `R62`, so a jump to a label needs `R62`
loaded first: that is what the assembler shorthand `JMR label` does ([assembler.md](assembler.md)).

## Preset E: building constants

The Preset E instructions write one byte of a register at a time. Four **target registers** (0-3) each remember *which
register* and *which byte of it* to write.

| Instruction | Does |
|-------------|------|
| `TARREG T, LSB, RX` or `TARREG T, MSB, RX` | target `T` now means the low or high byte of `RX` |
| `SETBYTE T, imm` | write the 8-bit `imm` into the byte target `T` designates |
| `BUILD T1, T2, RX` | `RX` = (byte `T1` designates) << 8 \| (byte `T2` designates) |
| `CPREG RX, RY` | `RY` = `RX`. The ids are 4 bits, extended to 6 by the register banks. |

To load `$0100` into `R7`:

```ppuasm
    TARREG 2, LSB, R7
    SETBYTE 2, 0x00
    TARREG 3, MSB, R7
    SETBYTE 3, 0x01        ; R7 = $0100
```

Use targets 2 and 3 for your own work. Targets 0 and 1 are overwritten by every assembler shorthand that jumps.

## Preset F: graphics and control

| Instruction | Does |
|-------------|------|
| `SETPOS RX, RY` | write `RX` and `RY` to the tile position cells `$0200` (X) and `$0202` (Y), the same cells a plain store reaches. The 4-bit ids are extended by the register banks. |
| `SETTILE RX, mode` | current tile ID = `RX & $FF`; tile mode = `mode` (0-3) |
| `SETTILEBANK RX` | current tile bank = `RX mod 4` |
| `SETDP RX` | `DP` = `RX` |
| `MOVDP RX` | store `RX` as a little-endian word at the address in `DP` (so a store to an I/O port triggers it). Nothing happens if `DP` is `$FFFF`. |
| `MOV RX` | load the word at the address in `DP` into `RX` |
| `SETREGBANK a, b` | register bank X = `a`, Y = `b` (each 0-3): the 4-bit ids in `SETPOS` and `CPREG` become `bank * 16 + id` |
| `SETRENDMOD n` | `n = 0` selects the 16-bit render mode, `n = 1` the 32-bit one |
| `PALETTE16` | load 16 palette entries from the address in `DP` |
| `PALETTE256` | load the 256-entry palette from the address in `DP` |
| `CLRTILE` | clear every tile in all four banks |
| `CLRPALETTE` | reset both palettes to grayscale |
| `TILEDRAW` | draw the current tile at the current position ([graphics.md](graphics.md)); waits if all eight blit channels are busy |
| `JMR` | jump to the address in `R62`, always |
| `CALL n` | push the return address and jump to the address `DEFCALL` stored in slot `n` (an 8-bit immediate) |
| `GBLS RX` | `RX` = 0 in V-blank, 1 in H-blank (and not V-blank), 65535 in the visible area |

`CALL` pushes the same way an interrupt does (`SP += 2`, word stored at `SP`), so it returns with the `RET` shorthand.

### Which width is the palette?

`PALETTE16` always reads 16 words. `PALETTE256` reads 256 entries whose size depends on the render mode: in 16-bit mode,
256 words in the same `BBBBBGGGGGRRRRRA` format as `PALETTE16`'s (512 bytes); in 32-bit mode, 256 four-byte `RRGGBBAA`
entries (1,024 bytes). Load it after setting the render mode.

### Rarely noticed

- `TILEDRAW` re-executes itself every cycle while all eight channels are busy, so a burst of draws throttles the program
  instead of dropping tiles.
- `MOVXP` gives the address **two instructions ahead**, because the pointer has already moved past the `MOVXP`.
- Nothing but `CMP` sets a flag, so `INC`/`DEC`/`ADD` loops must `CMP` to branch.
- A basic instruction that names a register field uses six bits (R0-R63); the Preset F ones that name two registers use four
  each, reaching R0-R15 unless a register bank is set.

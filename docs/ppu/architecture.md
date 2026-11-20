# PPU architecture

The PPU is a small graphics processor that runs its own program ("microcode") out of its own 64 KiB of memory, in step with
the display beam. It draws into a rolling framebuffer; the display reads the result out as it goes. The CPU loads a program
into the PPU, starts it, and can poke its memory at any time.

| | |
|---|---|
| Clock | 68.011355 MHz (the master clock, master / 1) |
| Instruction | one 16-bit word, **big-endian**: 4-bit opcode, 12 operand bits |
| Speed | 1 cycle for a simple instruction; some cost more ([instruction-set.md](instruction-set.md)) |
| Registers | 64 sixteen-bit registers, R0-R63 |
| Memory | 64 KiB, code, data and I/O ports together |
| Display | 256 x 256, 16-bit (RGBA5551) or 32-bit (RGBA8888) |
| Tiles | 8 x 8 pixels, 256 per bank, 4 banks (1,024 tiles) |

## Registers

| Register | Role |
|----------|------|
| `R0`-`R58` | general purpose |
| `R59` | V-blank interrupt handler address (0 = none) |
| `R60` | H-blank interrupt handler address (0 = none) |
| `R61` `SP` | stack pointer. Reset to `$8000`. The stack grows **up** and `SP` points at the top word. |
| `R62` `PC` | the **jump target register**: every jump instruction goes to the address in `R62`. It is not the execution pointer. |
| `R63` `DP` | data pointer for `MOVDP`, `MOV` and the palette loads |

There is also a hidden execution pointer (the real program counter, which `MOVXP` and `CALL` read), four **target
registers** used by the Preset E instructions to assemble 16-bit constants one byte at a time, two **register banks** that
extend the 4-bit register fields of `SETPOS` and `CPREG`, and a 256-entry call table filled by `DEFCALL`.

### Flags

Two flags, `zero` and `greater`. Only `CMP` sets them (`CLRF` clears them). The four conditional jumps read them:

| Jump | Taken if |
|------|----------|
| `JMZ` | `zero` |
| `JNZ` | not `zero` |
| `JMG` | `greater` (left operand > right, unsigned) |
| `JNG` (`JML`) | not `greater` (left <= right) |

## Memory map

The CPU sees all of this through its PPU window, bank `$B0` ([../cpu/memory-map.md](../cpu/memory-map.md)).

| Range | What |
|-------|------|
| `$0000-$00DF` | plain memory (the program starts at `$0000`) |
| `$00E0-$00E1` | **current scanline**, 16-bit, read-only |
| `$00F0-$00FF` | **VOC** registers: render mode, palette address, framebuffer bank order, tile attributes and blend ([graphics.md](graphics.md)) |
| `$0100-$010B` | pixel port: X, Y, palette index; a write to `$010A` draws the pixel |
| `$0200-$0203` | tile position (X, Y words) read by `TILEDRAW` |
| `$0204-$0207` | row-clip window `[$0204, $0206)`, used when the clip attribute is set |
| `$0300-$033F` | tile definition window: 64 bytes, keyed on the current tile bank and ID |
| `$0340-$034A` | line-blit port |
| `$0350-$0366` | tile-row queue |
| `$C000-$DFFF` | queue backing store; open bus in write-ahead mode |
| `$E000-$FFFF` | the rolling framebuffer (8 KiB); open bus in write-ahead mode |

Everything else is ordinary RAM. In write-ahead mode (`$0362` bit 0) `$C000-$FFFF` reads `$FF` and ignores writes, so keep
code and data below `$C000`.

## Starting and stopping

After a reset the PPU is **waiting for start**, with all registers 0 except `SP`, an empty memory, and grayscale palettes.
The CPU writes `PPUCTRL` (`$D80000`): bit 0 starts execution at address `$0000`, bit 1 resets. A reset from inside the PPU is
the VOC reset bit (`$00F0` bit 0), which resets the registers, tiles and blitters but **keeps memory**. Setting bit 2 of
`$00FD` halts it (in 16-bit render mode only).

The Boot ROM provides `COP #$F7` to load a PPU program into the PPU; see [../cpu/interrupts.md](../cpu/interrupts.md).

PPU programs are self-modifying-safe: writing to PPU memory discards the cached decode of any instruction it overwrote.

## Interrupts

The PPU has its own interrupts, unrelated to the CPU's. There are three, in priority order:

1. **CPU-raised**: the host can force a jump to a vector (used by the Boot ROM to tell a running program that its code is
   about to change). One-shot.
2. **V-blank**: enabled by VOC `$00F0` bit 4, handler address in `R59`.
3. **H-blank**: enabled by VOC `$00F0` bit 5, handler address in `R60`.

A blank interrupt fires on the **rising edge** of the blank line, once, and only if its enable bit is set **and** its vector
register is non-zero. It is serviced at the next instruction boundary: the PPU pushes the return address (`SP += 2`, then the
word is stored at `SP`) and jumps to the handler, costing 4 cycles. The handler ends with `RET`.

There is no mask and no nesting limit: an interrupt can arrive between any two instructions, including inside another handler.
Handlers share `DP`, the target registers and the flags with the main program, so a handler must save and restore anything it
uses.

**The jump-register trap.** Every jump goes through `R62`, and `RET` ends by loading the return address into it. So an interrupt
leaves `R62` holding the address it interrupted. A main loop built from the `JMR label` shorthand (four instructions to load
`R62`, then the jump) is wedged for good if an interrupt lands after the load and before the jump: the handler's `RET` leaves
`R62` pointing at the `JMR` itself, and the `JMR` jumps to itself forever. Measured on the emulator, **28 of 77** arrival cycles
did this to a one-label idle loop.

The safe way to wait for interrupts is a bare `JMR` that spins on itself, with `R62` loaded once before it:

```ppuasm
    TARREG 2, LSB, R62
    SETBYTE 2, idle
    TARREG 3, MSB, R62
    SETBYTE 3, idle        ; R62 = idle, loaded with targets 2 and 3
idle:
    JMR                    ; R62 already holds this address, and so does it after any handler's RET
isr:
    INC R8
    RET
```

Here a handler's `RET` returns to `idle` and sets `R62` to `idle`, which is what the loop needs. This held for every arrival
cycle tested (80 offsets, five interrupts each, `SP` balanced). Put real work in the handlers, or poll in a loop that
reloads `R62` immediately before every jump and can tolerate being restarted. Keep handlers short: the H-blank window is 84
cycles inside a 340-cycle line.

## Timing

One display line is 340 pixel clocks, which is 340 PPU cycles: 256 visible, then 84 of H-blank. A frame is 261 lines (256
visible, 5 of V-blank): 88,740 cycles, 1.3 ms. The PPU can find out where the beam is with `GBLS` (0 in V-blank, 1 in H-blank,
65535 in the visible area), or read the scanline at `$00E0`.

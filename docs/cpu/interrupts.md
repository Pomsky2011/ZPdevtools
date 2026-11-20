# Interrupts

The CPU checks for pending interrupts before every instruction. In priority order:

| Interrupt | Vector | Masked by `I`? | Raised by |
|-----------|--------|----------------|-----------|
| ABORT | `$00:FFF8` | no | nothing yet: reserved |
| NMI | `$00:FFFA` | no | nothing yet: reserved |
| IRQ | `$00:FFFE` | yes | timers and DMA completion, through the interrupt controller |
| `BRK #sig` | `$00:FFE6` | no (it is an instruction) | your code |
| `COP #sig` | `$00:FFE4` | no | your code, except `$F0-$FF` (Boot ROM calls, below) |

Each vector is a 16-bit address in bank `$00` of the cartridge image, and the handler runs in bank `$00`. See
[memory-map.md](memory-map.md). In revision 10 no register or device raises NMI or ABORT; only the host can. Treat their
vectors as reserved.

## What happens on an interrupt

1. `PB`, then `PC` (high byte, low byte), then `P` are pushed: four bytes on the stack in bank `$80`.
2. For IRQ, `BRK` and `COP`, `I` is set. NMI and ABORT leave it alone.
3. The decimal flag `D` is cleared (binary arithmetic).
4. `PB` becomes `$00` and `PC` is loaded from the vector.

Seven cycles pass. The stack afterwards, from the new `SP`:

| Address | Contents |
|---------|----------|
| `SP+1` | `P` |
| `SP+2` | `PC` low |
| `SP+3` | `PC` high |
| `SP+4` | `PB` |

`RTI` pulls them back in that order, restoring `P` (including `M` and `X`), `PC` and `PB`. The pushed `PC` is the address
of the next instruction; for `BRK` and `COP` that is the address after the signature byte, so the signature is at
`PC - 1`.

**`DB` and `D` are not saved.** A handler runs with whatever `DB` and `D` the interrupted code had, and there is no
`PLB`, so it cannot restore a `DB` it changed. Either keep `DB` the same value for your whole program and end
handlers with `SDB #that_value`, or avoid changing it in handlers and use long addressing instead. The direct-page
register `D` is not saved either.

## IRQ

There is one maskable line, shared by everything. An **interrupt controller** in front of it tells the handler which
source fired.

- A source raising its request always sets its bit in `IRQ_STATUS` (`$D80058`).
- If that source's bit is also set in `IRQ_ENABLE` (`$D80059`, all ones after reset) the CPU's IRQ latch is set.
- The latch is held until it is taken. An IRQ that arrives while `I` is 1 is not lost: it is taken as soon as `CLI`
  clears `I`. One request produces one interrupt.
- The handler reads `IRQ_STATUS`, does the work, and writes 1s back to acknowledge. Several sources that fire while
  interrupts are masked all show up in one `IRQ_STATUS` read.

| `IRQ_STATUS` bit | Source |
|------------------|--------|
| `$01` | display V-blank (see below) |
| `$02` | display H-blank (see below) |
| `$04` | a timer expired; read `TIMER_STATUS` for which |
| `$08` | a DMA transfer completed |

The two display sources exist in the controller, but in revision 10 nothing a program can write turns them on, so they
never fire. For something that runs once per frame or per line, use the **V** or **H timer**, or poll `DISP_STATUS`.
Note that the timers run on their own clocks, not the display's; see [memory-map.md](memory-map.md).

**DMA raises an IRQ every time a transfer finishes.** With `I` clear, the first finished transfer will take the IRQ
vector. If you use DMA without a handler, keep `I` set or mask the source by clearing bit 3 of `IRQ_ENABLE`.

### Timers

Set a timer's bit in `TIMER_INT_ENABLE` (`$D80052`) and `TIMER_CONTROL` (`$D80050`). When it expires its bit in
`TIMER_STATUS` (`$D80051`) is set and the timer source is raised. Clear the flag by **writing 1** to its bit. When every
`TIMER_STATUS` bit is clear, the controller's timer bit clears with it, so a handler that only handles timers need not
touch `IRQ_STATUS`.

This program counts 1/1024-second ticks while the CPU sleeps in `WAI`:

```cpuasm
.org $8000
start:  SEI
        SEP #$30            ; 8-bit A and index
        SDB #$80            ; data bank = work RAM
        STZ $0100           ; tick counter at $80:0100
        LDA #$04            ; timer T is bit 2
        STA $D80052         ; TIMER_INT_ENABLE
        STA $D80050         ; TIMER_CONTROL: run it
        CLI                 ; let the IRQ through
idle:   WAI
        BRA idle            ; not JMP: DB is $80 but PB is $00

.org $9000
isr:    PHA
        LDA #$04
        STA $D80051         ; TIMER_STATUS: write 1 to clear the T flag
        INC $0100           ; count it
        PLA
        RTI

.org $FFFE
.word $9000                 ; IRQ vector: the handler's address, written out
```

Run on the emulator, the counter reads 10 after 700,000 master cycles and 51 after 3,400,000. Two details worth copying:
the handler's address is a literal in the vector (`cpuasm` silently emits nothing for `.word label`, so put handlers at
fixed addresses), and the idle loop uses `BRA`, because with `DB = $80` a `JMP idle` would jump into bank `$80`.

## WAI

`WAI` stops the CPU until an interrupt is asserted, whatever `I` says. If `I` is clear the handler then runs, and `RTI`
returns to the instruction after the `WAI`. If `I` is set, execution simply continues after the `WAI` and the request
stays latched until `I` is cleared; this is how you sleep until an event without writing a handler.

While waiting, the cycle counter does not advance for the CPU (`WAI` itself costs 3), which is why a program that mostly
sleeps shows a tiny cycle count.

## Boot ROM calls

`COP #$F0` to `COP #$FF` do not go through `$00:FFE4`. They call services in the Boot ROM, which the emulator provides
natively when no real Boot ROM is loaded. The CPU is back at the next instruction, with `P`, `PC`, `PB`, `DB`, `D` and
`SP` as before, when the service returns.

| Call | Does |
|------|------|
| `COP #$F0`-`$F6` | `SIN`, `COS`, `SINCOS`, `ATAN2`, `ATAN`, `SQRT`, `HYPOT`: 16-bit binary angles, Q1.15 |
| `COP #$F7` | load a PPU program |
| `COP #$F8`-`$FE` | reserved |
| `COP #$FF` | verify a data chunk of a v3 signed cartridge (chunk index in `X`, 16-bit index mode) |

Use the wrappers in `include/zeropoint/zp_cop.h` and `lib/zp_cop.asm` instead of writing these by hand. `COP` with any
other signature, and `BRK`, are yours.

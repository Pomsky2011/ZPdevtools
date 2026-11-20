# APU architecture

The APU is an 8-bit processor with a hardware 16-channel mixer (the **MMP**) attached. The main CPU loads it with a
program and sample data; it then runs on its own, mixing audio and talking back through shared memory.

| | |
|---|---|
| Clock | 4.250710 MHz (master / 16) |
| Fast mode | 8.5 MHz (master / 8), opt-in, with bus contention |
| Instruction | one 16-bit word, **2 cycles**: 2.125 million per second (4.25 million in fast mode) |
| Registers | two 8-bit registers `X` and `Y`; special registers `PC` `SP` `RP` `DP` `DB` and four flags |
| Memory | 64 KiB address space, byte-wide, plus a banked 448 KiB audio ROM |
| Mixer | 16 sample channels with pitch, volume and pan, plus a stereo stream port; 48 kHz output |

## Memory map

The CPU sees all of this through its APU window, bank `$A0` ([../cpu/memory-map.md](../cpu/memory-map.md)).

| Range | Size | What |
|-------|------|------|
| `$0000-$00FF` | 256 B | MMP registers ([mmp.md](mmp.md)) |
| `$0100-$0FFF` | 3.75 KiB | General RAM: variables, stack, program code |
| `$1000-$17FF` | 2 KiB | CPU to APU mailbox (plain RAM) |
| `$1800-$1FFF` | 2 KiB | APU to CPU mailbox (plain RAM) |
| `$2000-$2FFF` | 4 KiB | ExIO: plain RAM; the APU has no I/O devices behind it |
| `$3000-$6FFF` | 16 KiB | ARAM: RAM, where the BIOS expects a program or score header |
| `$7000-$7FFF` | 4 KiB | STL, the sample table ([sst.md](sst.md)) |
| `$8000-$8FFF` | 4 KiB | BIOS: read-only. The APU starts executing at `$8000`. |
| `$9000-$CFFF` | 16 KiB | SST, sample RAM ([sst.md](sst.md)) |
| `$D000-$FFFF` | 12 KiB | AROM window onto the banked audio ROM |

Writes to the BIOS region and the AROM window are ignored. Both mailboxes are ordinary RAM: there are no ready flags and
no interrupts in either direction, so each side polls a byte. The convention the system software uses is `$1000` for the
CPU to say something and `$1800` for the APU to answer.

### AROM banks

The audio ROM is 448 KiB in 18 banks of 24 KiB. Only the first 12 KiB of the selected bank is visible at `$D000`. The CPU
selects the bank by writing `APU_ROMBANK` (`$D80016`); the APU has no instruction for it. `reset()` zero-fills the AROM
along with everything else, so a program cannot count on its contents surviving an APU reset.

## Registers

| Register | Width | Reset | Role |
|----------|-------|-------|------|
| `X`, `Y` | 8 | 0 | The only general registers. `X` is also the low byte, `Y` the high byte, of `JMX`/`JSX` and `BSP`. |
| `PC` | 16 | `$8000` | Program counter |
| `SP` | 16 | `$0000` | Stack pointer. The stack grows **up**. A program that uses it must `BSP` it into RAM first. |
| `RP` | 8 | `$80` | "ROM page": the high byte of the targets of `JMS`, `JSR`, `JZ`, `BEQ`, `BNE`, `BLT`, `BGT` |
| `DP` | 8 | 0 | Data page: the high byte of memory operands |
| `DB` | 8 | 0 | Data byte: the low byte of the "data pointer" `DP:DB` used by `LDA`, `STA`, `JDP` |
| flags | 4 | 0 | `zglc`: zero, greater, less, carry |

There are exactly two registers. Older documents mention R2-R127: those are write-only debug slots that nothing can read
back. See [instruction-set.md](instruction-set.md).

## Time

The APU has no timer. A program measures time by counting cycles: `NOP n` stalls for `2 + n` cycles, and a hardware loop
of stalls makes a delay. Every other instruction costs exactly 2 cycles, with no wait states anywhere.

| Interval | APU cycles | Instructions |
|----------|-----------|--------------|
| One 48 kHz audio sample | 88.6 | 44.3 |
| One display frame (1.3 ms, see the [CPU timing page](../cpu/timing.md)) | 5,546 | 2,773 |
| 1/120 s (the sequencer's tick) | 35,422.6 | 17,711 |
| 1/60 s | 70,845 | 35,422 |

In fast mode every figure doubles.

### Fast mode

The CPU switches fast mode on with bit 0 of `APUCLK` (`$D8001A`); a reset drops back to normal. It doubles the APU's speed
at a price: while it is on, each CPU access to the APU window costs one extra CPU cycle, and DMA to or from the window runs
at half rate. Upload first, switch to fast mode last. The cycle counter counts cycles of whichever clock is running, and
`NOP` stalls are in those cycles, so a stall is half as long in fast mode.

## Starting and stopping from the CPU

The CPU controls the APU through `$D80010-$D8001F` ([../cpu/memory-map.md](../cpu/memory-map.md)):

| Write to `APUCTRL` (`$D80010`) | Effect |
|--------------------------------|--------|
| bit 0 | reset: clear all APU memory and the AROM, restart at `$8000` |
| bit 1 | halt |
| bit 2 | run (resume) |

`APUPC` (`$D80012-13`) and `APUSP` (`$D80014-15`) can be written when the APU is halted.

Because a reset clears everything, and the APU arrives in whatever state its BIOS left it (an engine can leave flags, a stack
pointer and playing channels behind), a cartridge that runs its own APU program should do this, in order:

1. `APUCTRL = $03`: reset **and halt**. A plain reset would let the BIOS run again.
2. Upload the program and data through bank `$A0` (DMA is fine).
3. Write the entry address to `APUPC`.
4. `APUCTRL = $04`: run.
5. If you want fast mode, set `APUCLK` now.

Clear both mailboxes before step 4: the Boot ROM leaves a result byte in `$1000`.

### The BIOS and program headers

With nothing else loaded, the APU's BIOS waits for a header at `$3000`. A system without a BIOS image uses a built-in
native one (the "HLE BIOS") that behaves the same way:

| `$3000` | Meaning |
|---------|---------|
| `$0A` | an APU program: jump to the entry point at `$3004-05` |
| `$0B` | a ZPA score: play it with the BIOS sequencer ([zpa.md](zpa.md)) |

with an 8-bit wrapping checksum of `$3020-$5FFF` at `$301F`. The BIOS zeroes the magic byte as soon as it accepts a header,
so a program that returns to it does not run again. A program started this way begins with `RP=$80`, `DP=$30`, `SP=$0100`
and `X`/`Y` holding its entry address.

The Boot ROM uses this path for the boot jingle: it uploads a ZPA score before the splash screen and the sequencer plays it
until the signature check finishes.

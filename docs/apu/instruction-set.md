# APU instruction set

The APU has two 8-bit registers, `X` and `Y`, and a handful of special registers ([architecture.md](architecture.md)). Every
instruction is one 16-bit word, big-endian: a 5-bit opcode and 11 operand bits. Every instruction takes **2 cycles**, so
about 2.1 million run per second (4.25 million in fast mode). `NOP` is the only exception: it adds its operand to that.

The exact word each form assembles to is in [encodings.md](encodings.md), generated from `apuasm`. This page is what the
instructions do. Where an instruction names a register it is `X` or `Y`; "memory byte `$DPzz`" means the byte at
`(DP << 8) | zz`.

## Flags

The flag register is four bits, `zglc`: `Z` (8) zero, `G` (4) greater, `L` (2) less, `C` (1) carry. Nothing
branches on the flags: the branches compare registers themselves, and `STF` copies the flags into a register. Which
instructions touch them:

| Instruction | Flags afterwards |
|-------------|------------------|
| `NOR` `AND` `XOR` `CRB` | `Z` if the result is 0; `G`, `L` and `C` cleared |
| `ADD` `ADC` | `Z` if the result is 0, `C` on carry out |
| `SUB` `SBC` | `Z` if 0; `L` and `C` if the first operand was below the second; `G` if above |
| `CME` `CMN` `CMG` `CML` | `Z`, `G` or `L` from comparing the two registers |
| `SFR` `CF` `SF` | set, clear-by-mask, set-by-mask |
| everything else | unchanged |

## Registers and constants

| Instruction | Does |
|-------------|------|
| `SCR X, n` / `LDA X, n` | register = the constant `n` (0-255) |
| `ZOR X` | register = 0 |
| `MOV X, Y` / `MOV Y, X` | copy one register to the other |
| `EXC` | swap `X` and `Y` |
| `INC` / `DEC` `X` `Y` `DP` `SP` | add or subtract 1. `X`, `Y` and `DP` wrap at 8 bits; `SP` is 16 bits. No flags. |
| `SRP $xx` | `RP = xx` (the page that `JMS`, `JSR`, `JZ`, `BEQ` and friends jump into) |
| `SDP $xx` | `DP = xx` (the data page for memory operands) |
| `SDP 0, $xx` / `SDB $xx` | `DB = xx` (the low byte of the data pointer) |
| `SDP 1, $xx` | `DP = xx`, the same as `SDP $xx` |

`SDP` and `SRP` take only an immediate, so a page cannot be computed. To make a memory access whose address depends on a
register, use `STRX`/`STAX`; for a computed jump use `JMX`/`JSX`.

## Memory

All memory is bytes; a 16-bit value is two byte accesses, low byte at the lower address. The `BF` ("byte flip") flag in
older documents is dead state: nothing can set it, so every operation uses the low byte.

| Instruction | Does |
|-------------|------|
| `STR $yy, X` | `X` = memory byte `$DPyy` |
| `STA X, $yy` | memory byte `$DPyy` = `X`, written as a word (see below) |
| `LDA X` | `X` = the byte at `(DP << 8) | DB` |
| `STA X` | the byte at `(DP << 8) | DB` = `X` |
| `STA $xx` | the byte at `(DP << 8) | DB` = the constant `xx` |
| `ZOA $yy` | the word at `$DPyy` = 0 |
| `ZOA DP` | the byte at `(DP << 8) | DB` = 0 |
| `STRX off, data` | `data` = the byte at `(DP << 8) | off` where `off` is a register (X or Y) |
| `STAX data, off` | the byte at `(DP << 8) | off` = `data` |

**Word writes.** `STA X, $yy` and the memory-destination forms of `NOR`, `AND`, `ADD` and `SUB` write a *word*: your byte
at `$DPyy`, and the old contents of `$DPyy+1` written back to the next address. That second write does nothing to
ordinary RAM but is a real write to a hardware register. Storing to the low byte of an MMP pitch register this way
re-writes the high byte and so applies the pitch; storing to an odd address overwrites the *next* register with its own
old value. `STA X`, `STA $xx`, `STRX`/`STAX` and the `ADC`, `SBC`, `XOR`, `CRB` memory forms write a single byte.

`STRX`/`STAX` are how a channel number becomes an MMP register offset: with `DP = $00` they address `$0000 + register`.

## Arithmetic and logic

`NOR`, `AND`, `ADD`, `SUB`, `ADC`, `SBC`, `XOR` and `CRB` all take two source registers and a destination:

```text
OP  X, Y, Z        destination is a register (X or Y)
OP  X, Y, $zz      destination is the memory byte $DPzz
```

`NOR` is `~(a | b)`; `AND`, `XOR` are bitwise; `ADD` and `SUB` are unsigned 8-bit. `ADC` adds the carry flag in and `SBC`
subtracts it, for multi-byte arithmetic. `SUB X, Y, Z` computes `X - Y`.

`NOR`, `AND`, `ADD`, `SUB`, `ADC` and `SBC` have a 7-bit destination field. Only 0 (`X`) and 1 (`Y`) are registers.
Values up to `R127` are accepted by `apuasm` with a loud warning and write a debug scratch slot that nothing can read
back; do not use them. `XOR` and `CRB` have a one-bit destination.

`CRB a, b, Z` is a conditional shift: the low nibble of `b` is a signed shift count (-8 to 7). A positive count shifts `a`
right, a negative one shifts it left.

## Jumps and calls

Targets are encoded as an 8-bit offset, so a jump reaches only the 256-byte page held in `RP` (or the BIOS page `$80xx`).
`apuasm` tracks `RP` and refuses a jump whose label is in another page ([programming-guide.md](programming-guide.md)).

| Instruction | Does |
|-------------|------|
| `JMS $zz` | jump to `RP:zz`; no stack use |
| `JSR $zz` | the same, pushing the return address (a call) |
| `JDP` | jump to `DP:DB`; `JDPS` pushes the return address first |
| `JMX` | jump to `Y:X`; `JSX` pushes the return address first |
| `RET` | pull a 16-bit address into `PC` |
| `JZ r, $zz` | jump to `RP:zz` if the register is 0; `JZ r, 0, $zz` goes to `$80zz`. No push. |
| `JNZ r, $zz` | if the register is **not** 0: **push a return address and** jump to `$80zz`. Only the BIOS page. |
| `JMP label` | push a return address, then jump **forward**. The only backward forms are absolute ones. |
| `JMP 0, 1, $zz` / `JMP 0, 0, $zz` | push, then jump to `RP:zz` / `$80zz` |

`JNZ` and `JMP` always push, so they are calls. They are not loop branches; for a loop use `JZ` or `JMS`, the compare
branches, or a hardware loop.

## Compare and branch

| Instruction | Branches to `RP:zz` if |
|-------------|------------------------|
| `BEQ a, b, $zz` | `a == b` |
| `BNE a, b, $zz` | `a != b` |
| `BLT a, b, $zz` | `a < b` (unsigned) |
| `BGT a, b, $zz` | `a > b` |

No stack, no flags. The four `CM` instructions branch **relative** to the next instruction, by up to 63 words, and also
set `Z`/`G`/`L`:

| Instruction | Branches if | Direction |
|-------------|-------------|-----------|
| `CME a, b, d, $n` | `a == b` | forward |
| `CMG a, b, d, $n` | `a > b` | forward |
| `CMN a, b, d, $n` | `a != b` | **backward** |
| `CML a, b, d, $n` | `a < b` | **backward** |

The direction is part of the instruction's opcode, not the `d` operand. To skip forward when `X != 0`, compare against a
zero register with `CMG X, Y` (`Y = 0`).

## Hardware loops

`LST id, n` starts loop `id` (0-15) with a count of `n` (0-63) and remembers the address after itself. `LFN id` ends the
body: it decrements that loop's counter and jumps back while it is still above zero. The body runs `n` times (`n = 0`
runs it once). There are 16 loops, so unlike the CPU's they can nest, if you give each a different id. `BRT n` leaves
the active loop with the highest id, to `n` words past that loop's `LFN` (which must have run once). `BRP` does nothing.

## Stack

The stack grows **up**: a push writes at `SP` and then increments it. `SP` is 0 after reset, so the first thing a
program that uses the stack does is point it at RAM with `BSP` (`SP = Y:X`).

| Instruction | Does |
|-------------|------|
| `BSP` | `SP = (Y << 8) | X` |
| `PUX` `PUY` | push the register |
| `POX` `POY` | pull into the register |
| `PUDP` | push `DP` then `DB` |
| `PODP` | pull `DB` then `DP` |

## Flag instructions

| Instruction | Does |
|-------------|------|
| `SFR $m` | flags = `m` |
| `CF $m` | clear the flags in mask `m` |
| `SF $m` | set the flags in mask `m` |
| `STF r` | the high nibble of register `r` = the flags (its low nibble is kept) |

## System

| Instruction | Does |
|-------------|------|
| `NOP n` | do nothing for `2 + n` cycles (n up to `$7FF`); the APU has no timer, so this is how time is counted |
| `HLT` | stop the APU. The CPU can restart it. Two opcode values halt; `HLT` assembles to one of them. |

Opcodes `$1C` and `$1F` are unused and halt the APU if executed. Some unused sub-operations inside a used opcode (the
fourth form of `STA`, for instance) do nothing.

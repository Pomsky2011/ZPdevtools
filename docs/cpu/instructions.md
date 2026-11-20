# Instructions

What each group does, and where it differs from what you'd expect. Bytes, cycles, flags and exact opcodes are in
[opcode-table.md](opcode-table.md). Widths follow M and X as described in [architecture.md](architecture.md).

## Loads and stores

`LDA`, `LDX`, `LDY` load and set `N` and `Z`. `STA`, `STX`, `STY` store and touch no flags. `STZ` stores zero, at the
width of `M`. `LDA` and `STA` have the most modes, `LDX`/`LDY`/`STX`/`STY` the fewest: check the matrix in the opcode
table before assuming a form exists.

## Register transfers

| | |
|---|---|
| `TAX` `TAY` | `A` into `X`/`Y`. With `X=1` only the low byte of `A` is copied. With `X=0` and `M=1`, `A`'s low byte is zero-extended. |
| `TXA` `TYA` | `X`/`Y` into `A`. With `M=1` only `A`'s low byte is written. With `M=0` and `X=1`, the byte is zero-extended. |
| `TXY` `TYX` | Copy the whole 16-bit register. |
| `TCD` `TCS` | `A` (always all 16 bits) into `D` or `SP`. No flags. |
| `TDC` `TSC` | `D` or `SP` into all 16 bits of `A`. Sets `N` and `Z`. |

All the others set `N` and `Z`. There is no `TXS`/`TSX`, `XBA`, or any way to read `DB` or `PB` into a register except
by pushing them (`PHB`, `PHK`) and pulling with `PLA`.

## Stack

`PHA`, `PHX`, `PHY` push one byte, or two when that register is 16-bit. `PHP`, `PHB` and `PHK` push `P`, `DB` and `PB`
(one byte each). `PHD` pushes `D` (two). `PEA addr` and `PUSH #word` push a 16-bit constant. `PEI (dp)` pushes the 16-bit
word at that direct-page location, and `PER label` pushes the address of `label` (PC-relative).

The only pulls are `PLA` (sets `N` and `Z`) and `POPF`. To get a pushed `X`, `Y`, `D` or `DB` back, pull it into `A` and
transfer it, or set `DB` with `SDB`.

## Arithmetic

`ADC` and `SBC` use the carry flag (clear it with `CLC` before an add, set it with `SEC` before a subtract) and set `N V
Z C`. With `D` set they work on packed BCD in both widths.

`INC` and `DEC` work on `A` (`INC A`, one byte), on memory (`addr`, `addr,X`, `dp`, `dp,X`) and, as `INX INY DEX DEY`, on
the index registers. They set `N` and `Z` but not `C`. `INC.L addr` and `DEC.L addr` are the long-looking forms of the
same absolute address: they take a 16-bit `DB`-relative operand (3 bytes, like plain `INC addr`) and cost 6 cycles
instead of 3.

### MUL

`MUL operand` multiplies `A` by the operand, unsigned, in 8 cycles whatever the form.

- `M=1`: the low byte of `A` times a byte; the 16-bit product goes into `A` (both bytes). `X` is not touched.
- `M=0`: `A` times a word; the 32-bit product goes into **`A` (low word) and `X` (high word)**.

`Z` means the product is zero, `N` is the top bit of the product, and `C` and `V` are both set when the product does not
fit in the operand width (that is, when the high half is non-zero).

```cpuasm
        REP #$30
        LDA #$1234
        MUL #$0100          ; A = $3400, X = $0012, C = V = 1
```

### DIV

Division is unsigned and takes 8 cycles. Two forms:

- `DIV X,Y` divides `X` by `Y`. The quotient goes into `A` and the remainder into `X`. The width of the divide follows the
  `X` flag; the width of the stores follow `M` (quotient) and `X` (remainder).
- `DIV long,X` and `DIV long,Y` divide `A` by the byte or word at `long + X` (or `+ Y`); the quotient goes into `A` and the
  remainder into `X`. These are the only `DIV` forms with a memory operand, and they are always 24-bit.

`N` and `Z` are **not** set. `C` is the error flag: it is set on a divide by zero (and `A` and `X` are left alone), and
cleared by a good divide.

```cpuasm
        REP #$30
        LDX #100
        LDY #7
        DIV X,Y             ; A = 14, X = 2, C = 0
```

## Logic

`AND`, `ORA` and `XOR` set `N` and `Z`. `EOR` is the same opcode as `XOR` in the assembler. `BIT` ANDs `A` with the
operand but throws the result away: `Z` says whether it was zero, and `N` and `V` come from the top two bits of the
**operand**. That includes `BIT #const`, which on the 65816 changes only `Z`.

## Shifts and rotates

`ASL`, `LSR`, `ROL` and `ROR` shift `A` or memory by one bit and set `N`, `Z` and `C`. `RCL A` is `ROL A` at 4 cycles
instead of 2.

`SHL X`, `SHL Y`, `SHR X` and `SHR Y` shift an index register left or right by one (`SHR` is logical). They set **only
`C`**; `N` and `Z` are not updated, so follow them with a compare if you need to test the result.

## Compare

`CMP`, `CPX` and `CPY` subtract without storing. `C` is set when the register is greater than or equal to the operand
(unsigned) and `N` and `Z` come from the difference. `CPX` and `CPY` exist only with an absolute address; to compare an
index register with a constant, use `CMP` after a transfer.

## Branches, jumps and calls

| Instruction | Does |
|-------------|------|
| `BEQ` `BMI` `BCS` | branch if `Z`, `N`, `C` is set. `BGE` is another spelling of `BCS`. Targets are 24-bit addresses. |
| `BVS` | branch if `V` is set. 16-bit target. |
| `BRA` | always; a 16-bit target in the current bank. |
| `BRL label` | always; a signed 16-bit offset from the next instruction. |
| `BNE` `BPL` `BCC` `BVC` | **assembler pseudo-instructions**: the opposite branch over a `BRA`, 6 or 7 bytes. They reach anywhere in the bank. |
| `JMP`, `JSR` | see [addressing-modes.md](addressing-modes.md) for the target of each form, and mind `JMP addr` |
| `RTS` | pull a 16-bit address, add 1, resume there (pairs with `JSR`) |
| `CALL long` | a far call that pushes `PB` and a 16-bit address, 16 cycles |
| `RTL` | pull a 16-bit address and a bank, add 1 to the address, resume there |
| `RTI` | pull `P`, `PC` and `PB` (pairs with interrupts, `BRK`, `COP`) |
| `RET` | pulls exactly what `RTI` does, in 8 cycles instead of 6 |

### CALL and RTL resume one byte late

`CALL` pushes the address of the *next* instruction, but `RTL` adds 1 to what it pulls, so after `CALL far` returns, the
CPU resumes **one byte past** the instruction that follows. The instruction right after a `CALL` is skipped (or, if it is
longer than a byte, execution lands in its middle). The fix is to follow every `CALL` with a one-byte padding
instruction:

```cpuasm
        CALL far_routine
        NOP                 ; RTL lands after this byte
        INC A               ; first instruction that runs on return
        HLT
far_routine:
        RTL
```

`CALL` and `RTL` are the long call pair, `JSR` and `RTS` the near pair. `RET` is not the partner of `CALL`: `CALL` pushes 3
bytes and `RET` pulls 4.

### Conditional branches

A taken branch costs 3 cycles, 4 if its target is in a different 256-byte page from the instruction after it; not taken
costs 2. `BRA` is 3 or 4 and `BRL` is always 4. The pseudo-branches are an inverse branch over a `BRA`, so one that is taken costs the 2 cycles of the inverse test
plus the `BRA`'s 3 or 4, and one that is not taken costs the inverse test's taken price, 3 or 4.

## Hardware loop: LOOP and LPEND

```cpuasm
        REP #$30
        LDX #0
        LOOP #5             ; counter = 5
        INX                 ; body
        LPEND               ; counter -= 1, back to the body if it is still above 0
        HLT                 ; X = 5
```

`LOOP #n` stores `n` in the loop counter and remembers the address after itself. `LPEND` decrements the counter and goes
back to that address while it is still above zero. The body therefore runs `n` times, except that **`LOOP #0` runs it
once**, the same as `LOOP #1`. `LOOP` costs 2 cycles and each `LPEND` 1, so a one-instruction body is very cheap.

There is a single counter. You cannot nest `LOOP`s, an interrupt handler that uses `LOOP` destroys the interrupted
code's count, and leaving a loop early (with a branch out of the body) leaves the counter set. Use a counter register for
anything else.

## Block move: MVN and MVP

`MVN srcbk,destbk` and `MVP srcbk,destbk` copy `A + 1` bytes (so `A = $FFFF` moves nothing) from `srcbk:X` to
`destbk:Y`, one byte per cycle, then leave `A = $FFFF` and set `DB = destbk`. They always use the full 16 bits of `X` and
`Y`, whatever the `X` flag says. **The direction is the reverse of the 65816's:**

| | Direction | `X` and `Y` should start at |
|---|-----------|------------------------------|
| `MVN` | descending | the *last* byte of the source and destination |
| `MVP` | ascending | the *first* byte |

So `MVN` is the safe choice when the destination overlaps the source from above, and `MVP` when it overlaps from below.
The source bank comes first in the operands (the 65816 machine code puts the destination first).

```cpuasm
        REP #$30
        LDA #3              ; four bytes
        LDX #$0103          ; last byte of the source
        LDY #$0203          ; last byte of the destination
        MVN $80,$80         ; copy $80:0100-0103 to $80:0200-0203
```

## Flags

`SEC` `CLC` set and clear carry, `SED` `CLD` decimal mode, `SEI` `CLI` the interrupt mask, and `CLV` clears overflow.
`SEP #mask` and `REP #mask` set and clear any bits of `P`: these two take a **single-byte** mask (2 bytes in all). The
`#const` forms of the other instructions take two.

## System

| Instruction | Does |
|-------------|------|
| `NOP` | 2 cycles |
| `BRK #sig` | push `PB`, `PC`, `P`; set `I`, clear `D`; jump through `$00:FFE6`. 2 bytes (the signature byte is skipped, not interpreted). 7 cycles. |
| `COP #sig` | the same through `$00:FFE4`, except that `$F0-$FF` are Boot ROM calls. `INT` and `SWI` assemble as `COP`. |
| `WAI` | stop executing until an interrupt is asserted; see [interrupts.md](interrupts.md) |
| `HLT` | stop forever; only a reset restarts the CPU. 0 cycles. (`STP` assembles as `HLT`.) |
| `SDB #bank` | set `DB`. The only way to change `DB`: there is no `PLB` and no transfer into it. |

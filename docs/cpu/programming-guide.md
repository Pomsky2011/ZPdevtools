# Programming guide

Practical patterns for writing DEF88186 assembly with `cpuasm`. Every block marked `cpuasm` is assembled by
`python3 scripts/check_doc_asm.py`, and the ones with a stated result were run on the emulator. The traps that these
patterns work around are explained in [architecture.md](architecture.md) and [addressing-modes.md](addressing-modes.md);
read those first if something here looks odd.

## Starting up

The cartridge's entry point is recorded in its ROM header (see `zplink` and `zpbuild`), not in a reset vector. Assume
nothing about the flags when you arrive:

```cpuasm
.org $8000
start:  SEI                 ; no interrupts until we are ready
        REP #$30            ; 16-bit A, X and Y
        CLD                 ; binary arithmetic
        CLC
        SDB #$80            ; data bank = work RAM, for the whole program
        ; SP is $01FF, D is $0000
        BRA main
main:   HLT
```

`lib/crt0_rom.asm` does the same and then jumps to a C `main`. Keep `DB` at `$80` for the whole program if you can; see
[Interrupts](#interrupts).

## Choosing widths

`REP #$30` and `SEP #$30` flip `A`, `X` and `Y` together. You can also flip them separately: `REP #$20` / `SEP #$20` for
`A` alone (mask bit 5, `M`), `REP #$10` / `SEP #$10` for the index registers (bit 4, `X`). A byte-sized store inside
16-bit code:

```cpuasm
        REP #$30
        LDA #$1234
        SEP #$20            ; A is a byte for the next few instructions
        STA $0100           ; stores $34 only
        REP #$20
```

The assembler does not track widths. `LDA #$12` assembles the same either way (three bytes, as always), and a width
mismatch shows up only as the wrong amount of data moving. Pushes and pulls are the commonest victims: `PHA` pushes 2
bytes in 16-bit mode, so pull in the same mode.

## Variables, tables and the data bank

Work RAM is `$80:0000` up. Reach it with absolute addressing while `DB = $80`:

```cpuasm
        SDB #$80
        REP #$30
        LDA #$0042
        STA $0100           ; $80:0100
        INC $0100
        LDA $0100
```

Three habits keep you out of trouble:

- **Never use direct page for variables.** `STA $10` writes bank `$00`, which is ROM, so the store vanishes. To force the
  absolute form for an address that fits in a byte, write `STA !$10`.
- **Read ROM tables with a long address.** `LDA table,X` uses `DB`; with `DB = $80` it reads RAM. `LDA >table,X` always
  reads the bank the label is in:

```cpuasm
        SEP #$30
        SDB #$80
        LDX #2
        LDA >squares,X      ; ROM read, whatever DB is; A = 4
        STA $0100
        HLT
squares: .byte 0, 1, 4, 9, 16, 25
```

- **Use long addresses for the odd hardware access.** `STA $D80050` is already a long address (the operand is above
  `$FFFF`), so I/O registers work without touching `DB`.

## Branches

`BEQ`, `BMI`, `BCS` and `BVS` are real; the other four are expanded by the assembler. A countdown loop:

```cpuasm
        SEP #$30
        LDX #10
again:  DEX
        BNE again           ; assembles as: BEQ skip / BRA again
        HLT
```

`BCS`/`BCC` compare unsigned values: after `CMP`, `BCS` branches if `A >= operand`. There are no signed-compare branches;
test `N` and `V` yourself.

## Subroutines

`JSR`/`RTS` call within a bank and cost 10 cycles together. Pass values in registers when you can. When you can't, push
them and reach them with stack-relative addressing. Remember `SP` points at the *free* byte, so with a 2-byte return
address on top, the argument pushed last is at `3,S`:

```cpuasm
        REP #$30
        LDA #30
        PHA                 ; second argument, pushed first
        LDA #12
        PHA                 ; first argument
        JSR add2            ; returns the sum in X
        TSC                 ; the caller removes the two arguments
        CLC
        ADC #4
        TCS
        HLT

add2:   LDA 3,S             ; SP+1,2 = return address, SP+3,4 = first argument
        CLC
        ADC 5,S             ; SP+5,6 = second argument
        TAX
        RTS
```

Run on the emulator this leaves `X = 42` and `SP` back at `$01FF`. There is no `PLX`, `PLY` or `PLB`, so the callee
cannot pop into an index register; that is why the arguments are removed through `TSC`/`TCS`.

A far call is `CALL`, with a mandatory padding byte after it because `RTL` returns one byte late:

```cpuasm
        CALL far_routine
        NOP
        HLT
far_routine:
        RTL
```

## Loops and copying

A fixed count with no register to spare is what `LOOP`/`LPEND` is for; see [instructions.md](instructions.md) for its one
limit (no nesting, and not in an interrupt handler).

`MVN` and `MVP` are the memory copy. There is no fill instruction, but an overlapped `MVP` is one: store the value in the
first byte, then copy the region onto itself shifted by one, which propagates the byte forward. `A` is the count of
*additional* bytes:

```cpuasm
        REP #$30
        SEP #$20
        LDA #$AA
        STA $800200         ; the first byte
        REP #$20
        LDA #7              ; 7 more bytes
        LDX #$0200          ; source = first byte
        LDY #$0201          ; destination = the next byte
        MVP $80,$80         ; $80:0200-0208 are all $AA
        HLT
```

Because `MVN` runs downward and `MVP` upward, pick the one that reads each byte before overwriting it for an ordinary
overlapping copy, and use the other to *replicate*, as above.

## Multiply and divide

The hardware does both in 8 cycles. For a 16 x 16 to 32-bit multiply, `MUL` leaves the low word in `A` and the high word
in `X`. For division, `DIV X,Y` divides `X` by `Y` into `A` (quotient) and `X` (remainder), and sets `C` if `Y` was zero.
Examples are in [instructions.md](instructions.md).

## Waiting for the display

Read `DISP_STATUS` (`$D80044`). Bit 0 is set during V-blank, bit 1 during H-blank:

```cpuasm
        SEP #$30
wait_vb: LDA $D80044        ; DISP_STATUS
        AND #$01            ; bit 0 = V-blank
        BEQ wait_vb         ; spin until it is set
        HLT
```

A V-blank is 425 CPU cycles long (see [timing.md](timing.md)), so poll in a tight loop and don't do anything slow between
checks. To run code without polling, use the timers in [interrupts.md](interrupts.md).

## Reading the controller

`$D80030` holds the eight direction bits, `$D80031` the pad, bumpers, triggers and menu buttons, `$D80032` the four
face buttons in its high nibble. All three read live, and `$D80031` bit 0 says whether the controller is plugged in. The
bit layout is in [memory-map.md](memory-map.md).

## Interrupts

[interrupts.md](interrupts.md) has the vectors and a complete timer example. The points to carry into your handlers:

- Save every register you touch, and pull in the same width you pushed in.
- `RTI` restores `P`, so `M` and `X` come back by themselves.
- `DB` is not saved and cannot be restored (no `PLB`). Keep it constant and end handlers with `SDB`, or don't change it.
- Acknowledge by writing 1s to `TIMER_STATUS` (and `IRQ_STATUS` for other sources) before `RTI`, or the interrupt fires
  again.
- Don't use `LOOP` in a handler.

## Debugging

`HLT` stops the CPU and costs no cycles, so it is the natural end of a test program. `bin/debugger` in the emulator
repository steps one instruction at a time, shows the registers and cycle count, sets breakpoints on `PC`, and reads and
writes memory; `cpudisasm` turns a binary back into text that `cpuasm` accepts.

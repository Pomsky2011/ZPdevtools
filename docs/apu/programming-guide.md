# APU programming guide

Writing APU programs with `apuasm`. Every `apuasm` block here is assembled by `python3 scripts/check_doc_asm.py`, and the
programs with stated results were run on the emulator. Read [architecture.md](architecture.md) first for the registers
and memory map, and [instruction-set.md](instruction-set.md) for what each instruction does.

## The assembler

Registers are `X` and `Y`. Numbers are `$hex`, `0xhex` or decimal. `;` starts a comment. A label ends in `:`.

```text
.org $XXXX        pad with zero words up to this address (never backward)
.db a, b, ...     bytes, packed two to a word; a name that is a label becomes that label's low byte
                  (a jump table: `.db handler_a, handler_b`); .dbh gives the label's high byte instead
.include "f.uas"  assemble another file here (path relative to the including file); labels are shared
.rp $XX           declare which page RP holds from here on
```

**Keep a `.db` line to eight values.** `apuasm` silently drops tokens past the tenth on a line; a `.db` with sixteen values
loses half of them.

**The RP lint.** `JMS`, `JSR`, `JZ`, `BEQ`, `BNE`, `BLT` and `BGT` jump to `RP:` the low byte of the label. `apuasm` tracks
`RP` (every `SRP` sets it, `.rp` declares it where control arrives by a jump) and refuses a jump whose label is in
another page, so a missed `SRP` is a build error. Code that never uses `SRP` or `.rp` is not checked.

## A program skeleton

```apuasm
.org $8000            ; the test runner loads programs at $8000; a real one lives in ARAM ($3000-$6FFF)
start:
    srp $80           ; RP = $80: jump targets below are in page $80
    scr x, $00
    scr y, $01
    bsp               ; SP = $0100 (Y high, X low). The stack grows UP from here.

    ; ... your code ...

done:
    jms done          ; the MMP keeps mixing while the APU spins here
```

A program the CPU starts must not assume anything about the machine: the APU is **not reset** when a cartridge begins, and
the BIOS can leave flags, `SP`, `RP`, `DP` and channels behind. Start it as described in
[architecture.md](architecture.md) (reset and halt, upload, set `PC`, run), and set every register you rely on.

## Playing a sample

This program copies a 16-byte sample block into SST, builds an STL entry for it, and starts channel 0 at full volume,
panned right. It plays a looping sine; on the emulator the right output is `0 30 58 79 95 104 106 100 87 68 45 20`
(times 256), repeating.

```apuasm
.org $8000
start:  srp $80
        scr x, $00
        scr y, $01
        bsp                     ; stack at $0100

        ; copy the 16-byte block at $8100 to SST $9000
        zor x
copy:   sdp $81
        strx x, y               ; Y = table[X]
        sdp $90
        stax y, x               ; SST[X] = Y
        inc x
        scr y, 16
        cmn x, y, copy          ; repeat until X == 16

        ; STL entry 0 at $7000: start $9000, loop 0 (loop the whole block)
        sdp $70
        zor x
        sta x, $00
        scr y, $90
        sta y, $01
        zor x
        sta x, $02
        sta x, $03

        ; channel 0: pitch $1000, volume 255, pan 255 (right), then start it
        sdp $00
        scr x, $10
        sta x, $01              ; pitch high byte
        zor x
        sta x, $00              ; pitch low byte
        scr x, 255
        sta x, $20              ; volume
        sta x, $30              ; pan
        scr x, $70
        sta x, $51              ; STL address high byte: starts the channel

done:   jms done                ; the MMP keeps mixing

.org $8100
        .db $00, $04, $00, $00, $00, $1E, $3A, $4F
        .db $5F, $68, $6A, $64, $57, $44, $2D, $14
```

Points to notice:

- **A 16-bit register takes effect on its high byte.** Pitch is written high byte first here (its low byte is 0), and the
  STL address last, because writing its high byte is what starts the channel.
- `sta x, $yy` is a *word* write (see [instruction-set.md](instruction-set.md)): it also re-writes the next address with its
  old value. That is harmless in RAM and is why the pitch low-byte write after the high byte still leaves the pitch right.
- `STRX`/`STAX` copy bytes with the offset in a register. Their syntax is `strx offset, data` and `stax data, offset`.
- The block has W set (`$04`) and the STL loop word is 0, so the whole sample loops. For a one-shot use `$0C`.

## Patterns

### Counted loop

```apuasm
; X counts 1..10; Y holds the bound
    scr y, $0A
    zor x
count_loop:
    inc x
    cml x, y, count_loop   ; loop while X < Y (CML branches backward)
    hlt
```

`CMN` and `CML` branch backward only; `CME` and `CMG` forward only. To skip forward when `X != 0`, use `CMG X, Y` with `Y = 0`.

### Hardware loop

```apuasm
    lst 0, 16        ; start loop 0, 16 times
    nop 0            ; body
    lfn 0            ; end loop 0
    hlt
```

### Loop with a test that does not push

`JNZ` is a conditional **call**: it pushes a return address when taken and can only reach the BIOS page, so it leaks two
stack bytes per iteration as a loop branch. Test with `JZ` (which never pushes) and jump back with `JMS`:

```apuasm
.org $8000
    srp $80
    scr x, 5
loop:
    dec x
    jz x, loop_done
    jms loop
loop_done:
    hlt
```

### 16-bit add with carry

```apuasm
; ($0201:$0200) + ($0203:$0202) -> ($0205:$0204); here $01FF + $0001 = $0200
    sdp $02
    scr x, $FF
    sta x, $00
    scr x, $01
    sta x, $01
    sta x, $02
    zor x
    sta x, $03
    str $00, x
    str $02, y
    add x, y, $04        ; low bytes -> $0204, sets carry
    str $01, x
    str $03, y
    adc x, y, $05        ; high bytes + carry -> $0205
    hlt
```

### Copy a page

```apuasm
; copy 256 bytes from page $30 to page $40
    srp $80
    zor x
copy_loop:
    sdp $30
    strx x, y            ; Y = mem[$30:X]
    sdp $40
    stax y, x            ; mem[$40:X] = Y
    inc x
    scr y, 0
    cmn x, y, copy_loop  ; loop until X wraps back to 0
    hlt
```

### Delay

The APU runs 4,250,710 cycles a second and every instruction takes 2, so a millisecond is about 4,251 cycles.
`NOP $7FF` stalls for 2,049. Measured on the emulator, two passes of this routine take 8,510 cycles:

```apuasm
.org $8000
    srp $80
    scr x, 2             ; milliseconds
    jsr delay_ms
    hlt

delay_ms:                ; X = milliseconds (1-255); clobbers X
    nop $7FF             ; 2049 cycles
    nop $7FF             ; 2049
    nop $091             ; 147
    dec x                ; 2
    jz x, delay_done     ; 2
    jms delay_ms         ; 2   = 4,251 per pass
delay_done:
    ret
```

In fast mode a millisecond is twice as many cycles.

### Saving context around a call

```apuasm
.org $8000
    srp $80
    scr x, $00
    scr y, $01
    bsp
    pux              ; save X
    puy              ; save Y
    pudp             ; save the data pointer (DP, DB)
    jsr work
    podp             ; restore in the reverse order
    poy
    pox
    hlt
work:
    ret
```

## Talking to the CPU

The CPU and APU share RAM through the APU window, so a mailbox is a byte both poll. By convention `$1000` is written by the
CPU and `$1800` by the APU. The [ZPA sequencer](zpa.md) reads `$1000` for `WAIT`, `JZ` and `JEQ`, and writes `$1800` with
`POST`. There is no interrupt either way.

## Debugging checklist

1. Did the APU start, and from where? Check `APUSTATUS` bit 0 (halted) and `APUPC`.
2. Is `SP` set? It resets to 0, so the first `JSR`, `PUX` or `JMP` writes at `$0000`, which is MMP register space.
3. Is `RP` right for every jump? Without `SRP` or `.rp` there is no lint.
4. Did a `JNZ` or `JMP` push a return address you never popped?
5. Did a `sta x, $yy` store land on an odd hardware address and overwrite the next register?
6. For sound: is the channel's STL address nonzero, the sample's final block W-flagged, and the channel's volume non-zero
   (registers read back 0 until written, but the channel's volume starts at 255 internally)?
7. Is a 16-bit MMP register being written low-byte last, so the high-byte write applied an old low byte?
8. In fast mode, are you timing with `NOP` counts tuned for normal mode?

The emulator's `bin/test_apu program.bin [cycles]` runs a program at `$8000` and prints the registers, cycle and instruction
counts; `apudisasm` reads a binary back as text `apuasm` accepts.

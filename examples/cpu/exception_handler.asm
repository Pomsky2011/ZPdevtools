; exception_handler.asm - a handler image for the dev Boot ROM (docs/exceptions.md): print one line about the exception, then
; resume the interrupted program.
;
; The dev Boot ROM asks the host for this image when an exception reaches it, loads it at $81:0000 and jumps here with 8-bit
; A = the kind (0 ABORT, 1 NMI, 2 IRQ, 3 BRK, 4 COP), interrupts masked, DB = $81. The registers it clobbered, and what it
; knows about the exception, are in the context block at $81:FF00 (see exception_dev.def in ZPbootROM). The stack still
; holds the hardware's frame, so RTI resumes exactly where the program was.
;
; Output: "H<kind><fault kind>\n", e.g. H30 = a BRK, H01 = an ABORT raised by a write to ROM. Build and run (from ZPdevtools):
;   executables/cpuasm examples/cpu/exception_handler.asm /tmp/handler.bin
;   zeropoint_sdl game.zpb --boot ../ZPbootROM/build/bootrom_dev.bin --dev-unit --exception-handler /tmp/handler.bin
;
; no-roundtrip: the image is meant to run at $81:0000, not where cpudisasm would look for it (tests/disasm/check.py skips this file)

.org $0000
.bank $81

handler:
        sep #$30
        lda #$48                ; 'H'
        jsr putc
        lda $FF00               ; kind (DB is $81, so this is $81:FF00)
        clc
        adc #$30
        jsr putc
        lda $FF01               ; fault kind: 0 none, 1 write to ROM, 2 read of unmapped memory, 3 write to unmapped memory
        clc
        adc #$30
        jsr putc
        lda #$0A
        jsr putc

        lda $FF05               ; resume: put DB back (patched into the SDB below: it only takes an immediate) ...
        sta db_value
        rep #$30
        lda $FF12
        tcd                     ; ... D ...
        ldx $FF0C               ; ... X, Y, A
        ldy $FF0E
        lda $FF0A
        .byte $1B               ; SDB #db_value, written out so the operand has a label to patch
db_value:
        .byte $00
        rti                     ; P, PC, PB come off the stack: the program carries on

putc:
        pha
putc_wait:
        lda $D8003C
        and #$02
        beq putc_wait
        pla
        sta $D8003D
        rts

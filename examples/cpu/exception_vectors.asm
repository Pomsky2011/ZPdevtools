; exception_vectors.asm - a cartridge that points its exception vectors at the Boot ROM, then trips each one.
;
; Nothing routes an exception to the Boot ROM by itself: the CPU reads the handler's address from the cartridge's own
; vector table ($00:FFE4 COP, $FFE6 BRK, $FFF8 ABORT, $FFFA NMI, $FFFE IRQ). A cartridge that wants the Boot ROM's behaviour
; (a dev unit: ask the host for a handler image; a retail unit: lock) points a vector at a one-line `jml` into the Boot ROM's
; fixed entry table: ABORT $E0:0048, NMI $E0:004C, IRQ $E0:0050, BRK $E0:0054, COP $E0:0058.
;
; On a dev unit with the dev Boot ROM and examples/cpu/exception_handler.asm as the handler this prints
;   H30 (BRK), H40 (COP), H01 (write to ROM), H02 (read of unmapped memory), H03 (write to unmapped memory), in order, and the program
; carries on after each one. A dev unit's CPU raises those faults itself; a retail unit does not, and the BRK and COP lock it.
;
; no-roundtrip: the vector table is data past the code (tests/disasm/check.py skips this file)

.org $8000
start:
        sep #$20
        lda #$AA
        sta $800028             ; started (the tests tell "locked by the exception" from "never got here" with this)
        rep #$30
        lda #$1234
        ldx #$5678
        ldy #$9ABC
        brk #$00                ; -> stub_brk -> the Boot ROM
        sta $800020             ; A, X, Y must come back unchanged
        txa
        sta $800022
        tya
        sta $800024
        sep #$20
        lda #$11
        sta $800026
        cop #$01                ; a COP signature the Boot ROM does not claim
        lda #$22
        sta $800026
        lda #$42
        sta >$008100            ; a store to ROM: stores vanish on a retail unit; a dev unit raises ABORT
        lda #$33
        sta $800026
        lda >$C00000            ; a read of an unmapped bank
        lda #$44
        sta $800026
        lda #$55
        sta >$C00000            ; a write to one
        rep #$10
        ldx #$4000
drain:  dex                     ; (a halted CPU stops the serial line too: let the last report finish going out)
        bne drain
        sep #$30
        lda #$FF
        sta $800027             ; finished
        hlt

stub_brk:
        jml $E00054
stub_cop:
        jml $E00058
stub_abort:
        jml $E00048
stub_nmi:
        jml $E0004C
stub_irq:
        jml $E00050

.org $FFE4
        .word stub_cop          ; $FFE4 COP
        .word stub_brk          ; $FFE6 BRK
.org $FFF8
        .word stub_abort        ; $FFF8 ABORT
        .word stub_nmi          ; $FFFA NMI
        .word $0000             ; $FFFC reset (unused: the Boot ROM starts the cartridge at its entry point)
        .word stub_irq          ; $FFFE IRQ

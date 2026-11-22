; zp_save.asm - libzp accessors for the cartridge's battery save RAM at banks $C0-$C7 (zp_save.h).
;
; Same shape as zp_peek8/zp_poke8 (zp_mmio.asm), which do not reach these banks on purpose: programs without save RAM
; keep their exact cycle counts. Link this file only into a cartridge built with zpbuild --save-size.
;
; Calling convention (shared zpcc / def88186cc ABI, 16-bit mode):
;   zp_sram_poke8(addr, value): A = addr low 16, X = addr high 16 (bank in low byte), Y = value. No return.
;   zp_sram_peek8(addr)       : A = addr low 16, X = addr high 16. Returns the byte in A ($FF for a bank outside $C0-$C7).
; Both are true byte accesses (8-bit A), as in zp_mmio.asm.

; ---------------------------------------------------------------------------
; void zp_sram_poke8(zpaddr addr, zpu8 value)
; ---------------------------------------------------------------------------
zp_sram_poke8:
        PHA                     ; save offset (addr low 16)
        TXA                     ; A = bank in the low byte
        SEP #$20
        CMP #$C0
        BEQ __sram_poke_c0
        CMP #$C1
        BEQ __sram_poke_c1
        CMP #$C2
        BEQ __sram_poke_c2
        CMP #$C3
        BEQ __sram_poke_c3
        CMP #$C4
        BEQ __sram_poke_c4
        CMP #$C5
        BEQ __sram_poke_c5
        CMP #$C6
        BEQ __sram_poke_c6
        CMP #$C7
        BEQ __sram_poke_c7
        REP #$20                ; not a save bank: drop the write
        PLA
        RTS
__sram_poke_c0:
        REP #$20
        PLA
        TAX
        TYA
        SEP #$20
        STA $C00000,X
        REP #$20
        RTS
__sram_poke_c1:
        REP #$20
        PLA
        TAX
        TYA
        SEP #$20
        STA $C10000,X
        REP #$20
        RTS
__sram_poke_c2:
        REP #$20
        PLA
        TAX
        TYA
        SEP #$20
        STA $C20000,X
        REP #$20
        RTS
__sram_poke_c3:
        REP #$20
        PLA
        TAX
        TYA
        SEP #$20
        STA $C30000,X
        REP #$20
        RTS
__sram_poke_c4:
        REP #$20
        PLA
        TAX
        TYA
        SEP #$20
        STA $C40000,X
        REP #$20
        RTS
__sram_poke_c5:
        REP #$20
        PLA
        TAX
        TYA
        SEP #$20
        STA $C50000,X
        REP #$20
        RTS
__sram_poke_c6:
        REP #$20
        PLA
        TAX
        TYA
        SEP #$20
        STA $C60000,X
        REP #$20
        RTS
__sram_poke_c7:
        REP #$20
        PLA
        TAX
        TYA
        SEP #$20
        STA $C70000,X
        REP #$20
        RTS

; ---------------------------------------------------------------------------
; zpu8 zp_sram_peek8(zpaddr addr)  -> byte in A (upper byte zero)
; ---------------------------------------------------------------------------
zp_sram_peek8:
        PHA
        TXA
        SEP #$20
        CMP #$C0
        BEQ __sram_peek_c0
        CMP #$C1
        BEQ __sram_peek_c1
        CMP #$C2
        BEQ __sram_peek_c2
        CMP #$C3
        BEQ __sram_peek_c3
        CMP #$C4
        BEQ __sram_peek_c4
        CMP #$C5
        BEQ __sram_peek_c5
        CMP #$C6
        BEQ __sram_peek_c6
        CMP #$C7
        BEQ __sram_peek_c7
        REP #$20                ; not a save bank: open bus
        PLA
        LDA #$00FF
        RTS
__sram_peek_c0:
        REP #$20
        PLA
        TAX
        SEP #$20
        LDA $C00000,X
        BRA __sram_peek_done
__sram_peek_c1:
        REP #$20
        PLA
        TAX
        SEP #$20
        LDA $C10000,X
        BRA __sram_peek_done
__sram_peek_c2:
        REP #$20
        PLA
        TAX
        SEP #$20
        LDA $C20000,X
        BRA __sram_peek_done
__sram_peek_c3:
        REP #$20
        PLA
        TAX
        SEP #$20
        LDA $C30000,X
        BRA __sram_peek_done
__sram_peek_c4:
        REP #$20
        PLA
        TAX
        SEP #$20
        LDA $C40000,X
        BRA __sram_peek_done
__sram_peek_c5:
        REP #$20
        PLA
        TAX
        SEP #$20
        LDA $C50000,X
        BRA __sram_peek_done
__sram_peek_c6:
        REP #$20
        PLA
        TAX
        SEP #$20
        LDA $C60000,X
        BRA __sram_peek_done
__sram_peek_c7:
        REP #$20
        PLA
        TAX
        SEP #$20
        LDA $C70000,X
        BRA __sram_peek_done
__sram_peek_done:
        REP #$20
        AND #$00FF              ; the hidden high byte is whatever A held: return a clean byte
        RTS

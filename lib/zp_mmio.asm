; zp_mmio.asm - libzp bus accessors: zp_peek8 / zp_poke8.
;
; These are the single hardware-touch primitives the C SDK builds on
; (zp_dma.h, zp_ppu.h, zp_apu.h, zp_timer.h all reduce to peek8/poke8).  They
; reach any bank the SDK uses - RAM ($80), APU window ($A0), PPU window ($B0),
; and I/O ($D8) - with the DEF88186 absolute-long indexed store/load.
;
; Calling convention (shared zpcc / def88186cc ABI, 16-bit mode):
;   zp_poke8(addr, value): A = addr low 16, X = addr high 16 (bank in low byte),
;                          Y = value.  No return.
;   zp_peek8(addr)       : A = addr low 16, X = addr high 16.  Returns byte in A.
;
; Assemble standalone:  cpuasm zp_mmio.asm zp_mmio.bin
; (Or .include it into the program so the labels resolve at link time.)

; ---------------------------------------------------------------------------
; void zp_poke8(zpaddr addr, zpu8 value)
; ---------------------------------------------------------------------------
zp_poke8:
        PHA                     ; save offset (addr low 16)
        TXA                     ; A = addr high 16 (bank in low byte)
        SEP #$20                ; 8-bit A for the bank compare
        CMP #$D8                ; I/O bank?
        BEQ __poke_io
        CMP #$B0                ; PPU window?
        BEQ __poke_ppu
        CMP #$A0                ; APU window?
        BEQ __poke_apu
        CMP #$80                ; RAM?
        BEQ __poke_ram
        REP #$20                ; unknown/ROM bank: drop the write
        PLA
        RTS
__poke_io:
        REP #$20
        PLA                     ; A = offset
        TAX                     ; X = index
        TYA                     ; A = value
        STA $D80000,X
        RTS
__poke_ppu:
        REP #$20
        PLA
        TAX
        TYA
        STA $B00000,X
        RTS
__poke_apu:
        REP #$20
        PLA
        TAX
        TYA
        STA $A00000,X
        RTS
__poke_ram:
        REP #$20
        PLA
        TAX
        TYA
        STA $800000,X
        RTS

; ---------------------------------------------------------------------------
; zpu8 zp_peek8(zpaddr addr)  -> byte in A
; ---------------------------------------------------------------------------
zp_peek8:
        PHA
        TXA
        SEP #$20
        CMP #$D8
        BEQ __peek_io
        CMP #$B0
        BEQ __peek_ppu
        CMP #$A0
        BEQ __peek_apu
        CMP #$80
        BEQ __peek_ram
        REP #$20                ; ROM ($00) / unknown -> read from bank $00
        PLA
        TAX
        LDA $000000,X
        RTS
__peek_io:
        REP #$20
        PLA
        TAX
        LDA $D80000,X
        RTS
__peek_ppu:
        REP #$20
        PLA
        TAX
        LDA $B00000,X
        RTS
__peek_apu:
        REP #$20
        PLA
        TAX
        LDA $A00000,X
        RTS
__peek_ram:
        REP #$20
        PLA
        TAX
        LDA $800000,X
        RTS

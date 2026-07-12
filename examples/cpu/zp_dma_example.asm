; zp_dma_example.asm - demonstrates zp_dma.asm's zp_dma_load: two DMA copies
; from Work RAM into the PPU window, one that fits in a single 1024-byte
; transfer and one that needs two passes (exercises zp_dma_load's internal
; loop over multiple transfers).
;
; This runs as an alternate Boot ROM payload (see examples/boot-rom/), or
; .include it into a cartridge program the same way - just make sure
; .BANK above the .include (or before zp_dma_load's first use, if this file
; is included partway through a larger program) matches wherever this code
; actually gets loaded, and SDB the same value before the first same-bank
; JSR/JMP/LDA-absolute (see zp_dma.asm's header comment for why).
;
; Assumes Work RAM at $801000 and $802000 already holds whatever data you
; want copied (a real program would embed that data in ROM and either point
; src at the ROM copy directly, or stage it into RAM first).
.BANK $E0

start:
        SDB #$E0

        ; --- copy 1: $801000, 300 bytes -> PPU window $B01000 ---
        LDA #$00
        STA $800010              ; src low
        LDA #$10
        STA $800011              ; src mid
        LDA #$80
        STA $800012              ; src bank
        LDA #$00
        STA $800013              ; dst low
        LDA #$10
        STA $800014              ; dst mid (avoid the PPU's $00E0/$00E1
                                 ; live scanline register at the very start
                                 ; of the window - see docs/ppu.md)
        LDA #$B0
        STA $800015              ; dst bank
        LDA #$2C
        STA $800016              ; len low  ($012C = 300, rounds up to 512)
        LDA #$01
        STA $800017              ; len high
        LDA #$00
        STA $800018              ; chan 0
        JSR zp_dma_load

        ; --- copy 2: $802000, 1500 bytes -> PPU window $B00800 ---
        ; (1500 rounds up to 1536 = 6 units -> two passes: 1024 + 512)
        LDA #$00
        STA $800010
        LDA #$20
        STA $800011
        LDA #$80
        STA $800012
        LDA #$00
        STA $800013
        LDA #$08
        STA $800014
        LDA #$B0
        STA $800015
        LDA #$DC
        STA $800016              ; len low  ($05DC = 1500)
        LDA #$05
        STA $800017              ; len high
        LDA #$01
        STA $800018              ; chan 1
        JSR zp_dma_load

done:
        BRA done

.include "../../lib/zp_dma.asm"

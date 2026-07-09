; game.asm - Minimal ZeroPoint program demonstrating the two build modes.
;
; It includes the entry-mode crt0 startup stub (which slides into `main`),
; then DMA-loads a PPU blob into the PPU window and starts the PPU, before
; dropping into an idle loop.  The DMA registers are driven directly here so
; the sample assembles with cpuasm alone; C programs would use <zeropoint/*.h>.
;
; Build with: make          (see Makefile in this directory)

.org $8000

; --- C runtime startup: initialise CPU mode, then slide into main ----------
.include "../../lib/crt0_entry.asm"

; --- Program entry ---------------------------------------------------------
main:
        ; Kick a DMA copy of the PPU blob (ROM $100000) into the PPU window
        ; (bank $B0).  Config is streamed to $D80021 after resetting at $D80020.
        LDA #$00
        STA $D80020         ; reset DMA config index
        ; 9-byte config: mode/size/chan, src(3), dst(3), mult, irq
        LDA #$30            ; size_bits=0 (256B), mode=DataCopy(0)... (demo value)
        STA $D80021
        LDA #$00
        STA $D80021         ; src low
        LDA #$00
        STA $D80021         ; src mid
        LDA #$10
        STA $D80021         ; src high  -> $100000
        LDA #$00
        STA $D80021         ; dst low
        LDA #$04
        STA $D80021         ; dst mid
        LDA #$B0
        STA $D80021         ; dst high  -> $B00400 (PPU code base)
        LDA #$01
        STA $D80021         ; multiplier
        LDA #$00
        STA $D80021         ; irq/trigger -> fires the transfer

        ; Start the PPU core.
        LDA #$01
        STA $D80000         ; PPUCTRL: bit0 = start

idle:
        BRA idle

; Pan sweep
; Sweeps channel 0 from full left to full right, then stops it. Needs the sample from play_sample.asm already in SST/STL.
; Assemble: ../../executables/apuasm pan_sweep.asm
; Run:      ZeroPoint/build/bin/run_apu_demo <output>.bin
; (Walked through in docs/apu/programming-guide.txt.)

.org $8000
start:
    srp $80
    scr x, $00
    scr y, $01
    bsp
    sdp $00
    zor x
    sta x, $50
    lda x, $70
    sta x, $51          ; start channel 0 on the STL entry at $7000
    zor x               ; X = pan, starts at 0 (full left)
pan_loop:
    sta x, $30          ; ch0 pan = X
    inc x
    nop $200            ; stall between steps
    scr y, $00
    cmn x, y, pan_loop  ; loop until X wraps back to 0 (after 255)
    zor x
    sta x, $50          ; stop channel 0
    sta x, $51
done:
    jms done

; Pitch sweep (laser)
; Sweeps channel 1's pitch from $3000 down to $0800, then stops it. Needs the sample from play_sample.asm already in SST/STL.
; Assemble: ../../executables/apuasm pitch_sweep.asm
; Run:      ZeroPoint/build/bin/run_apu_demo <output>.bin
; (Walked through in docs/apu/programming-guide.txt.)

.org $8000
start:
    srp $80
    scr x, $00
    scr y, $01
    bsp
    sdp $00
    lda x, $30
    sta x, $03          ; ch1 pitch high ($02/$03 = channel 1)
    zor x
    sta x, $02          ; ch1 pitch low  -> $3000
    lda x, $C8
    sta x, $21          ; ch1 volume
    zor x
    sta x, $52          ; ch1 STL address low  ($52/$53 = channel 1)
    lda x, $70
    sta x, $53          ; ch1 STL address high -> $7000: starts playing
    lda x, $30          ; X = pitch high byte, sweeps $30 down to $08
    scr y, $07
sweep_loop:
    sta x, $03          ; update pitch high byte
    dec x
    nop $7FF            ; stall 2047 cycles between steps
    bgt x, y, sweep_loop
    zor x
    sta x, $52          ; stop channel 1: STL address = 0
    sta x, $53
done:
    jms done

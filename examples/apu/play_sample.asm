; Play a sample
; Writes a 12-sample sine into SST, points STL entry 0 at it and plays it on channel 0 at about 440 Hz.
; Assemble: ../../executables/apuasm play_sample.asm
; Run:      ZeroPoint/build/bin/run_apu_demo <output>.bin
; (Walked through in docs/apu/programming-guide.txt.)

.org $8000
start:
    srp $80             ; RP = $80: JSR/JMS targets below live in $80xx
    scr x, $00
    scr y, $01
    bsp                 ; SP = $0100 (stack grows UP from here)
    jsr write_sample
    jsr write_stl
    jsr start_playback
done:
    jms done

write_sample:
    sdp $90             ; DP = $90: SST at $9000
    lda x, $00
    sta x, $00          ; byte 0: XX (unused by the MMP)
    lda x, $04
    sta x, $01          ; byte 1: Y=0, L=0100 (W = final block, loops the sample)
    zoa $02             ; bytes 2-3: V/U/T/S clamping = 0
    lda x, $00
    sta x, $04          ; sample 0
    lda x, $40
    sta x, $05          ; sample 1
    lda x, $6E
    sta x, $06          ; sample 2
    lda x, $7F
    sta x, $07          ; sample 3 (peak)
    lda x, $6E
    sta x, $08          ; sample 4
    lda x, $40
    sta x, $09          ; sample 5
    lda x, $00
    sta x, $0A          ; sample 6
    lda x, $C0
    sta x, $0B          ; sample 7 (-64: samples are signed 8-bit)
    lda x, $92
    sta x, $0C          ; sample 8 (-110)
    lda x, $81
    sta x, $0D          ; sample 9 (-127, trough)
    lda x, $92
    sta x, $0E          ; sample 10
    lda x, $C0
    sta x, $0F          ; sample 11
    ret

write_stl:
    sdp $70             ; DP = $70: STL at $7000
    lda x, $00
    sta x, $00          ; sample address low
    lda x, $90
    sta x, $01          ; sample address high -> $9000
    zoa $02             ; loop word = 0: loop the whole sample
    ret

start_playback:
    sdp $00             ; DP = $00: MMP registers
    lda x, $01
    sta x, $01          ; ch0 pitch high   } $01C2 = 0.11x: 12 samples at
    lda x, $C2
    sta x, $00          ; ch0 pitch low    } 0.11x = about 440 Hz
    lda x, $80
    sta x, $20          ; ch0 volume
    sta x, $30          ; ch0 pan (center)
    zor x
    sta x, $50          ; ch0 STL address low
    lda x, $70
    sta x, $51          ; ch0 STL address high -> $7000: playback starts
    ret

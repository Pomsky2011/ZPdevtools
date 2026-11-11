; CPU command processor
; Polls the DEF88186 input buffer ($1000) for play/stop/volume commands from the CPU.
; Assemble: ../../executables/apuasm mailbox_commands.asm
; Run:      ZeroPoint/build/bin/run_apu_demo <output>.bin
; (Walked through in docs/apu/programming-guide.txt.)

.org $8000
start:
    srp $80
    scr x, $00
    scr y, $01
    bsp
cmd_loop:
    sdp $10             ; DP:DB = $1000, the command byte
    sdb $00
    lda x               ; X = command (0 = idle)
    scr y, $01
    beq x, y, cmd_play
    scr y, $02
    beq x, y, cmd_stop
    scr y, $03
    beq x, y, cmd_vol
    jms cmd_loop

cmd_play:               ; $01 ch stlHi stlLo
    sdb $01
    lda x               ; X = channel
    add x, x, x         ; X = ch * 2
    scr y, $50
    add x, y, x         ; X = $50 + ch * 2, the channel's STL register
    sdb $03
    lda y               ; Y = STL address low
    sdp $00
    stax y, x           ; (DP = $00, so STAX hits the MMP register)
    inc x
    sdp $10
    sdb $02
    lda y               ; Y = STL address high
    sdp $00
    stax y, x           ; writing the high byte starts playback
    jms ack

cmd_stop:               ; $02 ch
    sdb $01
    lda x
    add x, x, x
    scr y, $50
    add x, y, x
    zor y
    sdp $00
    stax y, x
    inc x
    stax y, x
    jms ack

cmd_vol:                ; $03 ch volume
    sdb $01
    lda x
    scr y, $20
    add x, y, x         ; X = $20 + ch, the channel's volume register
    sdb $02
    lda y               ; Y = volume
    sdp $00
    stax y, x
    jms ack

ack:
    sdp $10
    sdb $00
    sta $00             ; clear the command byte so the CPU knows it was taken
    jms cmd_loop

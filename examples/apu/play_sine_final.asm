; Play sine wave - FINAL CORRECTED VERSION
; Pitch 0x0100 (slow), Volume 16 (quiet), Loop address = 0 (CORRECT!)

start:
    ; Create STL entry at $7000
    SDP $70
    SDB $00
    WRH $90                 ; Sample start = $9000
    WRL $00
    SDB $02
    WRH $00                 ; Loop address = 0 (loop entire sample!)
    WRL $00

    ; Configure MMP channel 0
    SDP $00
    SDB $00
    WRH $01                 ; Pitch = 0x0100
    WRL $00

    ; Set volume to 16 (quiet)
    SDB $20
    SBF 0
    SCR X, 16
    STA X, $20

    ; Start playback
    SDB $54
    WRH $70
    WRL $00

    SRP $80

loop:
    NOP 1000
    BEQ X, X, loop

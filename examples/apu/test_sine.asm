; Simple sine wave test - 12 samples, one full period
; Plays a clean sine wave on left channel 0

start:
    ; Create sine wave sample in SST at $9000
    ; 12 samples spanning 0 to 2π
    SDP $90
    SDB $00
    WRH $FF                 ; Loop count = 255
    WRL $00
    SDB $01
    WRH $00                 ; Y=0 (loop from 0)
    WRL $40                 ; L=0100 (final block, W bit set)
    SDB $02
    WRH $00                 ; V=0, U=0 (no clamping)
    WRL $00                 ; T=0, S=0

    ; Write 12 sine wave samples (unsigned 8-bit, 128 = zero)
    ; Values calculated for smooth sine: 0°, 30°, 60°, 90°, 120°, 150°, 180°, 210°, 240°, 270°, 300°, 330°
    SDB $04
    SBF 0
    SCR X, 128              ; Sample 0: sin(0°) = 0 → 128
    STA X, $04
    SCR X, 192              ; Sample 1: sin(30°) ≈ 0.5 → 192
    STA X, $05
    SCR X, 239              ; Sample 2: sin(60°) ≈ 0.866 → 239
    STA X, $06
    SCR X, 255              ; Sample 3: sin(90°) = 1.0 → 255
    STA X, $07
    SCR X, 239              ; Sample 4: sin(120°) ≈ 0.866 → 239
    STA X, $08
    SCR X, 192              ; Sample 5: sin(150°) ≈ 0.5 → 192
    STA X, $09
    SCR X, 128              ; Sample 6: sin(180°) = 0 → 128
    STA X, $0A
    SCR X, 64               ; Sample 7: sin(210°) ≈ -0.5 → 64
    STA X, $0B
    SCR X, 17               ; Sample 8: sin(240°) ≈ -0.866 → 17
    STA X, $0C
    SCR X, 0                ; Sample 9: sin(270°) = -1.0 → 0
    STA X, $0D
    SCR X, 17               ; Sample 10: sin(300°) ≈ -0.866 → 17
    STA X, $0E
    SCR X, 64               ; Sample 11: sin(330°) ≈ -0.5 → 64
    STA X, $0F

    ; Create STL entry at $7000 pointing to SST block at $9000
    SDP $70
    SDB $00
    WRH $90                 ; Sample data address = $9000
    WRL $00
    SDB $02
    WRH $00                 ; Loop address = 0 (loop entire sample)
    WRL $00

    ; Configure MMP channel 0 (left ear)
    SDP $00                 ; MMP registers

    ; Set pitch to $0100 (1/16× speed, lower pitch)
    SDB $00
    WRH $01
    WRL $00

    ; Set volume to 255 (maximum gain = 2.0×)
    SDB $20
    SBF 0
    SCR X, 255
    STA X, $20

    ; Set STL address to $7000 (starts playback!)
    SDB $54
    WRH $70
    WRL $00

    ; Set RP for loop address
    SRP $80

    ; Infinite loop
loop:
    NOP 1000
    BEQ X, X, loop

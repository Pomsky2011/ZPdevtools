; Simple tone generator for APU
; Plays a 440 Hz square wave on left channel 0

; Memory map:
; $0000-$00FF: MMP registers
; $7000-$7FFF: STL entries
; $9000-$CFFF: SST blocks

start:
    ; Create a simple square wave sample in SST at $9000
    ; Block header: 255 loops, loop from sample 0, final block
    SDP $90
    SDB $00
    WRH $FF                 ; Loop count = 255
    WRL $00
    SDB $01
    WRH $00                 ; Y=0 (loop from 0), L=0010 (final block, normal loop)
    WRL $20                 ; Bits: 0010 = W bit set (final block)
    SDB $02
    WRH $00                 ; V=0, U=0 (no clamping)
    WRL $00                 ; T=0, S=0

    ; Write 12 samples: square wave (6 high, 6 low)
    SDB $04
    SBF 0
    SCR R0, 100             ; High value
    STA R0, $04
    STA R0, $05
    STA R0, $06
    STA R0, $07
    STA R0, $08
    STA R0, $09
    SCR R0, 155             ; Low value (negative in signed 8-bit)
    STA R0, $0A
    STA R0, $0B
    STA R0, $0C
    STA R0, $0D
    STA R0, $0E
    STA R0, $0F

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

    ; Set pitch to $1000 (1.0× speed, original pitch)
    SDB $00
    WRH $10
    WRL $00

    ; Set volume to 128 (moderate gain)
    SDB $20
    SBF 0
    SCR R0, 128
    STA R0, $20

    ; Set STL address to $7000 (starts playback!)
    SDB $54
    WRH $70
    WRL $00

    ; Infinite loop (audio plays in background via MMP hardware)
loop:
    NOP 1000
    JMP loop

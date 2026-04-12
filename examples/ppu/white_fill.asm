; White Screen - Direct Framebuffer Fill
; Properly initializes display before filling

start:
    ; Disable framebuffer auto-roll (VOC $00FB = 0)
    TARREG 2, LSB, DP
    TARREG 3, MSB, DP
    SETBYTE 2, 0xFB
    SETBYTE 3, 0x00         ; DP = $00FB
    TARREG 2, LSB, R0
    SETBYTE 2, 0x00         ; R0 = 0 (disable auto-roll)
    MOVDP R0

    ; Point DP to framebuffer start
    TARREG 3, LSB, DP
    SETBYTE 3, 0x00
    TARREG 3, MSB, DP
    SETBYTE 3, 0xE0         ; DP = $E000

    ; White pixel value
    TARREG 2, LSB, R0
    SETBYTE 2, 0xFF
    TARREG 2, MSB, R0
    SETBYTE 2, 0xFF         ; R0 = $FFFF

    ; Step = 2 bytes
    TARREG 2, LSB, R1
    SETBYTE 2, 0x02
    TARREG 2, MSB, R1
    SETBYTE 2, 0x00         ; R1 = $0002

    ; End address (last valid pixel address)
    TARREG 2, LSB, R2
    SETBYTE 2, 0xFE
    TARREG 2, MSB, R2
    SETBYTE 2, 0xFF         ; R2 = $FFFE

    ; Start address (for wrap detection)
    TARREG 2, LSB, R3
    SETBYTE 2, 0x00
    TARREG 2, MSB, R3
    SETBYTE 2, 0xE0         ; R3 = $E000

fill:
    CMP R3, DP      ; Check if start > DP (wrapped around)
    JMG done        ; If wrapped, reset
    CMP DP, R2      ; Check if past end
    JMG done        ; If past end, reset
    MOVDP R0        ; Write white to current position
    ADD DP, R1      ; Move to next pixel
    JMR fill        ; Continue filling

done:
    ; Reset to start and fill again (continuous refresh for rolling buffer)
    TARREG 3, LSB, DP
    SETBYTE 3, 0x00
    TARREG 3, MSB, DP
    SETBYTE 3, 0xE0 ; DP = $E000
    JMR fill        ; Loop forever

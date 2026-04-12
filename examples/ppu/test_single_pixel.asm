; Test Single Pixel - Minimal framebuffer write test
; Writes a few white pixels then halts

start:
    ; Point DP to framebuffer start ($E000)
    TARREG 2, LSB, DP
    TARREG 3, MSB, DP
    SETBYTE 2, 0x00
    SETBYTE 3, 0xE0

    ; Load white color ($FFFF) into R0
    TARREG 2, LSB, R0
    TARREG 3, MSB, R0
    SETBYTE 2, 0xFF
    SETBYTE 3, 0xFF

    ; Write 10 white pixels
    MOVDP R0
    INC DP
    INC DP
    MOVDP R0
    INC DP
    INC DP
    MOVDP R0
    INC DP
    INC DP
    MOVDP R0
    INC DP
    INC DP
    MOVDP R0
    INC DP
    INC DP
    MOVDP R0
    INC DP
    INC DP
    MOVDP R0
    INC DP
    INC DP
    MOVDP R0
    INC DP
    INC DP
    MOVDP R0
    INC DP
    INC DP
    MOVDP R0

done:
    HLT

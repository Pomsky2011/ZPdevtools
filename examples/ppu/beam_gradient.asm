;Beam-synced full-screen gradient. Fast unrolled ISR so it finishes well within
; one scanline (no overrun -> no re-entry -> stable). Stack low (0x8000), clear of
; framebuffer. slot = (scanline%16)*512 + $E000 ; color = 2*scanline+1.
R2 = C2
R4 = COLOR
R7 = TMP
R8 = ADDR
R9 = C16
R10 = X
R13 = EBASE
R14 = STEP
start:
    TARREG 2, LSB, SP
    SETBYTE 2, 0x00
    TARREG 3, MSB, SP
    SETBYTE 3, 0x80
    TARREG 2, LSB, C16
    SETBYTE 2, 0x10
    TARREG 3, MSB, C16
    SETBYTE 3, 0x00
    TARREG 2, LSB, C2
    SETBYTE 2, 0x02
    TARREG 3, MSB, C2
    SETBYTE 3, 0x00
    TARREG 2, LSB, EBASE
    SETBYTE 2, 0x00
    TARREG 3, MSB, EBASE
    SETBYTE 3, 0xE0
    TARREG 2, LSB, STEP
    SETBYTE 2, 0x00
    TARREG 3, MSB, STEP
    SETBYTE 3, 0x02
    TARREG 2, LSB, R60
    SETBYTE 2, hblank_handler
    TARREG 3, MSB, R60
    SETBYTE 3, hblank_handler
    TARREG 2, LSB, R5
    SETBYTE 2, 0x20
    TARREG 2, LSB, DP
    SETBYTE 2, 0xF0
    TARREG 3, MSB, DP
    SETBYTE 3, 0x00
    MOVDP R5
main:
    JMR main
hblank_handler:
    TARREG 2, LSB, DP
    SETBYTE 2, 0xE0
    TARREG 3, MSB, DP
    SETBYTE 3, 0x00
    MOV ADDR
    CLR COLOR
    ADD COLOR, ADDR
    ADD COLOR, COLOR
    INC COLOR
    CLR TMP
    ADD TMP, ADDR
    INTDIV TMP, C16
    MUL TMP, C16
    SUB ADDR, TMP
    MUL ADDR, STEP
    ADD ADDR, EBASE
    SETDP ADDR
    CLR X
loop16:
    MOVDP COLOR
    ADD DP, C2
    MOVDP COLOR
    ADD DP, C2
    MOVDP COLOR
    ADD DP, C2
    MOVDP COLOR
    ADD DP, C2
    MOVDP COLOR
    ADD DP, C2
    MOVDP COLOR
    ADD DP, C2
    MOVDP COLOR
    ADD DP, C2
    MOVDP COLOR
    ADD DP, C2
    MOVDP COLOR
    ADD DP, C2
    MOVDP COLOR
    ADD DP, C2
    MOVDP COLOR
    ADD DP, C2
    MOVDP COLOR
    ADD DP, C2
    MOVDP COLOR
    ADD DP, C2
    MOVDP COLOR
    ADD DP, C2
    MOVDP COLOR
    ADD DP, C2
    MOVDP COLOR
    ADD DP, C2
    INC X
    CMP X, C16
    JNZ loop16
    RET

; Simple pixel test - draw one pixel to verify memory-mapped I/O

init:
    ; Set 32-bit render mode
    SETRENDMOD 1

    ; Build DP = 256 (0x0100) using 16 * 16
    CLR R14
    INC R14
    INC R14      ; R14 = 2
    MUL R14, R14 ; R14 = 4
    MUL R14, R14 ; R14 = 16
    CLR DP
    ADD DP, R14
    MUL DP, R14  ; DP = 256

    ; Build constants
    CLR R1
    INC R1       ; R1 = 1

    ; Build 10 in R10
    CLR R10
    ADD R10, R1
    ADD R10, R1
    ADD R10, R1
    ADD R10, R1
    ADD R10, R1
    ADD R10, R1
    ADD R10, R1
    ADD R10, R1
    ADD R10, R1
    ADD R10, R1   ; R10 = 10

    ; Build 255 in R4
    CLR R4
    ADD R4, R1
    ADD R4, R1    ; R4 = 2
    MUL R4, R4    ; R4 = 4
    MUL R4, R4    ; R4 = 16
    MUL R4, R4    ; R4 = 256
    SUB R4, R1    ; R4 = 255

    ; Build 20 in R20
    CLR R20
    ADD R20, R10
    ADD R20, R10   ; R20 = 20

    ; Build 30 in R30
    CLR R30
    ADD R30, R10
    ADD R30, R10
    ADD R30, R10   ; R30 = 30

    ; Clear R0 for high bytes
    CLR R0

    ; Write pixel at (10, 10) with color (10, 20, 30, 255)
    ; Memory map is word-aligned (MOVDP writes 2 bytes)

    ; 0x0100-0x0101 = X position
    MOVDP R10

    ; 0x0102-0x0103 = Y position
    INC DP
    INC DP
    MOVDP R10

    ; 0x0104-0x0105 = Red
    INC DP
    INC DP
    MOVDP R10

    ; 0x0106-0x0107 = Green
    INC DP
    INC DP
    MOVDP R20

    ; 0x0108-0x0109 = Blue
    INC DP
    INC DP
    MOVDP R30

    ; 0x010A-0x010B = Alpha (triggers draw)
    INC DP
    INC DP
    MOVDP R4

    HALT

; Simple tile demo - manually define and draw tiles without loops

init:
    ; Set 32-bit render mode
    SETRENDMOD 1

    ; Build R14 = 16, R7 = 256
    CLR R14
    INC R14
    INC R14
    MUL R14, R14
    MUL R14, R14  ; R14 = 16

    CLR R7
    ADD R7, R14
    MUL R7, R14   ; R7 = 256

    ; Build R6 = 255
    CLR R1
    INC R1
    CLR R6
    ADD R6, R14
    MUL R6, R14
    SUB R6, R1    ; R6 = 255

    ; Define Tile 1: Bright tile
    ; Manually write some bytes to tile buffer
    STRTDEFTILE

    ; Set DP to tile buffer (0x0300)
    CLR DP
    ADD DP, R7
    ADD DP, R7
    ADD DP, R7    ; DP = 768 = 0x0300

    ; Write 255 to first few bytes of tile buffer
    MOVDP R6
    INC DP
    INC DP
    MOVDP R6
    INC DP
    INC DP
    MOVDP R6
    INC DP
    INC DP
    MOVDP R6
    INC DP
    INC DP
    MOVDP R6

    ENDDEFTILE 1

    ; Draw Tile 1 at position (64, 64)
    ; Set DP to tile drawing region (0x0200)
    CLR DP
    ADD DP, R7
    ADD DP, R7    ; DP = 512 = 0x0200

    ; Build R20 = 64 = 4 * 16
    CLR R20
    ADD R20, R14
    ADD R20, R14
    ADD R20, R14
    ADD R20, R14  ; R20 = 64

    ; Write X = 64
    MOVDP R20
    INC DP
    INC DP

    ; Write Y = 64
    MOVDP R20
    INC DP
    INC DP

    ; Write Tile ID = 1
    CLR R8
    INC R8
    MOVDP R8
    INC DP
    INC DP

    ; Write trigger = 1 (draws tile)
    MOVDP R1

    HALT

; Ultra-simple gradient demo - draws 4 pixels to show gradient concept
; Demonstrates that different X/Y positions can have different colors

init:
    ; Set 32-bit render mode
    SETRENDMOD 1

    ; Build DP = 256
    CLR R14
    INC R14
    INC R14      ; R14 = 2
    MUL R14, R14 ; R14 = 4
    MUL R14, R14 ; R14 = 16
    CLR DP
    ADD DP, R14
    MUL DP, R14  ; DP = 256

    ; Build R1 = 1
    CLR R1
    INC R1

    ; Build R6 = 255 (alpha)
    CLR R6
    ADD R6, R14
    MUL R6, R14  ; R6 = 256
    SUB R6, R1   ; R6 = 255

    ; Build some base values
    ; R10 = 50
    CLR R10
    ADD R10, R14
    ADD R10, R14
    ADD R10, R14
    ADD R10, R1
    ADD R10, R1  ; R10 = 50

    ; R20 = 100
    CLR R20
    ADD R20, R10
    ADD R20, R10  ; R20 = 100

    ; R30 = 150
    CLR R30
    ADD R30, R20
    ADD R30, R10  ; R30 = 150

    ; R40 = 200
    CLR R40
    ADD R40, R20
    ADD R40, R20  ; R40 = 200

    ; Pixel 1: (50, 50) -> Red=50, Green=100, Blue=50
    CLR R0
    ; Reset DP
    CLR DP
    ADD DP, R14
    MUL DP, R14

    MOVDP R10    ; X = 50
    INC DP
    INC DP
    MOVDP R10    ; Y = 50
    INC DP
    INC DP
    MOVDP R10    ; R = 50
    INC DP
    INC DP
    MOVDP R20    ; G = 100
    INC DP
    INC DP
    MOVDP R10    ; B = 50
    INC DP
    INC DP
    MOVDP R6     ; A = 255 (draw)

    ; Pixel 2: (100, 50) -> Red=50, Green=100, Blue=100 (more blue)
    CLR DP
    ADD DP, R14
    MUL DP, R14

    MOVDP R20    ; X = 100
    INC DP
    INC DP
    MOVDP R10    ; Y = 50
    INC DP
    INC DP
    MOVDP R10    ; R = 50
    INC DP
    INC DP
    MOVDP R20    ; G = 100
    INC DP
    INC DP
    MOVDP R20    ; B = 100 (bluer)
    INC DP
    INC DP
    MOVDP R6     ; A = 255 (draw)

    ; Pixel 3: (50, 100) -> Red=100, Green=100, Blue=50 (more red)
    CLR DP
    ADD DP, R14
    MUL DP, R14

    MOVDP R10    ; X = 50
    INC DP
    INC DP
    MOVDP R20    ; Y = 100
    INC DP
    INC DP
    MOVDP R20    ; R = 100 (redder)
    INC DP
    INC DP
    MOVDP R20    ; G = 100
    INC DP
    INC DP
    MOVDP R10    ; B = 50
    INC DP
    INC DP
    MOVDP R6     ; A = 255 (draw)

    ; Pixel 4: (100, 100) -> Red=100, Green=100, Blue=100 (neutral gray)
    CLR DP
    ADD DP, R14
    MUL DP, R14

    MOVDP R20    ; X = 100
    INC DP
    INC DP
    MOVDP R20    ; Y = 100
    INC DP
    INC DP
    MOVDP R20    ; R = 100
    INC DP
    INC DP
    MOVDP R20    ; G = 100
    INC DP
    INC DP
    MOVDP R20    ; B = 100
    INC DP
    INC DP
    MOVDP R6     ; A = 255 (draw)

    HALT

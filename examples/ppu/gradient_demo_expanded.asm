; Expanded gradient demo - draws pixels across the screen to show gradient
; Demonstrates: Blue horizontal gradient (X), Red vertical gradient (Y), Green=100

init:
    ; Set 32-bit render mode
    SETRENDMOD 1

    ; Build R14 = 16 and R7 = 256
    CLR R14
    INC R14
    INC R14      ; R14 = 2
    MUL R14, R14 ; R14 = 4
    MUL R14, R14 ; R14 = 16
    CLR R7
    ADD R7, R14
    MUL R7, R14  ; R7 = 256

    ; Build R1 = 1
    CLR R1
    INC R1

    ; Build R6 = 255 (alpha)
    CLR R6
    ADD R6, R14
    MUL R6, R14  ; R6 = 256
    SUB R6, R1   ; R6 = 255

    ; Build color/position values (multiples of 25)
    ; R10 = 25
    CLR R10
    ADD R10, R14
    ADD R10, R14
    DEC R10
    DEC R10
    DEC R10
    DEC R10
    DEC R10
    DEC R10
    DEC R10      ; R10 = 25

    ; R20 = 50
    CLR R20
    ADD R20, R10
    ADD R20, R10  ; R20 = 50

    ; R30 = 75
    CLR R30
    ADD R30, R20
    ADD R30, R10  ; R30 = 75

    ; R40 = 100
    CLR R40
    ADD R40, R20
    ADD R40, R20  ; R40 = 100

    ; R50 = 150
    CLR R50
    ADD R50, R40
    ADD R50, R20  ; R50 = 150

    ; R51 = 200
    CLR R51
    ADD R51, R40
    ADD R51, R40  ; R51 = 200

    ; Green constant = 100
    CLR R2
    ADD R2, R40

    ; Helper macro: Draw pixel at (RX, RY) with color (RY, 100, RX, 255)
    ; We'll manually unroll this for several key points

    ; (0, 0)
    CLR R0
    CLR R3
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R2
    INC DP
    INC DP
    MOVDP R0
    INC DP
    INC DP
    MOVDP R6

    ; (50, 50) - subtle gradient
    SETDP R7
    MOVDP R20
    INC DP
    INC DP
    MOVDP R20
    INC DP
    INC DP
    MOVDP R20
    INC DP
    INC DP
    MOVDP R2
    INC DP
    INC DP
    MOVDP R20
    INC DP
    INC DP
    MOVDP R6

    ; (100, 50) - more blue
    SETDP R7
    MOVDP R40
    INC DP
    INC DP
    MOVDP R20
    INC DP
    INC DP
    MOVDP R20
    INC DP
    INC DP
    MOVDP R2
    INC DP
    INC DP
    MOVDP R40
    INC DP
    INC DP
    MOVDP R6

    ; (150, 50) - even more blue
    SETDP R7
    MOVDP R50
    INC DP
    INC DP
    MOVDP R20
    INC DP
    INC DP
    MOVDP R20
    INC DP
    INC DP
    MOVDP R2
    INC DP
    INC DP
    MOVDP R50
    INC DP
    INC DP
    MOVDP R6

    ; (200, 50) - very blue
    SETDP R7
    MOVDP R51
    INC DP
    INC DP
    MOVDP R20
    INC DP
    INC DP
    MOVDP R20
    INC DP
    INC DP
    MOVDP R2
    INC DP
    INC DP
    MOVDP R51
    INC DP
    INC DP
    MOVDP R6

    ; (50, 100) - more red
    SETDP R7
    MOVDP R20
    INC DP
    INC DP
    MOVDP R40
    INC DP
    INC DP
    MOVDP R40
    INC DP
    INC DP
    MOVDP R2
    INC DP
    INC DP
    MOVDP R20
    INC DP
    INC DP
    MOVDP R6

    ; (100, 100) - balanced
    SETDP R7
    MOVDP R40
    INC DP
    INC DP
    MOVDP R40
    INC DP
    INC DP
    MOVDP R40
    INC DP
    INC DP
    MOVDP R2
    INC DP
    INC DP
    MOVDP R40
    INC DP
    INC DP
    MOVDP R6

    ; (150, 100) - purple-ish
    SETDP R7
    MOVDP R50
    INC DP
    INC DP
    MOVDP R40
    INC DP
    INC DP
    MOVDP R40
    INC DP
    INC DP
    MOVDP R2
    INC DP
    INC DP
    MOVDP R50
    INC DP
    INC DP
    MOVDP R6

    ; (200, 100)
    SETDP R7
    MOVDP R51
    INC DP
    INC DP
    MOVDP R40
    INC DP
    INC DP
    MOVDP R40
    INC DP
    INC DP
    MOVDP R2
    INC DP
    INC DP
    MOVDP R51
    INC DP
    INC DP
    MOVDP R6

    ; (50, 150) - more red
    SETDP R7
    MOVDP R20
    INC DP
    INC DP
    MOVDP R50
    INC DP
    INC DP
    MOVDP R50
    INC DP
    INC DP
    MOVDP R2
    INC DP
    INC DP
    MOVDP R20
    INC DP
    INC DP
    MOVDP R6

    ; (100, 150)
    SETDP R7
    MOVDP R40
    INC DP
    INC DP
    MOVDP R50
    INC DP
    INC DP
    MOVDP R50
    INC DP
    INC DP
    MOVDP R2
    INC DP
    INC DP
    MOVDP R40
    INC DP
    INC DP
    MOVDP R6

    ; (150, 150)
    SETDP R7
    MOVDP R50
    INC DP
    INC DP
    MOVDP R50
    INC DP
    INC DP
    MOVDP R50
    INC DP
    INC DP
    MOVDP R2
    INC DP
    INC DP
    MOVDP R50
    INC DP
    INC DP
    MOVDP R6

    ; (200, 150)
    SETDP R7
    MOVDP R51
    INC DP
    INC DP
    MOVDP R50
    INC DP
    INC DP
    MOVDP R50
    INC DP
    INC DP
    MOVDP R2
    INC DP
    INC DP
    MOVDP R51
    INC DP
    INC DP
    MOVDP R6

    ; (50, 200) - very red
    SETDP R7
    MOVDP R20
    INC DP
    INC DP
    MOVDP R51
    INC DP
    INC DP
    MOVDP R51
    INC DP
    INC DP
    MOVDP R2
    INC DP
    INC DP
    MOVDP R20
    INC DP
    INC DP
    MOVDP R6

    ; (100, 200)
    SETDP R7
    MOVDP R40
    INC DP
    INC DP
    MOVDP R51
    INC DP
    INC DP
    MOVDP R51
    INC DP
    INC DP
    MOVDP R2
    INC DP
    INC DP
    MOVDP R40
    INC DP
    INC DP
    MOVDP R6

    ; (150, 200)
    SETDP R7
    MOVDP R50
    INC DP
    INC DP
    MOVDP R51
    INC DP
    INC DP
    MOVDP R51
    INC DP
    INC DP
    MOVDP R2
    INC DP
    INC DP
    MOVDP R50
    INC DP
    INC DP
    MOVDP R6

    ; (200, 200)
    SETDP R7
    MOVDP R51
    INC DP
    INC DP
    MOVDP R51
    INC DP
    INC DP
    MOVDP R51
    INC DP
    INC DP
    MOVDP R2
    INC DP
    INC DP
    MOVDP R51
    INC DP
    INC DP
    MOVDP R6

    HALT

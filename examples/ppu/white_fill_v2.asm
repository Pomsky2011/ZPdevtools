; White fill v2 - simpler approach using pixel I/O
; Fills the screen with white pixels one at a time

start:
    ; Y = 0
    CLR R0

y_loop:
    ; X = 0
    CLR R1

x_loop:
    ; Write pixel at (X, Y) = white (255, 255, 255, 255)
    ; Set DP = 0x0100 (X position)
    TARREG 2, LSB, DP
    TARREG 3, MSB, DP
    SETBYTE 2, 0x00
    SETBYTE 3, 0x01
    MOVDP R1      ; Write X

    ; Set DP = 0x0102 (Y position)
    SETBYTE 2, 0x02
    MOVDP R0      ; Write Y

    ; Set R10 = 255 (color value)
    TARREG 2, LSB, R10
    SETBYTE 2, 0xFF

    ; Set DP = 0x0104 (R)
    SETBYTE 2, 0x04
    SETBYTE 3, 0x01
    MOVDP R10

    ; Set DP = 0x0106 (G)
    SETBYTE 2, 0x06
    MOVDP R10

    ; Set DP = 0x0108 (B)
    SETBYTE 2, 0x08
    MOVDP R10

    ; Set DP = 0x010A (A) - triggers draw
    SETBYTE 2, 0x0A
    MOVDP R10

    ; X++
    INC R1

    ; If X < 256, continue
    TARREG 2, LSB, R20
    TARREG 3, MSB, R20
    SETBYTE 2, 0x00
    SETBYTE 3, 0x01    ; R20 = 256
    CMP R1, R20
    JMR x_continue
    JMG x_loop

x_continue:
    ; Y++
    INC R0

    ; If Y < 256, continue
    CMP R0, R20
    JMR y_continue
    JMG y_loop

y_continue:
    HLT

; Quick Gradient Test - Draws just 8×8 pixels for fast testing

R10 = Y
R11 = X
R15 = TEMP

start:
    ; Set 32-bit color mode
    SETRENDMOD 1

    ; Y = 0
    CLR Y

y_loop:
    ; X = 0
    CLR X

x_loop:
    ; Set DP to pixel I/O (0x0100)
    TARREG 2, LSB, DP
    TARREG 3, MSB, DP
    SETBYTE 2, 0x00
    SETBYTE 3, 0x01

    ; Write X position (scale by 32 for visibility)
    CLR TEMP
    ADD TEMP, X
    ; Multiply by 32
    ADD TEMP, TEMP
    ADD TEMP, TEMP
    ADD TEMP, TEMP
    ADD TEMP, TEMP
    ADD TEMP, TEMP
    MOVDP TEMP
    INC DP
    INC DP

    ; Write Y position (scale by 32 for visibility)
    CLR TEMP
    ADD TEMP, Y
    ; Multiply by 32
    ADD TEMP, TEMP
    ADD TEMP, TEMP
    ADD TEMP, TEMP
    ADD TEMP, TEMP
    ADD TEMP, TEMP
    MOVDP TEMP
    INC DP
    INC DP

    ; Red = X * 32
    CLR TEMP
    ADD TEMP, X
    ADD TEMP, TEMP
    ADD TEMP, TEMP
    ADD TEMP, TEMP
    ADD TEMP, TEMP
    ADD TEMP, TEMP
    MOVDP TEMP
    INC DP
    INC DP

    ; Green = Y * 32
    CLR TEMP
    ADD TEMP, Y
    ADD TEMP, TEMP
    ADD TEMP, TEMP
    ADD TEMP, TEMP
    ADD TEMP, TEMP
    ADD TEMP, TEMP
    MOVDP TEMP
    INC DP
    INC DP

    ; Blue = 128
    TARREG 2, LSB, TEMP
    SETBYTE 2, 128
    MOVDP TEMP
    INC DP
    INC DP

    ; Alpha = 255
    TARREG 2, LSB, TEMP
    SETBYTE 2, 255
    MOVDP TEMP

    ; Next X
    INC X
    TARREG 2, LSB, TEMP
    SETBYTE 2, 7
    CMP X, TEMP
    JNG x_loop      ; Jump if X <= 7 (i.e., X < 8)

    ; Next Y
    INC Y
    TARREG 2, LSB, TEMP
    SETBYTE 2, 7
    CMP Y, TEMP
    JNG y_loop      ; Jump if Y <= 7 (i.e., Y < 8)

    ; Done
    HLT

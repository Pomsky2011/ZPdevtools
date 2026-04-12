; Simple Gradient Demo - Draws horizontal gradient lines
; Each scanline: R=Y, G=X, B=255-Y
; Draws 64 lines then halts

R10 = CURRENT_Y
R11 = CURRENT_X
R12 = COLOR_R
R13 = COLOR_G
R14 = COLOR_B
R15 = TEMP

$0x0100 = IO_BASE

start:
    ; Set 32-bit color mode
    SETRENDMOD 1

    ; Initialize Y to 0
    CLR CURRENT_Y

draw_line:
    ; Initialize X to 0
    CLR CURRENT_X

draw_pixel:
    ; Calculate colors
    ; R = CURRENT_Y
    CLR COLOR_R
    ADD COLOR_R, CURRENT_Y

    ; G = CURRENT_X
    CLR COLOR_G
    ADD COLOR_G, CURRENT_X

    ; B = 255 - CURRENT_Y
    TARREG 2, LSB, TEMP
    SETBYTE 2, 255
    CLR COLOR_B
    ADD COLOR_B, TEMP
    SUB COLOR_B, CURRENT_Y

    ; Set up DP to pixel I/O (0x0100)
    TARREG 2, LSB, DP
    TARREG 3, MSB, DP
    SETBYTE 2, 0x00
    SETBYTE 3, 0x01

    ; Write X position
    MOVDP CURRENT_X
    INC DP
    INC DP

    ; Write Y position
    MOVDP CURRENT_Y
    INC DP
    INC DP

    ; Write Red
    MOVDP COLOR_R
    INC DP
    INC DP

    ; Write Green
    MOVDP COLOR_G
    INC DP
    INC DP

    ; Write Blue
    MOVDP COLOR_B
    INC DP
    INC DP

    ; Write Alpha = 255 (triggers pixel draw)
    TARREG 2, LSB, TEMP
    SETBYTE 2, 255
    MOVDP TEMP

    ; Increment X
    INC CURRENT_X

    ; Check if X < 256 (loop while X <= 255)
    TARREG 2, LSB, TEMP
    SETBYTE 2, 255
    CMP CURRENT_X, TEMP
    JNG draw_pixel      ; Jump if X <= 255

    ; Move to next line
    INC CURRENT_Y

    ; Check if Y < 64 (loop while Y <= 63)
    TARREG 2, LSB, TEMP
    SETBYTE 2, 63
    CMP CURRENT_Y, TEMP
    JNG draw_line       ; Jump if Y <= 63

    ; Done - halt
    HLT

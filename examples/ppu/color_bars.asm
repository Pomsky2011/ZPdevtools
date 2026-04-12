; Color Bars Demo
; Draws 8 vertical color bars (32 pixels wide each)
; Demonstrates: 32-bit color mode, loops, pixel I/O

R10 = BAR
R11 = Y
R12 = X_START
R13 = X
R14 = Y_POS
R15 = TEMP

start:
    ; Enable 32-bit color mode
    SETRENDMOD 1

    ; Bar counter (0-7)
    CLR BAR

bar_loop:
    ; Calculate X start position: BAR * 32
    CLR X_START
    ADD X_START, BAR
    ; Multiply by 32 (shift left 5 times)
    ADD X_START, X_START
    ADD X_START, X_START
    ADD X_START, X_START
    ADD X_START, X_START
    ADD X_START, X_START

    ; Y position counter
    CLR Y

y_loop:
    ; X position counter (draw 32 pixels wide)
    CLR X

x_loop:
    ; Calculate absolute X position
    CLR TEMP
    ADD TEMP, X_START
    ADD TEMP, X

    ; Set DP to pixel I/O base (0x0100)
    TARREG 2, LSB, DP
    TARREG 3, MSB, DP
    SETBYTE 2, 0x00
    SETBYTE 3, 0x01

    ; Write X position (0x0100)
    MOVDP TEMP
    INC DP
    INC DP

    ; Write Y position (0x0102)
    MOVDP Y
    INC DP
    INC DP

    ; Write color based on BAR number
    ; Red channel (0x0104)
    CLR TEMP
    ; If BAR & 1, set red to 255
    CLR Y_POS  ; Use as temp
    ADD Y_POS, BAR
    ; Check if odd (can't do bitwise AND, so check remainder)
    ; For simplicity: bar 0,2,4,6 = no red; bar 1,3,5,7 = red
    ; We'll just make different colors for each bar
    CMP BAR, R0  ; BAR == 0?
    ; This is complex, let's just use BAR * 32 as color
    CLR TEMP
    ADD TEMP, BAR
    ADD TEMP, TEMP
    ADD TEMP, TEMP
    ADD TEMP, TEMP
    ADD TEMP, TEMP
    ADD TEMP, TEMP  ; TEMP = BAR * 32
    MOVDP TEMP
    INC DP
    INC DP

    ; Green channel (0x0106) - inverse of red
    TARREG 2, LSB, TEMP
    SETBYTE 2, 255
    SUB TEMP, BAR
    SUB TEMP, BAR
    SUB TEMP, BAR
    SUB TEMP, BAR
    SUB TEMP, BAR
    MOVDP TEMP
    INC DP
    INC DP

    ; Blue channel (0x0108)
    TARREG 2, LSB, TEMP
    SETBYTE 2, 128
    MOVDP TEMP
    INC DP
    INC DP

    ; Alpha channel (0x010A) - triggers draw
    TARREG 2, LSB, TEMP
    SETBYTE 2, 255
    MOVDP TEMP

    ; Next X
    INC X
    TARREG 2, LSB, TEMP
    SETBYTE 2, 31
    CMP X, TEMP
    JNG x_loop

    ; Next Y
    INC Y
    TARREG 2, LSB, TEMP
    SETBYTE 2, 63
    CMP Y, TEMP
    JNG y_loop

    ; Next bar
    INC BAR
    TARREG 2, LSB, TEMP
    SETBYTE 2, 7
    CMP BAR, TEMP
    JNG bar_loop

    ; Done!
    HLT

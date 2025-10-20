; Clean Color Bars Demo
; Draws 8 vertical color bars (32 pixels wide each)
; Each bar has a different color based on its index

R10 = BAR_NUM
R11 = Y_POS
R12 = X_POS
R15 = TEMP

start:
    ; Enable 32-bit color mode
    SETRENDMOD 1

    ; Initialize BAR counter (0-7)
    CLR BAR_NUM

bar_loop:
    ; Y loop: 0-31 (draw 32 pixels tall)
    CLR Y_POS

y_loop:
    ; X loop: 0-31 (draw 32 pixels wide per bar)
    CLR X_POS

x_loop:
    ; Calculate absolute X: BAR_NUM * 32 + X_POS
    CLR TEMP
    ADD TEMP, BAR_NUM
    ; Multiply BAR_NUM by 32
    ADD TEMP, TEMP
    ADD TEMP, TEMP
    ADD TEMP, TEMP
    ADD TEMP, TEMP
    ADD TEMP, TEMP
    ADD TEMP, X_POS  ; Add X offset

    ; Set DP to pixel I/O (0x0100)
    TARREG 2, LSB, DP
    TARREG 3, MSB, DP
    SETBYTE 2, 0x00
    SETBYTE 3, 0x01

    ; Write X position (0x0100)
    MOVDP TEMP
    INC DP
    INC DP

    ; Write Y position (0x0102)
    MOVDP Y_POS
    INC DP
    INC DP

    ; Red = BAR_NUM * 32
    CLR TEMP
    ADD TEMP, BAR_NUM
    ADD TEMP, TEMP
    ADD TEMP, TEMP
    ADD TEMP, TEMP
    ADD TEMP, TEMP
    ADD TEMP, TEMP
    MOVDP TEMP
    INC DP
    INC DP

    ; Green = 255 - (BAR_NUM * 32)
    TARREG 2, LSB, TEMP
    SETBYTE 2, 255
    CLR R0  ; Use R0 as temp
    ADD R0, BAR_NUM
    ADD R0, R0
    ADD R0, R0
    ADD R0, R0
    ADD R0, R0
    ADD R0, R0
    SUB TEMP, R0
    MOVDP TEMP
    INC DP
    INC DP

    ; Blue = 128 (constant)
    TARREG 2, LSB, TEMP
    SETBYTE 2, 128
    MOVDP TEMP
    INC DP
    INC DP

    ; Alpha = 255 (triggers draw)
    TARREG 2, LSB, TEMP
    SETBYTE 2, 255
    MOVDP TEMP

    ; Next X
    INC X_POS
    TARREG 2, LSB, TEMP
    SETBYTE 2, 31
    CMP X_POS, TEMP
    JNG x_loop     ; Loop while X_POS <= 31

    ; Next Y
    INC Y_POS
    TARREG 2, LSB, TEMP
    SETBYTE 2, 31
    CMP Y_POS, TEMP
    JNG y_loop     ; Loop while Y_POS <= 31

    ; Next bar
    INC BAR_NUM
    TARREG 2, LSB, TEMP
    SETBYTE 2, 7
    CMP BAR_NUM, TEMP
    JNG bar_loop   ; Loop while BAR_NUM <= 7

    ; All done!
    HLT

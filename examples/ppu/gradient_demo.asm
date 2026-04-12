; ZeroPoint PPU Full Screen Gradient Demo
; Fills entire 256x256 screen with gradients:
; - Blue increases horizontally (X axis: 0-255)
; - Red increases vertically (Y axis: 0-255)
; - Green is constant at 128

init:
    ; Set 32-bit render mode
    SETRENDMOD 1

    ; Build R15 = 16 (using it as temp, will need it for building other values)
    CLR R15
    INC R15
    INC R15          ; R15 = 2
    MUL R15, R15     ; R15 = 4
    MUL R15, R15     ; R15 = 16

    ; Build R6 = 255 (loop limit)
    CLR R6
    ADD R6, R15
    MUL R6, R15      ; R6 = 256
    CLR R14
    INC R14          ; R14 = 1
    SUB R6, R14      ; R6 = 255

    ; Build R7 = 256 (I/O base address)
    CLR R7
    ADD R7, R6
    INC R7           ; R7 = 256

    ; Build R2 = 128 (green value) = 8 * 16
    CLR R3
    ADD R3, R15      ; R3 = 16
    CLR R8
    INC R8           ; R8 = 1
    ADD R8, R8       ; R8 = 2
    MUL R8, R8       ; R8 = 4
    MUL R8, R8       ; R8 = 16
    DEC R8
    DEC R8
    DEC R8
    DEC R8
    DEC R8
    DEC R8
    DEC R8
    DEC R8           ; R8 = 8
    CLR R2
    ADD R2, R8
    MUL R2, R15      ; R2 = 8 * 16 = 128

    ; Initialize Y counter
    CLR R1           ; R1 = Y (0-255)

y_loop:
    ; Initialize X counter
    CLR R0           ; R0 = X (0-255)

x_loop:
    ; Set DP = 0x0100 (I/O base address)
    CLR DP
    ADD DP, R7

    ; Write X position (0x0100-0x0101)
    MOVDP R0
    INC DP
    INC DP

    ; Write Y position (0x0102-0x0103)
    MOVDP R1
    INC DP
    INC DP

    ; Write Red = Y (0x0104-0x0105)
    MOVDP R1
    INC DP
    INC DP

    ; Write Green = 128 (0x0106-0x0107)
    MOVDP R2
    INC DP
    INC DP

    ; Write Blue = X (0x0108-0x0109)
    MOVDP R0
    INC DP
    INC DP

    ; Write Alpha = 255 (0x010A-0x010B) - triggers draw
    MOVDP R6

    ; Increment X
    INC R0
    CMP R0, R6       ; Compare X with 255
    JNG x_loop       ; Jump if X <= 255 (runs for X=0-255, exits when X=256)

    ; Increment Y
    INC R1
    CMP R1, R6       ; Compare Y with 255
    JNG y_loop       ; Jump if Y <= 255 (runs for Y=0-255, exits when Y=256)

    ; Done - filled entire screen
    HALT

; Auto-generated gradient grid demo
; Draws 8x8 grid of pixels (every 32 units) with gradient colors

init:
    SETRENDMOD 1

    ; Build R14 = 16, R7 = 256
    CLR R14
    INC R14
    INC R14
    MUL R14, R14
    MUL R14, R14
    CLR R7
    ADD R7, R14
    MUL R7, R14

    ; Build R6 = 255
    CLR R1
    INC R1
    CLR R6
    ADD R6, R14
    MUL R6, R14
    SUB R6, R1

    ; Green = 128
    CLR R2
    ADD R2, R14
    DEC R2
    DEC R2
    DEC R2
    DEC R2
    DEC R2
    DEC R2
    DEC R2
    DEC R2
    MUL R2, R14

    ; Pixel at (0, 0) - Red=0, Blue=0
    CLR R0
    CLR R3
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (32, 0) - Red=0, Blue=32
    CLR R0
    ADD R0, R14
    ADD R0, R14
    CLR R3
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (64, 0) - Red=0, Blue=64
    CLR R0
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    CLR R3
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (96, 0) - Red=0, Blue=96
    CLR R0
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    CLR R3
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (128, 0) - Red=0, Blue=128
    CLR R0
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    CLR R3
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (160, 0) - Red=0, Blue=160
    CLR R0
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    CLR R3
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (192, 0) - Red=0, Blue=192
    CLR R0
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    CLR R3
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (224, 0) - Red=0, Blue=224
    CLR R0
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    CLR R3
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (0, 32) - Red=32, Blue=0
    CLR R0
    CLR R3
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (32, 32) - Red=32, Blue=32
    CLR R0
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (64, 32) - Red=32, Blue=64
    CLR R0
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (96, 32) - Red=32, Blue=96
    CLR R0
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (128, 32) - Red=32, Blue=128
    CLR R0
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (160, 32) - Red=32, Blue=160
    CLR R0
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (192, 32) - Red=32, Blue=192
    CLR R0
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (224, 32) - Red=32, Blue=224
    CLR R0
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (0, 64) - Red=64, Blue=0
    CLR R0
    CLR R3
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (32, 64) - Red=64, Blue=32
    CLR R0
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (64, 64) - Red=64, Blue=64
    CLR R0
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (96, 64) - Red=64, Blue=96
    CLR R0
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (128, 64) - Red=64, Blue=128
    CLR R0
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (160, 64) - Red=64, Blue=160
    CLR R0
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (192, 64) - Red=64, Blue=192
    CLR R0
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (224, 64) - Red=64, Blue=224
    CLR R0
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (0, 96) - Red=96, Blue=0
    CLR R0
    CLR R3
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (32, 96) - Red=96, Blue=32
    CLR R0
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (64, 96) - Red=96, Blue=64
    CLR R0
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (96, 96) - Red=96, Blue=96
    CLR R0
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (128, 96) - Red=96, Blue=128
    CLR R0
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (160, 96) - Red=96, Blue=160
    CLR R0
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (192, 96) - Red=96, Blue=192
    CLR R0
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (224, 96) - Red=96, Blue=224
    CLR R0
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (0, 128) - Red=128, Blue=0
    CLR R0
    CLR R3
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (32, 128) - Red=128, Blue=32
    CLR R0
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (64, 128) - Red=128, Blue=64
    CLR R0
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (96, 128) - Red=128, Blue=96
    CLR R0
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (128, 128) - Red=128, Blue=128
    CLR R0
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (160, 128) - Red=128, Blue=160
    CLR R0
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (192, 128) - Red=128, Blue=192
    CLR R0
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (224, 128) - Red=128, Blue=224
    CLR R0
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (0, 160) - Red=160, Blue=0
    CLR R0
    CLR R3
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (32, 160) - Red=160, Blue=32
    CLR R0
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (64, 160) - Red=160, Blue=64
    CLR R0
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (96, 160) - Red=160, Blue=96
    CLR R0
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (128, 160) - Red=160, Blue=128
    CLR R0
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (160, 160) - Red=160, Blue=160
    CLR R0
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (192, 160) - Red=160, Blue=192
    CLR R0
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (224, 160) - Red=160, Blue=224
    CLR R0
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (0, 192) - Red=192, Blue=0
    CLR R0
    CLR R3
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (32, 192) - Red=192, Blue=32
    CLR R0
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (64, 192) - Red=192, Blue=64
    CLR R0
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (96, 192) - Red=192, Blue=96
    CLR R0
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (128, 192) - Red=192, Blue=128
    CLR R0
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (160, 192) - Red=192, Blue=160
    CLR R0
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (192, 192) - Red=192, Blue=192
    CLR R0
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (224, 192) - Red=192, Blue=224
    CLR R0
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (0, 224) - Red=224, Blue=0
    CLR R0
    CLR R3
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (32, 224) - Red=224, Blue=32
    CLR R0
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (64, 224) - Red=224, Blue=64
    CLR R0
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (96, 224) - Red=224, Blue=96
    CLR R0
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (128, 224) - Red=224, Blue=128
    CLR R0
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (160, 224) - Red=224, Blue=160
    CLR R0
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (192, 224) - Red=224, Blue=192
    CLR R0
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    ; Pixel at (224, 224) - Red=224, Blue=224
    CLR R0
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    ADD R0, R14
    CLR R3
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    ADD R3, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R3  ; Red = Y
    INC DP
    INC DP
    MOVDP R2  ; Green = 128
    INC DP
    INC DP
    MOVDP R0  ; Blue = X
    INC DP
    INC DP
    MOVDP R6  ; Alpha = 255

    HALT

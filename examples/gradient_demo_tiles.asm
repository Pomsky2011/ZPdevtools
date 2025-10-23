; Gradient demo with 8x8 "tiles" - much more visible!
; Draws 8x8 pixel blocks across the screen to show gradient clearly

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

    ; Build tile size = 8
    CLR R8
    ADD R8, R14
    DEC R8
    DEC R8
    DEC R8
    DEC R8
    DEC R8
    DEC R8
    DEC R8
    DEC R8       ; R8 = 8

    ; Green constant = 128
    CLR R2
    ADD R2, R8
    MUL R2, R14  ; R2 = 8 * 16 = 128

    ; I'll draw 8x8 tiles at grid positions
    ; Tile colors: (tile_x * 32, tile_y * 32) for gradient effect
    ; Let me draw a 6x6 grid of tiles

    ; Helper: R10-R15 will hold multiplication results
    ; R20 = base X, R21 = base Y, R22 = color R, R23 = color B

    ; Tile at (0, 0) - size 8x8, color (0, 128, 0)
    CLR R20      ; base X = 0
    CLR R21      ; base Y = 0
    CLR R22      ; red = 0
    CLR R23      ; blue = 0
    ; Draw 8x8 block
    ; Row 0
    SETDP R7
    MOVDP R20
    INC DP
    INC DP
    MOVDP R21
    INC DP
    INC DP
    MOVDP R22
    INC DP
    INC DP
    MOVDP R2
    INC DP
    INC DP
    MOVDP R23
    INC DP
    INC DP
    MOVDP R6
    ; Continue for x=1
    CLR R10
    ADD R10, R20
    INC R10
    SETDP R7
    MOVDP R10
    INC DP
    INC DP
    MOVDP R21
    INC DP
    INC DP
    MOVDP R22
    INC DP
    INC DP
    MOVDP R2
    INC DP
    INC DP
    MOVDP R23
    INC DP
    INC DP
    MOVDP R6
    ; x=2
    CLR R10
    ADD R10, R20
    INC R10
    INC R10
    SETDP R7
    MOVDP R10
    INC DP
    INC DP
    MOVDP R21
    INC DP
    INC DP
    MOVDP R22
    INC DP
    INC DP
    MOVDP R2
    INC DP
    INC DP
    MOVDP R23
    INC DP
    INC DP
    MOVDP R6
    ; x=3
    CLR R10
    ADD R10, R20
    INC R10
    INC R10
    INC R10
    SETDP R7
    MOVDP R10
    INC DP
    INC DP
    MOVDP R21
    INC DP
    INC DP
    MOVDP R22
    INC DP
    INC DP
    MOVDP R2
    INC DP
    INC DP
    MOVDP R23
    INC DP
    INC DP
    MOVDP R6
    ; x=4
    CLR R10
    ADD R10, R20
    INC R10
    INC R10
    INC R10
    INC R10
    SETDP R7
    MOVDP R10
    INC DP
    INC DP
    MOVDP R21
    INC DP
    INC DP
    MOVDP R22
    INC DP
    INC DP
    MOVDP R2
    INC DP
    INC DP
    MOVDP R23
    INC DP
    INC DP
    MOVDP R6
    ; x=5
    CLR R10
    ADD R10, R20
    INC R10
    INC R10
    INC R10
    INC R10
    INC R10
    SETDP R7
    MOVDP R10
    INC DP
    INC DP
    MOVDP R21
    INC DP
    INC DP
    MOVDP R22
    INC DP
    INC DP
    MOVDP R2
    INC DP
    INC DP
    MOVDP R23
    INC DP
    INC DP
    MOVDP R6
    ; x=6
    CLR R10
    ADD R10, R20
    INC R10
    INC R10
    INC R10
    INC R10
    INC R10
    INC R10
    SETDP R7
    MOVDP R10
    INC DP
    INC DP
    MOVDP R21
    INC DP
    INC DP
    MOVDP R22
    INC DP
    INC DP
    MOVDP R2
    INC DP
    INC DP
    MOVDP R23
    INC DP
    INC DP
    MOVDP R6
    ; x=7
    CLR R10
    ADD R10, R20
    INC R10
    INC R10
    INC R10
    INC R10
    INC R10
    INC R10
    INC R10
    SETDP R7
    MOVDP R10
    INC DP
    INC DP
    MOVDP R21
    INC DP
    INC DP
    MOVDP R22
    INC DP
    INC DP
    MOVDP R2
    INC DP
    INC DP
    MOVDP R23
    INC DP
    INC DP
    MOVDP R6

    ; Row 1 (y=1)
    CLR R10
    ADD R10, R21
    INC R10      ; y = 1
    ; x=0-7
    SETDP R7
    MOVDP R20
    INC DP
    INC DP
    MOVDP R10
    INC DP
    INC DP
    MOVDP R22
    INC DP
    INC DP
    MOVDP R2
    INC DP
    INC DP
    MOVDP R23
    INC DP
    INC DP
    MOVDP R6
    ; x=1
    CLR R11
    ADD R11, R20
    INC R11
    SETDP R7
    MOVDP R11
    INC DP
    INC DP
    MOVDP R10
    INC DP
    INC DP
    MOVDP R22
    INC DP
    INC DP
    MOVDP R2
    INC DP
    INC DP
    MOVDP R23
    INC DP
    INC DP
    MOVDP R6
    ; x=2
    CLR R11
    ADD R11, R20
    INC R11
    INC R11
    SETDP R7
    MOVDP R11
    INC DP
    INC DP
    MOVDP R10
    INC DP
    INC DP
    MOVDP R22
    INC DP
    INC DP
    MOVDP R2
    INC DP
    INC DP
    MOVDP R23
    INC DP
    INC DP
    MOVDP R6
    ; x=3
    CLR R11
    ADD R11, R20
    INC R11
    INC R11
    INC R11
    SETDP R7
    MOVDP R11
    INC DP
    INC DP
    MOVDP R10
    INC DP
    INC DP
    MOVDP R22
    INC DP
    INC DP
    MOVDP R2
    INC DP
    INC DP
    MOVDP R23
    INC DP
    INC DP
    MOVDP R6
    ; x=4
    CLR R11
    ADD R11, R20
    INC R11
    INC R11
    INC R11
    INC R11
    SETDP R7
    MOVDP R11
    INC DP
    INC DP
    MOVDP R10
    INC DP
    INC DP
    MOVDP R22
    INC DP
    INC DP
    MOVDP R2
    INC DP
    INC DP
    MOVDP R23
    INC DP
    INC DP
    MOVDP R6
    ; x=5
    CLR R11
    ADD R11, R20
    INC R11
    INC R11
    INC R11
    INC R11
    INC R11
    SETDP R7
    MOVDP R11
    INC DP
    INC DP
    MOVDP R10
    INC DP
    INC DP
    MOVDP R22
    INC DP
    INC DP
    MOVDP R2
    INC DP
    INC DP
    MOVDP R23
    INC DP
    INC DP
    MOVDP R6
    ; x=6
    CLR R11
    ADD R11, R20
    INC R11
    INC R11
    INC R11
    INC R11
    INC R11
    INC R11
    SETDP R7
    MOVDP R11
    INC DP
    INC DP
    MOVDP R10
    INC DP
    INC DP
    MOVDP R22
    INC DP
    INC DP
    MOVDP R2
    INC DP
    INC DP
    MOVDP R23
    INC DP
    INC DP
    MOVDP R6
    ; x=7
    CLR R11
    ADD R11, R20
    INC R11
    INC R11
    INC R11
    INC R11
    INC R11
    INC R11
    INC R11
    SETDP R7
    MOVDP R11
    INC DP
    INC DP
    MOVDP R10
    INC DP
    INC DP
    MOVDP R22
    INC DP
    INC DP
    MOVDP R2
    INC DP
    INC DP
    MOVDP R23
    INC DP
    INC DP
    MOVDP R6

    ; This is going to be HUGE... let me draw just a few key tiles instead
    ; Actually, let me draw larger 32x32 blocks at strategic positions

    HALT

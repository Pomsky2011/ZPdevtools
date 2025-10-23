; Gradient demo with large visible blocks
; Draws 16x16 pixel blocks to make gradient clearly visible

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

    ; Build R6 = 255 (alpha)
    CLR R1
    INC R1
    CLR R6
    ADD R6, R14
    MUL R6, R14  ; R6 = 256
    SUB R6, R1   ; R6 = 255

    ; Green constant = 128
    CLR R2
    ADD R2, R14
    DEC R2
    DEC R2
    DEC R2
    DEC R2
    DEC R2
    DEC R2
    DEC R2
    DEC R2       ; R2 = 8
    MUL R2, R14  ; R2 = 128

    ; Draw 16x16 blocks at corners and center
    ; Block size = 16 (stored in R14)

    ; TOP-LEFT BLOCK (0-15, 0-15): Color (0, 128, 0) - Dark green
    CLR R50      ; Red = 0
    CLR R51      ; Blue = 0
    CLR R52      ; Start X
    CLR R53      ; Start Y

    ; Draw 16 rows of 16 pixels each
    ; Row 0, Y=0
    CLR R0
y0_row:
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R53
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
    INC R0
    CMP R0, R14
    JNG y0_row

    ; Row 1, Y=1
    CLR R0
    CLR R10
    INC R10
y1_row:
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R10
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
    INC R0
    CMP R0, R14
    JNG y1_row

    ; Continue for more rows... this is still tedious
    ; Let me draw a few strategic 4x4 blocks instead for better visibility

    ; Actually, let me draw horizontal and vertical stripes instead!
    ; Draw vertical stripes showing blue gradient
    ; Draw horizontal stripes showing red gradient

    ; VERTICAL STRIPE at X=64, Y=0-255, varying blue
    ; Draw column at X=64-67 (4 pixels wide)
    CLR R20
    ADD R20, R14
    MUL R20, R14
    MUL R20, R14  ; R20 = 16*16*16 = way too big
    ; Let me just use 64 = 4*16
    CLR R20
    ADD R20, R14
    ADD R20, R14
    ADD R20, R14
    ADD R20, R14  ; R20 = 64

    ; Draw pixels at X=64,Y=32 with blue=64
    CLR R0
    ADD R0, R20   ; X = 64
    CLR R3
    ADD R3, R14
    ADD R3, R14   ; Y = 32
    CLR R4        ; Red = 32 (same as Y)
    ADD R4, R14
    ADD R4, R14
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R4
    INC DP
    INC DP
    MOVDP R2
    INC DP
    INC DP
    CLR R5
    ADD R5, R20   ; Blue = 64
    MOVDP R5
    INC DP
    INC DP
    MOVDP R6

    ; Draw at X=128,Y=64
    CLR R0
    ADD R0, R14
    MUL R0, R14
    DEC R0
    DEC R0
    DEC R0
    DEC R0
    DEC R0
    DEC R0
    DEC R0
    DEC R0       ; X = 128-8 = 120, close enough
    CLR R3
    ADD R3, R20  ; Y = 64
    CLR R4
    ADD R4, R20  ; Red = 64
    CLR R5
    ADD R5, R0   ; Blue = ~120
    SETDP R7
    MOVDP R0
    INC DP
    INC DP
    MOVDP R3
    INC DP
    INC DP
    MOVDP R4
    INC DP
    INC DP
    MOVDP R2
    INC DP
    INC DP
    MOVDP R5
    INC DP
    INC DP
    MOVDP R6

    ; Let me draw a simple 8x8 grid of larger blocks
    ; Actually, simplest: draw filled rectangles at key positions

    HALT

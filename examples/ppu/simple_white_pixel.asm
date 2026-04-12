; Simple white pixel test - write a few white pixels using memory-mapped I/O
; Uses the pixel drawing interface at $0100-$010A

start:
    ; Initialize SP
    TARREG 2, LSB, SP
    TARREG 3, MSB, SP
    SETBYTE 2, 0xFF
    SETBYTE 3, 0xFF      ; SP = 0xFFFF

    ; Draw white pixel at (10, 10)
    ; Set X = 10
    TARREG 2, LSB, DP
    TARREG 3, MSB, DP
    SETBYTE 2, 0x00
    SETBYTE 3, 0x01      ; DP = 0x0100 (X position)

    TARREG 2, LSB, R5
    SETBYTE 2, 0x0A      ; R5 = 10
    MOVDP R5             ; Write X=10

    ; Set Y = 10
    TARREG 2, LSB, DP
    SETBYTE 2, 0x02
    SETBYTE 3, 0x01      ; DP = 0x0102 (Y position)
    MOVDP R5             ; Write Y=10

    ; Set R = 255
    TARREG 2, LSB, R6
    SETBYTE 2, 0xFF      ; R6 = 255
    TARREG 2, LSB, DP
    SETBYTE 2, 0x04
    SETBYTE 3, 0x01      ; DP = 0x0104 (R)
    MOVDP R6

    ; Set G = 255
    TARREG 2, LSB, DP
    SETBYTE 2, 0x06      ; DP = 0x0106 (G)
    MOVDP R6

    ; Set B = 255
    TARREG 2, LSB, DP
    SETBYTE 2, 0x08      ; DP = 0x0108 (B)
    MOVDP R6

    ; Set A = 255 (triggers pixel draw)
    TARREG 2, LSB, DP
    SETBYTE 2, 0x0A      ; DP = 0x010A (A)
    MOVDP R6             ; Triggers draw

    HLT

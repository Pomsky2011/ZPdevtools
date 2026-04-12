; Test ADD instruction
; Set R63 = 0xE000, R1 = 2
; Execute ADD R63, R1
; Expected: R63 = 0xE002

start:
    ; Set R63 (DP) = 0xE000
    TARREG 2, LSB, DP
    TARREG 3, MSB, DP
    SETBYTE 2, 0x00
    SETBYTE 3, 0xE0

    ; Set R1 = 2
    TARREG 2, LSB, R1
    SETBYTE 2, 0x02
    TARREG 3, MSB, R1
    SETBYTE 3, 0x00

    ; ADD DP, R1
    ADD DP, R1

    ; DP should now be 0xE002
    ; Write result to memory for verification
    ; Store DP value in R10
    TARREG 2, LSB, R10
    TARREG 3, MSB, R10
    SETBYTE 2, 0x00
    SETBYTE 3, 0x00
    BUILD 3, 2, R10

    ; Copy DP to R10
    SWAPREG R10, DP
    SWAPREG R10, DP

    HLT

; Memory operations example
; Write value to memory and read it back

    CLR R0
    CLR R1
    CLR R2

    ; Set R0 = 16 (memory address)
    INC R0
    INC R0
    INC R0
    INC R0
    INC R0
    INC R0
    INC R0
    INC R0
    INC R0
    INC R0
    INC R0
    INC R0
    INC R0
    INC R0
    INC R0
    INC R0

    ; Set DP from R0
    SETDP R0

    ; Write R1 to (DP)
    MOVDP R1

    ; Clear R2
    CLR R2

    ; Read from (DP) to R2
    MOV R2

    HALT

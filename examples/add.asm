; Simple addition example
; R2 = R0 + R1

    CLR R0
    CLR R1
    CLR R2

    ; R0 = 5
    INC R0
    INC R0
    INC R0
    INC R0
    INC R0

    ; R1 = 10
    INC R1
    INC R1
    INC R1
    INC R1
    INC R1
    INC R1
    INC R1
    INC R1
    INC R1
    INC R1

    ; R2 = R0 + R1
    ADD R2, R0
    ADD R2, R1

    HALT

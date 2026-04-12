; Count from 0 to 5 using comparison and jumps
; R0 = counter
; R1 = target (5)
; PC = jump target

    CLR R0
    CLR R1

    ; Set R1 = 5
    INC R1
    INC R1
    INC R1
    INC R1
    INC R1

    ; Set PC = loop address (16 bytes from start)
    CLR PC
    INC PC
    INC PC
    INC PC
    INC PC
    INC PC
    INC PC
    INC PC
    INC PC
    INC PC
    INC PC
    INC PC
    INC PC
    INC PC
    INC PC
    INC PC
    INC PC

loop:
    INC R0
    CMP R0, R1
    JNG        ; Jump to PC if R0 <= R1

    HALT

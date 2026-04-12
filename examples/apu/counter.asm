; Counter - APU Test Program
; Count from 0 to 10 using hardware loops

start:
    ; Zero out X register
    ZOR X

    ; Loop 10 times
    LST 0, 10

loop:
    ; Increment X
    SCR Y, 1
    ADD X, Y, X

    LFN 0

    ; X should now be 10
    ; Halt
    HLT

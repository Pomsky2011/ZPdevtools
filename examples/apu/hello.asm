; Hello World - APU Test Program
; Simple program that sets registers and halts

start:
    ; Set X register to 42
    SCR X, 42

    ; Set Y register to 100
    SCR Y, 100

    ; Add them together into X
    ADD X, Y, X

    ; Halt
    HLT

; init code

CLR R0  ; 0x0000
CLR R1  ; 0x0002
INC R1  ; 0x0004
CALL 1  ; Jump to Pythagorean Call, 0x0006

CLR PC ; 0x0008
INC PC
INC PC
INC PC
INC PC
INC PC
INC PC
JMR     ; Return to 0x0006


DEFCALL 1   ; Pythagorean Function
    ADD R0, R1  ; R0 += R1
    SWAPREG R0, R1
ENDDEFCALL
; Test new DEFCALL/ENDDEFCALL syntax

start:
    ; Set up function address in R10
    TARREG 2, LSB, R10
    TARREG 3, MSB, R10
    SETBYTE 2, my_function
    SETBYTE 3, my_function

    ; Set up function ID in R11
    TARREG 2, LSB, R11
    SETBYTE 2, 42           ; Function ID = 42

    ; Define the call
    DEFCALL R10, R11

my_function:
    INC R0
    ENDDEFCALL R11          ; End definition for ID 42

    HLT

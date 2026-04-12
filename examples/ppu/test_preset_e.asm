; Test Preset E instructions and new shorthands
; This tests TARREG, SETBYTE, HLT, and label-based jumps

; Define some aliases
$0x0100 = IO_BASE
R10 = TEST_REG

init:
    ; Test 1: Load a 16-bit constant using TARREG + SETBYTE
    ; Load 0x1234 into R5
    TARREG 0, LSB, R5      ; Target register 0 points to R5's LSB
    TARREG 1, MSB, R5      ; Target register 1 points to R5's MSB
    SETBYTE 0, 0x34        ; Set LSB to 0x34
    SETBYTE 1, 0x12        ; Set MSB to 0x12
    ; R5 now contains 0x1234

    ; Test 2: Use alias
    CLR TEST_REG           ; Clear R10 using alias

    ; Test 3: Use label-based jump (should expand to TARREG + SETBYTE + JMR)
    JMR end_test

middle:
    ; This should be skipped
    INC R0

end_test:
    ; Test 4: Use HLT shorthand (expands to infinite loop)
    HLT

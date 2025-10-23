; math_lib.asm - Reusable math routines for DEF88186

; Add function: adds A and value at $10, result in A
add_func:
    CLC
    ADC $10
    RTS

; Subtract function: subtracts value at $10 from A, result in A
sub_func:
    SEC
    SBC $10
    RTS

; Multiply by 2: shifts A left
mul2:
    ASL A
    RTS

; Divide by 2: shifts A right
div2:
    LSR A
    RTS

; Constants for use in main program
.byte 10, 20, 30, 40

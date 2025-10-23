; main_multifile.asm - Main program using multiple includes
; Demonstrates multi-file assembly with .include directive

.org $8000

; Include library files
.include "math_lib.asm"
.include "hardware.asm"

; Main program entry point
main:
    REP #$30        ; 16-bit mode

    ; Test math library
    LDA #$0042
    LDX #$0008
    JSR add_func    ; A = A + X = 0x42 + 0x08 = 0x4A

    ; Store result
    STA $0010

    ; Test hardware library
    LDA #$FF
    JSR write_screen

    ; Test multiply by 2
    LDA #$0005
    JSR mul2        ; A = 0x0A

    ; Initialize interrupt system
    JSR disable_irq
    ; ... setup code ...
    JSR enable_irq

    ; Main loop
loop:
    JSR read_input
    CMP #$00
    BEQ loop

    ; Exit - infinite loop
    JMP loop

; Interrupt vectors (if needed)
.org $FFFE
.word main

; DEF88186 Test Program
; Simple program to test the assembler

.org $8000

reset:
    ; Initialize processor
    SEI                 ; Disable interrupts
    CLD                 ; Binary mode
    REP #$30            ; 16-bit A, X, Y

    ; Set up stack
    LDA #$1FFF
    ; TCS would go here (transfer to stack)

    ; Set data bank to 0
    SDB #$00

    ; Simple arithmetic test
    LDA #$1234          ; Load immediate 16-bit
    LDX #$0010          ; Load X
    ADC #$0100          ; Add immediate
    STA $2000           ; Store to memory

    ; Loop test using hardware loop
    LDX #$0000
    LOOP #100
loop_body:
    LDA data_table,X
    STA output,X
    INX
    INX
    LPEND

    ; Hardware multiply test
    LDA #50
    MUL #10             ; 50 * 10 = 500
    STA result

    ; Jump to main
    JMP main

main:
    ; Main loop
    NOP
    NOP
    JMP main

; Data section
data_table:
    .word $0001, $0002, $0003, $0004
    .word $0005, $0006, $0007, $0008

output:
    .word $0000, $0000, $0000, $0000
    .word $0000, $0000, $0000, $0000

result:
    .word $0000

; Interrupt vectors (at end of bank)
.org $FFFC
    .word reset         ; RESET vector
    .word $0000         ; IRQ vector

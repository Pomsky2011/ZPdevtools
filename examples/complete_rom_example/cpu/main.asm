; main.asm - CPU Main Program
; This is the entry point for the ZeroPoint system

.org $8000

; Entry point (referenced in ROM metadata)
main:
    ; Initialize DEF88186 CPU
    REP #$30            ; Set 16-bit mode for A, X, Y

    ; Initialize stack pointer
    LDA #$1FFF
    TCS                 ; Stack at 0x1FFF

    ; Clear accumulator
    LDA #$0000
    STA $0010           ; Store at 0x0010

    ; Initialize counter
    LDX #$0000

; Main loop
game_loop:
    ; Increment counter
    INX
    TXA
    STA $0012           ; Store counter

    ; Simple computation
    LDA $0010
    CLC
    ADC #$0001
    STA $0010

    ; Check if counter reached limit
    CPX #$00FF
    BNE game_loop

    ; Reset counter and continue
    LDX #$0000
    JMP game_loop

; Reset vector (DEF88186 starts here after reset)
.org $FFFC
.word main

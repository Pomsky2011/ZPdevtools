; Simple test
.org $8000

start:
    NOP
    LDA #$42
    STA $2000
    JMP start

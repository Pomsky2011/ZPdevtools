; hardware.asm - Hardware I/O routines

; Write byte to screen
write_screen:
    STA $2001
    RTS

; Read input
read_input:
    LDA $3000
    RTS

; Enable interrupts
enable_irq:
    CLI
    RTS

; Disable interrupts
disable_irq:
    SEI
    RTS

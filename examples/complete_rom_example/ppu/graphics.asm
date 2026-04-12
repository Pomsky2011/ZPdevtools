; graphics.asm - PPU Graphics Code
; Simple graphics rendering program

; Initialize stack pointer
TARREG 2, LSB, SP
TARREG 3, MSB, SP
SETBYTE 2, 0xFF
SETBYTE 3, 0xFF         ; SP = 0xFFFF

; Main graphics loop
main_loop:
    ; Clear screen counter
    CLR R0

    ; Draw loop
draw_loop:
    ; Set pixel position (R0 for both X and Y)
    TARREG 2, LSB, R1
    SETBYTE 2, 0x00     ; X = 0
    TARREG 2, LSB, R2
    SETBYTE 2, 0x00     ; Y = 0

    ; Set color based on counter
    MOV R3, R0          ; Color = counter value

    ; Increment position
    INC R0
    CMP R0, R10

    ; Loop back
    JMR main_loop

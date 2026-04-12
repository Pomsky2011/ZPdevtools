; Gradient Demo - Uses V-Blank interrupt to write ahead into rolling framebuffer
; Demonstrates:
; - V-Blank interrupt handling with automatic return address push
; - Writing to framebuffer banks ahead of the display
; - 32-bit color mode with smooth gradients

; Register aliases
R10 = CURRENT_LINE
R11 = COLOR_R
R12 = COLOR_G
R13 = COLOR_B
R14 = X_POS
R15 = TEMP

$0x0100 = IO_BASE

start:
    ; Set 32-bit color mode
    SETRENDMOD 1

    ; Initialize stack pointer (REQUIRED for interrupts!)
    TARREG 2, LSB, SP
    TARREG 3, MSB, SP
    SETBYTE 2, 0xFF
    SETBYTE 3, 0xFF        ; SP = 0xFFFF

    ; Set up V-Blank interrupt handler
    TARREG 2, LSB, R59
    TARREG 3, MSB, R59
    SETBYTE 2, vblank_handler
    SETBYTE 3, vblank_handler

    ; Initialize current line to 0
    CLR CURRENT_LINE

main_loop:
    ; Main loop - just wait for interrupts
    JMR main_loop

; V-Blank interrupt handler
; Draws the next 8 scanlines into the rolling framebuffer
vblank_handler:
    ; Save X position counter
    CLR X_POS

draw_scanline:
    ; Calculate colors based on current line
    ; R = CURRENT_LINE (0-255)
    ; G = X_POS (0-255)
    ; B = 255 - CURRENT_LINE

    ; Set R = CURRENT_LINE
    CLR COLOR_R
    ADD COLOR_R, CURRENT_LINE

    ; Set G = X_POS
    CLR COLOR_G
    ADD COLOR_G, X_POS

    ; Set B = 255 - CURRENT_LINE
    TARREG 2, LSB, TEMP
    SETBYTE 2, 255
    CLR COLOR_B
    ADD COLOR_B, TEMP
    SUB COLOR_B, CURRENT_LINE

    ; Set up DP to point to pixel I/O base (0x0100)
    TARREG 2, LSB, DP
    TARREG 3, MSB, DP
    SETBYTE 2, 0x00
    SETBYTE 3, 0x01

    ; Write X position (0x0100-0x0101)
    MOVDP X_POS
    INC DP
    INC DP

    ; Write Y position (0x0102-0x0103)
    MOVDP CURRENT_LINE
    INC DP
    INC DP

    ; Write Red (0x0104-0x0105)
    MOVDP COLOR_R
    INC DP
    INC DP

    ; Write Green (0x0106-0x0107)
    MOVDP COLOR_G
    INC DP
    INC DP

    ; Write Blue (0x0108-0x0109)
    MOVDP COLOR_B
    INC DP
    INC DP

    ; Write Alpha = 255 (0x010A-0x010B) - triggers pixel draw
    TARREG 2, LSB, TEMP
    SETBYTE 2, 255
    MOVDP TEMP

    ; Increment X position
    INC X_POS

    ; Check if we've drawn all 256 pixels in this scanline
    TARREG 2, LSB, TEMP
    SETBYTE 2, 255
    CMP X_POS, TEMP
    JNG draw_scanline      ; If X_POS <= 255, continue drawing

    ; Move to next scanline
    INC CURRENT_LINE

    ; Wrap around at scanline 256
    TARREG 2, LSB, TEMP
    SETBYTE 2, 255
    CMP CURRENT_LINE, TEMP
    JNG vblank_done        ; If CURRENT_LINE <= 255, we're done

    ; Wrap back to 0
    CLR CURRENT_LINE

vblank_done:
    ; Return from interrupt (return address already on stack)
    RET

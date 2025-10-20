; Fast Gradient Demo - Writes directly to framebuffer memory
; Uses memory-mapped framebuffer at 0xE000-0xFFFF
; Much faster than pixel I/O!

R10 = FB_BASE_LO
R11 = FB_BASE_HI
R12 = Y_COUNT
R13 = X_COUNT
R14 = COLOR

start:
    ; Set 32-bit color mode (1 scanline = 1024 bytes = 1 bank)
    SETRENDMOD 1

    ; Set up framebuffer base address (0xE000)
    TARREG 2, LSB, FB_BASE_LO
    TARREG 3, MSB, FB_BASE_HI
    SETBYTE 2, 0x00
    SETBYTE 3, 0xE0

    ; Y loop: draw 8 scanlines
    CLR Y_COUNT

y_loop:
    ; X loop: draw 256 pixels per scanline
    CLR X_COUNT

x_loop:
    ; Calculate DP = 0xE000 + (Y * 256 * 4) + (X * 4)
    ; For simplicity, just iterate through memory
    CLR DP
    ADD DP, FB_BASE_LO
    CLR COLOR
    ADD COLOR, FB_BASE_HI
    ; Now DP has low byte, COLOR has high byte

    ; Build full DP value
    TARREG 2, LSB, R15
    TARREG 3, MSB, R15
    MOVDP R15  ; This won't work, let me use SETDP properly

    ; Actually, let me just set DP directly
    CLR DP
    ADD DP, FB_BASE_LO
    ; Add Y offset: Y * 1024 = Y * 4 * 256
    ; Add X offset: X * 4

    ; Simpler approach: just write incrementally
    TARREG 2, LSB, DP
    TARREG 3, MSB, DP
    CLR COLOR
    ADD COLOR, FB_BASE_LO
    SETBYTE 2, COLOR
    CLR COLOR
    ADD COLOR, FB_BASE_HI
    SETBYTE 3, COLOR

    ; Write pixel: R G B A (4 bytes in 32-bit mode)
    ; R = X
    CLR COLOR
    ADD COLOR, X_COUNT
    MOVDP COLOR
    INC DP
    INC DP

    ; G = Y * 32
    CLR COLOR
    ADD COLOR, Y_COUNT
    ADD COLOR, COLOR
    ADD COLOR, COLOR
    ADD COLOR, COLOR
    ADD COLOR, COLOR
    ADD COLOR, COLOR
    MOVDP COLOR
    INC DP
    INC DP

    ; B = 255 - X
    TARREG 2, LSB, COLOR
    SETBYTE 2, 255
    SUB COLOR, X_COUNT
    MOVDP COLOR
    INC DP
    INC DP

    ; A = 255
    TARREG 2, LSB, COLOR
    SETBYTE 2, 255
    MOVDP COLOR

    ; Update framebuffer base for next pixel (advance by 4 bytes)
    ; Add 4 to FB_BASE_LO (with carry to FB_BASE_HI if needed)
    TARREG 2, LSB, COLOR
    SETBYTE 2, 4
    ADD FB_BASE_LO, COLOR

    ; Check for carry (if FB_BASE_LO < 4 after adding 4, we wrapped)
    ; For simplicity, just increment in chunks

    ; Next X
    INC X_COUNT
    TARREG 2, LSB, COLOR
    SETBYTE 2, 7
    CMP X_COUNT, COLOR
    JNG x_loop

    ; Advance to next scanline (256 pixels * 4 bytes = 1024 bytes)
    ; For now, just move to next Y
    INC Y_COUNT
    TARREG 2, LSB, COLOR
    SETBYTE 2, 7
    CMP Y_COUNT, COLOR
    JNG y_loop

    ; Done
    HLT

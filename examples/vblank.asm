; VBlank/HBlank status check
; Get blank status into R0
; R0 = 0 (VBlank), 1 (HBlank), or 65535 (active rendering)

    GBLS R0
    HALT

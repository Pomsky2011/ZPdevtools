; Simple Tone Generator
; Writes a sine wave pattern to memory at $3000 (ARAM)

start:
    ; Set data page to $30 (ARAM)
    SDP $30

    ; Initialize counter
    ZOR X

    ; Write 16 samples
    LST 0, 16

write_loop:
    ; Set data byte to current position
    SDB X

    ; Write sample values (simplified sine wave approximation)
    ; Just write counter value as sample
    SBF 0           ; Low byte
    WRL $00         ; Write X value to $30XX

    ; Increment counter
    SCR Y, 1
    ADD X, Y, X

    LFN 0

done:
    HLT

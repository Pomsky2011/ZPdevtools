; audio.asm - APU Audio Code
; Simple audio generation program

; Initialize
start:
    MOV R0, #0          ; Counter = 0
    MOV R1, #100        ; Frequency divider

; Main audio loop
audio_loop:
    ; Increment counter
    ADD R0, R0, #1

    ; Check if we should generate a sample
    CMP R0, R1
    JNZ audio_loop

    ; Reset counter
    MOV R0, #0

    ; Generate sample (simple square wave)
    MOV R2, #128        ; Mid-point
    MOV R3, #255        ; Max amplitude

    ; Write to audio output
    ; (In real APU, this would write to MMP channels)

    ; Loop forever
    JMP audio_loop

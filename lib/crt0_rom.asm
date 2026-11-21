; crt0_rom.asm - ZeroPoint C runtime startup, ROM build mode.
;
; ROM mode is the shipping path: the image is wrapped in a signed ROM header
; (link with zplink, then sign with zpbuild).  The reset/entry code establishes a fully defined
; machine state before sliding into main:
;   - interrupts masked
;   - 16-bit accumulator + index (the C ABI)
;   - binary arithmetic (CLD)
;   - carry clear
;   - a stack at the top of the first 64 KB of Work RAM
; S and DB are not assumed: the real Boot ROM hands off with S = $01FF (a 512-byte
; stack) and DB = $80, and a JMP takes its bank from DB, not PB, so the stub sets S
; and enters main with BRA (see crt0_entry.asm).  Zeroing of C static
; storage is performed by the compiler-emitted data image, so no runtime BSS
; loop is required here (and the current cpuasm has no 16-bit CPX/BNE to write
; one cleanly).
;
; Usage:  .include "lib/crt0_rom.asm" at the TOP of your program, under your
;         own '.org'.  (This file carries no .org of its own.)

__zp_start:
        SEI                     ; interrupts off during bring-up
        REP #$30                ; 16-bit accumulator + index
        CLD                     ; binary arithmetic
        CLC                     ; defined carry state
        LDA #$F000
        TCS                     ; the stack at the top of the first 64 KB of Work RAM
        BRA main                ; BRA, not JMP: the current bank, whatever DB is

; 'main' is provided by the user program.

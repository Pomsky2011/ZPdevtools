; crt0_rom.asm - ZeroPoint C runtime startup, ROM build mode.
;
; ROM mode is the shipping path: the image is wrapped in a signed ROM header
; (link with zplink, then sign with zpbuild).  The reset/entry code establishes a fully defined
; machine state before sliding into main:
;   - interrupts masked
;   - 16-bit accumulator + index (the C ABI)
;   - binary arithmetic (CLD)
;   - carry clear
; S/DP/DB keep their reset values ($01FF / $0000 / $00).  Zeroing of C static
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
        JMP main                ; hand control to the C entry point

; 'main' is provided by the user program.

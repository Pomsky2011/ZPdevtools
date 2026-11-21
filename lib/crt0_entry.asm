; crt0_entry.asm - ZeroPoint C runtime startup, ENTRYPOINT build mode.
;
; Entrypoint mode is the fast development path: the image is just an
; initialization stub that slides straight into main.  Assemble this file as
; the FIRST translation unit (or .include it at the top of your program) so
; label __zp_start sits at the image entry.  The C ABI shared by zpcc and
; def88186cc runs in 16-bit mode (REP #$30), and the stub establishes that mode
; and a stack before it hands control to main.
;
; It must not assume the reset values of S and DB.  The real Boot ROM hands off
; with S = $01FF (a 512-byte stack: the C runtime's call chains can be deeper) and
; DB = $80, while the native boot and a bare reset leave DB = $00.  A JMP takes
; its bank from DB, not PB (docs/cpu/addressing-modes.md), so the stub enters
; main with BRA, a 16-bit target in the current bank whatever DB is. (Neither
; compiler emits JMP; JSR ignores DB.)
;
; Usage:  .include "lib/crt0_entry.asm" at the TOP of your program, under your
;         own '.org'.  The linker (zplink) uses __zp_start as the
;         ELF entry point.  (This file carries no .org of its own.)

__zp_start:
        SEI                     ; interrupts off until the game installs vectors
        REP #$30                ; 16-bit accumulator + index (C ABI)
        CLC                     ; defined flag state on entry to main
        LDA #$F000
        TCS                     ; the stack: top of the first 64 KB of Work RAM, clear of the low page and of low data
        BRA main                ; slide into the C entry point

; 'main' is provided by the user program (C main lowers to a 'main' label).

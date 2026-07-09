; crt0_entry.asm - ZeroPoint C runtime startup, ENTRYPOINT build mode.
;
; Entrypoint mode is the fast development path: the image is just an
; initialization stub that slides straight into main.  Assemble this file as
; the FIRST translation unit (or .include it at the top of your program) so
; label __zp_start sits at the image entry.  The C ABI shared by zpcc and
; def88186cc runs in 16-bit mode (REP #$30), with S/DP/DB already set to their
; reset values ($01FF / $0000 / $00), so the stub only has to establish CPU
; mode and hand control to main.
;
; Usage:  .include "lib/crt0_entry.asm" at the TOP of your program, under your
;         own '.org'.  The linker (zplink) uses __zp_start as the
;         ELF entry point.  (This file carries no .org of its own.)

__zp_start:
        SEI                     ; interrupts off until the game installs vectors
        REP #$30                ; 16-bit accumulator + index (C ABI)
        CLC                     ; defined flag state on entry to main
        ; S = $01FF, DP = $0000, DB = $00 from CPU reset -> nothing to load.
        JMP main                ; slide into the C entry point

; 'main' is provided by the user program (C main lowers to a 'main' label).

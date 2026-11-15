; zp_cop.asm - libzp access to every COP signature: zp_cop / zp_cop32.
;
; COP takes its signature as an instruction *operand* ("COP #$F0"), not a register, so a
; routine that can call any of them has to build the instruction at run time. This one
; writes a three-byte stub into Work RAM, long-calls it, and returns the registers:
;
;       COP  #sig       ; $AF sig
;       SDB  #db        ; $1B db   - puts the caller's DB back (see below)
;       RTL             ; $18
;
; which covers the whole COP #imm space: $F0-$FE (Boot ROM services: SIN/COS/SINCOS/ATAN2/
; ATAN/SQRT/HYPOT, $F7 PPU load, $F8-$FE reserved - docs/boot-math-services.md,
; docs/boot-handoff.md), $FF (chunk verify - zpb-format.md) and, with the real Boot ROM or
; not, every signature below $F0 (the cartridge's own software interrupt via $00:FFE4).
;
; C ABI (shared by zpcc and def88186cc; 16-bit mode, see docs/cpu/conventions.txt):
;
;       zpu32 zp_cop32(zpu8 sig, zpu16 a, zpu16 x, zpu16 y);
;               A = sig, X = a, Y = x, [S+3] = y (4th argument, pushed by the caller)
;       returns A = the COP's A on return, X = its X  (32-bit: A low word, X high word)
;
; C callers: use zp_cop3 (below) or a typed wrapper. A 4-argument call from C works, but both
; compilers pop the stack argument with an A-clobbering TSC/CLC/ADC/TCS, so its returned A is
; garbage; from assembly zp_cop32 is fine.
;
; "a", "x" and "y" are the values the service sees in its A, X and Y registers. zp_cop is
; the same entry point under another name for callers that only want A (zpu16 in C).
; The typed wrappers (zp_sin, zp_hypot, ...) are in this file too, not in C: def88186cc moves
; register-passed arguments wrong (it clobbers A before it reads the first parameter) and drops
; the high word of a 32-bit one, so a C wrapper like zp_hypot(x, y) would miscompile. Here every
; register move is explicit. Their C declarations are in include/zeropoint/zp_cop.h.
;
; Why DB is restored here: COP #$FF (verify_chunk) leaves DB at whatever the Boot ROM last
; set - DB is not saved across BRK/COP/IRQ and there is no PLB - and the C ABI makes DB
; callee-saved. The math/PPU services preserve it themselves. cpuasm's SDB only takes an
; immediate, so the stub carries the caller's DB (read with PHB/PLA) in its own operand.
;
; Contract: M/X width and the flags the caller had are restored (PHP/PLP; the COP's flags
; are discarded). D and SP survive. Y is clobbered. Not re-entrant: it uses a fixed Work RAM
; scratch block ($80:2B00-$80:2B14, clear of the Boot ROM services' own scratch
; $80:2A00-$80:2A4F and $80:2800-$80:29FF) and must not be called from an IRQ handler that
; can interrupt another zp_cop. A COP signature with no handler behind it (default Boot ROM
; stub, no cartridge vector) goes wherever $00:FFE4 points.
;
; Only long addressing and stack-relative loads touch data, so it runs from any bank and any
; DB. (cpuasm has no 16-bit "#imm", so every 16-bit value goes through memory; the stub's
; bytes are written with 8-bit immediates.)

; Scratch block (Work RAM; the stub is rebuilt on every call):
;   $802B02 a   $802B04 x   $802B06 y   $802B08 sig (8-bit)   $802B10-$802B14 the stub
; zc_core takes its inputs from there, so the wrappers below only fill it and BRA in.

; ---- generic: zp_cop32(sig, a, x, y) -------------------------------------------------
zp_cop:
zp_cop32:
        PHP                     ; caller's M/X/flags, restored by zc_core
        REP #$30
        PHA                     ; sig
        TXA
        STA $802B02             ; a
        TYA
        STA $802B04             ; x
        LDA 6,S                 ; y: [ret:2][P:1][sig:2] sit on top of the caller's 4th argument
        STA $802B06             ; y
        PLA                     ; sig (16-bit pull; the high byte is the C caller's, ignored)
        SEP #$20
        STA $802B08
        REP #$30
        BRA zc_core

; ---- generic, three registers: zp_cop3(sig, a, x), y = 0 ----------------------------------
; For C. A call with a 4th argument pushes it and the caller then pops it with TSC/CLC/ADC/TCS,
; which destroys the returned A in both zpcc and def88186cc, so zp_cop32's result is garbage
; from C (it is fine from assembly). zp_cop3 needs no stack argument; Y is 0, which is all the
; services without a typed wrapper below ever want (SIN/COS/SINCOS/ATAN/SQRT, the reserved
; $F8-$FE, and the cartridge's own signatures).
zp_cop3:
        PHP
        REP #$30
        PHA                     ; sig
        TXA
        STA $802B02             ; a
        TYA
        STA $802B04             ; x
        PLA                     ; sig
        SEP #$20
        STA $802B08
        LDA #$00
        STA $802B06
        STA $802B07             ; y = 0
        REP #$30
        BRA zc_core

; ---- typed wrappers (C ABI: first three arguments in A, X, Y; result in A, 32-bit in A:X) ----
; zps16 zp_sin(zpu16 angle)  /  zp_cos / zp_atan(zps16 ratio_q8_8)
zp_sin:
        PHP                     ; caller's M/X/flags, restored by zc_core
        REP #$30
        STA $802B02
        SEP #$20
        LDA #$F0
        BRA zc_sig
zp_cos:
        PHP                     ; caller's M/X/flags, restored by zc_core
        REP #$30
        STA $802B02
        SEP #$20
        LDA #$F1
        BRA zc_sig
zp_atan:
        PHP                     ; caller's M/X/flags, restored by zc_core
        REP #$30
        STA $802B02
        SEP #$20
        LDA #$F4
        BRA zc_sig

; zpu32 zp_sincos(zpu16 angle): A = sin, X = cos (returned as sin | cos << 16)
zp_sincos:
        PHP                     ; caller's M/X/flags, restored by zc_core
        REP #$30
        STA $802B02
        SEP #$20
        LDA #$F2
        BRA zc_sig

; zpu16 zp_atan2(zps16 y, zps16 x): the service wants X = x, Y = y
zp_atan2:
        PHP                     ; caller's M/X/flags, restored by zc_core
        REP #$30
        STA $802B06             ; y
        TXA
        STA $802B04             ; x
        SEP #$20
        LDA #$F3
        BRA zc_sig

; zpu16 zp_hypot(zps16 x, zps16 y): the service wants X = x, Y = y
zp_hypot:
        PHP                     ; caller's M/X/flags, restored by zc_core
        REP #$30
        STA $802B04             ; x
        TXA
        STA $802B06             ; y
        SEP #$20
        LDA #$F6
        BRA zc_sig

; zpu16 zp_sqrt(zpu16 lo, zpu16 hi): floor(sqrt(hi:lo)); the service wants A = low, X = high
zp_sqrt:
        PHP                     ; caller's M/X/flags, restored by zc_core
        REP #$30
        STA $802B02
        TXA
        STA $802B04
        SEP #$20
        LDA #$F5
        BRA zc_sig

; void zp_boot_ppu_load(zpu8 bank, zpu16 offset, zpu16 pages): A/X/Y go straight through
zp_boot_ppu_load:
        PHP                     ; caller's M/X/flags, restored by zc_core
        REP #$30
        STA $802B02
        TXA
        STA $802B04
        TYA
        STA $802B06
        SEP #$20
        LDA #$F7
        BRA zc_sig

; void zp_verify_chunk(zpu16 index): the service wants X = chunk index
zp_verify_chunk:
        PHP                     ; caller's M/X/flags, restored by zc_core
        REP #$30
        STA $802B04
        SEP #$20
        LDA #$FF

zc_sig:                         ; 8-bit A = signature
        STA $802B08
        REP #$30

; ---- core: scratch block in, COP out ------------------------------------------------------
zc_core:
        SEP #$20                ; 8-bit A for the stub bytes
        LDA $802B08
        STA $802B11             ; COP's operand = sig
        PHB
        PLA                     ; A = caller's DB
        STA $802B13             ; SDB's operand
        LDA #$AF
        STA $802B10             ; COP
        LDA #$1B
        STA $802B12             ; SDB
        LDA #$18
        STA $802B14             ; RTL
        REP #$30
        LDA $802B06
        TAY                     ; Y = y
        LDA $802B04
        TAX                     ; X = x
        LDA $802B02             ; A = a
        CALL $802B10            ; COP #sig ; SDB #db ; RTL - results come back in A/X
        NOP                     ; (a CALL/RTL pair may resume one byte late: keep this a 1-byte pad)
        PLP
        RTS

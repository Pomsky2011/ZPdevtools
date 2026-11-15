; zp_long.asm - 32-bit integer helpers for the C compilers (zpcc and def88186cc).
;
; The CPU is 16-bit, so a C `long` lives in two words and the compilers call these routines for every 32-bit
; operator other than load/store/convert. Link it with any program that uses `long` / `unsigned long`
; arithmetic (tests/cc/run.sh includes it automatically).
;
; Calling convention (not the C ABI: these are the compilers' own runtime):
;
;       left operand   $80:2C00 (low word), $80:2C02 (high word)
;       right operand  $80:2C04 (low word), $80:2C06 (high word)   (a shift count is the low word)
;       result         A = low word, X = high word
;       compares       A = -1 / 0 / +1  (left <, ==, > right)
;
; Clobbers A, X, Y and the scratch words $80:2C00-$80:2C2F; preserves D, DB, SP and the M/X widths (it assumes
; 16-bit A/X/Y on entry, as compiled code has). Not re-entrant. All data access is long-addressed or
; stack-relative, so it works with any DB. There is no ADC/SBC/AND/... with a long operand and no PLX/PLY, so
; the right operand is copied to the stack and used as "n,S".
;
; Division by zero: the quotient comes out $FFFFFFFF and the remainder equals the dividend (the shift-subtract
; loop never refuses to subtract).

; ---- add / subtract / bitwise ---------------------------------------------------------------------------

__zp_ladd:
        LDA $802C06
        PHA
        LDA $802C04
        PHA                     ; stack: right low @1, right high @3
        LDA $802C00
        CLC
        ADC 1,S
        TAY
        LDA $802C02
        ADC 3,S
        TAX
        PLA
        PLA
        TYA
        RTS

__zp_lsub:
        LDA $802C06
        PHA
        LDA $802C04
        PHA
        LDA $802C00
        SEC
        SBC 1,S
        TAY
        LDA $802C02
        SBC 3,S
        TAX
        PLA
        PLA
        TYA
        RTS

__zp_land:
        LDA $802C06
        PHA
        LDA $802C04
        PHA
        LDA $802C00
        AND 1,S
        TAY
        LDA $802C02
        AND 3,S
        TAX
        PLA
        PLA
        TYA
        RTS

__zp_lor:
        LDA $802C06
        PHA
        LDA $802C04
        PHA
        LDA $802C00
        ORA 1,S
        TAY
        LDA $802C02
        ORA 3,S
        TAX
        PLA
        PLA
        TYA
        RTS

__zp_lxor:
        LDA $802C06
        PHA
        LDA $802C04
        PHA
        LDA $802C00
        XOR 1,S
        TAY
        LDA $802C02
        XOR 3,S
        TAX
        PLA
        PLA
        TYA
        RTS

; ---- multiply: low 32 bits of left * right ------------------------------------------------------------
; Llo*Rlo gives the full 32-bit product (MUL leaves the high word in X); Llo*Rhi + Lhi*Rlo only matter modulo
; 2^16, as extra high-word terms.
__zp_lmul:
        LDA $802C06
        PHA
        LDA $802C04
        PHA                     ; Rlo @1, Rhi @3
        LDA $802C00
        MUL 1,S                 ; A = low, X = high of Llo * Rlo
        PHX                     ; product high  @3 (shifting R down to @5/@7)
        PHA                     ; product low   @1
        LDA $802C00
        MUL 7,S                 ; Llo * Rhi (low word)
        CLC
        ADC 3,S
        STA 3,S
        LDA $802C02
        MUL 5,S                 ; Lhi * Rlo (low word)
        CLC
        ADC 3,S
        STA 3,S
        LDA 3,S
        TAX
        LDA 1,S
        TAY
        PLA
        PLA
        PLA
        PLA
        TYA
        RTS

; ---- compares: A = -1 / 0 / +1 ---------------------------------------------------------------------
__zp_lcmpu:
        LDA $802C06
        PHA
        LDA $802C04
        PHA
        LDA $802C02
        CMP 3,S
        BEQ __zlcu_lo
        BCS __zlcu_gt
        BRA __zlcu_lt
__zlcu_lo:
        LDA $802C00
        CMP 1,S
        BEQ __zlcu_eq
        BCS __zlcu_gt
__zlcu_lt:
        LDA #$FFFF
        BRA __zlcu_out
__zlcu_gt:
        LDA #$0001
        BRA __zlcu_out
__zlcu_eq:
        LDA #$0000
__zlcu_out:
        TAY
        PLA
        PLA
        TYA
        RTS

; signed: the high words compare signed (N != V after SBC means less), the low words unsigned
__zp_lcmps:
        LDA $802C06
        PHA
        LDA $802C04
        PHA
        LDA $802C02
        SEC
        SBC 3,S
        BEQ __zlcs_lo
        BVS __zlcs_ovf
        BMI __zlcs_lt
        BRA __zlcs_gt
__zlcs_ovf:
        BMI __zlcs_gt
        BRA __zlcs_lt
__zlcs_lo:
        LDA $802C00
        CMP 1,S
        BEQ __zlcs_eq
        BCS __zlcs_gt
__zlcs_lt:
        LDA #$FFFF
        BRA __zlcs_out
__zlcs_gt:
        LDA #$0001
        BRA __zlcs_out
__zlcs_eq:
        LDA #$0000
__zlcs_out:
        TAY
        PLA
        PLA
        TYA
        RTS

; ---- shifts: left by the low word of the right operand -------------------------------------------------
__zp_lshl:
        LDA $802C04
        TAX
        INX
__zlsl_loop:
        DEX
        BEQ __zlsl_done
        LDA $802C00
        ASL A
        STA $802C00
        LDA $802C02
        ROL A
        STA $802C02
        BRA __zlsl_loop
__zlsl_done:
        LDA $802C02
        TAX
        LDA $802C00
        RTS

__zp_llsr:
        LDA $802C04
        TAX
        INX
__zllr_loop:
        DEX
        BEQ __zllr_done
        LDA $802C02
        LSR A
        STA $802C02
        LDA $802C00
        ROR A
        STA $802C00
        BRA __zllr_loop
__zllr_done:
        LDA $802C02
        TAX
        LDA $802C00
        RTS

; arithmetic: CMP #$8000 sets C when the high word is negative, and ROR rotates that sign back in
__zp_lasr:
        LDA $802C04
        TAX
        INX
__zlar_loop:
        DEX
        BEQ __zlar_done
        LDA $802C02
        CMP #$8000
        ROR A
        STA $802C02
        LDA $802C00
        ROR A
        STA $802C00
        BRA __zlar_loop
__zlar_done:
        LDA $802C02
        TAX
        LDA $802C00
        RTS

; ---- divide / remainder ---------------------------------------------------------------------------------
; __zp_udivmod: left / right, unsigned. Out: quotient $802C10/12, remainder $802C14/16. Classic shift-subtract
; over 32 bits: the dividend shifts left into the remainder, which is reduced by the divisor whenever it can be
; (a carry out of the remainder's top bit means it certainly can).
__zp_udivmod:
        LDA $802C06
        PHA
        LDA $802C04
        PHA                     ; divisor: low @1, high @3
        LDA $802C00
        STA $802C10
        LDA $802C02
        STA $802C12             ; quotient starts as the dividend
        LDA #$0000
        STA $802C14
        STA $802C16             ; remainder = 0
        LDA #$0020
        STA $802C18             ; 32 steps
__zud_loop:
        LDA $802C10
        ASL A
        STA $802C10
        LDA $802C12
        ROL A
        STA $802C12
        LDA $802C14
        ROL A
        STA $802C14
        LDA #$0000
        STA $802C1E             ; overflow flag
        LDA $802C16
        ROL A
        STA $802C16
        BCC __zud_noovf
        LDA #$0001
        STA $802C1E             ; the remainder shifted out of 32 bits
__zud_noovf:
        LDA $802C14
        SEC
        SBC 1,S
        STA $802C1A
        LDA $802C16
        SBC 3,S
        STA $802C1C
        BCS __zud_take          ; no borrow: remainder >= divisor
        LDA $802C1E
        BEQ __zud_next          ; borrow and no overflow: remainder < divisor
__zud_take:
        LDA $802C1A
        STA $802C14
        LDA $802C1C
        STA $802C16
        LDA $802C10
        ORA #$0001
        STA $802C10
__zud_next:
        LDA $802C18
        SEC
        SBC #$0001
        STA $802C18
        BEQ __zud_done
        BRA __zud_loop
__zud_done:
        PLA
        PLA
        RTS

__zp_ludiv:
        JSR __zp_udivmod
        LDA $802C12
        TAX
        LDA $802C10
        RTS

__zp_lumod:
        JSR __zp_udivmod
        LDA $802C16
        TAX
        LDA $802C14
        RTS

; signed: divide the magnitudes, then negate. $802C20 = sign of the quotient (operand signs differ),
; $802C22 = sign of the remainder (the dividend's), both in bit 15.
__zlsd_prep:
        LDA $802C02
        PHA
        LDA $802C06
        XOR 1,S
        STA $802C20
        PLA
        LDA $802C02
        STA $802C22
        LDA $802C02             ; |left|
        BPL __zlsd_lpos
        LDA $802C00
        EOR #$FFFF
        CLC
        ADC #$0001
        STA $802C00
        LDA $802C02
        EOR #$FFFF
        ADC #$0000
        STA $802C02
__zlsd_lpos:
        LDA $802C06             ; |right|
        BPL __zlsd_rpos
        LDA $802C04
        EOR #$FFFF
        CLC
        ADC #$0001
        STA $802C04
        LDA $802C06
        EOR #$FFFF
        ADC #$0000
        STA $802C06
__zlsd_rpos:
        JSR __zp_udivmod
        RTS

__zp_lsdiv:
        JSR __zlsd_prep
        LDA $802C20
        BPL __zlsdv_pos
        LDA $802C10
        EOR #$FFFF
        CLC
        ADC #$0001
        STA $802C10
        LDA $802C12
        EOR #$FFFF
        ADC #$0000
        STA $802C12
__zlsdv_pos:
        LDA $802C12
        TAX
        LDA $802C10
        RTS

__zp_lsmod:
        JSR __zlsd_prep
        LDA $802C22
        BPL __zlsmd_pos
        LDA $802C14
        EOR #$FFFF
        CLC
        ADC #$0001
        STA $802C14
        LDA $802C16
        EOR #$FFFF
        ADC #$0000
        STA $802C16
__zlsmd_pos:
        LDA $802C16
        TAX
        LDA $802C14
        RTS

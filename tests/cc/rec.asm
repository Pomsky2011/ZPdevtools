; rec.asm - test recorder for the compiler regression tests (see tests/cc/run.sh, ZeroPoint's tools/zp_run_c.cpp).
;
; rec(v):   A = v        appends one 16-bit word at $80:8100 + count, count at $80:8000 (bytes); scratch at $80:8010-$80:8014
; rec32(v): A = low, X = high word of a 32-bit argument (the C ABI's register pair): records low, then high
rec:
        STA $808010
        LDA $808000
        TAX
        LDA $808010
        STA $808100,X
        INX
        INX
        TXA
        STA $808000
        RTS

rec32:
        STA $808014             ; low word (STX has no long form: go through A)
        TXA
        STA $808012             ; high word
        LDA $808014
        JSR rec
        LDA $808012
        JSR rec
        RTS

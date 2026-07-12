; zp_dma.asm - Upload an arbitrary-length block to any point on the bus via
; the DMA controller, from hand-written cpuasm code (Boot ROM or otherwise).
; The C SDK already has an equivalent (include/zeropoint/zp_dma.h's
; zp_dma_load), but a Boot ROM has no C runtime, so this is the same
; primitive built directly on cpuasm.
;
; Destination can be RAM ($80-$9F), the APU window ($A0), or the PPU window
; ($B0): DMA writes go through the CPU's mapped write path
; (System::dmaWriteCallback -> CPU::writeByte), exactly like an STA would -
; this is a genuine memory-to-memory copy, not RAM-only (verified against
; the emulator). Landing a blob in $A0:0000-$A0:00FF (the APU's MMP register
; block) triggers the same per-byte MMP register side effects an STA would
; (see APU::writeByte) - harmless if those bytes are code that hasn't
; executed yet (this is exactly what CPU-driven byte copies already do
; today), but worth knowing if you target that range.
;
; Calling convention: fill the 9-byte parameter block below (fixed RAM,
; clear of the $800000-$800003 boot-stub trampoline scratch - see
; CLAUDE.md's Boot ROM section), then JSR zp_dma_load. Blocks (busy-waits)
; on the channel until the whole transfer has actually landed; no return
; value. Preserves A/X/P (saved and restored internally), so it's safe to
; call from any register-width context.
;
;   $800010-12  src   24-bit source address, little-endian
;   $800013-15  dst   24-bit destination address, little-endian
;   $800016-17  len   16-bit length in bytes, little-endian (rounded up to
;                     the next 256-byte multiple internally - pad blobs to
;                     256 bytes)
;   $800018     chan  DMA channel, 0-15
;
; Uses $800019-$80001B as internal scratch (256-byte units still to
; transfer, the config byte under construction, and units for the current
; pass).
;
; --- Why this is written the way it is (all findings from building it) ---
;
; 1. Bank portability: a Boot ROM runs in a non-zero bank (e.g. $E0), but
;    cpuasm's labels only carry a 16-bit offset - resolving a same-file
;    label as a 24-bit branch/jump target used to always encode bank $00,
;    silently misjumping out of the running bank the moment it was taken
;    (confirmed on real bytes: PB flipped from $E0 to $00 mid-loop). Fixed
;    in cpuasm with a new ".BANK $xx" directive that this file relies on:
;    whatever file includes this one should declare ".BANK" for its actual
;    load bank *before* the .include, since label banks are recorded at
;    definition time and inherited through .include (shared assembler
;    state, single pass). Only BEQ/BCS/BRA (all correctly bank-aware now)
;    are used below for that reason - see also point 3.
;
; 2. No same-bank ROM *or RAM* data reads through a bare label/absolute
;    address: plain "LDA addr" (AM_ABSOLUTE, 16-bit) resolves through the
;    DB (data bank) register in the interpreter, not PB, and DB defaults to
;    $00 at reset with no way to repoint it at this routine's own bank
;    (PHB exists but there's no PLB to pull a pushed PB back into DB, and
;    no direct "set DB" instruction either). Every memory operand below is
;    instead a literal 24-bit address (cpuasm auto-promotes any literal
;    over $FFFF to the bank-safe AM_ABSOLUTE_LONG form - see
;    detect_addressing_mode), which is why the parameter block lives at
;    $800010+ rather than being reached via a label.
;
; 3. AM_ABSOLUTE_LONG doesn't exist for every mnemonic: LDA/STA support it,
;    but ADC/SBC/AND/ORA/INC/DEC/LDX/LDY do not (checked directly against
;    cpuasm's instruction table - they only go up to AM_ABSOLUTE, i.e. the
;    DB-relative 16-bit form from point 2). So this routine never does
;    "op $8000xx" for those mnemonics; two-operand arithmetic on a long
;    address is done by LDA'ing it in (long-safe), operating with an
;    immediate, and STA'ing it back out (long-safe) - and where the second
;    operand is itself a runtime value (not a compile-time immediate,
;    e.g. adding units_this_pass to a pointer), it's done as a tiny counted
;    loop of "add 1 / add $40" instead of a single add-with-memory-operand.
;    Likewise "chan" is loaded into X via LDA-long + TAX rather than
;    LDX (long unsupported for LDX).
;
; 4. No BCC: cpuasm accepts "BCC" but encodes it as opcode $0A - the same
;    opcode as BCS - because the interpreter has no real "branch if carry
;    clear" handler at all (confirmed: a BCC after a comparison that sets
;    the carry still branches). BGE is a deliberate, correct alias for BCS;
;    BCC is not - it's a latent bug. Avoided below by only ever branching
;    on carry-SET (BCS) or zero (BEQ), restructuring every "less than"
;    check as its "greater or equal" complement.
;
; 5. No 16-bit "#imm": cpuasm always encodes an immediate operand as a
;    single byte (see the AM_IMMEDIATE table in cpuasm.c) regardless of
;    the runtime M-width flag, so any "#imm" used after REP #$20 (16-bit
;    accumulator) silently loses its high byte at runtime (confirmed:
;    "LDA #$1234" after REP #$20 assembles to a 1-byte $34 operand). Side-
;    stepped by keeping this whole routine in 8-bit A/X mode (SEP #$30 at
;    entry) - a 256-byte transfer unit conveniently lines up with the
;    *middle* byte of a 24-bit pointer, so "advance by N units" is just
;    "add 1 to the middle byte N times, rippling carry into the bank byte
;    each time", entirely 8-bit.
; ============================================================================

zp_dma_load:
        PHP                       ; save caller's P (M/X width, flags)
        SEP #$30                  ; this routine runs entirely in 8-bit A/X

        ; --- remaining_units ($800019) = ceil(len / 256) ---
        LDA $800017                ; len high byte
        STA $800019                ; remaining_units = len_high, provisionally
        LDA $800016                ; len low byte
        BEQ zdl_no_bump            ; low == 0 -> exact multiple of 256 already
        LDA $800019
        CLC
        ADC #$01
        STA $800019                 ; round up: one more partial unit
zdl_no_bump:

zdl_loop:
        LDA $800019                 ; remaining_units
        BEQ zdl_done                ; none left -> whole transfer has been queued

        ; units_this_pass = min(remaining_units, 4) (1024 bytes/transfer max)
        CMP #$04
        BCS zdl_clamp                ; remaining_units >= 4 -> override with 4
        BRA zdl_have_units           ; remaining_units < 4 -> A already correct
zdl_clamp:
        LDA #$04
zdl_have_units:
        STA $80001B                  ; units_this_pass, kept in RAM (not stack)

        ; --- cfgbyte = ((units-1)<<6) | (chan & $0F); mode bits = 0 (COPY) ---
        LDA $800018                  ; chan
        AND #$0F
        STA $80001A                  ; cfgbyte := chan field

        LDA $80001B                  ; units_this_pass
        SEC
        SBC #$01                     ; units-1 (0..3)
        BEQ zdl_cfg_done             ; 0 -> no <<6 contribution to add
        TAY                          ; Y = units-1, loop counter
zdl_cfg_loop:
        LDA $80001A
        CLC
        ADC #$40                     ; one "unit" of the <<6 field (0x40 = 1<<6)
        STA $80001A
        DEY
        BEQ zdl_cfg_done
        BRA zdl_cfg_loop
zdl_cfg_done:

        ; --- wait for the channel to go idle before configuring it ---
zdl_wait1:
        LDA $800018                  ; chan (LDX has no long form - see note 3)
        TAX
        LDA $D80023,X
        BEQ zdl_wait1_done           ; 0 = Idle
        CMP #$04
        BEQ zdl_wait1_done           ; 4 = Complete
        BRA zdl_wait1
zdl_wait1_done:

        ; --- stream the 9-byte config; the 9th byte auto-fires it ---
        LDA #$00
        STA $D80020                   ; reset config write index
        LDA $80001A
        STA $D80021                   ; config byte
        LDA $800010
        STA $D80021                   ; src low
        LDA $800011
        STA $D80021                   ; src mid
        LDA $800012
        STA $D80021                   ; src bank
        LDA $800013
        STA $D80021                   ; dst low
        LDA $800014
        STA $D80021                   ; dst mid
        LDA $800015
        STA $D80021                   ; dst bank
        LDA #$01
        STA $D80021                   ; size multiplier = 1
        LDA #$00
        STA $D80021                   ; irq/trigger = 0 (fires the transfer)

        ; --- advance src/dst by units_this_pass * 256 (bump the middle
        ;     byte, ripple carry into the bank byte, once per unit) and
        ;     decrement remaining_units by the same count ---
        LDA $80001B                    ; units_this_pass (always >= 1 here)
        TAY                             ; Y = loop counter
zdl_bump_loop:
        LDA $800011
        CLC
        ADC #$01
        STA $800011                     ; src mid += 1
        BCS zdl_src_carry                ; carry set -> ripple into the bank byte
        BRA zdl_src_cont
zdl_src_carry:
        LDA $800012
        CLC
        ADC #$01
        STA $800012                      ; src bank += 1
zdl_src_cont:
        LDA $800014
        CLC
        ADC #$01
        STA $800014                       ; dst mid += 1
        BCS zdl_dst_carry                  ; carry set -> ripple into the bank byte
        BRA zdl_dst_cont
zdl_dst_carry:
        LDA $800015
        CLC
        ADC #$01
        STA $800015                        ; dst bank += 1
zdl_dst_cont:
        LDA $800019
        SEC
        SBC #$01
        STA $800019                         ; remaining_units -= 1
        DEY
        BEQ zdl_bump_done
        BRA zdl_bump_loop
zdl_bump_done:

        BRA zdl_loop

zdl_done:
        ; --- final wait: block until the last transfer has actually landed ---
zdl_wait2:
        LDA $800018
        TAX
        LDA $D80023,X
        BEQ zdl_wait2_done
        CMP #$04
        BEQ zdl_wait2_done
        BRA zdl_wait2
zdl_wait2_done:

        PLP                             ; restore caller's P
        RTS

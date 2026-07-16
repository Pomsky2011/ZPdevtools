; zp_verify_chunk.asm - reference implementation of the ZeroPoint v3
; signed-ROM runtime chunk-verification contract. See
; ZeroPoint/docs/zpb-format.md's "Runtime chunk verification" section for
; the on-disk manifest layout, and ZPbootROM/def88186/rsa.def /
; blake2s.def for the boot-time half of this (verifying code_digest and
; manifest_digest synchronously before jumping to the cartridge).
;
; WHY THIS NEEDS NO RSA AND NO CALL BACK INTO THE BOOT ROM: by the time
; cartridge code is running, the boot ROM has already verified that
; manifest_digest = BLAKE2s(manifest) matches what was RSA-signed, and it
; will not have jumped here otherwise. manifest[] (bank $E1, offset
; $016C+) is therefore already authenticated. The code below - which does
; nothing but recompute BLAKE2s over one chunk and compare it against
; manifest[i] - is itself part of the RSA-verified CODE region (it must be
; linked into your signed cartridge before codeSize, like any other game
; code). An attacker can't swap manifest[] without breaking
; manifest_digest, and can't strip this check out of your code without
; breaking code_digest - so this comparison carries exactly the same
; tamper resistance as the boot-time checks, without ever touching the
; private key or re-deriving a signature.
;
; WHY THIS IS A DIFFERENT (SIMPLER) IMPLEMENTATION THAN blake2s.def: the
; boot ROM has to hash up to the whole cartridge's worth of code plus a
; manifest, synchronously, once, at power-on - blake2s.def is hand-tuned
; (fixed G-call slots, fully unrolled rounds, MUL-based fast rotates) to
; make that as cheap as possible per byte. This routine only ever hashes
; ONE 16384-byte chunk, occasionally, whenever your game is about to load
; that chunk - clarity is worth more than cycles here, so this uses plain
; runtime-indexed G/round tables instead. Both are the same RFC 7693
; BLAKE2s algorithm and produce bit-identical digests for the same input -
; this file's arithmetic was checked against Python's hashlib.blake2s the
; same way blake2s.def's was (see that file's header comment).
;
; NOTE ON ADDRESSING: cpuasm has no operand arithmetic ("LABEL+2,X" is not
; a valid operand - see ZPbootROM/def88186/rsa.def's BLOCK_BUF_P2-style
; equates for the same constraint), so every fixed sub-offset used with
; indexed (,X) addressing below has its own dedicated .equ rather than
; being computed from a base label.
;
; ----------------------------------------------------------------------------
; USAGE
; ----------------------------------------------------------------------------
; This CPU's long (24-bit) addressing only allows a compile-time-literal
; bank with a runtime 16-bit offset (see ZPdevtools/lib/zp_dma.asm's
; header comment, point 2) - so this routine cannot hash "wherever the
; chunk happens to live" directly; the chunk must first be staged into the
; fixed single-bank buffer below (exactly the same DMA-stage-then-hash
; pattern ZPbootROM/def88186/blake2s.def's blake2s_hash_multibank uses for
; the boot-time code/data regions):
;
;   1. DMA the chunk's 16384 bytes (or fewer, for the data region's last,
;      short chunk) from ROM into CHUNK_STAGE_BUF - e.g. via
;      ZPdevtools/lib/zp_dma.asm's zp_dma_load.
;   2. Set CHUNK_LEN_LO/HI to the chunk's actual byte length (16384 for
;      every chunk except possibly the last one in the data region).
;   3. Set MANIFEST_IDX to the chunk's index i (0-based).
;   4. JSR zp_verify_chunk.
;   5. Check VERIFY_OK: 1 = hash matched manifest[i] (safe to use the
;      chunk), 0 = mismatch (treat exactly like a failed boot signature -
;      refuse to use it).
;
; Assumes 16-bit A/X/Y (REP #$30) and DB=$80 (Work RAM) on entry - every
; equate below except the manifest reads is a 16-bit DB-relative offset,
; not a 24-bit long address, because most of this CPU's arithmetic/compare
; opcodes (ADC/SBC/AND/ORA/XOR/CMP/INC/DEC/LDX/LDY) have no long-addressed
; form at all (only LDA/STA do - see ZPdevtools/lib/zp_dma.asm's header
; comment, point 3); the manifest itself is still read via bank-explicit
; $E1016C,X addressing (LDA only, then staged through CMP_TMP for the
; actual compare) regardless of DB, same convention rsa.def uses. Leaves
; A/X/Y 16-bit on return. Clobbers A/X/Y and every scratch address this
; file defines.
; ============================================================================

.BANK $00

; ---- caller-facing interface -----------------------------------------------
.equ CHUNK_STAGE_BUF $3000   ; stage a chunk here (DMA) before calling
.equ CHUNK_LEN_LO    $1F00   ; actual chunk length, caller-set (<= 16384)
.equ CHUNK_LEN_HI    $1F02
.equ MANIFEST_IDX    $1F04   ; chunk index i, caller-set
.equ VERIFY_OK       $1F06   ; result: 1 = match, 0 = mismatch

; ---- BLAKE2s state (RFC 7693, unkeyed, 32-byte digest) --------------------
; H[0..7]: 8 x 32-bit words, 4 bytes each = 32 bytes ($1F10-$1F2F).
.equ H0_LO $1F10
.equ H0_HI $1F12
.equ H1_LO $1F14
.equ H1_HI $1F16
.equ H2_LO $1F18
.equ H2_HI $1F1A
.equ H3_LO $1F1C
.equ H3_HI $1F1E
.equ H4_LO $1F20
.equ H4_HI $1F22
.equ H5_LO $1F24
.equ H5_HI $1F26
.equ H6_LO $1F28
.equ H6_HI $1F2A
.equ H7_LO $1F2C
.equ H7_HI $1F2E

; V[0..15]: 16 x 32-bit working-state words, 4 bytes each = 64 bytes
; ($1F30-$1F6F). V0_LO/V0_HI are also used as the ",X"-indexed base
; for the whole array (X = 0,4,8,...,60 addresses v[X/4]) - see G_mix.
.equ V0_LO  $1F30
.equ V0_HI  $1F32
.equ V8_LO  $1F50
.equ V8_HI  $1F52
.equ V9_LO  $1F54
.equ V9_HI  $1F56
.equ V10_LO $1F58
.equ V10_HI $1F5A
.equ V11_LO $1F5C
.equ V11_HI $1F5E
.equ V12_LO $1F60
.equ V12_HI $1F62
.equ V13_LO $1F64
.equ V13_HI $1F66
.equ V14_LO $1F68
.equ V14_HI $1F6A
.equ V15_LO $1F6C
.equ V15_HI $1F6E

.equ BLOCK_BUF    $1F70   ; current 64-byte message block
.equ BLOCK_BUF_HI $1F72   ; BLOCK_BUF+2, indexed alias (see G_mix message
                            ; word reads: each 32-bit word's HI half)

.equ COUNTER_LO $1FB0   ; running byte counter t (32-bit)
.equ COUNTER_HI $1FB2
.equ REM_LO     $1FB4   ; bytes left to consume from CHUNK_STAGE_BUF
.equ REM_HI     $1FB6
.equ SRC_OFS    $1FB8   ; current read offset into CHUNK_STAGE_BUF
.equ IS_LAST    $1FBA
.equ ROUND_IDX  $1FBC   ; 0..9, current round
.equ SLOT_IDX   $1FBE   ; 0..7, current G-call within the round

; ---- G-call scratch --------------------------------------------------------
.equ G_A_OFS   $1FC0   ; byte offset into V0_LO for v[a]/v[b]/v[c]/v[d]
.equ G_B_OFS   $1FC2
.equ G_C_OFS   $1FC4
.equ G_D_OFS   $1FC6
.equ MSG_X_LO  $1FC8
.equ MSG_X_HI  $1FCA
.equ MSG_Y_LO  $1FCC
.equ MSG_Y_HI  $1FCE

; ---- add32 / rotr32 scratch -------------------------------------------------
.equ ADD_A_LO $1FD0
.equ ADD_A_HI $1FD2
.equ ADD_B_LO $1FD4
.equ ADD_B_HI $1FD6
.equ ROT_LO   $1FD8
.equ ROT_HI   $1FDA
.equ ROT_TMP  $1FDC
.equ ROT_BIT  $1FDE

.equ COPY_COUNTER $1FE0
.equ IDX_TMP      $1FE2   ; zero-extended table-byte scratch
.equ ROW_BASE     $1FE4   ; sigma_table row-base scratch (round_idx*16)
.equ CMP_TMP      $1FE6   ; manifest byte staged here before CMP (see below)
.equ XOR_TMP_LO   $1FE8   ; XOR has no indexed addressing mode at all on
.equ XOR_TMP_HI   $1FEA   ; this CPU (AM_IMMEDIATE/AM_ABSOLUTE only - see
                          ; cpuasm.c's instruction table), so every
                          ; "XOR something,X" below loads the indexed
                          ; operand here first, then XORs against these
                          ; fixed (non-indexed) cells instead.

; ---- manifest location (bank $E1, fixed - see docs/zpb-format.md) ----------
; $016C = header(64B) + trailer prefix(8B) + digest(32B) + signature(256B)
; + codeSize(4B) = 364.

; ============================================================================
; TABLES (RFC 7693 section 2.7's SIGMA permutation, plus this file's own
; QUAD table - the (a,b,c,d) v[]-index quadruple for each of the 8 G-calls
; within a round, constant across all 10 rounds; see blake2s.def's header
; comment for the derivation of both. Read at runtime via X-indexed 8-bit
; LDA - much shorter than encoding 10 rounds x 8 slots at assembly time the
; way blake2s.def does, at the cost of a few extra instructions per G call
; that don't matter here (see this file's header comment).
; ============================================================================
sigma_table:
    .byte 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
    .byte 14,10,4,8,9,15,13,6,1,12,0,2,11,7,5,3
    .byte 11,8,12,0,5,2,15,13,10,14,3,6,7,1,9,4
    .byte 7,9,3,1,13,12,11,14,2,6,5,10,4,0,15,8
    .byte 9,0,5,7,2,4,10,15,14,1,11,12,6,8,3,13
    .byte 2,12,6,10,0,11,8,3,4,13,7,5,15,14,1,9
    .byte 12,5,1,15,14,13,4,10,0,7,6,3,9,2,8,11
    .byte 13,11,7,14,12,1,3,9,5,0,15,4,8,6,2,10
    .byte 6,15,14,9,11,3,0,8,12,2,13,7,1,4,10,5
    .byte 10,2,8,4,7,6,1,5,15,11,9,14,3,12,13,0

quad_table:
    .byte 0,4,8,12
    .byte 1,5,9,13
    .byte 2,6,10,14
    .byte 3,7,11,15
    .byte 0,5,10,15
    .byte 1,6,11,12
    .byte 2,7,8,13
    .byte 3,4,9,14

; ============================================================================
; add32: (ADD_A_HI:ADD_A_LO) += (ADD_B_HI:ADD_B_LO), mod 2^32, result left
; in ADD_A_LO/HI.
; ============================================================================
add32:
    clc
    lda ADD_A_LO
    adc ADD_B_LO
    sta ADD_A_LO
    lda ADD_A_HI
    adc ADD_B_HI
    sta ADD_A_HI
    rts

; ============================================================================
; rotr32_1 / rotr32_16 / rotr32_12 / rotr32_8 / rotr32_7: rotate
; (ROT_HI:ROT_LO) right by 1/16/12/8/7 bits in place. rotr32_1 is a plain,
; obviously-correct bit-level rotate (save V's bit0, logical-shift the pair
; right by 1 via CLC+ROR+ROR, then re-inject the saved bit as HI's new top
; bit) - the other rotate amounts are just rotr32_1 called in a counted
; loop (no hardware LOOP/LPEND here on purpose: keeping the loop as a plain
; DEX/BNE avoids any question about whether a conditional branch is legal
; inside a hardware-loop body). rotr32_16 is the one case that's cheaper
; to special-case: rotating a HI:LO pair by exactly 16 is just swapping
; which half is which.
; ============================================================================
rotr32_1:
    lda ROT_LO
    and #$0001
    sta ROT_BIT
    clc
    ror ROT_HI
    ror ROT_LO
    lda ROT_BIT
    beq rotr32_1_done
    lda ROT_HI
    ora #$8000
    sta ROT_HI
rotr32_1_done:
    rts

rotr32_16:
    lda ROT_LO
    sta ROT_TMP
    lda ROT_HI
    sta ROT_LO
    lda ROT_TMP
    sta ROT_HI
    rts

rotr32_12:
    ldx #12
rotr32_12_loop:
    jsr rotr32_1
    dex
    bne rotr32_12_loop
    rts

rotr32_8:
    ldx #8
rotr32_8_loop:
    jsr rotr32_1
    dex
    bne rotr32_8_loop
    rts

rotr32_7:
    ldx #7
rotr32_7_loop:
    jsr rotr32_1
    dex
    bne rotr32_7_loop
    rts

; ============================================================================
; read_table_byte: zero-extend the byte at (table),X into IDX_TMP as a
; clean 16-bit value (0..15). The 16-bit cell is zeroed FIRST (in 16-bit
; mode) so the byte written in 8-bit mode doesn't leave garbage in the
; high half when read back as a 16-bit word - X is NOT touched by the
; SEP/REP width switch itself, only by the actual table read.
; ============================================================================
; sigma_table/quad_table are ROM data assembled into bank $00 (.BANK $00,
; the same bank this whole file's code lives in) - but plain "table,X"
; addressing resolves through DB, not PB (see this file's header note and
; ZPdevtools/lib/zp_dma.asm's point 2), and DB is assumed to be $80 (Work
; RAM) for everything else in this file. Reading these tables therefore
; needs the same SDB-ping-pong every other file in this codebase uses for
; ROM-resident tables (e.g. rsa.def's K-table reads, blake2s.def's
; blake2s_stage_next) - SDB #$00 around the read, SDB #$80 restored right
; after. Missing this the first time round produced an all-zero read
; regardless of X (DB=$80 was fresh, zeroed Work RAM at that offset).
read_sigma_byte:
    lda #0
    sta IDX_TMP
    sep #$20
    sdb #$00
    lda sigma_table,X
    sdb #$80
    sta IDX_TMP
    rep #$20
    rts

read_quad_byte:
    lda #0
    sta IDX_TMP
    sep #$20
    sdb #$00
    lda quad_table,X
    sdb #$80
    sta IDX_TMP
    rep #$20
    rts

; ============================================================================
; G(a,b,c,d,x,y): the BLAKE2s mixing function (RFC 7693 section 3.1).
; a/b/c/d are BYTE offsets into V0_LO (index*4, since each v[] word is 4
; bytes: LO at +0, HI at +2) - caller sets G_A_OFS/G_B_OFS/G_C_OFS/G_D_OFS.
; x/y are the two message words for this call - caller sets
; MSG_X_LO/HI/MSG_Y_LO/HI. All four V0_LO reads/writes below go through X
; reloaded from the relevant *_OFS cell immediately before each access,
; since only one index register is available and the four offsets differ
; per call.
;
;   v[a] = v[a] + v[b] + x
;   v[d] = rotr32(v[d] ^ v[a], 16)
;   v[c] = v[c] + v[d]
;   v[b] = rotr32(v[b] ^ v[c], 12)
;   v[a] = v[a] + v[b] + y
;   v[d] = rotr32(v[d] ^ v[a], 8)
;   v[c] = v[c] + v[d]
;   v[b] = rotr32(v[b] ^ v[c], 7)
; ============================================================================
G_mix:
    ; v[a] += v[b]
    ldx G_A_OFS
    lda V0_LO,X
    sta ADD_A_LO
    lda V0_HI,X
    sta ADD_A_HI
    ldx G_B_OFS
    lda V0_LO,X
    sta ADD_B_LO
    lda V0_HI,X
    sta ADD_B_HI
    jsr add32
    ; v[a] += x
    lda MSG_X_LO
    sta ADD_B_LO
    lda MSG_X_HI
    sta ADD_B_HI
    jsr add32
    ldx G_A_OFS
    lda ADD_A_LO
    sta V0_LO,X
    lda ADD_A_HI
    sta V0_HI,X

    ; v[d] = rotr32(v[d] ^ v[a], 16)
    ldx G_D_OFS
    lda V0_LO,X
    sta ROT_LO
    lda V0_HI,X
    sta ROT_HI
    ldx G_A_OFS
    lda V0_LO,X
    sta XOR_TMP_LO
    lda V0_HI,X
    sta XOR_TMP_HI
    lda ROT_LO
    xor XOR_TMP_LO
    sta ROT_LO
    lda ROT_HI
    xor XOR_TMP_HI
    sta ROT_HI
    jsr rotr32_16
    ldx G_D_OFS
    lda ROT_LO
    sta V0_LO,X
    lda ROT_HI
    sta V0_HI,X

    ; v[c] += v[d]
    ldx G_C_OFS
    lda V0_LO,X
    sta ADD_A_LO
    lda V0_HI,X
    sta ADD_A_HI
    ldx G_D_OFS
    lda V0_LO,X
    sta ADD_B_LO
    lda V0_HI,X
    sta ADD_B_HI
    jsr add32
    ldx G_C_OFS
    lda ADD_A_LO
    sta V0_LO,X
    lda ADD_A_HI
    sta V0_HI,X

    ; v[b] = rotr32(v[b] ^ v[c], 12)
    ldx G_B_OFS
    lda V0_LO,X
    sta ROT_LO
    lda V0_HI,X
    sta ROT_HI
    ldx G_C_OFS
    lda V0_LO,X
    sta XOR_TMP_LO
    lda V0_HI,X
    sta XOR_TMP_HI
    lda ROT_LO
    xor XOR_TMP_LO
    sta ROT_LO
    lda ROT_HI
    xor XOR_TMP_HI
    sta ROT_HI
    jsr rotr32_12
    ldx G_B_OFS
    lda ROT_LO
    sta V0_LO,X
    lda ROT_HI
    sta V0_HI,X

    ; v[a] += v[b]
    ldx G_A_OFS
    lda V0_LO,X
    sta ADD_A_LO
    lda V0_HI,X
    sta ADD_A_HI
    ldx G_B_OFS
    lda V0_LO,X
    sta ADD_B_LO
    lda V0_HI,X
    sta ADD_B_HI
    jsr add32
    ; v[a] += y
    lda MSG_Y_LO
    sta ADD_B_LO
    lda MSG_Y_HI
    sta ADD_B_HI
    jsr add32
    ldx G_A_OFS
    lda ADD_A_LO
    sta V0_LO,X
    lda ADD_A_HI
    sta V0_HI,X

    ; v[d] = rotr32(v[d] ^ v[a], 8)
    ldx G_D_OFS
    lda V0_LO,X
    sta ROT_LO
    lda V0_HI,X
    sta ROT_HI
    ldx G_A_OFS
    lda V0_LO,X
    sta XOR_TMP_LO
    lda V0_HI,X
    sta XOR_TMP_HI
    lda ROT_LO
    xor XOR_TMP_LO
    sta ROT_LO
    lda ROT_HI
    xor XOR_TMP_HI
    sta ROT_HI
    jsr rotr32_8
    ldx G_D_OFS
    lda ROT_LO
    sta V0_LO,X
    lda ROT_HI
    sta V0_HI,X

    ; v[c] += v[d]
    ldx G_C_OFS
    lda V0_LO,X
    sta ADD_A_LO
    lda V0_HI,X
    sta ADD_A_HI
    ldx G_D_OFS
    lda V0_LO,X
    sta ADD_B_LO
    lda V0_HI,X
    sta ADD_B_HI
    jsr add32
    ldx G_C_OFS
    lda ADD_A_LO
    sta V0_LO,X
    lda ADD_A_HI
    sta V0_HI,X

    ; v[b] = rotr32(v[b] ^ v[c], 7)
    ldx G_B_OFS
    lda V0_LO,X
    sta ROT_LO
    lda V0_HI,X
    sta ROT_HI
    ldx G_C_OFS
    lda V0_LO,X
    sta XOR_TMP_LO
    lda V0_HI,X
    sta XOR_TMP_HI
    lda ROT_LO
    xor XOR_TMP_LO
    sta ROT_LO
    lda ROT_HI
    xor XOR_TMP_HI
    sta ROT_HI
    jsr rotr32_7
    ldx G_B_OFS
    lda ROT_LO
    sta V0_LO,X
    lda ROT_HI
    sta V0_HI,X
    rts

; ============================================================================
; blake2s_compress: one 64-byte block at BLOCK_BUF into H0..H7 (updated in
; place). IS_LAST must be set (1) or cleared (0) first; COUNTER_LO/HI must
; already include this block's bytes.
; ============================================================================
blake2s_compress:
    ; v[0..7] = h[0..7] (straight 32-byte block copy - H0_LO..H7_HI and
    ; V0_LO..v[7]'s HI word are both contiguous 32-byte runs).
    ldx #0
    lda #16
    sta COPY_COUNTER
blake2s_compress_copy_h:
    lda H0_LO,X
    sta V0_LO,X
    inx
    inx
    dec COPY_COUNTER
    bne blake2s_compress_copy_h

    ; v[8..15] = IV[0..7] (raw IV, NOT the param-block-XORed H0 value -
    ; v[8] must be IV[0]=0x6A09E667 unmodified; only H0 itself gets XORed
    ; with the digest-length/key parameter block, per RFC 7693 section 3.2)
    lda #$E667
    sta V8_LO
    lda #$6A09
    sta V8_HI
    lda #$AE85
    sta V9_LO
    lda #$BB67
    sta V9_HI
    lda #$F372
    sta V10_LO
    lda #$3C6E
    sta V10_HI
    lda #$F53A
    sta V11_LO
    lda #$A54F
    sta V11_HI
    lda #$527F
    sta V12_LO
    lda #$510E
    sta V12_HI
    lda #$688C
    sta V13_LO
    lda #$9B05
    sta V13_HI
    lda #$D9AB
    sta V14_LO
    lda #$1F83
    sta V14_HI
    lda #$CD19
    sta V15_LO
    lda #$5BE0
    sta V15_HI

    ; v[12] ^= t mod 2^32 (COUNTER_LO/HI)
    lda V12_LO
    xor COUNTER_LO
    sta V12_LO
    lda V12_HI
    xor COUNTER_HI
    sta V12_HI
    ; v[13] ^= 0 (t div 2^32 - always 0, max chunk is 16384 bytes)

    ; v[14] ^= 0xFFFFFFFF, only on the final block
    lda IS_LAST
    beq blake2s_compress_rounds
    lda V14_LO
    xor #$FFFF
    sta V14_LO
    lda V14_HI
    xor #$FFFF
    sta V14_HI

blake2s_compress_rounds:
    lda #0
    sta ROUND_IDX
blake2s_compress_round_loop:
    lda #0
    sta SLOT_IDX
blake2s_compress_slot_loop:
    ; quad_table row = SLOT_IDX*4 -> a,b,c,d byte offsets into V0_LO
    ; (multiply the 0..15 v-index by 4 to get a byte offset). The row
    ; index is kept in ROW_BASE (memory), NOT carried in X via repeated
    ; INX: every "mul #4" below (to turn a 0..15 v-index into a byte
    ; offset) clobbers X with its own product's high word - a documented
    ; gotcha throughout this codebase (see rsa.def's compute_S0 comment) -
    ; so X can't be trusted to still hold "row+1/2/3" after the FIRST
    ; read_quad_byte call's own conversion mul runs. Recomputing X from
    ; ROW_BASE fresh before each read_quad_byte call sidesteps that
    ; entirely (this was a real bug here: X previously got silently reset
    ; near 0-1 after the first read, making every slot after the first
    ; read the same one or two quad_table entries instead of its own row).
    lda SLOT_IDX
    mul #4
    sta ROW_BASE
    lda ROW_BASE
    tax
    jsr read_quad_byte
    lda IDX_TMP
    mul #4
    sta G_A_OFS
    lda ROW_BASE
    clc
    adc #1
    tax
    jsr read_quad_byte
    lda IDX_TMP
    mul #4
    sta G_B_OFS
    lda ROW_BASE
    clc
    adc #2
    tax
    jsr read_quad_byte
    lda IDX_TMP
    mul #4
    sta G_C_OFS
    lda ROW_BASE
    clc
    adc #3
    tax
    jsr read_quad_byte
    lda IDX_TMP
    mul #4
    sta G_D_OFS

    ; sigma_table row = ROUND_IDX*16, column = SLOT_IDX*2 (x) / +1 (y)
    lda ROUND_IDX
    mul #16
    sta ROW_BASE
    lda SLOT_IDX
    mul #2
    clc
    adc ROW_BASE
    tax
    jsr read_sigma_byte
    lda IDX_TMP
    mul #4
    tax
    lda BLOCK_BUF,X
    sta MSG_X_LO
    lda BLOCK_BUF_HI,X
    sta MSG_X_HI

    lda SLOT_IDX
    mul #2
    clc
    adc #1
    adc ROW_BASE
    tax
    jsr read_sigma_byte
    lda IDX_TMP
    mul #4
    tax
    lda BLOCK_BUF,X
    sta MSG_Y_LO
    lda BLOCK_BUF_HI,X
    sta MSG_Y_HI

    jsr G_mix

    lda SLOT_IDX
    clc
    adc #1
    sta SLOT_IDX
    cmp #8
    bne blake2s_compress_slot_loop

    lda ROUND_IDX
    clc
    adc #1
    sta ROUND_IDX
    cmp #10
    bne blake2s_compress_round_loop

    ; h[i] ^= v[i] ^ v[i+8], for i=0..7
    ldx #0
    lda #8
    sta COPY_COUNTER
blake2s_compress_finalize:
    lda V0_LO,X
    sta XOR_TMP_LO
    lda V8_LO,X
    sta XOR_TMP_HI
    lda H0_LO,X
    xor XOR_TMP_LO
    xor XOR_TMP_HI
    sta H0_LO,X
    lda V0_HI,X
    sta XOR_TMP_LO
    lda V8_HI,X
    sta XOR_TMP_HI
    lda H0_HI,X
    xor XOR_TMP_LO
    xor XOR_TMP_HI
    sta H0_HI,X
    inx
    inx
    inx
    inx
    dec COPY_COUNTER
    bne blake2s_compress_finalize
    rts

; ============================================================================
; zp_verify_chunk: see this file's header comment for the full contract.
; ============================================================================
zp_verify_chunk:
    ; ---- H[0..7] init: IV[] XOR the parameter block (digest_len=32,
    ; no key) - identical constants/derivation to blake2s.def.
    lda #$E647
    sta H0_LO
    lda #$6B08
    sta H0_HI
    lda #$AE85
    sta H1_LO
    lda #$BB67
    sta H1_HI
    lda #$F372
    sta H2_LO
    lda #$3C6E
    sta H2_HI
    lda #$F53A
    sta H3_LO
    lda #$A54F
    sta H3_HI
    lda #$527F
    sta H4_LO
    lda #$510E
    sta H4_HI
    lda #$688C
    sta H5_LO
    lda #$9B05
    sta H5_HI
    lda #$D9AB
    sta H6_LO
    lda #$1F83
    sta H6_HI
    lda #$CD19
    sta H7_LO
    lda #$5BE0
    sta H7_HI

    lda #0
    sta COUNTER_LO
    sta COUNTER_HI
    sta SRC_OFS
    lda CHUNK_LEN_LO
    sta REM_LO
    lda CHUNK_LEN_HI
    sta REM_HI

    ; empty-chunk special case (shouldn't occur in practice - every real
    ; chunk is > 0 bytes - but mirrors blake2s.def's own handling for
    ; completeness): hash exactly one all-zero block.
    lda CHUNK_LEN_LO
    ora CHUNK_LEN_HI
    bne zp_verify_chunk_normal
    jsr zp_zero_block_buf
    lda #1
    sta IS_LAST
    jsr blake2s_compress
    bra zp_verify_chunk_compare

zp_verify_chunk_normal:
zp_verify_chunk_loop:
    lda REM_HI
    bne zp_verify_chunk_full_block
    lda REM_LO
    cmp #64
    bge zp_verify_chunk_full_block

    ; final partial block: REM_LO < 64
    jsr zp_zero_block_buf
    clc
    lda COUNTER_LO
    adc REM_LO
    sta COUNTER_LO
    lda COUNTER_HI
    adc REM_HI
    sta COUNTER_HI
    jsr zp_copy_partial
    lda #1
    sta IS_LAST
    jsr blake2s_compress
    bra zp_verify_chunk_compare

zp_verify_chunk_full_block:
    jsr zp_copy_full
    clc
    lda COUNTER_LO
    adc #64
    sta COUNTER_LO
    lda COUNTER_HI
    adc #0
    sta COUNTER_HI
    sec
    lda REM_LO
    sbc #64
    sta REM_LO
    lda REM_HI
    sbc #0
    sta REM_HI
    clc
    lda SRC_OFS
    adc #64
    sta SRC_OFS
    lda REM_LO
    ora REM_HI
    bne zp_verify_chunk_not_last
    lda #1
    sta IS_LAST
    jsr blake2s_compress
    bra zp_verify_chunk_compare
zp_verify_chunk_not_last:
    lda #0
    sta IS_LAST
    jsr blake2s_compress
    bra zp_verify_chunk_loop

zp_verify_chunk_compare:
    ; manifest[i] lives at bank $E1 offset $016C + MANIFEST_IDX*32.
    ; Compute the byte offset (MANIFEST_IDX*32) via MUL, then compare all
    ; 16 words of H0_LO.. against it, byte-exact.
    lda MANIFEST_IDX
    mul #32
    tax
    lda #1
    sta VERIFY_OK
    ldy #0
    lda #16
    sta COPY_COUNTER
zp_verify_chunk_cmp_loop:
    ; CMP has no bank-explicit long+indexed form (only LDA/STA do - see
    ; this file's header note and ZPdevtools/lib/zp_dma.asm's point 3), so
    ; the manifest byte is loaded into CMP_TMP first and compared there.
    lda $E1016C,X
    sta CMP_TMP
    lda H0_LO,Y
    cmp CMP_TMP
    beq zp_verify_chunk_cmp_ok1
    lda #0
    sta VERIFY_OK
zp_verify_chunk_cmp_ok1:
    inx
    inx
    iny
    iny
    dec COPY_COUNTER
    bne zp_verify_chunk_cmp_loop
    rts

; ---- shared block-copy helpers (CHUNK_STAGE_BUF -> BLOCK_BUF) -------------
zp_zero_block_buf:
    ldy #0
    lda #32
    sta COPY_COUNTER
    lda #0
zp_zero_block_buf_loop:
    sta BLOCK_BUF,Y
    iny
    iny
    dec COPY_COUNTER
    bne zp_zero_block_buf_loop
    rts

zp_copy_full:
    ldx SRC_OFS
    ldy #0
    lda #32
    sta COPY_COUNTER
zp_copy_full_loop:
    lda CHUNK_STAGE_BUF,X
    sta BLOCK_BUF,Y
    inx
    inx
    iny
    iny
    dec COPY_COUNTER
    bne zp_copy_full_loop
    rts

zp_copy_partial:
    ldx SRC_OFS
    ldy #0
    sep #$20
zp_copy_partial_loop:
    lda REM_LO
    beq zp_copy_partial_done
    lda CHUNK_STAGE_BUF,X
    sta BLOCK_BUF,Y
    inx
    iny
    dec REM_LO
    bra zp_copy_partial_loop
zp_copy_partial_done:
    rep #$20
    rts

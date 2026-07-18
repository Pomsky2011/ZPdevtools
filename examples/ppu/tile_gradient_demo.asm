; Tile-based gradient demo
; Defines gradient tiles and draws them across the screen
;
; STALE: this predates the current tile API and will NOT assemble against
; the current ppuasm/PPU. STRTDEFTILE/ENDDEFTILE and HALT were removed (see
; docs/ppu/preset-e-and-shorthands.txt "Breaking changes"). Current
; equivalent: SETTILE reg,mode to select a tile, write pixel data directly
; to 0x0300-0x033F, SETPOS/MOVDP to 0x0200-0x0203 for position, then
; TILEDRAW to draw — see preset-e-and-shorthands.txt section 4.2's worked
; example, and use the HLT assembler shorthand instead of HALT.

init:
    ; Set 32-bit render mode
    SETRENDMOD 1

    ; Build R14 = 16
    CLR R14
    INC R14
    INC R14
    MUL R14, R14
    MUL R14, R14  ; R14 = 16

    ; Build R7 = 256 (I/O base for tile drawing)
    CLR R7
    ADD R7, R14
    MUL R7, R14   ; R7 = 256

    ; Build R1 = 1
    CLR R1
    INC R1

    ; Define Tile 0: Dark tile (all zeros - already done by reset)
    ; Define Tile 1: Bright tile (all 255s)

    ; Start defining Tile 1
    STRTDEFTILE

    ; Build R6 = 255
    CLR R6
    ADD R6, R14
    MUL R6, R14
    SUB R6, R1    ; R6 = 255

    ; Fill tile buffer at 0x0300-0x033F with 255
    ; Need to write 64 bytes = 32 16-bit words
    CLR R10
    ADD R10, R14
    ADD R10, R14
    ADD R10, R14  ; R10 = 48 (0x0300 >> 4 = 48, then *16)
    CLR DP
    ADD DP, R10
    MUL DP, R14
    MUL DP, R14   ; DP = 48 * 16 * 16 = way too much...

    ; Let me build 0x0300 differently
    ; 0x0300 = 768 = 3 * 256
    CLR DP
    ADD DP, R7    ; DP = 256
    ADD DP, R7    ; DP = 512
    ADD DP, R7    ; DP = 768 = 0x0300

    ; Write 255 to all 64 bytes of tile buffer
    ; Write as 32 16-bit words using MOVDP
    CLR R0        ; Counter (0-31)
    CLR R11
    ADD R11, R14
    ADD R11, R14  ; R11 = 32 (loop limit)

fill_tile_loop:
    MOVDP R6      ; Write 255 to current DP location
    INC DP
    INC DP        ; Move to next word
    INC R0
    CMP R0, R11
    JNG fill_tile_loop

    ; End tile definition, commit as Tile ID 1
    CLR R8
    INC R8        ; R8 = 1
    ENDDEFTILE 1

    ; Define Tile 2: Medium gradient tile (128)
    STRTDEFTILE

    ; Build R5 = 128
    CLR R5
    ADD R5, R14
    DEC R5
    DEC R5
    DEC R5
    DEC R5
    DEC R5
    DEC R5
    DEC R5
    DEC R5
    MUL R5, R14   ; R5 = 8 * 16 = 128

    ; Reset DP to 0x0300
    CLR DP
    ADD DP, R7
    ADD DP, R7
    ADD DP, R7

    ; Fill with 128
    CLR R0
fill_tile2_loop:
    MOVDP R5
    INC DP
    INC DP
    INC R0
    CMP R0, R11
    JNG fill_tile2_loop

    ; End tile definition, commit as Tile ID 2
    CLR R9
    ADD R9, R8
    INC R9        ; R9 = 2
    ENDDEFTILE 2

    ; Now draw tiles across the screen in a gradient pattern
    ; Draw dark (0), medium (2), bright (1) tiles
    ; Each tile is 8x8, so space them out

    ; Draw Tile 0 at (0, 0)
    CLR DP
    ADD DP, R7    ; DP = 0x0200
    ADD DP, R7

    CLR R0        ; X = 0
    MOVDP R0
    INC DP
    INC DP

    CLR R3        ; Y = 0
    MOVDP R3
    INC DP
    INC DP

    CLR R4        ; Tile ID = 0
    MOVDP R4
    INC DP
    INC DP

    CLR R12
    INC R12       ; Trigger value
    MOVDP R12     ; Draw tile

    ; Draw Tile 2 (medium) at (64, 64)
    CLR DP
    ADD DP, R7
    ADD DP, R7    ; DP = 0x0200

    ; Build R20 = 64
    CLR R20
    ADD R20, R14
    MUL R20, R14
    DEC R20
    DEC R20
    DEC R20
    DEC R20
    DEC R20
    DEC R20
    DEC R20
    DEC R20       ; R20 = 256 - 192 = 64

    MOVDP R20     ; X = 64
    INC DP
    INC DP
    MOVDP R20     ; Y = 64
    INC DP
    INC DP
    MOVDP R9      ; Tile ID = 2
    INC DP
    INC DP
    MOVDP R12     ; Draw tile

    ; Draw Tile 1 (bright) at (128, 128)
    CLR DP
    ADD DP, R7
    ADD DP, R7    ; DP = 0x0200

    ; Build R21 = 128
    CLR R21
    ADD R21, R5   ; R21 = 128

    MOVDP R21     ; X = 128
    INC DP
    INC DP
    MOVDP R21     ; Y = 128
    INC DP
    INC DP
    MOVDP R8      ; Tile ID = 1
    INC DP
    INC DP
    MOVDP R12     ; Draw tile

    ; Draw more tiles to fill screen...
    ; Draw Tile 0 at (192, 192)
    CLR DP
    ADD DP, R7
    ADD DP, R7

    ; Build R22 = 192
    CLR R22
    ADD R22, R21
    ADD R22, R20  ; R22 = 128 + 64 = 192

    MOVDP R22
    INC DP
    INC DP
    MOVDP R22
    INC DP
    INC DP
    MOVDP R4      ; Tile ID = 0
    INC DP
    INC DP
    MOVDP R12

    HALT

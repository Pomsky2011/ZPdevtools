; serial_echo.asm - the debug serial port as a console: print a banner, then echo every byte the host sends.
;
; Needs a dev unit (zeropoint_sdl --dev-unit): on a retail unit nothing reads the transmit queue, so the banner never
; arrives. Build and run (from ZPdevtools):
;   executables/cpuasm examples/cpu/serial_echo.asm /tmp/echo.bin
;   executables/zplink -o /tmp/echo.elf --cpu /tmp/echo.bin@0x8000 --entry 0x8000
;   executables/zpbuild /tmp/echo.elf -o /tmp/echo.zpb --title ECHO
;   zeropoint_sdl /tmp/echo.zpb --dev-unit --serial-tcp 5000        (then: nc 127.0.0.1 5000)
;
; SER_STATUS ($D8003C): bit 0 = a received byte is waiting, bit 1 = the transmit queue has room.
; SER_DATA   ($D8003D): read = next received byte, write = queue a byte (115200 baud: ~1476 CPU cycles each).
; The addresses are long (bank $D8), so they never mean direct page.
;
; no-roundtrip: the message below is data, which cpudisasm would print as instructions (tests/disasm/check.py skips this file)

.org $8000

start:
        sep #$30                ; 8-bit A, X, Y
        ldx #0
banner:
        lda >message,x          ; long indexed: the message is in bank $00 whatever DB is
        beq echo
wait_room:
        lda $D8003C
        and #$02
        beq wait_room
        lda >message,x          ; (A was clobbered by the status read)
        sta $D8003D
        inx
        bra banner

echo:
        lda $D8003C
        and #$01
        beq echo                ; wait for a byte from the host
        lda $D8003D
        pha
echo_room:
        lda $D8003C
        and #$02
        beq echo_room
        pla
        sta $D8003D             ; send it back
        bra echo

message:
        .byte $48, $65, $6C, $6C, $6F, $20, $66, $72, $6F, $6D, $20     ; "Hello from "
        .byte $5A, $65, $72, $6F, $50, $6F, $69, $6E, $74, $21, $0A, $00 ; "ZeroPoint!", newline, end

# Memory map

The CPU has a 24-bit address bus: 256 banks of 64 KiB, written `$BB:OOOO` (bank, offset) or `$BBOOOO`. Multi-byte values
are little-endian.

| Banks | What | Notes |
|-------|------|-------|
| `$00-$7F` | Cartridge ROM (8 MB) | Read-only; writes are ignored. Holds the vectors, direct-page reads and `JMP [addr]` pointers (all bank `$00`). |
| `$80-$9F` | Work RAM | The emulator maps 2 MB. **The console has 128 KB** (`$80:0000-$81:FFFF`); plan for that. The stack lives here. |
| `$A0` | APU window | 64 KB: the APU's address space, readable and writable from the CPU. |
| `$B0` | PPU window | 64 KB: the PPU's address space, including its framebuffer at `$B0:E000-$FFFF`. |
| `$BE-$BF` | Shadow work RAM | 128 KB of plain RAM. |
| `$D8` | I/O registers | `$D80000-$D8009F` are used; the rest of the bank is unmapped. |
| `$E0` | Boot ROM | 64 KB, read-only. `$E1` is reserved for it to grow into. |
| `$E2` | Signed-ROM metadata | Read-only, present only for a signed cartridge: the ZPB header and trailer. |
| everything else | Unmapped | Reads return `$FF`; writes are ignored. |

Reading an empty cartridge slot (banks `$00-$7F` and `$E2`) returns the pattern `CA FE CA FE ...`, which is neither
`$FF` (`HLT`) nor the poison value below, so a pulled cartridge is recognisable.

For a signed cartridge with a v3 trailer, bytes past the signed code region read as poison (`$DE`, `$AF` alternating)
until the Boot ROM has verified their 16 KiB chunk with `COP #$FF`. See [interrupts.md](interrupts.md) and the emulator's
`docs/zpb-format.md`.

## The stack

The stack is in **bank `$80`**, not bank `$00`. `SP` starts at `$01FF` and is a full 16 bits: it can use all of the bank,
and it wraps from `$0000` to `$FFFF`. `PHA`, `JSR` and friends store at `$80:SP` and then decrement `SP`, so `SP` always
points at the next free byte and the top of the stack is at `SP+1`. Stack-relative operands (`n,S`) are offsets from `SP`
in the same bank, so `LDA 1,S` reads the top byte.

Pushes go high byte first, so a 16-bit value sits low byte at the lower address, as everywhere else.

| Operation | Bytes pushed (first to last) |
|-----------|------------------------------|
| `JSR` | return address minus 1: high, low (2) |
| `CALL` | `PB`, then the address of the next instruction: high, low (3) |
| `BRK`, `COP`, IRQ, NMI, ABORT | `PB`, `PC` high, `PC` low, `P` (4) |
| `PHA` `PHX` `PHY` | 1 byte, or 2 if the register is 16-bit |
| `PHP` `PHB` `PHK` | 1 byte |
| `PHD` `PEA` `PEI` `PER` `PUSH` | 2 bytes |

## Vectors

All vectors are in bank `$00` of the cartridge, so they are part of your ROM image. Each is a 16-bit handler address;
the handler runs in bank `$00`.

| Address | Vector | Taken by |
|---------|--------|----------|
| `$00:FFE4` | COP | `COP #sig` (except the Boot ROM's `$F0-$FF` calls) |
| `$00:FFE6` | BRK | `BRK` |
| `$00:FFF8` | ABORT | `triggerAbort` (nothing raises it yet) |
| `$00:FFFA` | NMI | `triggerNMI` (nothing raises it yet) |
| `$00:FFFE` | IRQ | the maskable IRQ line: timers and DMA completion |

There is no reset vector (see [architecture.md](architecture.md)).

## I/O registers (bank `$D8`)

Names follow `include/zeropoint/zp_mmio.h`. "r" and "w" mean readable and writable. Unlisted offsets read 0 and ignore
writes.

### PPU control, `$D80000-$D8000F`

| Address | Name | |
|---------|------|--|
| `$D80000` | `PPUCTRL` | w: bit 0 start, bit 1 reset |
| `$D80001` | `PPUSTATUS` | r |
| `$D80002-03` | `PPUPC` | rw, live execution pointer |
| `$D80004-05` | `PPUSP` | rw (PPU register R61) |
| `$D80006-07` | `PPUDP` | rw (R63) |
| `$D80008` | `PPUREG_ADDR` | rw: pick a PPU register R0-R63 |
| `$D80009-0A` | `PPUREG_DATA` | rw: the picked register, low then high byte |
| `$D8000B-0C` | `PPU_VBLANK_INT` | rw (R59) |
| `$D8000D-0E` | `PPU_HBLANK_INT` | rw (R60) |

### APU control, `$D80010-$D8001F`

| Address | Name | |
|---------|------|--|
| `$D80010` | `APUCTRL` | w: bit 0 reset, bit 1 halt, bit 2 run |
| `$D80011` | `APUSTATUS` | r: bit 0 = halted |
| `$D80012-13` | `APUPC` | rw |
| `$D80014-15` | `APUSP` | rw |
| `$D80016` | `APU_ROMBANK` | w: banked APU ROM |
| `$D80017` | `APU_IOBANK` | w |
| `$D80018` | `APUREG_ADDR` | not implemented: writes are ignored |
| `$D80019` | `APUREG_DATA` | not implemented: reads 0, writes are ignored |
| `$D8001A` | `APUCLK` | rw: bit 0 = fast mode (APU at master / 8, at the price of bus contention) |

### DMA, `$D80020-$D8002F`

| Address | Name | |
|---------|------|--|
| `$D80020` | `DMA_RESET` | w: any value resets the config write index |
| `$D80021` | `DMA_CONFIG` | w: write **nine** bytes to this register; the ninth queues the transfer |
| `$D80021` | `DMA_STATUS` | r: `(queued << 4) \| active` |
| `$D80022` | `DMA_IRQ` | r: 1 while paused by an interrupt |
| `$D80023-2E` | `DMA_CHAN(n)` | r: state of channels 0 to 11 |

The nine config bytes and the four transfer modes are described in `lib/zp_dma.asm` and the emulator's `docs/dma.md`.

### Controllers and debug serial, `$D80030-$D8003F`

| Address | Name | |
|---------|------|--|
| `$D80030` | `P1_DIR` | r: eight direction bits (up-left `$80` ... left `$01`) |
| `$D80031` | `P1_CTRL` | r: `$80` pad press, `$40`/`$20` bumpers, `$10`/`$08` triggers, `$04` menu, `$02` pause, `$01` connected |
| `$D80032` | `P1_BTN` | r: face buttons 4,3,2,1 in bits 7-4 |
| `$D80038-3A` | `P2_DIR/CTRL/BTN` | r: same layout; reads 0 (unplugged) until a frontend plugs it in |
| `$D8003C` | `SER_STATUS` | r: bit 0 receive ready, bit 1 transmit has room; on a dev unit also bit 2 overrun, bit 3 transmit done |
| `$D8003D` | `SER_DATA` | rw: 255-byte queues. On a retail unit nothing drains the transmit side; on a dev unit it is a 115200-baud UART (one byte per 1476 CPU cycles) |
| `$D8003E` | `SER_CTRL` | w: bit 0 clears both queues, bit 1 clears the overrun flag (dev unit) |

The serial port is a UART only on a **dev unit**, a host setting a program cannot change (`zeropoint_sdl --dev-unit`). See the
emulator's `docs/debug-protocol.md` and `examples/cpu/serial_echo.asm`.

### Display status, `$D80040-$D80047` (read-only)

| Address | Name | |
|---------|------|--|
| `$D80040-41` | `DISP_SCANLINE` | r: current line, 0-260 (0-255 visible) |
| `$D80042-43` | `DISP_PIXEL` | r: pixel in the line, 0-339 (0-255 visible) |
| `$D80044` | `DISP_STATUS` | r: bit 0 V-blank, bit 1 H-blank, bit 2 visible area |
| `$D80045` | `DISP_MODE` | r: 0 = RGBA16, 1 = RGBA32 |

### System control, `$D80048-$D8004F`

| Address | Name | |
|---------|------|--|
| `$D80048` | `DEV_MODE` | r: bit 0 the old dev-mode byte (guest-writable, nothing acts on it), **bit 1 the dev-unit latch** (set by the host, no program can write it: the debug serial port is a UART and signature checks always pass) |
| `$D80049-4B` | `CART_ENTRY` | r: cartridge entry point (low, middle, bank) |

### Timers, `$D80050-$D80052`

Eight independent timers share three registers, one bit each, in the order `V H S Q E T A R` (bit 7 down to bit 0).

| Address | Name | |
|---------|------|--|
| `$D80050` | `TIMER_CONTROL` | rw: 1 = timer runs |
| `$D80051` | `TIMER_STATUS` | r: 1 = expired. **Write 1 to clear.** |
| `$D80052` | `TIMER_INT_ENABLE` | rw: 1 = this timer may raise an IRQ |

| Bit | Timer | Period in master cycles | Real time |
|-----|-------|------------------------|-----------|
| 7 `V` | V-blank | 1,133,523 | 16.667 ms |
| 6 `H` | H-blank | 16,604 | 244 µs |
| 5 `S` | second | 68,011,355 | 1 s |
| 4 `Q` | quarter second | 17,002,839 | 250 ms |
| 3 `E` | eighth second | 8,501,419 | 125 ms |
| 2 `T` | 1/1024 second | 66,417 | 976 µs |
| 1 `A` | about a millisecond | 68,010 | 1 ms |
| 0 `R` | 60 V-blanks | 68,011,380 | ~1 s |

These timers count on their own; they are **not** locked to the display (they are free-running, and their phase is not tied to the raster). Since core revision 13 a display frame is
261 lines of 4343 cycles, 1,133,523 master cycles, which is exactly the `V` timer's period, so the `V` timer fires once per display frame. (Before, a frame was 340-cycle
lines, 88,740 cycles or 1.3 ms, and the `V` timer fired about every 12.8 frames.)

### Interrupt controller, `$D80058-$D80059`

| Address | Name | |
|---------|------|--|
| `$D80058` | `IRQ_STATUS` | r: pending sources. **Write 1 to clear.** Bits: `$01` V-blank, `$02` H-blank, `$04` timer, `$08` DMA |
| `$D80059` | `IRQ_ENABLE` | rw: top-level mask, `$FF` at reset |

### BLAKE2s engine, `$D80060-$D8009F`

A hardware hash of cartridge ROM, used by the Boot ROM. `$D80060-62` source address, `$D80064-67` length,
`$D80068` command (write 1 to start, `$80` to reset), `$D80069` status (bit 0 busy, bit 1 done, bit 2 rejected),
`$D80080-9F` the 32-byte digest. See the emulator's `docs/blake2s-engine.md`.

# DEF88186 Memory Mapping Reference

> **Note**: This is an early design-time map. Several regions below (APU/PPU
> window sizes, `$D8-$DF`, `$E0-$FF`) predate the CPU's current
> implementation and no longer match it exactly — see `ZeroPoint/CLAUDE.md`'s
> "CPU Memory Map" for the authoritative, implementation-accurate layout
> (in particular: the PPU/APU windows are single 64 KB banks at `$B0`/`$A0`
> with the rest of their ranges unmapped, `$D8` is 96 bytes of I/O registers
> rather than "reserved," and the boot/security region below has been split
> further — see the correction in that section).

## Address Space Overview

- **Total Space**: 16 MB (16,777,216 bytes)
- **Organization**: 256 banks × 64 KB each
- **Address Format**: `$BB:OOOO` (BB = bank, OOOO = offset)
- **Bank Size**: 65536 bytes (0x10000) per bank

## Bank Map by Range

### Banks $00-$7F (128 banks, 8 MB) - ROM
**Purpose**: Program ROM, system code, cartridge data

### Banks $80-$9F (32 banks, 2 MB) - Work RAM
**Purpose**: General-purpose RAM, game save data, buffers

### Banks $A0-$AF (16 banks, 1 MB) - APU Communication
**Purpose**: APU memory window and expansion
- `$A0:0000-$A0:FFFF`: APU memory window (64 KB)
- `$A1:0000-$AF:FFFF`: Reserved for APU expansion

### Banks $B0-$B7 (8 banks, 512 KB) - PPU Communication
**Purpose**: PPU memory window and expansion
- `$B0:0000-$B0:FFFF`: PPU memory window (64 KB)
- `$B1:0000-$B1:FFFF`: PPU tile data upload
- `$B2:0000-$B2:FFFF`: PPU palette data
- `$B3:0000-$B7:FFFF`: Reserved for PPU expansion

### Banks $B8-$BD (6 banks, 384 KB) - Expansion
**Purpose**: Reserved for future expansion

### Banks $BE-$BF (2 banks, 128 KB) - Work RAM (Shadow)
**Purpose**: Large work area for game data
- `$BE:0000-$BE:FFFF`: Primary work RAM (64 KB)
- `$BF:0000-$BF:FFFF`: Secondary work RAM (64 KB)

Common usage:
- Sprite attribute tables
- Tile map data
- Decompression buffers
- Dynamic allocation

### Banks $C0-$DF (32 banks, 2 MB) - Hardware Regions
**Purpose**: AROM, coprocessor registers, system memory

Special regions:
- `$C0-$C7`: AROM (APU ROM banks)
- `$C8-$CF`: Additional AROM
- `$D0-$D7`: Coprocessor registers, initial PPUMEM
- `$D8-$DF`: Reserved

### Banks $E0-$FE (31 banks, 1.984 MB) - Boot / Security
**Purpose**: Boot ROM, signed-ROM metadata, system ROM

As actually implemented (see `ZeroPoint/CLAUDE.md`'s CPU Memory Map and
`ZeroPoint/docs/zpb-format.md`), this range is not one undifferentiated
blob:
- `$E0`: Boot ROM (64 KB), read-only, mapped at construction. Hardware reset
  lands the CPU here instead of bank `$00` whenever a boot ROM is loaded.
- `$E1`: reserved for Boot ROM growth past 64 KB (a >64 KB Boot ROM spans
  `$E0-$E1` as one region).
- `$E2`: signed-ROM metadata — read-only, mapped only when a zpbuild-signed
  ROM is loaded. Holds the raw 64-byte ZPB header, the "ZPSG" trailer
  (magic/version/keysize/siglen/digest/signature, plus `codeSize` for
  trailer versions 2/3 and the per-chunk BLAKE2s manifest for version 3),
  and, for version 3, the chunk-verified bitmap immediately after the
  manifest. Deliberately kept one bank clear of `$E0` so a Boot ROM
  spanning `$E0-$E1` can't collide with it.
- `$E3-$FE`: unmapped/reserved.

### Bank $FF (1 bank, 64 KB) - Final Boot ROM
**Purpose**: Last initialized values, final boot sector

- `$FF:0000-$FF:DFFF`: System initialization data
- `$FF:E000-$FF:FFFF`: Interrupt vectors and boot code

## Banking Registers

The DEF88186 uses three bank registers:

1. **PB (Program Bank)** - Bank for instruction fetches
2. **DB (Data Bank)** - Default bank for data access
3. **D (Direct Page)** - Not a bank, but 16-bit offset for DP addressing

## Bank Selection by Addressing Mode

| Mode | Bank Used |
|------|-----------|
| Implied/Register | No memory access |
| Immediate | PB (instruction stream) |
| Absolute | DB |
| Absolute Long | Specified in instruction |
| Direct Page | Bank 0 (always) |
| Indirect | Pointer in bank 0, data uses DB |
| Indirect Long | Pointer in bank 0, data bank from pointer |
| Stack | Bank 0 (always) |

## Setting Bank Registers

```asm
; Set Data Bank (DB)
LDA #$7E
SDB                 ; DB = $7E (DEF88186 extension)

; Set Program Bank (PB)
JMP $020000         ; JMP long sets PB = $02

; Set Direct Page (D)
LDA #$2000
TCD                 ; D = $2000
```

## Memory Access Examples

```asm
; Access within current data bank
LDA $8000           ; Reads from DB:$8000

; Access across banks
LDA $7E8000         ; Reads from $7E:$8000 (absolute long)

; Access via pointer
LDA [$10]           ; Reads from 24-bit pointer at bank 0 DP $10
```

## Notes

- All banks are 64 KB (65536 bytes), not 4 KB
- Bank boundaries are at multiples of $10000
- Some regions vary by system model (check specifications)
- WRAM availability in banks $A0-$BF is model-specific
- Always use symbolic constants for memory regions

## References

See `/Users/alexanderwhite/Documents/Code/ZPdevtools/docs/cpu/memory-map.txt` for detailed memory map documentation.

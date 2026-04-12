# Memory Mapping Documentation

## Overview

The DEF88186 has a **24-bit address space** organized as:
- **256 banks × 64 KB each** = 16 MB total
- Each bank is **65536 bytes** (0x10000), not 4096 bytes
- Address format: `$BB:OOOO` where BB = bank (8 bits), OOOO = offset (16 bits)

## Documentation Files

### Memory-Mapping.md (RECOMMENDED)
**Clear bank-by-bank breakdown**
- Shows each bank range and its purpose
- Explains banking registers (PB, DB, D)
- Provides examples of bank switching
- **Use this as your primary reference**

### Memory-Mapping-Regions.csv
**Cross-reference table showing what's at each offset across bank ranges**
- Rows: Offset ranges within banks (16 rows covering $0000-$FFFF)
- Columns: Bank ranges (8 columns covering banks $00-$FF)
- Shows what memory type exists at each offset in different bank ranges
- Example: Offset $0000-$0FFF contains ROM in banks $00-$7F, but Expansion Space in banks $80-$9F

### Memory Mapping.csv (OLD - CONFUSING)
**Original file with confusing labels**
- ⚠️ **ISSUE**: Labels rows as "Page\Bank" but pages are NOT banks
- ⚠️ **ISSUE**: Uses 4 KB chunks, but banks are 64 KB
- The row labels (0000-0FFF, 1000-1FFF, etc.) are **offset ranges**, not bank numbers
- Each row represents a 4 KB region **within** a bank
- Kept for reference but should not be used for new development

## Key Points

1. **Banks are 64 KB**: Each bank spans addresses $0000-$FFFF (65536 bytes)
2. **Banks increment every 64 KB**: Bank 0 = $00:0000-$00:FFFF, Bank 1 = $01:0000-$01:FFFF, etc.
3. **Offsets within banks**: The same offset (e.g., $8000) in different banks points to different physical memory
4. **Banking is NOT paging**: Don't confuse 4 KB pages with 64 KB banks

## Examples

### Accessing Different Banks
```asm
; Access bank 0, offset $8000
LDA $00:8000        ; Absolute long

; Access bank $7E, offset $8000 (completely different memory)
LDA $7E:8000        ; Absolute long

; Set data bank and use absolute addressing
LDA #$7E
SDB                 ; DB = $7E
LDA $8000           ; Now accesses $7E:8000
```

### Understanding the Tables

The CSV shows that at offset $0000-$0FFF:
- Banks $00-$7F: Contains ROM
- Banks $80-$9F: Contains Expansion Space
- Banks $A0-$BF: Contains WRAM (model-specific)
- Banks $E0-$FF: Contains Boot ROM

This means:
- `$00:0000` = ROM byte 0
- `$80:0000` = Expansion space byte 0
- `$E0:0000` = Boot ROM byte 0

All at the same offset, but different banks = different physical locations!

## References

For complete details, see:
- `/Users/alexanderwhite/Documents/Code/ZPdevtools/docs/cpu/memory-map.txt`
- `/Users/alexanderwhite/Documents/Code/ZeroPoint/CLAUDE.md`

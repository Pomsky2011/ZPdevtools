# ZeroPoint APU Documentation

Complete documentation for the ZeroPoint Audio Processing Unit (APU) and DSP system.

## Getting Started

**New to the APU?** Start here:
1. Read `overview.txt` - High-level architecture and capabilities
2. Skim `instruction-set.txt` - Get familiar with available instructions
3. Read `programming-guide.txt` - See practical examples

## Documentation Files

### overview.txt (6.3 KB)
High-level overview of the APU architecture, performance characteristics, and
major subsystems. Start here to understand what the APU can do.

**Contents:**
- Core specifications (8-bit RISC, 4.2 MHz, 1.048 MIPS)
- Subsystem overview (CPU, MMP, SST, DEF88186)
- Memory organization summary
- Comparison with PPU
- Performance tips

### instruction-set.txt (21 KB)
Complete reference for all 41 APU instructions with detailed descriptions,
encoding formats, examples, and usage notes.

**Contents:**
- Instruction format (16-bit: 5-bit opcode + 11-bit operands)
- Full instruction reference (NOP through CML)
- Opcode summary table
- Timing information
- Programming tips

### memory-map.txt (12 KB)
Detailed memory layout, region descriptions, and access patterns.

**Contents:**
- Complete memory map ($0000-$FFFF)
- MMP control registers ($0000-$00FF)
- RAM regions (General purpose, ARAM)
- DEF88186 interface ($1000-$1FFF)
- SST and STL regions ($7000-$CFFF)
- AROM banking system ($D000-$FFFF)
- Memory access examples

### registers.txt (11 KB)
Register descriptions, usage conventions, and best practices.

**Contents:**
- General purpose registers (X, Y, R0-R31)
- Special purpose registers (PC, RP, DP, DB, BF)
- Function calling conventions
- Stack management
- Register preservation examples

### sst.txt (12 KB)
Sample Storage System format specification.

**Contents:**
- Block format (16 bytes: 4-byte header + 12 samples)
- Header field definitions (loop count, clamping, etc.)
- STL (Sample Table List) format
- Clamping system for dynamic range
- Sample capacity calculations
- Programming examples

### mmp.txt (14 KB)
Music Mixing Processor register reference and operation guide.

**Contents:**
- MMP architecture (16 stereo channels, effects)
- Memory-mapped register layout ($0000-$00FF)
- Channel pitch control (resampling)
- Channel volume control
- Reverb and echo configuration
- STL address registers
- Gaussian interpolation
- Advanced techniques (panning, Doppler, arpeggios)

### programming-guide.txt (22 KB)
Practical examples and programming patterns for common tasks.

**Contents:**
- Minimal program structure
- Example 1: Playing a single sample
- Example 2: Multi-sample music sequence
- Example 3: Sound effect with pitch sweep
- Example 4: Stereo panning
- Example 5: Reverb and echo
- Example 6: Real-time waveform synthesis
- Example 7: Dynamic channel allocation
- Example 8: Music player with CPU commands
- Common patterns (16-bit math, delays, memory copy)
- Tips and debugging checklist

## Quick Reference

### Key Specifications
- **Architecture:** 8-bit RISC
- **Clock:** 4.2 MHz, 4 CPI → 1.048 MIPS
- **Memory:** 64 KiB addressable (32K words)
- **AROM:** 448 KiB (18 banks × 24 KiB)
- **Channels:** 16 stereo (32 mono total)
- **Sample Storage:** 12,288 samples (16 KiB SST)

### Common Instruction Patterns
```
; Load immediate
SCR R0, 0x42      ; R0 = 0x42

; 16-bit memory access
SDP $10           ; Data page = $10
SDB $50           ; Data byte = $50
SBF 0             ; Low byte
STR $50, R0       ; Load low byte of $1050
SBF 1             ; High byte
STR $50, R1       ; Load high byte of $1050

; Jump to address
SRP $D0           ; ROM page = $D0
JMP $50           ; Jump to $D050

; Function call
CCF function_id   ; Call function

; Loop
LST 0, 10         ; Loop ID 0, 10 iterations
  ; loop body
LFN 0             ; End loop
```

### MMP Quick Start
```
; Play sample on channel 0
SDP $00           ; MMP registers

; Set pitch to 1.0×
SDB $00
WRH $10
WRL $00

; Set volume to 128
SDB $20
WRH $00
WRL $80

; Set STL address (starts playback)
SDB $54
WRH $70           ; Point to STL entry at $7000
WRL $00
```

## File Organization Suggestion

For larger projects, organize your APU code:

```
/apu_project/
  /src/
    main.asm        - Entry point and initialization
    music.asm       - Music player routines
    sfx.asm         - Sound effect functions
    mmp.asm         - MMP control helpers
  /data/
    samples.asm     - Sample data (SST blocks)
    stl.asm         - STL entries
    music_data.asm  - Music sequences
  /include/
    constants.asm   - Register addresses, magic numbers
    macros.asm      - Common code patterns
```

## Additional Resources

- **ZeroPoint Main Docs:** /Users/alexanderwhite/Documents/Code/ZeroPoint/CLAUDE.md
- **PPU Docs:** /Users/alexanderwhite/Documents/Code/ZPdevtools/docs/ppu/
- **Assembler:** /Users/alexanderwhite/Documents/Code/ZPdevtools/zpasm.c

## Version History

- 2025-10-19: Initial comprehensive documentation created
  - 7 documentation files covering all aspects of APU/DSP
  - Total: ~98 KB of documentation
  - Examples and practical programming patterns included

## Contributing

When updating this documentation:
1. Keep examples concise and focused
2. Explain "why" not just "what"
3. Include both conceptual and practical information
4. Cross-reference related files
5. Update this README when adding new files

## License

See project LICENSE file in ZeroPoint repository.

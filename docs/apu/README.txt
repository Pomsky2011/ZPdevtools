# ZeroPoint APU Documentation

Complete documentation for the ZeroPoint Audio Processing Unit (APU) and DSP system.

## Getting Started

**New to the APU?** Start here:
1. Read `overview.txt` - High-level architecture and capabilities
2. Skim `instruction-set.txt` - Get familiar with available instructions
3. Read `programming-guide.txt` - See practical examples

## Documentation Files

### overview.txt
High-level overview of the APU architecture, performance characteristics, and
major subsystems. Start here to understand what the APU can do.

**Contents:**
- Core specifications (8-bit RISC, 4.25 MHz, 1.06 MIPS)
- Subsystem overview (CPU, MMP, SST, DEF88186)
- Memory organization summary
- Comparison with PPU
- Performance tips

### instruction-set.txt
Complete reference for all 57 APU instruction mnemonics with detailed descriptions,
encoding formats, examples, and usage notes.

**Contents:**
- Instruction format (16-bit: 5-bit opcode + 11-bit operands)
- Full instruction reference (NOP through CML)
- Opcode summary table
- Timing information
- Programming tips

### memory-map.txt
Detailed memory layout, region descriptions, and access patterns.

**Contents:**
- Complete memory map ($0000-$FFFF)
- MMP control registers ($0000-$00FF)
- RAM regions (General purpose, ARAM)
- DEF88186 interface ($1000-$1FFF)
- SST and STL regions ($7000-$CFFF)
- AROM banking system ($D000-$FFFF)
- Memory access examples

### registers.txt
Register descriptions, usage conventions, and best practices.

**Contents:**
- General purpose registers (just X and Y; R2-R127 are debug-only scratch slots)
- Special purpose registers (PC, SP, RP, DP, DB, FLAGS; BF is reserved)
- Function calling conventions
- Stack management
- Register preservation examples

### sst.txt
Sample Storage System format specification.

**Contents:**
- Block format (16 bytes: 4-byte header + 12 samples)
- Header field definitions (final-block, one-shot, loop count, clamping)
- STL (Sample Table List) format and the loop point
- Clamping system for dynamic range (defined, not yet honoured by the mixer)
- Sample capacity calculations
- Programming examples

### mmp.txt
Music Mixing Processor register reference and operation guide.

**Contents:**
- MMP architecture (16 mono channels with per-channel pan)
- Memory-mapped register layout ($0000-$00FF)
- Channel pitch control (resampling with cubic interpolation)
- Channel volume and pan control
- STL address registers (starting and stopping playback)
- Initialization, channel allocation
- Advanced techniques (panning, Doppler, arpeggios)

### zpa.txt and zpsnd.txt
The ZPA sound pack and BIOS sequencer design (the pack format, the score event stream,
the CPU mailbox protocol), and the `zpsnd` baker that builds packs from a text score.
The baker and its reference interpreter exist; the BIOS engines are not written yet.

### programming-guide.txt
Practical examples and programming patterns for common tasks.

**Contents:**
- Minimal program structure
- Example 1: Playing a single sample
- Example 2: Pitch sweep sound effect
- Example 3: Stereo panning
- Example 4: CPU command processor
- Common patterns (loops, 16-bit math, delays, memory copy)
- Tips and debugging checklist

## Quick Reference

### Key Specifications
- **Architecture:** 8-bit RISC
- **Clock:** 4.250710 MHz, 4 CPI → 1.06 MIPS (about 22 instructions per 48 kHz sample)
- **Memory:** 64 KiB addressable (byte-wide; instructions are 16-bit)
- **AROM:** 448 KiB array, 18 banks (24 KiB stride, 12 KiB window)
- **Channels:** 16 mono, each with pan
- **Sample Storage:** 12,288 samples (16 KiB SST)

### Common Instruction Patterns
```
; Load immediate
LDA X, 0x42       ; X = 0x42

; 16-bit memory access: two byte accesses, low byte at the lower address
SDP $10           ; Data page = $10
STR $50, X        ; X = low byte of the word at $1050
STR $51, Y        ; Y = high byte

; Jump to address
SRP $80           ; ROM page = $80
JMS $50           ; Jump to $8050 (no return address pushed)

; Function call
JSR func          ; Call (RET returns)

; Loop
LST 0, 10         ; Loop ID 0, 10 iterations
  ; loop body
LFN 0             ; End loop
```

### MMP Quick Start
```
; Play sample on channel 0
SDP $00           ; MMP registers

; Set pitch to 1.0x ($1000: high byte at $01, low byte at $00)
LDA X, $10
STA X, $01
ZOR X
STA X, $00

; Set volume to 128
LDA X, $80
STA X, $20

; Set STL address (starts playback): low byte first, high byte last
ZOR X
STA X, $50
LDA X, $70        ; Point to STL entry at $7000
STA X, $51
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

## Building the APU Assembler

```bash
cd ZPdevtools
make assemblers
```

This builds `executables/apuasm` (source: `src/apuasm.c`) along with ppuasm and cpuasm.

## Using the APU Assembler

```bash
executables/apuasm input.asm [output.bin]
```

If no output file is specified, uses the input filename with `.bin` extension.

### Example
```bash
executables/apuasm examples/apu/hello.asm examples/apu/hello.bin
```

## Building the APU Emulator

The APU emulator is built as part of the ZeroPoint project:

```bash
cd ZeroPoint
mkdir -p build
cd build
cmake ..
make
```

This creates:
- `bin/test_apu` - APU test runner (no audio)
- `bin/run_apu_demo` - APU demo runner (with SDL audio output)

## Testing APU Programs

### Headless Testing
```bash
./bin/test_apu program.bin [max_cycles]
```

Example:
```bash
./bin/test_apu examples/apu/hello.bin 10000
```

### Audio Testing
```bash
./bin/run_apu_demo program.bin
```

Press ESC or Q to quit.

## Example Programs

Located in `ZPdevtools/examples/apu/`:

- **hello.asm** - Simple register operations (X=42, Y=100, X=X+Y)
- **counter.asm** - Hardware loop counting from 0 to 10
- **play_sample.asm** - Write a sine sample into SST/STL and play it (programming guide, Example 1)
- **pitch_sweep.asm** - Laser-style pitch sweep (Example 2)
- **pan_sweep.asm** - Left-to-right pan sweep (Example 3)
- **mailbox_commands.asm** - CPU command processor (Example 4)

Assemble them with:
```bash
executables/apuasm examples/apu/play_sample.asm
```
The three sweep/command programs expect the sample from `play_sample.asm` to
already be in SST/STL (Example 1 loads it).

## Additional Resources

- **ZeroPoint Main Docs:** ZeroPoint/CLAUDE.md
- **PPU Docs:** ZPdevtools/docs/ppu/
- **PPU Assembler:** ZPdevtools/src/ppuasm.c
- **APU Assembler:** ZPdevtools/src/apuasm.c

## Version History

- 2025-10-19: Initial comprehensive documentation created
  - 7 documentation files covering all aspects of APU/DSP
  - Examples and practical programming patterns included
- 2026-11-10: Reconciled the docs with the emulator and assembler
  - Removed instructions that no longer exist (SBF, WRH/WRL, RBC, IO/function ops)
  - Stack direction, clock rate, bank layout, JMP/JNZ encodings, SST flag bits corrected
  - Examples rewritten with programs that assemble and run

## Contributing

When updating this documentation:
1. Keep examples concise and focused
2. Explain "why" not just "what"
3. Include both conceptual and practical information
4. Cross-reference related files
5. Update this README when adding new files

## License

See project LICENSE file in ZeroPoint repository.

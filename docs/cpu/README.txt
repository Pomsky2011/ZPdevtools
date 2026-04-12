DEF88186 Main CPU Documentation Index
=====================================

The DEF88186 is the main CPU of the ZeroPoint fantasy console. It is a hybrid
processor combining the 65C816 architecture with 8086-inspired enhancements.
As the system master, it has arbitration over all resources including the PPU
(graphics), APU (audio), memory, and I/O.

While the PPU runs at 64 MHz for racing-the-beam graphics rendering, the main
CPU handles general-purpose computing, game logic, physics, and system
coordination.

## Quick Start

1. Read overview.txt for architecture fundamentals
2. Review addressing-modes.txt to understand memory access
3. Consult instruction-set.txt for complete opcode reference
4. Study programming-guide.txt for practical examples
5. Reference flags.txt when working with processor status

## Documentation Files

### overview.txt
- Architecture fundamentals (hybrid 65C816/8086)
- Register descriptions (A, X, Y, SP, D, PC, PB, DB, P)
- Memory map and banking (24-bit addressing, 16 MB)
- Custom extensions (LOOP/LPEND, MUL/DIV, XCHG, etc.)
- Comparison with PPU and APU
- Programming model and initialization

### instruction-set.txt
- All 256 instructions organized by category:
  * Data Transfer (LDA, STA, LDX, STX, LDY, STY, STZ)
  * Arithmetic (ADC, SBC, MUL, DIV, INC, DEC)
  * Logical (AND, ORA, XOR, BIT)
  * Shifts/Rotates (ASL, LSR, ROL, ROR, SHL, SHR, RCL)
  * Branches (BMI, BRA, BRL, BVS, BCS/BGE, BEQ)
  * Jumps/Calls (JMP, JSR, CALL, RTS, RTL, RTI, RET)
  * Stack (PHA, PLA, PHX, PHY, PHP, PLP, PEA, PEI, PER, PUSH)
  * Comparison (CMP, CPX, CPY)
  * Register Transfer (TXY, TYA, TYX, XBA, XCHG)
  * Flags (SEP, REP, SEC, SED, SEI, CLC, CLD, CLI, CLV)
  * Control (NOP, BRK, COP, WAI, HLT, LOOP, LPEND)
  * Block Move (MVN, MVP)
  * Special (SDB)
- Cycle counts and addressing modes
- Footnotes explaining conditional timing

### addressing-modes.txt
- Complete addressing mode reference
- Examples and memory access patterns:
  * Immediate, Accumulator, Implied
  * Absolute, Absolute Long
  * Direct Page (fast zero-page-like access)
  * Indexed (X and Y)
  * Indirect, Indirect Long
  * Stack Relative
  * PC Relative
  * Block Move
- Performance considerations
- Mode selection guide

### flags.txt
- Processor Status Register (P) detailed reference
- Individual flag descriptions:
  * C (Carry): Arithmetic carry, bit shifts, unsigned comparisons
  * Z (Zero): Result is zero
  * I (Interrupt Disable): IRQ masking
  * D (Decimal): BCD mode for ADC/SBC
  * X (Index Size): 8-bit vs 16-bit X/Y registers
  * M (Memory/Accumulator Size): 8-bit vs 16-bit accumulator
  * V (Overflow): Signed arithmetic overflow
  * N (Negative): Sign bit of result
- Flag manipulation (SEP, REP, PHP, PLP)
- Common patterns and gotchas
- Flag effects by instruction category

### programming-guide.txt
- Practical programming patterns and examples:
  * System initialization
  * 16-bit and 32-bit arithmetic
  * Hardware multiply/divide usage
  * Memory access patterns (direct page, pointers, stack)
  * Control flow (loops, switches, conditionals)
  * Register exchange operations
  * Block memory operations
  * Subroutines and function calls
  * Interrupt handling
  * Mixing 8-bit and 16-bit code
  * Performance optimization
  * Common algorithms
  * Debugging techniques

### DEF88186.csv
- Raw instruction set data (source material)
- CSV format: Assembler, Alias, Name, Hex, Addressing, Flags, Bytes, Cycles
- Note: Some cycle counts may be incomplete or inaccurate (use with caution)
- Derived from Super Famicom developer documentation

## Key Features

### 65C816 Base Architecture
- Full 65816 instruction set compatibility
- 24-bit addressing (16 MB address space)
- 8/16-bit configurable data sizes
- Rich addressing modes (14+ modes)
- Direct page for fast variable access
- Stack-relative addressing
- Block move instructions

### 8086-Inspired Extensions
- **Hardware Loops**: LOOP/LPEND for efficient iteration
- **Hardware Multiply**: MUL instruction (8-13 cycles)
- **Hardware Divide**: DIV instruction (8 cycles)
- **Register Exchange**: XCHG for atomic swaps
- **Extended Shifts**: SHL/SHR on index registers
- **Direct Bank Set**: SDB for fast bank switching
- **System Calls**: CALL/RET for OS services

## Performance Characteristics

Strengths:
1. Fast arithmetic via hardware MUL/DIV
2. Efficient loops with LOOP/LPEND
3. Flexible memory access via multiple addressing modes
4. Fast block transfers with MVN/MVP
5. 8/16-bit mode switching for size/speed tradeoffs

Typical Instruction Timing:
- Simple operations: 2 cycles
- Register loads/stores: 2-7 cycles
- Hardware multiply: 8-13 cycles
- Hardware divide: 8 cycles
- Block move: 1 cycle per byte
- System calls: 16 cycles

## Programming Tips

1. **Use Direct Page**: Keep frequently-accessed variables in direct page
   - Page-align D register (D = $xx00) to avoid +1 cycle penalty
   - Direct page access is 2 bytes, 3 cycles (vs 3 bytes for absolute)

2. **Hardware vs Software**:
   - Use MUL for multiplication (8-13 cycles vs 100+ in software)
   - Use DIV for division (8 cycles)
   - Use LOOP/LPEND for simple loops

3. **Mode Management**:
   - Use REP #$30 for 16-bit mode (both A and X/Y)
   - Use SEP #$20 for 8-bit accumulator, REP #$10 for 16-bit indexes
   - Save/restore flags with PHP/PLP around mode changes

4. **Memory Access**:
   - Use absolute mode when in current data bank
   - Use absolute long mode when crossing banks
   - Use indexed modes for arrays
   - Use indirect modes for pointers

5. **Optimization**:
   - Keep hot values in registers (A, X, Y)
   - Unroll small loops
   - Use block moves for large copies
   - Minimize bank crossings

## Comparison with Other Processors

| Feature | DEF88186 | PPU | APU |
|---------|----------|-----|-----|
| Type | Main CPU | Graphics Co-processor | Audio Co-processor |
| Architecture | 16-bit CISC | 16-bit Microcode | 8-bit RISC |
| Clock | TBD | 64 MHz | 4.2 MHz |
| Purpose | General computing | Tile rendering | Audio synthesis |
| Arbitration | Master (controls all) | Slave | Slave |
| Memory | 16 MB (24-bit) | 64 KB | 64 KB + 448 KB AROM |
| Strengths | Flexible, powerful | Fast graphics | Audio mixing |

## Common Gotchas

1. **M and X flags change instruction encoding**
   - Immediate values change size based on flags
   - Code assembled for M=0 may crash if executed with M=1

2. **Decimal mode only affects ADC/SBC**
   - INC, DEC, and other arithmetic is always binary
   - Always CLD after SED

3. **SEP/REP affect multiple flags**
   - SEP #$30 sets both M and X
   - Can't set one while clearing the other in single instruction

4. **Changing M doesn't change accumulator value**
   - Switching to 8-bit mode hides high byte
   - High byte persists and reappears when switching back

5. **Stack operations respect M/X flags**
   - PHA pushes 1 or 2 bytes depending on M
   - Mismatched push/pull corrupts stack

## Related Documentation

Main System:
- ZeroPoint/CLAUDE.md - Complete system documentation
- ZeroPoint/docs/display.md - Display system details

PPU (Graphics):
- ZPdevtools/docs/ppu/ - PPU microcode documentation

APU (Audio):
- ZPdevtools/docs/apu/ - APU instruction set and audio system

## Development Status

The DEF88186 documentation is complete based on the CSV specification.

Documentation Status:
✓ Complete architecture documentation
✓ All 256 instructions documented
✓ All addressing modes explained
✓ Binary encoding reference
✓ Interrupt system guide
✓ Memory map guide
✓ Calling conventions and ABI
✓ Timing and performance guide
✓ 65816 vs 8086 comparison

Assembler Status:
✓ cpuasm - Full assembler implemented
✓ Supports all major instructions
✓ Label support with forward references
✓ Multiple addressing modes
✓ Little-endian encoding
✓ Directives (.org, .byte, .word)

Implementation Status:
- Hardware implementation: Not started
- Emulator: Not started
- Memory map: Specified but not implemented
- Clock speed: Not yet determined

Note: Some cycle counts may be incomplete or inaccurate (from CSV source)

This is a fantasy console specification, so details may evolve as the
system is implemented and tested.

## Credits

Documentation created by Claude Code based on DEF88186.csv specification.
CSV data derived from Super Famicom (SNES) developer documentation.

## License

See ZeroPoint/LICENSE for project license.

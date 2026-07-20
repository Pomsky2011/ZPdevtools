/*
 * CPU Disassembler for DEF88186
 *
 * Disassembles DEF88186 binary files into human-readable assembly.
 * Supports all 256 opcodes including 65816 base and 8086 extensions.
 *
 * Usage: cpudisasm <input.bin> [-o output.asm] [-s start_addr] [-e end_addr]
 *
 * Options:
 *   -o <file>    Output to file instead of stdout
 *   -s <addr>    Start address (hex, default: 0)
 *   -e <addr>    End address (hex, default: file size)
 *   -c           Show comments with instruction info
 *   -b           Show bytes alongside assembly
 */

#include <stdio.h>
#include <stdlib.h>
#include "compat.h"
#include <string.h>
#include <ctype.h>

#undef MAX_FILE_SIZE
#define MAX_FILE_SIZE (16 * 1024 * 1024)  /* 16 MB max */

/* Addressing modes - mirrors ZPdevtools/src/cpuasm.c's AM_* enum (same
 * names, same operand syntax) so the assembler and disassembler stay easy
 * to cross-check by hand, plus three disassembler-only AM_REG_* entries for
 * the SHL/SHR/DIV register-operand forms that cpuasm.c special-cases
 * directly in assemble_line() rather than modeling as a real addressing
 * mode (there's no memory operand at all - the register name IS the
 * operand, and it selects the opcode). */
typedef enum {
    AM_IMPLIED,                    /* NOP */
    AM_ACCUMULATOR,                /* INC A */
    AM_IMMEDIATE,                  /* LDA #$42 */
    AM_ABSOLUTE,                   /* LDA $1234 */
    AM_ABSOLUTE_X,                 /* LDA $1234,X */
    AM_ABSOLUTE_Y,                 /* LDA $1234,Y */
    AM_ABSOLUTE_LONG,              /* LDA $123456 */
    AM_ABSOLUTE_LONG_X,            /* LDA $123456,X */
    AM_ABSOLUTE_LONG_Y,            /* DIV $123456,Y */
    AM_DIRECT_PAGE,                /* LDA $12 */
    AM_DP_X,                       /* LDA $12,X */
    AM_DP_Y,                       /* LDX $12,Y */
    AM_DP_INDIRECT,                /* LDA ($12) */
    AM_DP_INDIRECT_X,              /* LDA ($12,X) */
    AM_DP_INDIRECT_Y,              /* LDA ($12),Y */
    AM_DP_INDIRECT_LONG,           /* LDA [$12] */
    AM_DP_INDIRECT_LONG_Y,         /* LDA [$12],Y */
    AM_ABSOLUTE_INDIRECT,          /* JMP ($1234) */
    AM_ABSOLUTE_INDIRECT_LONG,     /* JMP [$1234] */
    AM_ABSOLUTE_INDEXED_INDIRECT,  /* JMP ($1234,X) */
    AM_STACK_RELATIVE,             /* LDA $12,S */
    AM_SR_INDIRECT_Y,              /* LDA ($12,S),Y */
    AM_PC_RELATIVE_LONG,           /* BRL label / PER label */
    AM_BLOCK_MOVE,                 /* MVP $12,$34 */
    AM_REG_X,                      /* SHL X / SHR X */
    AM_REG_Y,                      /* SHL Y / SHR Y */
    AM_REG_XY                      /* DIV X,Y */
} AddressingMode;

/* Instruction info structure */
typedef struct {
    const char *mnemonic;
    AddressingMode mode;
    int length;  /* Total instruction length in bytes, including the opcode */
} InstrInfo;

/* DEF88186 instruction table (256 opcodes).
 *
 * GENERATED against ZeroPoint/src/cpu_instructions.cpp (the emulator's
 * authoritative opcode dispatch table) and ZeroPoint/src/cpu.cpp (whose
 * addrXXX()/fetch()/fetch16()/fetch24() helpers give the real per-opcode
 * operand byte count) - not hand-transcribed from ZPdevtools/docs, the way
 * the previous ~80-entry stand-in table here was. Every entry below was
 * cross-checked against both files; the two known bugs in
 * cpu_instructions.cpp's own comments (0x6E "INC long" and 0x76 "DEC long")
 * are called out below since they're commented as long/4-byte but actually
 * call addrAbsolute() (3-byte instruction) - the mode/length columns here
 * reflect the real addrXXX() call, not the misleading comment text.
 *
 * Mnemonic aliases collapse to one canonical name here (COP/INT/SWI -> COP,
 * XOR/EOR -> XOR, HLT/STP/KIL -> HLT) since a disassembler only needs to
 * print one true name per opcode - see cpuasm.c's init_instructions() for
 * the alias registrations on the assembler side. */
static const InstrInfo instructions[256] = {
    {"NOP", AM_IMPLIED, 1},  /* 0x00: NOP */
    {"BIT", AM_IMMEDIATE, 3},  /* 0x01: BIT #const */
    {"BIT", AM_ABSOLUTE, 3},  /* 0x02: BIT addr */
    {"BIT", AM_ABSOLUTE_X, 3},  /* 0x03: BIT addr,X */
    {"BIT", AM_DIRECT_PAGE, 2},  /* 0x04: BIT dp */
    {"BIT", AM_DP_X, 2},  /* 0x05: BIT dp,X */
    {"BMI", AM_ABSOLUTE_LONG, 4},  /* 0x06: BMI long */
    {"BRA", AM_ABSOLUTE, 3},  /* 0x07: BRA long (opBRA() actually fetch16()s a 16-bit target) */
    {"BRL", AM_PC_RELATIVE_LONG, 3},  /* 0x08: BRL label */
    {"BVS", AM_ABSOLUTE, 3},  /* 0x09: BVS long (opBVS() actually fetch16()s a 16-bit target) */
    {"BCS", AM_ABSOLUTE_LONG, 4},  /* 0x0A: BCS long */
    {"BEQ", AM_ABSOLUTE_LONG, 4},  /* 0x0B: BEQ long */
    {"JMP", AM_ABSOLUTE_INDEXED_INDIRECT, 3},  /* 0x0C: JMP (addr,X) */
    {"JMP", AM_ABSOLUTE_INDIRECT, 3},  /* 0x0D: JMP (addr) */
    {"JMP", AM_ABSOLUTE_INDIRECT_LONG, 3},  /* 0x0E: JMP [addr] */
    {"JMP", AM_ABSOLUTE, 3},  /* 0x0F: JMP addr */
    {"JMP", AM_ABSOLUTE_LONG, 4},  /* 0x10: JMP long */
    {"JSR", AM_ABSOLUTE_INDEXED_INDIRECT, 3},  /* 0x11: JSR (addr,X) */
    {"JSR", AM_ABSOLUTE, 3},  /* 0x12: JSR addr */
    {"LOOP", AM_IMMEDIATE, 3},  /* 0x13: LOOP word */
    {"LPEND", AM_IMPLIED, 1},  /* 0x14: LPEND */
    {"CALL", AM_ABSOLUTE_LONG, 4},  /* 0x15: CALL long */
    {"RET", AM_IMPLIED, 1},  /* 0x16: RET */
    {"RTI", AM_IMPLIED, 1},  /* 0x17: RTI */
    {"RTL", AM_IMPLIED, 1},  /* 0x18: RTL */
    {"RTS", AM_IMPLIED, 1},  /* 0x19: RTS */
    {"SEP", AM_IMMEDIATE, 2},  /* 0x1A: SEP #const */
    {"SDB", AM_IMMEDIATE, 2},  /* 0x1B: SDB byte */
    {"WAI", AM_IMPLIED, 1},  /* 0x1C: WAI */
    {"TDC", AM_IMPLIED, 1},  /* 0x1D: TDC */
    {"TSC", AM_IMPLIED, 1},  /* 0x1E: TSC */
    {"TCS", AM_IMPLIED, 1},  /* 0x1F: TCS */
    {"TAX", AM_IMPLIED, 1},  /* 0x20: TAX */
    {"TXA", AM_IMPLIED, 1},  /* 0x21: TXA */
    {"TAY", AM_IMPLIED, 1},  /* 0x22: TAY */
    {"TCD", AM_IMPLIED, 1},  /* 0x23: TCD */
    {"TXY", AM_IMPLIED, 1},  /* 0x24: TXY */
    {"TYA", AM_IMPLIED, 1},  /* 0x25: TYA */
    {"TYX", AM_IMPLIED, 1},  /* 0x26: TYX */
    {"PUSH", AM_IMMEDIATE, 3},  /* 0x27: PUSH word */
    {"PEA", AM_ABSOLUTE, 3},  /* 0x28: PEA addr */
    {"PEI", AM_DP_INDIRECT, 2},  /* 0x29: PEI (dp) */
    {"PER", AM_PC_RELATIVE_LONG, 3},  /* 0x2A: PER label */
    {"PHA", AM_IMPLIED, 1},  /* 0x2B: PHA */
    {"PHB", AM_IMPLIED, 1},  /* 0x2C: PHB */
    {"PHD", AM_IMPLIED, 1},  /* 0x2D: PHD */
    {"PHK", AM_IMPLIED, 1},  /* 0x2E: PHK */
    {"PHP", AM_IMPLIED, 1},  /* 0x2F: PHP */
    {"PHX", AM_IMPLIED, 1},  /* 0x30: PHX */
    {"PHY", AM_IMPLIED, 1},  /* 0x31: PHY */
    {"PLA", AM_IMPLIED, 1},  /* 0x32: PLA */
    {"POPF", AM_IMPLIED, 1},  /* 0x33: POPF (aka PLP) */
    {"LDA", AM_SR_INDIRECT_Y, 2},  /* 0x34: LDA (sr,S),Y */
    {"LDA", AM_DP_INDIRECT_LONG, 2},  /* 0x35: LDA [dp] */
    {"LDA", AM_DP_INDIRECT_LONG_Y, 2},  /* 0x36: LDA [dp],Y */
    {"LDA", AM_IMMEDIATE, 3},  /* 0x37: LDA #const */
    {"LDA", AM_ABSOLUTE, 3},  /* 0x38: LDA addr */
    {"LDA", AM_ABSOLUTE_X, 3},  /* 0x39: LDA addr,X */
    {"LDA", AM_ABSOLUTE_Y, 3},  /* 0x3A: LDA addr,Y */
    {"LDA", AM_DIRECT_PAGE, 2},  /* 0x3B: LDA dp */
    {"LDA", AM_DP_X, 2},  /* 0x3C: LDA dp,X */
    {"LDA", AM_ABSOLUTE_LONG, 4},  /* 0x3D: LDA long */
    {"LDA", AM_ABSOLUTE_LONG_X, 4},  /* 0x3E: LDA long,X */
    {"LDA", AM_STACK_RELATIVE, 2},  /* 0x3F: LDA sr,S */
    {"LDX", AM_IMMEDIATE, 3},  /* 0x40: LDX #const */
    {"LDX", AM_ABSOLUTE, 3},  /* 0x41: LDX addr */
    {"LDX", AM_ABSOLUTE_Y, 3},  /* 0x42: LDX addr,Y */
    {"LDX", AM_DIRECT_PAGE, 2},  /* 0x43: LDX dp */
    {"LDX", AM_DP_Y, 2},  /* 0x44: LDX dp,Y */
    {"LDY", AM_IMMEDIATE, 3},  /* 0x45: LDY #const */
    {"LDY", AM_ABSOLUTE, 3},  /* 0x46: LDY addr */
    {"LDY", AM_ABSOLUTE_X, 3},  /* 0x47: LDY addr,X */
    {"LDY", AM_DIRECT_PAGE, 2},  /* 0x48: LDY dp */
    {"LDY", AM_DP_X, 2},  /* 0x49: LDY dp,X */
    {"STA", AM_DP_X, 2},  /* 0x4A: STA dp,X */
    {"STA", AM_DP_INDIRECT_X, 2},  /* 0x4B: STA (dp,X) */
    {"STA", AM_DP_INDIRECT, 2},  /* 0x4C: STA (dp) */
    {"STA", AM_DP_INDIRECT_Y, 2},  /* 0x4D: STA (dp),Y */
    {"STA", AM_SR_INDIRECT_Y, 2},  /* 0x4E: STA (sr,S),Y */
    {"STA", AM_DP_INDIRECT_LONG, 2},  /* 0x4F: STA [dp] */
    {"STA", AM_DP_INDIRECT_LONG_Y, 2},  /* 0x50: STA [dp],Y */
    {"STA", AM_ABSOLUTE, 3},  /* 0x51: STA addr */
    {"STA", AM_ABSOLUTE_X, 3},  /* 0x52: STA addr,X */
    {"STA", AM_ABSOLUTE_Y, 3},  /* 0x53: STA addr,Y */
    {"STA", AM_DIRECT_PAGE, 2},  /* 0x54: STA dp */
    {"STA", AM_ABSOLUTE_LONG, 4},  /* 0x55: STA long */
    {"STA", AM_ABSOLUTE_LONG_X, 4},  /* 0x56: STA long,X */
    {"STA", AM_STACK_RELATIVE, 2},  /* 0x57: STA sr,S */
    {"STX", AM_ABSOLUTE, 3},  /* 0x58: STX addr */
    {"STX", AM_DIRECT_PAGE, 2},  /* 0x59: STX dp */
    {"STX", AM_DP_Y, 2},  /* 0x5A: STX dp,Y */
    {"STY", AM_ABSOLUTE, 3},  /* 0x5B: STY addr */
    {"STY", AM_DIRECT_PAGE, 2},  /* 0x5C: STY dp */
    {"STY", AM_DP_X, 2},  /* 0x5D: STY dp,X */
    {"STZ", AM_ABSOLUTE, 3},  /* 0x5E: STZ addr */
    {"STZ", AM_ABSOLUTE_X, 3},  /* 0x5F: STZ addr,X */
    {"STZ", AM_DIRECT_PAGE, 2},  /* 0x60: STZ dp */
    {"STZ", AM_DP_X, 2},  /* 0x61: STZ dp,X */
    {"BRK", AM_IMMEDIATE, 2},  /* 0x62: BRK */
    {"SEC", AM_IMPLIED, 1},  /* 0x63: SEC */
    {"SED", AM_IMPLIED, 1},  /* 0x64: SED */
    {"SEI", AM_IMPLIED, 1},  /* 0x65: SEI */
    {"ADC", AM_DIRECT_PAGE, 2},  /* 0x66: ADC dp */
    {"MVN", AM_BLOCK_MOVE, 3},  /* 0x67: MVN srcbk,destbk */
    {"MVP", AM_BLOCK_MOVE, 3},  /* 0x68: MVP srcbk,destbk */
    {"INC", AM_ACCUMULATOR, 1},  /* 0x69: INC A */
    {"INC", AM_ABSOLUTE, 3},  /* 0x6A: INC addr */
    {"INC", AM_ABSOLUTE_X, 3},  /* 0x6B: INC addr,X */
    {"INC", AM_DIRECT_PAGE, 2},  /* 0x6C: INC dp */
    {"INC", AM_DP_X, 2},  /* 0x6D: INC dp,X */
    {"INC", AM_ABSOLUTE, 3},  /* 0x6E: INC addr - commented "INC long" in cpu_instructions.cpp but actually calls addrAbsolute() (3 bytes, not 4); a functionally-redundant duplicate of 0x6A at a different (higher) cycle cost */
    {"INX", AM_IMPLIED, 1},  /* 0x6F: INX */
    {"INY", AM_IMPLIED, 1},  /* 0x70: INY */
    {"DEC", AM_ACCUMULATOR, 1},  /* 0x71: DEC A */
    {"DEC", AM_ABSOLUTE, 3},  /* 0x72: DEC addr */
    {"DEC", AM_ABSOLUTE_X, 3},  /* 0x73: DEC addr,X */
    {"DEC", AM_DIRECT_PAGE, 2},  /* 0x74: DEC dp */
    {"DEC", AM_DP_X, 2},  /* 0x75: DEC dp,X */
    {"DEC", AM_ABSOLUTE, 3},  /* 0x76: DEC addr - commented "DEC long" in cpu_instructions.cpp but actually calls addrAbsolute() (3 bytes, not 4); mirrors 0x6E above */
    {"DEX", AM_IMPLIED, 1},  /* 0x77: DEX */
    {"DEY", AM_IMPLIED, 1},  /* 0x78: DEY */
    {"CPX", AM_ABSOLUTE, 3},  /* 0x79: CPX addr */
    {"CPY", AM_ABSOLUTE, 3},  /* 0x7A: CPY addr */
    {"CLD", AM_IMPLIED, 1},  /* 0x7B: CLD */
    {"CLI", AM_IMPLIED, 1},  /* 0x7C: CLI */
    {"CLV", AM_IMPLIED, 1},  /* 0x7D: CLV */
    {"CLC", AM_IMPLIED, 1},  /* 0x7E: CLC */
    {"REP", AM_IMMEDIATE, 2},  /* 0x7F: REP #const */
    {"ROL", AM_ACCUMULATOR, 1},  /* 0x80: ROL A */
    {"ROL", AM_ABSOLUTE, 3},  /* 0x81: ROL addr */
    {"ROL", AM_ABSOLUTE_X, 3},  /* 0x82: ROL addr,X */
    {"ROL", AM_DIRECT_PAGE, 2},  /* 0x83: ROL dp */
    {"ROR", AM_ACCUMULATOR, 1},  /* 0x84: ROR A */
    {"ROR", AM_ABSOLUTE, 3},  /* 0x85: ROR addr */
    {"ROR", AM_ABSOLUTE_X, 3},  /* 0x86: ROR addr,X */
    {"ROR", AM_DIRECT_PAGE, 2},  /* 0x87: ROR dp */
    {"SHL", AM_REG_X, 1},  /* 0x88: SHL X */
    {"SHL", AM_REG_Y, 1},  /* 0x89: SHL Y */
    {"SHR", AM_REG_X, 1},  /* 0x8A: SHR X */
    {"SHR", AM_REG_Y, 1},  /* 0x8B: SHR Y */
    {"RCL", AM_ACCUMULATOR, 1},  /* 0x8C: RCL A */
    {"LSR", AM_ACCUMULATOR, 1},  /* 0x8D: LSR A */
    {"LSR", AM_ABSOLUTE, 3},  /* 0x8E: LSR addr */
    {"LSR", AM_ABSOLUTE_X, 3},  /* 0x8F: LSR addr,X */
    {"LSR", AM_DIRECT_PAGE, 2},  /* 0x90: LSR dp */
    {"LSR", AM_DP_X, 2},  /* 0x91: LSR dp,X */
    {"DIV", AM_REG_XY, 1},  /* 0x92: DIV X,Y */
    {"DIV", AM_ABSOLUTE_LONG_X, 4},  /* 0x93: DIV long,X */
    {"DIV", AM_ABSOLUTE_LONG_Y, 4},  /* 0x94: DIV long,Y */
    {"XOR", AM_ABSOLUTE, 3},  /* 0x95: XOR addr (aka EOR) */
    {"XOR", AM_ABSOLUTE_X, 3},  /* 0x96: XOR addr,X (aka EOR) */
    {"XOR", AM_ABSOLUTE_Y, 3},  /* 0x97: XOR addr,Y (aka EOR) */
    {"ASL", AM_ACCUMULATOR, 1},  /* 0x98: ASL A */
    {"ASL", AM_ABSOLUTE, 3},  /* 0x99: ASL addr */
    {"ASL", AM_ABSOLUTE_X, 3},  /* 0x9A: ASL addr,X */
    {"ASL", AM_DIRECT_PAGE, 2},  /* 0x9B: ASL dp */
    {"ASL", AM_DP_X, 2},  /* 0x9C: ASL dp,X */
    {"XOR", AM_DP_INDIRECT_X, 2},  /* 0x9D: XOR (dp,X) */
    {"XOR", AM_DP_INDIRECT, 2},  /* 0x9E: XOR (dp) */
    {"XOR", AM_DP_INDIRECT_Y, 2},  /* 0x9F: XOR (dp),Y */
    {"CMP", AM_DP_INDIRECT_X, 2},  /* 0xA0: CMP (dp,X) */
    {"CMP", AM_DP_INDIRECT, 2},  /* 0xA1: CMP (dp) */
    {"CMP", AM_DP_INDIRECT_Y, 2},  /* 0xA2: CMP (dp),Y */
    {"CMP", AM_SR_INDIRECT_Y, 2},  /* 0xA3: CMP (sr,S),Y */
    {"CMP", AM_DP_INDIRECT_LONG, 2},  /* 0xA4: CMP [dp] */
    {"CMP", AM_DP_INDIRECT_LONG_Y, 2},  /* 0xA5: CMP [dp],Y */
    {"CMP", AM_IMMEDIATE, 3},  /* 0xA6: CMP #const */
    {"CMP", AM_ABSOLUTE, 3},  /* 0xA7: CMP addr */
    {"CMP", AM_ABSOLUTE_X, 3},  /* 0xA8: CMP addr,X */
    {"CMP", AM_ABSOLUTE_Y, 3},  /* 0xA9: CMP addr,Y */
    {"CMP", AM_DIRECT_PAGE, 2},  /* 0xAA: CMP dp */
    {"CMP", AM_DP_X, 2},  /* 0xAB: CMP dp,X */
    {"CMP", AM_ABSOLUTE_LONG, 4},  /* 0xAC: CMP long */
    {"CMP", AM_ABSOLUTE_LONG_X, 4},  /* 0xAD: CMP long,X */
    {"CMP", AM_STACK_RELATIVE, 2},  /* 0xAE: CMP sr,S */
    {"COP", AM_IMMEDIATE, 2},  /* 0xAF: COP #const (aka INT, SWI) */
    {"ADC", AM_DP_INDIRECT_Y, 2},  /* 0xB0: ADC (dp),Y */
    {"ADC", AM_DP_INDIRECT_X, 2},  /* 0xB1: ADC (dp,X) */
    {"ADC", AM_DP_INDIRECT, 2},  /* 0xB2: ADC (dp) */
    {"ADC", AM_SR_INDIRECT_Y, 2},  /* 0xB3: ADC (sr,S),Y */
    {"ADC", AM_DP_INDIRECT_LONG, 2},  /* 0xB4: ADC [dp] */
    {"ADC", AM_DP_INDIRECT_LONG_Y, 2},  /* 0xB5: ADC [dp],Y */
    {"ADC", AM_IMMEDIATE, 3},  /* 0xB6: ADC #const */
    {"ADC", AM_ABSOLUTE, 3},  /* 0xB7: ADC addr */
    {"ADC", AM_ABSOLUTE_X, 3},  /* 0xB8: ADC addr,X */
    {"ADC", AM_ABSOLUTE_Y, 3},  /* 0xB9: ADC addr,Y */
    {"ADC", AM_DP_X, 2},  /* 0xBA: ADC dp,X */
    {"ADC", AM_ABSOLUTE_LONG, 4},  /* 0xBB: ADC long */
    {"ADC", AM_ABSOLUTE_LONG_X, 4},  /* 0xBC: ADC long,X */
    {"ADC", AM_STACK_RELATIVE, 2},  /* 0xBD: ADC sr,S */
    {"AND", AM_DP_INDIRECT_X, 2},  /* 0xBE: AND (dp,X) */
    {"AND", AM_DP_INDIRECT, 2},  /* 0xBF: AND (dp) */
    {"AND", AM_DP_INDIRECT_Y, 2},  /* 0xC0: AND (dp),Y */
    {"AND", AM_SR_INDIRECT_Y, 2},  /* 0xC1: AND (sr,S),Y */
    {"AND", AM_DP_INDIRECT_LONG, 2},  /* 0xC2: AND [dp] */
    {"AND", AM_DP_INDIRECT_LONG_Y, 2},  /* 0xC3: AND [dp],Y */
    {"AND", AM_IMMEDIATE, 3},  /* 0xC4: AND #const */
    {"AND", AM_ABSOLUTE, 3},  /* 0xC5: AND addr */
    {"AND", AM_ABSOLUTE_X, 3},  /* 0xC6: AND addr,X */
    {"AND", AM_ABSOLUTE_Y, 3},  /* 0xC7: AND addr,Y */
    {"AND", AM_DIRECT_PAGE, 2},  /* 0xC8: AND dp */
    {"AND", AM_DP_X, 2},  /* 0xC9: AND dp,X */
    {"AND", AM_ABSOLUTE_LONG, 4},  /* 0xCA: AND long */
    {"AND", AM_ABSOLUTE_LONG_X, 4},  /* 0xCB: AND long,X */
    {"AND", AM_STACK_RELATIVE, 2},  /* 0xCC: AND sr,S */
    {"ORA", AM_DP_INDIRECT_X, 2},  /* 0xCD: ORA (dp,X) */
    {"ORA", AM_DP_INDIRECT, 2},  /* 0xCE: ORA (dp) */
    {"ORA", AM_DP_INDIRECT_Y, 2},  /* 0xCF: ORA (dp),Y */
    {"ORA", AM_SR_INDIRECT_Y, 2},  /* 0xD0: ORA (sr,S),Y */
    {"ORA", AM_DP_INDIRECT_LONG, 2},  /* 0xD1: ORA [dp] */
    {"ORA", AM_DP_INDIRECT_LONG_Y, 2},  /* 0xD2: ORA [dp],Y */
    {"ORA", AM_IMMEDIATE, 3},  /* 0xD3: ORA #const */
    {"ORA", AM_ABSOLUTE, 3},  /* 0xD4: ORA addr */
    {"ORA", AM_ABSOLUTE_X, 3},  /* 0xD5: ORA addr,X */
    {"ORA", AM_ABSOLUTE_Y, 3},  /* 0xD6: ORA addr,Y */
    {"ORA", AM_DIRECT_PAGE, 2},  /* 0xD7: ORA dp */
    {"ORA", AM_DP_X, 2},  /* 0xD8: ORA dp,X */
    {"ORA", AM_ABSOLUTE_LONG, 4},  /* 0xD9: ORA long */
    {"ORA", AM_ABSOLUTE_LONG_X, 4},  /* 0xDA: ORA long,X */
    {"ORA", AM_STACK_RELATIVE, 2},  /* 0xDB: ORA sr,S */
    {"SBC", AM_DP_INDIRECT_X, 2},  /* 0xDC: SBC (dp,X) */
    {"SBC", AM_DP_INDIRECT_Y, 2},  /* 0xDD: SBC (dp),Y */
    {"SBC", AM_SR_INDIRECT_Y, 2},  /* 0xDE: SBC (sr,S),Y */
    {"SBC", AM_DP_INDIRECT_LONG, 2},  /* 0xDF: SBC [dp] */
    {"SBC", AM_DP_INDIRECT_LONG_Y, 2},  /* 0xE0: SBC [dp],Y */
    {"SBC", AM_IMMEDIATE, 3},  /* 0xE1: SBC #const */
    {"SBC", AM_ABSOLUTE, 3},  /* 0xE2: SBC addr */
    {"SBC", AM_ABSOLUTE_X, 3},  /* 0xE3: SBC addr,X */
    {"SBC", AM_ABSOLUTE_Y, 3},  /* 0xE4: SBC addr,Y */
    {"SBC", AM_DIRECT_PAGE, 2},  /* 0xE5: SBC dp */
    {"SBC", AM_DP_X, 2},  /* 0xE6: SBC dp,X */
    {"SBC", AM_ABSOLUTE_LONG, 4},  /* 0xE7: SBC long */
    {"SBC", AM_ABSOLUTE_LONG_X, 4},  /* 0xE8: SBC long,X */
    {"SBC", AM_STACK_RELATIVE, 2},  /* 0xE9: SBC sr,S */
    {"MUL", AM_DP_INDIRECT_X, 2},  /* 0xEA: MUL (dp,X) */
    {"MUL", AM_DP_INDIRECT_Y, 2},  /* 0xEB: MUL (dp),Y */
    {"MUL", AM_SR_INDIRECT_Y, 2},  /* 0xEC: MUL (sr,S),Y */
    {"MUL", AM_DP_INDIRECT_LONG, 2},  /* 0xED: MUL [dp] */
    {"MUL", AM_DP_INDIRECT_LONG_Y, 2},  /* 0xEE: MUL [dp],Y */
    {"MUL", AM_IMMEDIATE, 3},  /* 0xEF: MUL #const */
    {"MUL", AM_ABSOLUTE, 3},  /* 0xF0: MUL addr */
    {"MUL", AM_ABSOLUTE_X, 3},  /* 0xF1: MUL addr,X */
    {"MUL", AM_ABSOLUTE_Y, 3},  /* 0xF2: MUL addr,Y */
    {"MUL", AM_DIRECT_PAGE, 2},  /* 0xF3: MUL dp */
    {"MUL", AM_DP_X, 2},  /* 0xF4: MUL dp,X */
    {"MUL", AM_ABSOLUTE_LONG, 4},  /* 0xF5: MUL long */
    {"MUL", AM_ABSOLUTE_LONG_X, 4},  /* 0xF6: MUL long,X */
    {"MUL", AM_STACK_RELATIVE, 2},  /* 0xF7: MUL sr,S */
    {"XOR", AM_SR_INDIRECT_Y, 2},  /* 0xF8: XOR (sr,S),Y */
    {"XOR", AM_IMMEDIATE, 3},  /* 0xF9: XOR #const (aka EOR) */
    {"XOR", AM_DIRECT_PAGE, 2},  /* 0xFA: XOR dp */
    {"XOR", AM_DP_X, 2},  /* 0xFB: XOR dp,X */
    {"XOR", AM_ABSOLUTE_LONG, 4},  /* 0xFC: XOR long */
    {"XOR", AM_ABSOLUTE_LONG_X, 4},  /* 0xFD: XOR long,X */
    {"XOR", AM_STACK_RELATIVE, 2},  /* 0xFE: XOR sr,S */
    {"HLT", AM_IMPLIED, 1}  /* 0xFF: HLT (aka STP, KIL) */
};

/* Read file into buffer */
static uint8_t* read_file(const char* filename, size_t* size) {
    FILE* f;
    uint8_t* buffer;

    f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", filename);
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    *size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (*size > MAX_FILE_SIZE) {
        fprintf(stderr, "Error: File too large (max %d bytes)\n", MAX_FILE_SIZE);
        fclose(f);
        return NULL;
    }

    buffer = malloc(*size);
    if (!buffer) {
        fprintf(stderr, "Error: Out of memory\n");
        fclose(f);
        return NULL;
    }

    if (fread(buffer, 1, *size, f) != *size) {
        fprintf(stderr, "Error: Failed to read file\n");
        free(buffer);
        fclose(f);
        return NULL;
    }

    fclose(f);
    return buffer;
}

/* Parse hex address */
static uint32_t parse_hex(const char* str) {
    uint32_t value = 0;
    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        str += 2;
    }
    sscanf(str, "%x", &value);
    return value;
}

/* Format an instruction's operand into buf (caller-sized, >= 32 bytes is
 * always enough - the longest operand text is "($123456,X)" at 12 chars)
 * given the addressing mode, the raw instruction bytes, and the address the
 * opcode byte lives at (needed for AM_PC_RELATIVE_LONG, whose operand is a
 * displacement rather than a literal address). Bytes beyond the end of the
 * buffer (a truncated trailing instruction) are simply not read - only as
 * many operand bytes as detect_addressing_mode()/find_instruction() on the
 * assembler side would actually consume are ever touched here, matching
 * `length - 1` for every mode except the three register forms. Leaves buf
 * empty ("") for AM_IMPLIED/AM_REG_XY, which take no operand text at all. */
static void format_operand(char *buf, AddressingMode mode, const uint8_t *data,
                            size_t offset, size_t max_size, uint32_t addr, int length) {
    uint32_t w;
    uint32_t l;
    int16_t rel;
    uint32_t target;

    buf[0] = '\0';

    switch (mode) {
        case AM_IMPLIED:
        case AM_REG_XY:
            break;

        case AM_ACCUMULATOR:
            strcpy(buf, " A");
            break;

        case AM_REG_X:
            strcpy(buf, " X");
            break;

        case AM_REG_Y:
            strcpy(buf, " Y");
            break;

        case AM_IMMEDIATE:
            /* Immediate operand width varies by instruction (1-byte for
             * BRK/COP/SEP/REP/SDB, 2-byte for everything else - see the
             * "#const immediate forms" comment in cpuasm.c) - use the
             * instruction's own length rather than a fixed width. */
            if (length - 1 >= 2) {
                if (offset + 2 < max_size) {
                    w = (uint32_t)data[offset + 1] | ((uint32_t)data[offset + 2] << 8);
                    sprintf(buf, " #$%04X", w);
                }
            } else if (offset + 1 < max_size) {
                sprintf(buf, " #$%02X", data[offset + 1]);
            }
            break;

        case AM_ABSOLUTE:
            if (offset + 2 < max_size) {
                w = (uint32_t)data[offset + 1] | ((uint32_t)data[offset + 2] << 8);
                sprintf(buf, " $%04X", w);
            }
            break;

        case AM_ABSOLUTE_X:
            if (offset + 2 < max_size) {
                w = (uint32_t)data[offset + 1] | ((uint32_t)data[offset + 2] << 8);
                sprintf(buf, " $%04X,X", w);
            }
            break;

        case AM_ABSOLUTE_Y:
            if (offset + 2 < max_size) {
                w = (uint32_t)data[offset + 1] | ((uint32_t)data[offset + 2] << 8);
                sprintf(buf, " $%04X,Y", w);
            }
            break;

        case AM_ABSOLUTE_LONG:
            if (offset + 3 < max_size) {
                l = (uint32_t)data[offset + 1] | ((uint32_t)data[offset + 2] << 8) |
                    ((uint32_t)data[offset + 3] << 16);
                sprintf(buf, " $%06X", l);
            }
            break;

        case AM_ABSOLUTE_LONG_X:
            if (offset + 3 < max_size) {
                l = (uint32_t)data[offset + 1] | ((uint32_t)data[offset + 2] << 8) |
                    ((uint32_t)data[offset + 3] << 16);
                sprintf(buf, " $%06X,X", l);
            }
            break;

        case AM_ABSOLUTE_LONG_Y:
            if (offset + 3 < max_size) {
                l = (uint32_t)data[offset + 1] | ((uint32_t)data[offset + 2] << 8) |
                    ((uint32_t)data[offset + 3] << 16);
                sprintf(buf, " $%06X,Y", l);
            }
            break;

        case AM_DIRECT_PAGE:
            if (offset + 1 < max_size) {
                sprintf(buf, " $%02X", data[offset + 1]);
            }
            break;

        case AM_DP_X:
            if (offset + 1 < max_size) {
                sprintf(buf, " $%02X,X", data[offset + 1]);
            }
            break;

        case AM_DP_Y:
            if (offset + 1 < max_size) {
                sprintf(buf, " $%02X,Y", data[offset + 1]);
            }
            break;

        case AM_DP_INDIRECT:
            if (offset + 1 < max_size) {
                sprintf(buf, " ($%02X)", data[offset + 1]);
            }
            break;

        case AM_DP_INDIRECT_X:
            if (offset + 1 < max_size) {
                sprintf(buf, " ($%02X,X)", data[offset + 1]);
            }
            break;

        case AM_DP_INDIRECT_Y:
            if (offset + 1 < max_size) {
                sprintf(buf, " ($%02X),Y", data[offset + 1]);
            }
            break;

        case AM_DP_INDIRECT_LONG:
            if (offset + 1 < max_size) {
                sprintf(buf, " [$%02X]", data[offset + 1]);
            }
            break;

        case AM_DP_INDIRECT_LONG_Y:
            if (offset + 1 < max_size) {
                sprintf(buf, " [$%02X],Y", data[offset + 1]);
            }
            break;

        case AM_ABSOLUTE_INDIRECT:
            if (offset + 2 < max_size) {
                w = (uint32_t)data[offset + 1] | ((uint32_t)data[offset + 2] << 8);
                sprintf(buf, " ($%04X)", w);
            }
            break;

        case AM_ABSOLUTE_INDIRECT_LONG:
            /* JMP [addr]: the operand itself is a 16-bit pointer (see
             * CPU::addrAbsoluteIndirectLong(), src/cpu.cpp) even though the
             * mnemonic says "long" - 2 operand bytes, not 3. */
            if (offset + 2 < max_size) {
                w = (uint32_t)data[offset + 1] | ((uint32_t)data[offset + 2] << 8);
                sprintf(buf, " [$%04X]", w);
            }
            break;

        case AM_ABSOLUTE_INDEXED_INDIRECT:
            if (offset + 2 < max_size) {
                w = (uint32_t)data[offset + 1] | ((uint32_t)data[offset + 2] << 8);
                sprintf(buf, " ($%04X,X)", w);
            }
            break;

        case AM_STACK_RELATIVE:
            if (offset + 1 < max_size) {
                sprintf(buf, " $%02X,S", data[offset + 1]);
            }
            break;

        case AM_SR_INDIRECT_Y:
            if (offset + 1 < max_size) {
                sprintf(buf, " ($%02X,S),Y", data[offset + 1]);
            }
            break;

        case AM_PC_RELATIVE_LONG:
            /* BRL/PER: a signed 16-bit displacement added to PC as it
             * stands right after the 2-byte operand is fetched - i.e.
             * relative to the address of the NEXT instruction (addr +
             * length), not to this opcode's own address. Printed as the
             * resolved absolute target rather than the raw offset, since
             * cpuasm.c's AM_PC_RELATIVE_LONG emission path recomputes the
             * offset from a literal absolute value the same way it would
             * from a label. */
            if (offset + 2 < max_size) {
                rel = (int16_t)((uint32_t)data[offset + 1] | ((uint32_t)data[offset + 2] << 8));
                target = (uint32_t)((int32_t)(addr + (uint32_t)length) + rel) & 0xFFFFu;
                sprintf(buf, " $%04X", target);
            }
            break;

        case AM_BLOCK_MOVE:
            if (offset + 2 < max_size) {
                sprintf(buf, " $%02X,$%02X", data[offset + 1], data[offset + 2]);
            }
            break;

        default:
            break;
    }
}

/* Disassemble one instruction */
static int disassemble_instruction(uint8_t* data, size_t offset, size_t max_size,
                                   FILE* out, int show_comments, int show_bytes, uint32_t addr) {
    uint8_t opcode;
    const InstrInfo* instr;
    char operand[32];
    int i;

    if (offset >= max_size) return 0;

    opcode = data[offset];
    instr = &instructions[opcode];

    /* Show address */
    fprintf(out, "%06X:  ", addr);

    /* Show bytes if requested */
    if (show_bytes) {
        for (i = 0; i < instr->length && offset + (size_t)i < max_size; i++) {
            fprintf(out, "%02X ", data[offset + (size_t)i]);
        }
        /* Pad to 4 bytes worth of columns (the longest instruction here is
         * 4 bytes: opcode + a 24-bit long operand). */
        for (i = instr->length; i < 4; i++) {
            fprintf(out, "   ");
        }
        fprintf(out, "  ");
    }

    /* Mnemonic + operand */
    format_operand(operand, instr->mode, data, offset, max_size, addr, instr->length);
    fprintf(out, "%s%s", instr->mnemonic, operand);

    /* Flag a trailing instruction that ran off the end of the buffer -
     * format_operand() above will have left the operand blank rather than
     * reading past max_size, so make that visible instead of silently
     * printing a bare mnemonic that looks like a 1-byte implied opcode. */
    if (offset + (size_t)instr->length > max_size) {
        fprintf(out, " ; truncated (needs %d bytes, %lu available)",
                instr->length, (unsigned long)(max_size - offset));
    } else if (show_comments) {
        fprintf(out, " ; opcode $%02X", opcode);
    }

    fprintf(out, "\n");

    return instr->length;
}

int main(int argc, char* argv[]) {
    const char* input_file;
    const char* output_file;
    uint32_t start_addr;
    uint32_t end_addr;
    int show_comments;
    int show_bytes;
    int i;
    size_t file_size;
    uint8_t* data;
    FILE* out;
    size_t offset;
    uint32_t addr;
    int len;

    input_file = NULL;
    output_file = NULL;
    start_addr = 0;
    end_addr = 0xFFFFFFFF;
    show_comments = 0;
    show_bytes = 0;

    /* Parse arguments */
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output_file = argv[++i];
        } else if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            start_addr = parse_hex(argv[++i]);
        } else if (strcmp(argv[i], "-e") == 0 && i + 1 < argc) {
            end_addr = parse_hex(argv[++i]);
        } else if (strcmp(argv[i], "-c") == 0) {
            show_comments = 1;
        } else if (strcmp(argv[i], "-b") == 0) {
            show_bytes = 1;
        } else if (argv[i][0] != '-') {
            input_file = argv[i];
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            return 1;
        }
    }

    if (!input_file) {
        printf("CPU Disassembler for DEF88186\n");
        printf("Usage: %s <input.bin> [options]\n", argv[0]);
        printf("\nOptions:\n");
        printf("  -o <file>    Output to file\n");
        printf("  -s <addr>    Start address (hex)\n");
        printf("  -e <addr>    End address (hex)\n");
        printf("  -c           Show comments\n");
        printf("  -b           Show bytes\n");
        return 1;
    }

    /* Read input file */
    data = read_file(input_file, &file_size);
    if (!data) return 1;

    /* Adjust end address */
    if (end_addr > file_size) end_addr = file_size;
    if (start_addr > file_size) start_addr = file_size;

    /* Open output */
    out = stdout;
    if (output_file) {
        out = fopen(output_file, "w");
        if (!out) {
            fprintf(stderr, "Error: Cannot open output file '%s'\n", output_file);
            free(data);
            return 1;
        }
    }

    /* Header */
    fprintf(out, "; Disassembly of %s\n", input_file);
    fprintf(out, "; Size: %zu bytes (0x%zX)\n", file_size, file_size);
    fprintf(out, "; Range: $%06X - $%06X\n\n", start_addr, end_addr);

    /* Disassemble */
    offset = start_addr;
    addr = start_addr;

    while (offset < end_addr && offset < file_size) {
        len = disassemble_instruction(data, offset, file_size, out,
                                           show_comments, show_bytes, addr);
        if (len == 0) {
            /* Unknown instruction, show as data byte */
            fprintf(out, "%06X:  ", addr);
            if (show_bytes) fprintf(out, "%02X           ", data[offset]);
            fprintf(out, ".byte $%02X\n", data[offset]);
            len = 1;
        }
        offset += len;
        addr += len;
    }

    /* Cleanup */
    if (output_file) fclose(out);
    free(data);

    return 0;
}

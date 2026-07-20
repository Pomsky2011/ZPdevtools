/*
 * cpuasm - DEF88186 Main CPU Assembler
 *
 * Assembles DEF88186 assembly language to binary machine code.
 * Supports all 256 instructions with full addressing mode detection.
 *
 * Usage: ./cpuasm input.asm [output.bin]
 *
 * Features:
 * - All 256 DEF88186 instructions
 * - Automatic addressing mode detection
 * - Label support with forward references
 * - Directives: .org, .byte, .word, .long, .include
 * - Multi-file support via .include directive
 * - Comments (;)
 * - Constant definitions (.equ)
 * - Little-endian encoding
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "compat.h"

#define MAX_LINE 256
#define MAX_LABELS 256
#define MAX_CODE 131072
#define MAX_FIXUPS 256

/* Addressing modes */
typedef enum {
    AM_IMPLIED,             /* NOP */
    AM_ACCUMULATOR,         /* INC A */
    AM_IMMEDIATE,           /* LDA #$42 */
    AM_ABSOLUTE,            /* LDA $1234 */
    AM_ABSOLUTE_X,          /* LDA $1234,X */
    AM_ABSOLUTE_Y,          /* LDA $1234,Y */
    AM_ABSOLUTE_LONG,       /* LDA $123456 */
    AM_ABSOLUTE_LONG_X,     /* LDA $123456,X */
    AM_ABSOLUTE_LONG_Y,     /* DIV $123456,Y */
    AM_DIRECT_PAGE,         /* LDA $12 */
    AM_DP_X,                /* LDA $12,X */
    AM_DP_Y,                /* LDX $12,Y */
    AM_DP_INDIRECT,         /* LDA ($12) */
    AM_DP_INDIRECT_X,       /* LDA ($12,X) */
    AM_DP_INDIRECT_Y,       /* LDA ($12),Y */
    AM_DP_INDIRECT_LONG,    /* LDA [$12] */
    AM_DP_INDIRECT_LONG_Y,  /* LDA [$12],Y */
    AM_ABSOLUTE_INDIRECT,   /* JMP ($1234) */
    AM_ABSOLUTE_INDIRECT_LONG, /* JMP [$1234] */
    AM_ABSOLUTE_INDEXED_INDIRECT, /* JMP ($1234,X) */
    AM_STACK_RELATIVE,      /* LDA $12,S */
    AM_SR_INDIRECT_Y,       /* LDA ($12,S),Y */
    AM_PC_RELATIVE_LONG,    /* BRL label */
    AM_BLOCK_MOVE,          /* MVP $12,$34 */
    AM_SPECIAL              /* Special cases */
} AddressingMode;

/* Instruction entry */
typedef struct {
    char mnemonic[8];
    AddressingMode mode;
    uint8_t opcode;
    int bytes;
    int cycles;
} Instruction;

/* Label entry */
typedef struct {
    char name[64];
    uint16_t address;
    uint8_t bank;    /* output bank in effect when the label was defined (see .BANK) */
    int defined;
} Label;

/* Fixup entry (for forward references) */
typedef struct {
    uint16_t address;
    char label[64];
    AddressingMode mode;
    int size;  /* 1, 2, or 3 bytes */
} Fixup;

/* .EQU entry: a named compile-time constant (not an address - no bank, no
 * fixups). Resolved immediately, so a .EQU must appear before its first use
 * in the file (true separately in each pass; that's fine for straight-line
 * top-down sources). Documented since the assembler's original header
 * comment but never actually implemented until now. */
typedef struct {
    char name[64];
    uint32_t value;
    int defined;
} Equate;

/* Global data */
static Instruction instructions[512];
static int num_instructions = 0;
static Equate equates[MAX_LABELS];
static int num_equates = 0;
static Label labels[MAX_LABELS];
static int num_labels = 0;
static Fixup fixups[MAX_FIXUPS];
static int num_fixups = 0;
static uint8_t code[MAX_CODE];
static uint16_t pc = 0;
static uint16_t org = 0;
static int pass = 1;
static int errors = 0;
/* Output bank for label resolution (see .BANK). cpuasm's pc/org are 16-bit
 * offsets only - it has no idea what bank the assembled bytes will end up
 * mapped at (that's decided later, e.g. System maps a Boot ROM to bank $E0).
 * A label's long-address form (BEQ/BCS/BMI/JMP long/LDA long,X etc.) used to
 * always encode bank $00 for that reason, silently mis-jumping whenever the
 * code actually runs in a different bank. .BANK lets a program tell cpuasm
 * which bank it's being assembled for, so those forms encode the real bank. */
static uint8_t current_bank = 0;

/* Function prototypes */
void init_instructions(void);
void error(const char *msg);
void warning(const char *msg);
char *skip_whitespace(char *p);
char *parse_token(char *p, char *token);
int parse_number(const char *str, uint32_t *value);
int find_equate(const char *name);
void add_equate(const char *name, uint32_t value);
void substitute_equates(char *operand);
int find_label(const char *name);
void add_label(const char *name, uint16_t addr);
void add_fixup(uint16_t addr, const char *label, AddressingMode mode, int size);
void resolve_fixups(void);
AddressingMode detect_addressing_mode(const char *operand, const char *mnemonic);
const Instruction *find_instruction(const char *mnemonic, AddressingMode mode);
void emit_byte(uint8_t byte);
void emit_word(uint16_t word);
void emit_long(uint32_t value);
int assemble_line(char *line);
void assemble_file(const char *filename);

/* Error handling */
void error(const char *msg) {
    fprintf(stderr, "Error: %s\n", msg);
    errors++;
}

void warning(const char *msg) {
    fprintf(stderr, "Warning: %s\n", msg);
}

/* Skip whitespace */
char *skip_whitespace(char *p) {
    while (*p && isspace(*p)) p++;
    return p;
}

/* Parse a token */
char *parse_token(char *p, char *token) {
    int i;
    p = skip_whitespace(p);
    i = 0;
    while (*p && !isspace(*p) && *p != ',' && *p != ';') {
        token[i++] = *p++;
    }
    token[i] = '\0';
    return p;
}

/* Parse a number (decimal, hex, binary) */
int parse_number(const char *str, uint32_t *value) {
    if (!str || !*str) return 0;

    if (str[0] == '$') {
        /* Hexadecimal. %n reports how many characters sscanf consumed so
         * trailing garbage - e.g. accidental operand arithmetic like
         * "$1B00+2,X", which this assembler doesn't support - is a parse
         * failure instead of silently resolving to "$1B00" with the "+2"
         * dropped (sscanf's return value alone only reports whether a
         * conversion happened, not whether the whole string matched). */
        int consumed = 0;
        int ok = sscanf(str + 1, "%x%n", value, &consumed);
        return ok == 1 && str[1 + consumed] == '\0';
    } else if (str[0] == '0' && str[1] == 'x') {
        /* 0x hexadecimal */
        int consumed = 0;
        int ok = sscanf(str + 2, "%x%n", value, &consumed);
        return ok == 1 && str[2 + consumed] == '\0';
    } else if (str[0] == '0' && str[1] == 'b') {
        /* Binary */
        const char *p;
        *value = 0;
        for (p = str + 2; *p; p++) {
            if (*p == '0' || *p == '1') {
                *value = (*value << 1) | (*p - '0');
            } else {
                return 0;
            }
        }
        return 1;
    } else if (str[0] == '#') {
        /* Immediate value */
        return parse_number(str + 1, value);
    } else {
        /* Decimal */
        char *endptr;
        *value = strtoul(str, &endptr, 10);
        return *endptr == '\0';
    }
}

/* Find a .EQU constant by name; returns its index or -1 */
int find_equate(const char *name) {
    int i;
    for (i = 0; i < num_equates; i++) {
        if (strcmp(equates[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

/* Define (or redefine) a .EQU constant */
void add_equate(const char *name, uint32_t value) {
    int idx = find_equate(name);
    if (idx >= 0) {
        equates[idx].value = value;
        equates[idx].defined = 1;
        return;
    }
    if (num_equates >= MAX_LABELS) {
        error("Too many .EQU constants");
        return;
    }
    strcpy(equates[num_equates].name, name);
    equates[num_equates].value = value;
    equates[num_equates].defined = 1;
    num_equates++;
}

/* Substitute any .EQU name appearing in operand with its literal hex value
 * (e.g. "ADD32_A_LO,X" -> "$800000,X"). Addressing-mode detection and the
 * later operand-value resolution both decide 16-bit-vs-24-bit purely from
 * the literal numeric TEXT (digit count / value range) - an opaque equate
 * name gives them nothing to go on, so a bare 24-bit-valued equate used to
 * silently get assembled as a 16-bit absolute operand (discarding its high
 * byte). Runs once, early, over the whole operand text, before either of
 * those. Real labels and bare register letters (X/Y/S/A in ",X"/",Y"/",S"/A)
 * fall through unchanged since they aren't found in the equate table. */
void substitute_equates(char *operand) {
    char result[MAX_LINE];
    int ri = 0;
    int i = 0;
    int n = (int)strlen(operand);
    while (i < n) {
        if (isalpha((unsigned char)operand[i]) || operand[i] == '_') {
            char ident[64];
            int ilen = 0;
            int idx;
            while (i < n && (isalnum((unsigned char)operand[i]) || operand[i] == '_')) {
                if (ilen < 63) ident[ilen++] = operand[i];
                i++;
            }
            ident[ilen] = '\0';
            idx = find_equate(ident);
            if (idx >= 0) {
                char hex[16];
                int hl, k;
                sprintf(hex, "$%lX", (unsigned long)equates[idx].value);
                hl = (int)strlen(hex);
                for (k = 0; k < hl && ri < MAX_LINE - 1; k++) result[ri++] = hex[k];
            } else {
                int k;
                for (k = 0; k < ilen && ri < MAX_LINE - 1; k++) result[ri++] = ident[k];
            }
        } else {
            if (ri < MAX_LINE - 1) result[ri++] = operand[i];
            i++;
        }
    }
    result[ri] = '\0';
    strcpy(operand, result);
}

/* Find label by name */
int find_label(const char *name) {
    int i;
    for (i = 0; i < num_labels; i++) {
        if (strcmp(labels[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

/* Add a label */
void add_label(const char *name, uint16_t addr) {
    int idx = find_label(name);
    if (idx >= 0) {
        if (labels[idx].defined && pass == 2) {
            error("Duplicate label");
            return;
        }
        labels[idx].address = addr;
        labels[idx].bank = current_bank;
        labels[idx].defined = 1;
    } else {
        if (num_labels >= MAX_LABELS) {
            error("Too many labels");
            return;
        }
        strcpy(labels[num_labels].name, name);
        labels[num_labels].address = addr;
        labels[num_labels].bank = current_bank;
        labels[num_labels].defined = 1;
        num_labels++;
    }
}

/* Add a fixup for forward reference */
void add_fixup(uint16_t addr, const char *label, AddressingMode mode, int size) {
    if (num_fixups >= MAX_FIXUPS) {
        error("Too many fixups");
        return;
    }
    fixups[num_fixups].address = addr;
    strcpy(fixups[num_fixups].label, label);
    fixups[num_fixups].mode = mode;
    fixups[num_fixups].size = size;
    num_fixups++;
}

/* Resolve all fixups */
void resolve_fixups(void) {
    int i;
    for (i = 0; i < num_fixups; i++) {
        Fixup *f;
        int idx;
        uint32_t addr;
        char msg[128];

        f = &fixups[i];
        idx = find_label(f->label);

        if (idx < 0 || !labels[idx].defined) {
            sprintf(msg, "Undefined label: %s", f->label);
            error(msg);
            continue;
        }

        /* Fold in the label's bank for the 24-bit (long) form (f->size==3
         * below); PC-relative and the 8/16-bit forms only ever use the low
         * 8/16 bits so this is a no-op for them. */
        addr = ((uint32_t)labels[idx].bank << 16) | labels[idx].address;

        if (f->mode == AM_PC_RELATIVE_LONG) {
            /* Calculate relative offset (bank-agnostic - BRL/PER are
             * same-bank). f->address was recorded right after the opcode
             * byte was emitted (add_fixup(pc, ...) above, called before the
             * 2-byte operand placeholder) - i.e. f->address is already
             * (opcode address + 1). The real reference point for the
             * displacement is the address of the NEXT instruction, which is
             * (opcode address + 3) for these always-3-byte instructions,
             * i.e. f->address + 2 - the old "+ 3" here (mirroring the same
             * mistake in the literal-operand branch above) put the
             * reference point one byte past the real next-instruction
             * address, so a forward "PER label"/"BRL label" landed one byte
             * short of the intended target. */
            int16_t offset = (int16_t)(labels[idx].address - (f->address + 2));
            code[f->address - org] = offset & 0xFF;
            code[f->address - org + 1] = (offset >> 8) & 0xFF;
        } else if (f->size == 1) {
            code[f->address - org] = addr & 0xFF;
        } else if (f->size == 2) {
            code[f->address - org] = addr & 0xFF;
            code[f->address - org + 1] = (addr >> 8) & 0xFF;
        } else if (f->size == 3) {
            code[f->address - org] = addr & 0xFF;
            code[f->address - org + 1] = (addr >> 8) & 0xFF;
            code[f->address - org + 2] = (addr >> 16) & 0xFF;
        }
    }
}

/* Detect addressing mode from operand */
AddressingMode detect_addressing_mode(const char *operand, const char *mnemonic) {
    uint32_t val;
    char temp[MAX_LINE];
    const char *start;
    const char *end;
    const char *close;
    const char *num_str;
    char *comma;
    int len;

    if (!operand || !*operand) {
        /* Check for accumulator mode */
        if (strcmp(mnemonic, "INC") == 0 || strcmp(mnemonic, "DEC") == 0 ||
            strcmp(mnemonic, "ASL") == 0 || strcmp(mnemonic, "LSR") == 0 ||
            strcmp(mnemonic, "ROL") == 0 || strcmp(mnemonic, "ROR") == 0 ||
            strcmp(mnemonic, "RCL") == 0) {
            return AM_ACCUMULATOR;
        }
        return AM_IMPLIED;
    }

    /* Special case: operand is 'A' for accumulator */
    if (strcmp(operand, "A") == 0 || strcmp(operand, "a") == 0) {
        return AM_ACCUMULATOR;
    }

    /* BRL/PER always take a PC-relative label/offset operand - a 16-bit
     * signed displacement computed at assemble time, not a real absolute
     * address - even though the user-facing syntax ("BRL label") looks
     * identical to a bare absolute value. Without this, a label/numeric
     * operand falls through to the generic detection below and comes back
     * AM_ABSOLUTE, which find_instruction() can never match against BRL/PER
     * (registered only under AM_PC_RELATIVE_LONG) - "Unknown instruction or
     * addressing mode" for every use of either mnemonic. */
    if (strcmp(mnemonic, "BRL") == 0 || strcmp(mnemonic, "PER") == 0) {
        return AM_PC_RELATIVE_LONG;
    }

    /* Immediate */
    if (operand[0] == '#') {
        return AM_IMMEDIATE;
    }

    /* Detect various indirect modes */
    if (operand[0] == '(') {
        /* Look for closing ) */
        close = strchr(operand, ')');
        if (close) {
            /* Check what comes after ) */
            if (*(close + 1) == ',') {
                if (*(close + 2) == 'Y' || *(close + 2) == 'y') {
                    /* ($12),Y or ($12,S),Y */
                    if (strstr(operand, ",S")) {
                        return AM_SR_INDIRECT_Y;
                    }
                    return AM_DP_INDIRECT_Y;
                }
            } else if (*(close + 1) == '\0' || *(close + 1) == ' ') {
                /* ($12) or ($1234) */
                /* Determine if DP or absolute based on value */
                start = operand + 1;
                end = close;
                len = end - start;
                strncpy(temp, start, len);
                temp[len] = '\0';

                /* Check for ,X inside parens */
                if (strchr(temp, ',')) {
                    /* ($12,X) / ($1234,X) / (label,X) - determine DP vs
                     * absolute the same way the non-indirect ,X case does:
                     * a small numeric base is DP, a large one or a label
                     * (parse_number fails) is absolute (JMP/JSR (addr,X)). */
                    char base[MAX_LINE];
                    char *paren_comma = strchr(temp, ',');
                    int blen = (int)(paren_comma - temp);
                    strncpy(base, temp, blen);
                    base[blen] = '\0';
                    if (parse_number(base, &val) && val <= 0xFF) {
                        return AM_DP_INDIRECT_X;
                    }
                    return AM_ABSOLUTE_INDEXED_INDIRECT;
                }

                if (parse_number(temp, &val)) {
                    if (val <= 0xFF) {
                        return AM_DP_INDIRECT;
                    } else {
                        return AM_ABSOLUTE_INDIRECT;
                    }
                }
                return AM_DP_INDIRECT;  /* Default */
            }
        }
    }

    /* Bracket notation for long indirect */
    if (operand[0] == '[') {
        close = strchr(operand, ']');
        if (close) {
            if (*(close + 1) == ',') {
                return AM_DP_INDIRECT_LONG_Y;
            } else {
                /* [$12] (dp) vs [$1234] (JMP absolute indirect long) -
                 * decide by value, the same way the parenthesized-indirect
                 * case above does. The old check ("close == operand + 7")
                 * was an off-by-one against "[$1234]" (7 chars, ']' at
                 * index 6, i.e. operand + 6) so it never actually matched a
                 * normal 4-hex-digit address - AM_ABSOLUTE_INDIRECT_LONG
                 * was unreachable and every "JMP [addr]" silently fell
                 * through to AM_DP_INDIRECT_LONG's 1-byte operand instead
                 * of the real 2-byte one. */
                start = operand + 1;
                end = close;
                len = end - start;
                strncpy(temp, start, len);
                temp[len] = '\0';
                if (parse_number(temp, &val) && val <= 0xFF) {
                    return AM_DP_INDIRECT_LONG;
                }
                return AM_ABSOLUTE_INDIRECT_LONG;
            }
        }
    }

    /* Check for indexed modes */
    if (strstr(operand, ",X")) {
        /* Determine absolute vs DP vs long based on value */
        strcpy(temp, operand);
        comma = strstr(temp, ",");
        if (comma) *comma = '\0';
        if (parse_number(temp, &val)) {
            if (val <= 0xFF) {
                return AM_DP_X;
            } else if (val <= 0xFFFF) {
                return AM_ABSOLUTE_X;
            } else {
                return AM_ABSOLUTE_LONG_X;
            }
        }
        return AM_ABSOLUTE_X;  /* Default */
    }

    if (strstr(operand, ",Y")) {
        strcpy(temp, operand);
        comma = strstr(temp, ",");
        if (comma) *comma = '\0';
        if (parse_number(temp, &val)) {
            if (val <= 0xFF) {
                return AM_DP_Y;
            } else if (val <= 0xFFFF) {
                return AM_ABSOLUTE_Y;
            } else {
                return AM_ABSOLUTE_LONG_Y;
            }
        }
        return AM_ABSOLUTE_Y;
    }

    if (strstr(operand, ",S")) {
        return AM_STACK_RELATIVE;
    }

    /* Direct value - determine based on size */
    /* Make sure to handle $ prefix even without # */
    num_str = operand;
    if (*num_str == '$' || *num_str == '0' || isdigit(*num_str)) {
        if (parse_number(operand, &val)) {
            if (val <= 0xFF) {
                return AM_DIRECT_PAGE;
            } else if (val <= 0xFFFF) {
                return AM_ABSOLUTE;
            } else {
                return AM_ABSOLUTE_LONG;
            }
        }
    }

    /* If it's a label, assume absolute for now */
    return AM_ABSOLUTE;
}

/* Find instruction by mnemonic and addressing mode */
const Instruction *find_instruction(const char *mnemonic, AddressingMode mode) {
    int i;
    /* First try exact match */
    for (i = 0; i < num_instructions; i++) {
        if (strcmp(instructions[i].mnemonic, mnemonic) == 0 &&
            instructions[i].mode == mode) {
            return &instructions[i];
        }
    }

    /* Try flexible matching for some instructions */
    if (mode == AM_IMPLIED) {
        /* Some instructions default to accumulator */
        for (i = 0; i < num_instructions; i++) {
            if (strcmp(instructions[i].mnemonic, mnemonic) == 0 &&
                instructions[i].mode == AM_ACCUMULATOR) {
                return &instructions[i];
            }
        }
    }

    /* If AM_ABSOLUTE didn't match, try AM_ABSOLUTE_LONG (for labels) */
    if (mode == AM_ABSOLUTE) {
        for (i = 0; i < num_instructions; i++) {
            if (strcmp(instructions[i].mnemonic, mnemonic) == 0 &&
                instructions[i].mode == AM_ABSOLUTE_LONG) {
                return &instructions[i];
            }
        }
    }

    return NULL;
}

/* Emit a byte */
void emit_byte(uint8_t byte) {
    if ((uint16_t)(pc - org) >= MAX_CODE) {
        error("Code too large");
        return;
    }
    if (pass == 2) {
        code[pc - org] = byte;
    }
    pc++;
}

/* Emit a word (16-bit, little-endian) */
void emit_word(uint16_t word) {
    emit_byte(word & 0xFF);
    emit_byte((word >> 8) & 0xFF);
}

/* Emit a long (24-bit, little-endian) */
void emit_long(uint32_t value) {
    emit_byte(value & 0xFF);
    emit_byte((value >> 8) & 0xFF);
    emit_byte((value >> 16) & 0xFF);
}

/* Initialize instruction table */
void init_instructions(void) {
    /* This is a simplified version - in a full assembler, you'd parse the CSV */
    /* For now, I'll add some key instructions manually */

    #define INST(mn, md, op, by, cy) do { \
        strcpy(instructions[num_instructions].mnemonic, mn); \
        instructions[num_instructions].mode = md; \
        instructions[num_instructions].opcode = op; \
        instructions[num_instructions].bytes = by; \
        instructions[num_instructions].cycles = cy; \
        num_instructions++; \
    } while(0)

    /* Control */
    INST("NOP", AM_IMPLIED, 0x00, 1, 2);
    /* HLT/STP/KIL (0xFF, all the same real opcode - cpu_instructions.cpp
     * cpu_inst_0xFF) documented in instruction-set.txt but never
     * registered here. */
    INST("HLT", AM_IMPLIED, 0xFF, 1, 0);
    INST("STP", AM_IMPLIED, 0xFF, 1, 0);
    INST("KIL", AM_IMPLIED, 0xFF, 1, 0);
    INST("BRK", AM_IMMEDIATE, 0x62, 2, 7);
    /* COP #$sig - GBA-SWI-style software interrupt (src/cpu.cpp opCOP),
     * vectors through $00:FFE4-FFE5, one slot below BRK's $00:FFE6. INT and
     * SWI are aliases for the x86/ARM-familiar spellings of the same
     * opcode. */
    INST("COP", AM_IMMEDIATE, 0xAF, 2, 7);
    INST("INT", AM_IMMEDIATE, 0xAF, 2, 7);  /* Alias (x86-style) */
    INST("SWI", AM_IMMEDIATE, 0xAF, 2, 7);  /* Alias (ARM-style) */
    INST("RTI", AM_IMPLIED, 0x17, 1, 6);
    INST("RTS", AM_IMPLIED, 0x19, 1, 6);
    INST("RTL", AM_IMPLIED, 0x18, 1, 6);

    /* #const immediate forms (LDA/LDX/LDY/ADC/SBC/AND/ORA/XOR/EOR/CMP/MUL)
     * are always a fixed 3 bytes (opcode + 16-bit operand) regardless of
     * the M/X flags - see CPU::addrImmediate()'s comment in src/cpu.cpp for
     * why (a real fetch/interpret width mismatch between LDX/LDY's M-keyed
     * fetch and X-keyed interpretation). In 8-bit mode the CPU just reads
     * the low byte and ignores the high one, so emitting both bytes here
     * unconditionally always matches what gets fetched. */

    /* Bit test - BIT (cpu_instructions.cpp 0x01-0x05, never registered here) */
    INST("BIT", AM_IMMEDIATE, 0x01, 3, 2);
    INST("BIT", AM_ABSOLUTE, 0x02, 3, 3);
    INST("BIT", AM_ABSOLUTE_X, 0x03, 3, 3);
    INST("BIT", AM_DIRECT_PAGE, 0x04, 2, 3);
    INST("BIT", AM_DP_X, 0x05, 2, 4);

    /* Loads - LDA */
    INST("LDA", AM_IMMEDIATE, 0x37, 3, 2);
    INST("LDA", AM_ABSOLUTE, 0x38, 3, 3);
    INST("LDA", AM_ABSOLUTE_X, 0x39, 3, 3);
    INST("LDA", AM_ABSOLUTE_Y, 0x3A, 3, 3);
    INST("LDA", AM_DIRECT_PAGE, 0x3B, 2, 3);
    INST("LDA", AM_DP_X, 0x3C, 2, 4);
    INST("LDA", AM_ABSOLUTE_LONG, 0x3D, 4, 5);
    INST("LDA", AM_ABSOLUTE_LONG_X, 0x3E, 4, 5);
    INST("LDA", AM_STACK_RELATIVE, 0x3F, 2, 4);
    INST("LDA", AM_SR_INDIRECT_Y, 0x34, 2, 7);
    INST("LDA", AM_DP_INDIRECT_LONG, 0x35, 2, 6);
    INST("LDA", AM_DP_INDIRECT_LONG_Y, 0x36, 2, 6);

    /* Loads - LDX */
    INST("LDX", AM_IMMEDIATE, 0x40, 3, 2);
    INST("LDX", AM_ABSOLUTE, 0x41, 3, 3);
    INST("LDX", AM_ABSOLUTE_Y, 0x42, 3, 3);
    INST("LDX", AM_DIRECT_PAGE, 0x43, 2, 3);
    INST("LDX", AM_DP_Y, 0x44, 2, 4);

    /* Loads - LDY */
    INST("LDY", AM_IMMEDIATE, 0x45, 3, 2);
    INST("LDY", AM_ABSOLUTE, 0x46, 3, 3);
    INST("LDY", AM_ABSOLUTE_X, 0x47, 3, 3);
    INST("LDY", AM_DIRECT_PAGE, 0x48, 2, 3);
    INST("LDY", AM_DP_X, 0x49, 2, 4);

    /* Stores - STA */
    INST("STA", AM_ABSOLUTE, 0x51, 3, 3);
    INST("STA", AM_ABSOLUTE_X, 0x52, 3, 3);
    INST("STA", AM_ABSOLUTE_Y, 0x53, 3, 3);
    INST("STA", AM_DIRECT_PAGE, 0x54, 2, 3);
    INST("STA", AM_DP_X, 0x4A, 2, 4);
    INST("STA", AM_ABSOLUTE_LONG, 0x55, 4, 5);
    INST("STA", AM_ABSOLUTE_LONG_X, 0x56, 4, 5);
    INST("STA", AM_STACK_RELATIVE, 0x57, 2, 4);
    INST("STA", AM_DP_INDIRECT_X, 0x4B, 2, 6);
    INST("STA", AM_DP_INDIRECT, 0x4C, 2, 5);
    INST("STA", AM_DP_INDIRECT_Y, 0x4D, 2, 6);
    /* STA (sr,S),Y / [dp] / [dp],Y: real opcodes, documented in
     * instruction-set.txt, but never registered here. */
    INST("STA", AM_SR_INDIRECT_Y, 0x4E, 2, 7);
    INST("STA", AM_DP_INDIRECT_LONG, 0x4F, 2, 6);
    INST("STA", AM_DP_INDIRECT_LONG_Y, 0x50, 2, 6);

    /* Stores - STX */
    INST("STX", AM_ABSOLUTE, 0x58, 3, 3);
    INST("STX", AM_DIRECT_PAGE, 0x59, 2, 3);
    INST("STX", AM_DP_Y, 0x5A, 2, 4);

    /* Stores - STY */
    INST("STY", AM_ABSOLUTE, 0x5B, 3, 3);
    INST("STY", AM_DIRECT_PAGE, 0x5C, 2, 3);
    INST("STY", AM_DP_X, 0x5D, 2, 4);

    /* Stores - STZ */
    INST("STZ", AM_ABSOLUTE, 0x5E, 3, 3);
    INST("STZ", AM_ABSOLUTE_X, 0x5F, 3, 3);
    INST("STZ", AM_DIRECT_PAGE, 0x60, 2, 3);
    INST("STZ", AM_DP_X, 0x61, 2, 4);

    /* Arithmetic - ADC */
    INST("ADC", AM_IMMEDIATE, 0xB6, 3, 2);
    INST("ADC", AM_ABSOLUTE, 0xB7, 3, 3);
    INST("ADC", AM_ABSOLUTE_X, 0xB8, 3, 3);
    INST("ADC", AM_ABSOLUTE_Y, 0xB9, 3, 3);
    INST("ADC", AM_DIRECT_PAGE, 0x66, 2, 3);
    INST("ADC", AM_DP_X, 0xBA, 2, 4);
    /* ADC's remaining addressing-mode variants (0xB0-0xB5, 0xBB-0xBD): real
     * opcodes, documented in instruction-set.txt, but never registered here. */
    INST("ADC", AM_DP_INDIRECT_Y, 0xB0, 2, 5);
    INST("ADC", AM_DP_INDIRECT_X, 0xB1, 2, 6);
    INST("ADC", AM_DP_INDIRECT, 0xB2, 2, 5);
    INST("ADC", AM_SR_INDIRECT_Y, 0xB3, 2, 7);
    INST("ADC", AM_DP_INDIRECT_LONG, 0xB4, 2, 6);
    INST("ADC", AM_DP_INDIRECT_LONG_Y, 0xB5, 2, 6);
    INST("ADC", AM_ABSOLUTE_LONG, 0xBB, 4, 5);
    INST("ADC", AM_ABSOLUTE_LONG_X, 0xBC, 4, 5);
    INST("ADC", AM_STACK_RELATIVE, 0xBD, 2, 4);

    /* Arithmetic - SBC */
    INST("SBC", AM_IMMEDIATE, 0xE1, 3, 2);
    INST("SBC", AM_ABSOLUTE, 0xE2, 3, 3);
    INST("SBC", AM_ABSOLUTE_Y, 0xE4, 3, 3);
    INST("SBC", AM_DIRECT_PAGE, 0xE5, 2, 3);
    /* SBC's remaining addressing-mode variants (0xDC-0xE0, 0xE3, 0xE6-0xE9):
     * real opcodes, documented in instruction-set.txt, but never registered
     * here. */
    INST("SBC", AM_DP_INDIRECT_X, 0xDC, 2, 6);
    INST("SBC", AM_DP_INDIRECT_Y, 0xDD, 2, 5);
    INST("SBC", AM_SR_INDIRECT_Y, 0xDE, 2, 7);
    INST("SBC", AM_DP_INDIRECT_LONG, 0xDF, 2, 6);
    INST("SBC", AM_DP_INDIRECT_LONG_Y, 0xE0, 2, 6);
    INST("SBC", AM_ABSOLUTE_X, 0xE3, 3, 3);
    INST("SBC", AM_DP_X, 0xE6, 2, 4);
    INST("SBC", AM_ABSOLUTE_LONG, 0xE7, 4, 5);
    INST("SBC", AM_ABSOLUTE_LONG_X, 0xE8, 4, 5);
    INST("SBC", AM_STACK_RELATIVE, 0xE9, 2, 4);

    /* Arithmetic - INC/DEC */
    INST("INC", AM_ACCUMULATOR, 0x69, 1, 2);
    INST("INC", AM_ABSOLUTE, 0x6A, 3, 3);
    INST("INC", AM_ABSOLUTE_X, 0x6B, 3, 3);
    INST("INC", AM_DIRECT_PAGE, 0x6C, 2, 5);
    INST("INC", AM_DP_X, 0x6D, 2, 6);
    /* 0x6E (cpu_inst_0x6E) is commented "INC long" in cpu_instructions.cpp
     * but actually calls addrAbsolute() - a plain 2-byte/3-byte-instruction
     * absolute operand, not a real 24-bit long form (that would need
     * addrAbsoluteLong()/fetch24()). Its addressing syntax is therefore
     * identical to 0x6A's (both "INC $1234"), just at a higher fixed cycle
     * cost (6 vs 3) - functionally a redundant duplicate encoding. Registered
     * here for documentation/table completeness and so cpudisasm.c's byte
     * count agrees with this file, but find_instruction() always returns the
     * first AM_ABSOLUTE match (0x6A), so this entry is unreachable from
     * plain "INC $1234" source - that's expected, not a bug: there is no
     * addressing-mode distinction between the two encodings to key off of. */
    INST("INC", AM_ABSOLUTE, 0x6E, 3, 6);
    INST("INX", AM_IMPLIED, 0x6F, 1, 2);
    INST("INY", AM_IMPLIED, 0x70, 1, 2);

    INST("DEC", AM_ACCUMULATOR, 0x71, 1, 2);
    INST("DEC", AM_ABSOLUTE, 0x72, 3, 3);
    INST("DEC", AM_ABSOLUTE_X, 0x73, 3, 3);
    INST("DEC", AM_DIRECT_PAGE, 0x74, 2, 5);
    INST("DEC", AM_DP_X, 0x75, 2, 6);
    /* 0x76 mirrors 0x6E above ("DEC long" comment, actually addrAbsolute(),
     * same shadowed-duplicate situation vs 0x72). */
    INST("DEC", AM_ABSOLUTE, 0x76, 3, 6);
    INST("DEX", AM_IMPLIED, 0x77, 1, 2);
    INST("DEY", AM_IMPLIED, 0x78, 1, 2);

    /* Logic */
    INST("AND", AM_IMMEDIATE, 0xC4, 3, 2);
    INST("AND", AM_ABSOLUTE, 0xC5, 3, 3);
    /* AND's remaining addressing-mode variants (0xBE-0xC3, 0xC6-0xCC): real
     * opcodes, documented in instruction-set.txt, but never registered here. */
    INST("AND", AM_DP_INDIRECT_X, 0xBE, 2, 6);
    INST("AND", AM_DP_INDIRECT, 0xBF, 2, 5);
    INST("AND", AM_DP_INDIRECT_Y, 0xC0, 2, 5);
    INST("AND", AM_SR_INDIRECT_Y, 0xC1, 2, 7);
    INST("AND", AM_DP_INDIRECT_LONG, 0xC2, 2, 6);
    INST("AND", AM_DP_INDIRECT_LONG_Y, 0xC3, 2, 6);
    INST("AND", AM_ABSOLUTE_X, 0xC6, 3, 3);
    INST("AND", AM_ABSOLUTE_Y, 0xC7, 3, 3);
    INST("AND", AM_DIRECT_PAGE, 0xC8, 2, 3);
    INST("AND", AM_DP_X, 0xC9, 2, 4);
    INST("AND", AM_ABSOLUTE_LONG, 0xCA, 4, 5);
    INST("AND", AM_ABSOLUTE_LONG_X, 0xCB, 4, 5);
    INST("AND", AM_STACK_RELATIVE, 0xCC, 2, 4);

    INST("ORA", AM_IMMEDIATE, 0xD3, 3, 2);
    INST("ORA", AM_ABSOLUTE, 0xD4, 3, 3);
    /* ORA's remaining addressing-mode variants (0xCD-0xD2, 0xD5-0xDB): real
     * opcodes, documented in instruction-set.txt, but never registered here. */
    INST("ORA", AM_DP_INDIRECT_X, 0xCD, 2, 6);
    INST("ORA", AM_DP_INDIRECT, 0xCE, 2, 5);
    INST("ORA", AM_DP_INDIRECT_Y, 0xCF, 2, 5);
    INST("ORA", AM_SR_INDIRECT_Y, 0xD0, 2, 7);
    INST("ORA", AM_DP_INDIRECT_LONG, 0xD1, 2, 6);
    INST("ORA", AM_DP_INDIRECT_LONG_Y, 0xD2, 2, 6);
    INST("ORA", AM_ABSOLUTE_X, 0xD5, 3, 3);
    INST("ORA", AM_ABSOLUTE_Y, 0xD6, 3, 3);
    INST("ORA", AM_DIRECT_PAGE, 0xD7, 2, 3);
    INST("ORA", AM_DP_X, 0xD8, 2, 4);
    INST("ORA", AM_ABSOLUTE_LONG, 0xD9, 4, 5);
    INST("ORA", AM_ABSOLUTE_LONG_X, 0xDA, 4, 5);
    INST("ORA", AM_STACK_RELATIVE, 0xDB, 2, 4);

    INST("XOR", AM_IMMEDIATE, 0xF9, 3, 2);
    INST("EOR", AM_IMMEDIATE, 0xF9, 3, 2);  /* Alias */
    /* XOR addr: real opcode (cpu_inst_0x95, commented "EOR addr" there but
     * calls the same opXOR()), documented in instruction-set.txt, but never
     * registered here - only the immediate form was. */
    INST("XOR", AM_ABSOLUTE, 0x95, 3, 3);
    INST("EOR", AM_ABSOLUTE, 0x95, 3, 3);  /* Alias */
    /* XOR's remaining addressing-mode variants (0x96-0x97, 0x9D-0x9F,
     * 0xF8, 0xFA-0xFE): real opcodes (cpu_instructions.cpp inconsistently
     * comments some of these "EOR" too - same alias, same opXOR() handler),
     * documented in instruction-set.txt, but never registered here. */
    INST("XOR", AM_ABSOLUTE_X, 0x96, 3, 3);
    INST("EOR", AM_ABSOLUTE_X, 0x96, 3, 3);  /* Alias */
    INST("XOR", AM_ABSOLUTE_Y, 0x97, 3, 3);
    INST("EOR", AM_ABSOLUTE_Y, 0x97, 3, 3);  /* Alias */
    INST("XOR", AM_DP_INDIRECT_X, 0x9D, 2, 6);
    INST("XOR", AM_DP_INDIRECT, 0x9E, 2, 5);
    INST("XOR", AM_DP_INDIRECT_Y, 0x9F, 2, 5);
    INST("XOR", AM_SR_INDIRECT_Y, 0xF8, 2, 7);
    INST("XOR", AM_DIRECT_PAGE, 0xFA, 2, 3);
    INST("XOR", AM_DP_X, 0xFB, 2, 4);
    INST("XOR", AM_ABSOLUTE_LONG, 0xFC, 4, 5);
    INST("XOR", AM_ABSOLUTE_LONG_X, 0xFD, 4, 5);
    INST("XOR", AM_STACK_RELATIVE, 0xFE, 2, 4);

    /* Shifts */
    INST("ASL", AM_ACCUMULATOR, 0x98, 1, 2);
    INST("ASL", AM_ABSOLUTE, 0x99, 3, 3);
    /* ASL/LSR addr,X,dp,dp,X and ROL/ROR addr,X,dp: real opcodes
     * (cpu_instructions.cpp 0x8F-0x91, 0x81-0x83, 0x85-0x87, 0x9A-0x9C),
     * documented in instruction-set.txt, but never registered here - only
     * the accumulator/plain-absolute forms were. */
    INST("ASL", AM_ABSOLUTE_X, 0x9A, 3, 3);
    INST("ASL", AM_DIRECT_PAGE, 0x9B, 2, 5);
    INST("ASL", AM_DP_X, 0x9C, 2, 6);
    INST("LSR", AM_ACCUMULATOR, 0x8D, 1, 2);
    INST("LSR", AM_ABSOLUTE, 0x8E, 3, 3);
    INST("LSR", AM_ABSOLUTE_X, 0x8F, 3, 3);
    INST("LSR", AM_DIRECT_PAGE, 0x90, 2, 5);
    INST("LSR", AM_DP_X, 0x91, 2, 6);
    INST("ROL", AM_ACCUMULATOR, 0x80, 1, 2);
    INST("ROL", AM_ABSOLUTE, 0x81, 3, 3);
    INST("ROL", AM_ABSOLUTE_X, 0x82, 3, 3);
    INST("ROL", AM_DIRECT_PAGE, 0x83, 2, 5);
    INST("ROR", AM_ACCUMULATOR, 0x84, 1, 2);
    INST("ROR", AM_ABSOLUTE, 0x85, 3, 3);
    INST("ROR", AM_ABSOLUTE_X, 0x86, 3, 3);
    INST("ROR", AM_DIRECT_PAGE, 0x87, 2, 5);
    /* RCL A (0x8C): real opcode, documented in instruction-set.txt, but
     * never registered here. Bare "RCL" (no operand) resolves to
     * AM_ACCUMULATOR too, via detect_addressing_mode()'s implied-accumulator
     * mnemonic list, which already includes RCL. */
    INST("RCL", AM_ACCUMULATOR, 0x8C, 1, 4);
    /* SHL/SHR X,Y (0x88-0x8B) are NOT registered here - see the dedicated
     * special-case block in assemble_line() (mirrors the DIV X,Y handling
     * below): both take a bare register name (X or Y) as their only operand,
     * with the register itself selecting the opcode, which the
     * mnemonic+AM_* lookup table has no way to express (AM_IMPLIED can only
     * ever map a mnemonic to a single opcode). */

    /* Jumps */
    INST("JMP", AM_ABSOLUTE, 0x0F, 3, 4);
    INST("JMP", AM_ABSOLUTE_LONG, 0x10, 4, 4);
    INST("JMP", AM_ABSOLUTE_INDEXED_INDIRECT, 0x0C, 3, 4);
    /* JMP (addr) / JMP [addr] (0x0D/0x0E): real opcodes, documented in
     * instruction-set.txt, but never registered here. */
    INST("JMP", AM_ABSOLUTE_INDIRECT, 0x0D, 3, 4);
    INST("JMP", AM_ABSOLUTE_INDIRECT_LONG, 0x0E, 3, 4);
    INST("JSR", AM_ABSOLUTE, 0x12, 3, 4);
    INST("JSR", AM_ABSOLUTE_INDEXED_INDIRECT, 0x11, 3, 4);
    INST("BRL", AM_PC_RELATIVE_LONG, 0x08, 3, 4);

    /* Branches */
    INST("BMI", AM_ABSOLUTE_LONG, 0x06, 4, 2);
    /* BRA/BVS use opBRA()/opBVS() in the interpreter, both fetch16() (a
     * 16-bit same-bank absolute address) - so the encoded instruction is
     * 3 bytes (opcode + 2-byte operand), not 2. The old "2" here truncated
     * a label's resolved address to its low byte (see cpuasm docs). */
    INST("BRA", AM_ABSOLUTE_LONG, 0x07, 3, 3);
    INST("BVS", AM_ABSOLUTE_LONG, 0x09, 3, 2);
    INST("BCS", AM_ABSOLUTE_LONG, 0x0A, 4, 2);
    INST("BGE", AM_ABSOLUTE_LONG, 0x0A, 4, 2);  /* Alias for BCS */
    INST("BEQ", AM_ABSOLUTE_LONG, 0x0B, 4, 2);
    /* JML is documented (instruction-set.txt) as an alias for JMP long, but
     * was never actually registered here. */
    INST("JML", AM_ABSOLUTE_LONG, 0x10, 4, 4);

    /* Stack */
    INST("PHA", AM_IMPLIED, 0x2B, 1, 3);
    INST("PLA", AM_IMPLIED, 0x32, 1, 4);
    INST("PHX", AM_IMPLIED, 0x30, 1, 3);
    INST("PHY", AM_IMPLIED, 0x31, 1, 3);
    INST("PHP", AM_IMPLIED, 0x2F, 1, 3);
    INST("POPF", AM_IMPLIED, 0x33, 1, 3);
    INST("PLP", AM_IMPLIED, 0x33, 1, 3);  /* Alias */
    INST("PHD", AM_IMPLIED, 0x2D, 1, 4);
    INST("PHB", AM_IMPLIED, 0x2C, 1, 3);
    /* PHK (0x2E): real opcode, documented in instruction-set.txt, but never
     * registered here. */
    INST("PHK", AM_IMPLIED, 0x2E, 1, 3);
    /* WAI (0x1C): real opcode, documented in instruction-set.txt, but never
     * registered here. */
    INST("WAI", AM_IMPLIED, 0x1C, 1, 3);
    /* PUSH word / PEA addr / PEI (dp) / PER label (0x27-0x2A): real opcodes,
     * documented in instruction-set.txt, but never registered here.
     * PUSH/PEA are functionally identical in cpu.cpp (both fetch16() a
     * literal 16-bit value and push16() it) - only the mnemonic differs, so
     * "PUSH #$1234" (immediate, x86-PUSH-style syntax) and "PEA $1234"
     * (bare absolute, real-65816-PEA-style syntax) are kept as separate
     * addressing-mode conventions rather than forcing one syntax onto both
     * mnemonics. PEI takes a direct-page-indirect operand like real 65816
     * PEI. PER takes a PC-relative label/offset like BRL - see the
     * BRL/PER carve-out added to detect_addressing_mode() above. */
    INST("PUSH", AM_IMMEDIATE, 0x27, 3, 3);
    INST("PEA", AM_ABSOLUTE, 0x28, 3, 3);
    INST("PEI", AM_DP_INDIRECT, 0x29, 2, 6);
    INST("PER", AM_PC_RELATIVE_LONG, 0x2A, 3, 6);

    /* Flags */
    INST("SEC", AM_IMPLIED, 0x63, 1, 2);
    INST("CLC", AM_IMPLIED, 0x7E, 1, 2);
    INST("SEI", AM_IMPLIED, 0x65, 1, 2);
    INST("CLI", AM_IMPLIED, 0x7C, 1, 2);
    INST("SED", AM_IMPLIED, 0x64, 1, 2);
    INST("CLD", AM_IMPLIED, 0x7B, 1, 2);
    INST("CLV", AM_IMPLIED, 0x7D, 1, 2);
    INST("SEP", AM_IMMEDIATE, 0x1A, 2, 3);
    INST("REP", AM_IMMEDIATE, 0x7F, 2, 3);

    /* Transfers */
    INST("TDC", AM_IMPLIED, 0x1D, 1, 2);
    INST("TSC", AM_IMPLIED, 0x1E, 1, 2);
    INST("TCS", AM_IMPLIED, 0x1F, 1, 2);
    INST("TAX", AM_IMPLIED, 0x20, 1, 2);
    INST("TXA", AM_IMPLIED, 0x21, 1, 2);
    INST("TAY", AM_IMPLIED, 0x22, 1, 2);
    INST("TCD", AM_IMPLIED, 0x23, 1, 2);
    INST("TXY", AM_IMPLIED, 0x24, 1, 2);
    INST("TYA", AM_IMPLIED, 0x25, 1, 2);
    INST("TYX", AM_IMPLIED, 0x26, 1, 2);

    /* Compare */
    INST("CMP", AM_IMMEDIATE, 0xA6, 3, 2);
    INST("CMP", AM_ABSOLUTE, 0xA7, 3, 3);
    INST("CMP", AM_ABSOLUTE_X, 0xA8, 3, 3);
    INST("CMP", AM_ABSOLUTE_Y, 0xA9, 3, 3);
    /* CMP's remaining addressing-mode variants (0xA0-0xA5, 0xAA-0xAE): real
     * opcodes, documented in instruction-set.txt, but never registered here. */
    INST("CMP", AM_DP_INDIRECT_X, 0xA0, 2, 6);
    INST("CMP", AM_DP_INDIRECT, 0xA1, 2, 5);
    INST("CMP", AM_DP_INDIRECT_Y, 0xA2, 2, 5);
    INST("CMP", AM_SR_INDIRECT_Y, 0xA3, 2, 7);
    INST("CMP", AM_DP_INDIRECT_LONG, 0xA4, 2, 6);
    INST("CMP", AM_DP_INDIRECT_LONG_Y, 0xA5, 2, 6);
    INST("CMP", AM_DIRECT_PAGE, 0xAA, 2, 3);
    INST("CMP", AM_DP_X, 0xAB, 2, 4);
    INST("CMP", AM_ABSOLUTE_LONG, 0xAC, 4, 5);
    INST("CMP", AM_ABSOLUTE_LONG_X, 0xAD, 4, 5);
    INST("CMP", AM_STACK_RELATIVE, 0xAE, 2, 4);
    INST("CPX", AM_ABSOLUTE, 0x79, 3, 3);
    INST("CPY", AM_ABSOLUTE, 0x7A, 3, 3);

    /* Hardware extensions (8086-inspired) - MUL: A * operand -> A:X            */
    /* (opcodes per docs/cpu/DEF88186.csv)                                      */
    INST("MUL", AM_IMMEDIATE,          0xEF, 3, 8);
    INST("MUL", AM_ABSOLUTE,           0xF0, 3, 9);
    INST("MUL", AM_ABSOLUTE_X,         0xF1, 3, 9);
    INST("MUL", AM_ABSOLUTE_Y,         0xF2, 3, 9);
    INST("MUL", AM_DIRECT_PAGE,        0xF3, 2, 9);
    INST("MUL", AM_DP_X,               0xF4, 2, 10);
    INST("MUL", AM_ABSOLUTE_LONG,      0xF5, 4, 11);
    INST("MUL", AM_ABSOLUTE_LONG_X,    0xF6, 4, 11);
    INST("MUL", AM_STACK_RELATIVE,     0xF7, 2, 10);
    INST("MUL", AM_DP_INDIRECT_X,      0xEA, 2, 12);
    INST("MUL", AM_DP_INDIRECT_Y,      0xEB, 2, 11);
    INST("MUL", AM_DP_INDIRECT_LONG,   0xED, 2, 12);
    INST("MUL", AM_DP_INDIRECT_LONG_Y, 0xEE, 2, 12);
    INST("MUL", AM_SR_INDIRECT_Y,      0xEC, 2, 13);
    /* DIV X,Y is handled as a special operand form below; long forms here.     */
    INST("DIV", AM_ABSOLUTE_LONG_X,    0x93, 4, 8);
    INST("DIV", AM_ABSOLUTE_LONG_Y,    0x94, 4, 8);
    INST("LOOP", AM_IMMEDIATE, 0x13, 3, 2);
    INST("LPEND", AM_IMPLIED, 0x14, 1, 1);
    INST("SDB", AM_IMMEDIATE, 0x1B, 2, 2);
    INST("CALL", AM_ABSOLUTE_LONG, 0x15, 4, 16);
    INST("RET", AM_IMPLIED, 0x16, 1, 8);

    /* Block move */
    INST("MVP", AM_BLOCK_MOVE, 0x68, 3, 1);
    INST("MVN", AM_BLOCK_MOVE, 0x67, 3, 1);

    #undef INST

    printf("Initialized %d instructions\n", num_instructions);
}

/* Assemble a single line */
int assemble_line(char *line) {
    char *p;
    char token[MAX_LINE];
    char *comment;
    char *colon;
    char *t;
    char filename[MAX_LINE];
    char *src;
    char *end;
    uint32_t addr;
    uint32_t val;
    char operand[MAX_LINE];
    AddressingMode mode;
    const Instruction *inst;
    char msg[128];
    uint32_t value;
    int is_label;
    const char *val_str;
    char clean_val[MAX_LINE];
    int i, j;
    int force_absolute;

    p = line;

    /* Remove comments */
    comment = strchr(line, ';');
    if (comment) *comment = '\0';

    /* Skip leading whitespace */
    p = skip_whitespace(p);
    if (!*p) return 0;  /* Empty line */

    /* Check for label */
    if (isalpha(*p) || *p == '_' || *p == '.') {
        colon = strchr(p, ':');
        if (colon) {
            /* It's a label */
            *colon = '\0';
            if (pass == 1) {
                add_label(p, pc);
            }
            p = colon + 1;
            p = skip_whitespace(p);
            if (!*p) return 0;  /* Label only */
        }
    }

    /* Parse instruction mnemonic */
    p = parse_token(p, token);
    if (!*token) return 0;

    /* Convert to uppercase */
    for (t = token; *t; t++) *t = toupper(*t);

    /* BNE/BPL/BVC/BCC have no real DEF88186 opcode - all 256 dispatch-table
     * entries in cpu_instructions.cpp are already assigned, so unlike a
     * documentation gap this can't be filled with a new hardware opcode.
     * These mnemonics are used throughout ZPdevtools/docs/cpu (see the .txt
     * files there) and existing .def sources anyway, so synthesize them from the real
     * complementary branch plus an unconditional BRA:
     *   <complementary branch> skip   ; taken iff the real condition is false
     *   BRA target
     * skip:
     * (BCC used to be wrongly aliased straight to BCS's opcode - carry SET,
     * not clear - which silently inverted the branch; that alias is gone.) */
    if (strcmp(token, "BNE") == 0 || strcmp(token, "BPL") == 0 ||
        strcmp(token, "BVC") == 0 || strcmp(token, "BCC") == 0) {
        uint8_t cond_opcode;
        int cond_bytes;
        uint32_t skip_addr;
        char synth[MAX_LINE];

        if (strcmp(token, "BNE") == 0)      { cond_opcode = 0x0B; cond_bytes = 4; } /* BEQ */
        else if (strcmp(token, "BPL") == 0) { cond_opcode = 0x06; cond_bytes = 4; } /* BMI */
        else if (strcmp(token, "BVC") == 0) { cond_opcode = 0x09; cond_bytes = 3; } /* BVS */
        else                                { cond_opcode = 0x0A; cond_bytes = 4; } /* BCS */

        p = skip_whitespace(p);

        /* skip: lands right after the BRA that follows the complementary
         * branch - both have fixed lengths regardless of the operand. */
        skip_addr = ((uint32_t)current_bank << 16) |
                    (uint32_t)(pc + cond_bytes + 3);

        emit_byte(cond_opcode);
        if (cond_bytes == 4) emit_long(skip_addr & 0xFFFFFFUL);
        else emit_word((uint16_t)(skip_addr & 0xFFFF));

        sprintf(synth, "BRA %s", p);
        assemble_line(synth);
        return 0;
    }

    /* Check for directives */
    if (strcmp(token, ".DATA") == 0) {
        /* Data section marker - just ignore for now */
        return 0;
    }

    if (strcmp(token, ".CODE") == 0) {
        /* Code section marker - just ignore for now */
        return 0;
    }

    if (strcmp(token, ".INCLUDE") == 0) {
        /* Include another assembly file */
        p = skip_whitespace(p);
        p = parse_token(p, token);
        if (*token) {
            /* Remove quotes if present */
            src = token;
            if (*src == '"') src++;
            strcpy(filename, src);
            end = filename + strlen(filename) - 1;
            if (*end == '"') *end = '\0';

            /* Recursively assemble the included file */
            assemble_file(filename);
        }
        return 0;
    }

    if (strcmp(token, ".ORG") == 0) {
        p = skip_whitespace(p);
        p = parse_token(p, token);
        if (parse_number(token, &addr)) {
            pc = addr & 0xFFFF;
            org = addr & 0xFFFF;
        }
        return 0;
    }

    /* .BANK $xx - declare which bank this file's code will be mapped at, so
     * label operands resolved to a long (24-bit) address - BEQ/BCS/BMI/
     * JMP long/LDA long,X and friends - encode that bank instead of the
     * default $00. Takes effect for labels defined after it; does not
     * affect pc/org (still 16-bit offsets). */
    if (strcmp(token, ".BANK") == 0) {
        p = skip_whitespace(p);
        p = parse_token(p, token);
        if (parse_number(token, &addr)) {
            current_bank = (uint8_t)(addr & 0xFF);
        }
        return 0;
    }

    /* .EQU NAME value - named compile-time constant. Documented in this
     * file's header comment since the beginning but never actually wired
     * up (parse_number would just fail on the name and every use of it
     * silently fell through to "unknown instruction"). Substitutes
     * anywhere a numeric operand is expected, resolved immediately - a
     * .EQU must appear before its first use. */
    if (strcmp(token, ".EQU") == 0) {
        char name[64];
        p = skip_whitespace(p);
        p = parse_token(p, name);
        p = skip_whitespace(p);
        p = parse_token(p, token);
        if (*name && parse_number(token, &val)) {
            add_equate(name, val);
        } else {
            error(".EQU expects: .EQU NAME value");
        }
        return 0;
    }

    if (strcmp(token, ".BYTE") == 0) {
        p = skip_whitespace(p);
        while (*p) {
            p = parse_token(p, token);
            if (!*token) break;
            if (parse_number(token, &val)) {
                emit_byte(val & 0xFF);
            }
            p = skip_whitespace(p);
            if (*p == ',') p++;
        }
        return 0;
    }

    if (strcmp(token, ".WORD") == 0) {
        p = skip_whitespace(p);
        while (*p) {
            p = parse_token(p, token);
            if (!*token) break;
            if (parse_number(token, &val)) {
                emit_word(val & 0xFFFF);
            }
            p = skip_whitespace(p);
            if (*p == ',') p++;
        }
        return 0;
    }

    /* Parse operand */
    operand[0] = '\0';
    p = skip_whitespace(p);
    if (*p) {
        strcpy(operand, p);
        /* Trim trailing whitespace */
        end = operand + strlen(operand) - 1;
        while (end > operand && isspace(*end)) *end-- = '\0';
    }
    substitute_equates(operand);

    /* Leading '!' forces absolute (DB-relative) addressing even when the
     * operand value is <=0xFF, which detect_addressing_mode() would
     * otherwise silently encode as direct page. Direct page addressing is
     * ALWAYS bank-0-relative on this CPU (matching real 65C816 semantics -
     * see CPU::addrDirectPage(), src/cpu.cpp), completely ignoring the
     * current DB register. Any small-valued I/O/window register access
     * after SDB to a non-zero bank MUST use '!' or it silently targets
     * bank $00 instead - writes there are dropped (ROM) or land in the
     * wrong RAM, and reads pull whatever is actually at that bank-0
     * address instead of the intended register. This was a real,
     * previously-undiagnosed bug: ZPbootROM's init.def used bare small
     * addresses (e.g. `sta $0021`) after `sdb #$D8`, so its DMA
     * configuration writes were silently discarded into cartridge ROM
     * and its wait_dma poll read cartridge payload bytes instead of the
     * DMA status register - see bootrom-dp-addressing-db-bug memory. */
    force_absolute = 0;
    if (operand[0] == '!') {
        force_absolute = 1;
        memmove(operand, operand + 1, strlen(operand));
    }

    /* DIV has a register form (DIV X,Y -> 0x92) plus long,X / long,Y forms that
     * divide the accumulator by a memory operand.  A bare address with ,X / ,Y
     * is always a 24-bit long-indexed operand for DIV, so force that mode
     * (labels would otherwise default to 16-bit absolute). */
    if (strcmp(token, "DIV") == 0) {
        char norm[MAX_LINE];
        int k = 0;
        for (i = 0; operand[i] && k < MAX_LINE - 1; i++) {
            char ch = (char)toupper((unsigned char)operand[i]);
            if (ch != ' ' && ch != '\t') norm[k++] = ch;
        }
        norm[k] = '\0';
        if (strcmp(norm, "X,Y") == 0 || strcmp(norm, "X") == 0) {
            emit_byte(0x92);  /* DIV X,Y */
            return 0;
        } else if (k >= 2 && norm[k - 2] == ',' && norm[k - 1] == 'X') {
            mode = AM_ABSOLUTE_LONG_X;   /* DIV long,X -> 0x93 */
        } else if (k >= 2 && norm[k - 2] == ',' && norm[k - 1] == 'Y') {
            mode = AM_ABSOLUTE_LONG_Y;   /* DIV long,Y -> 0x94 */
        } else {
            error("DIV expects 'X,Y', 'long,X' or 'long,Y'");
            return -1;
        }
    } else if (strcmp(token, "SHL") == 0 || strcmp(token, "SHR") == 0) {
        /* SHL/SHR X,Y (0x88-0x8B): each mnemonic covers two opcodes - one
         * per register - selected by the bare register name in the operand
         * (there is no memory-operand form at all). That's not expressible
         * as a single (mnemonic, AM_*) table entry the way every other
         * instruction here is, so - like DIV X,Y above - dispatch on the
         * operand text directly instead of going through
         * detect_addressing_mode()/find_instruction(). */
        char norm[MAX_LINE];
        int k = 0;
        for (i = 0; operand[i] && k < MAX_LINE - 1; i++) {
            char ch = (char)toupper((unsigned char)operand[i]);
            if (ch != ' ' && ch != '\t') norm[k++] = ch;
        }
        norm[k] = '\0';
        if (strcmp(norm, "X") == 0) {
            emit_byte(token[2] == 'L' ? 0x88 : 0x8A);  /* SHL X / SHR X */
            return 0;
        } else if (strcmp(norm, "Y") == 0) {
            emit_byte(token[2] == 'L' ? 0x89 : 0x8B);  /* SHL Y / SHR Y */
            return 0;
        } else {
            error("SHL/SHR expects 'X' or 'Y'");
            return -1;
        }
    } else {
        /* Detect addressing mode */
        mode = detect_addressing_mode(operand, token);

        if (force_absolute) {
            switch (mode) {
                case AM_DIRECT_PAGE:        mode = AM_ABSOLUTE; break;
                case AM_DP_X:                mode = AM_ABSOLUTE_X; break;
                case AM_DP_Y:                mode = AM_ABSOLUTE_Y; break;
                case AM_DP_INDIRECT:         mode = AM_ABSOLUTE_INDIRECT; break;
                case AM_DP_INDIRECT_X:       mode = AM_ABSOLUTE_INDEXED_INDIRECT; break;
                default: break;  /* already absolute/long, or not DP-family */
            }
        }
    }

    /* Find instruction */
    inst = find_instruction(token, mode);
    if (!inst) {
        sprintf(msg, "Unknown instruction or addressing mode: %s %s", token, operand);
        error(msg);
        return -1;
    }

    /* Emit opcode */
    emit_byte(inst->opcode);

    /* Emit operand bytes */
    if (inst->bytes > 1) {
        /* Parse operand value */
        value = 0;
        is_label = 0;

        /* Extract actual value (preserve $ for parse_number) */
        val_str = operand;
        if (*val_str == '#') val_str++;  /* Skip immediate marker */
        if (*val_str == '(' || *val_str == '[') val_str++;  /* Skip indirect markers */

        /* Remove closing ), ], and index registers */
        j = 0;
        for (i = 0; val_str[i] && j < MAX_LINE - 1; i++) {
            if (val_str[i] != ')' && val_str[i] != ']' && val_str[i] != ',') {
                clean_val[j++] = val_str[i];
            } else if (val_str[i] == ',') {
                break;  /* Stop at comma */
            }
        }
        clean_val[j] = '\0';

        if (parse_number(clean_val, &value)) {
            /* Plain numeric literal - value already set above. */
        } else if (find_equate(clean_val) >= 0) {
            /* .EQU constant - a plain numeric literal, not an address, so
             * no bank-folding or fixup involved. */
            value = equates[find_equate(clean_val)].value;
        } else {
            /* It's a label */
            is_label = 1;
            if (pass == 2) {
                /* Look up label */
                int idx = find_label(clean_val);
                if (idx >= 0 && labels[idx].defined) {
                    /* Fold in the label's bank for the 24-bit (long) operand
                     * forms; bytes==2/3 emission below only ever keeps the
                     * low 8/16 bits, so this is a no-op for those. */
                    value = ((uint32_t)labels[idx].bank << 16) | labels[idx].address;
                } else {
                    /* Add fixup */
                    add_fixup(pc, clean_val, mode, inst->bytes - 1);
                }
            }
        }

        /* Emit bytes based on instruction size */
        if (inst->bytes == 2) {
            emit_byte(value & 0xFF);
        } else if (inst->bytes == 3) {
            if (mode == AM_PC_RELATIVE_LONG && !is_label) {
                /* Calculate relative offset. BRL/PER (cpu.cpp opBRL()/
                 * opPER()) add the signed offset to PC as it stands right
                 * after the 2-byte operand is fetched - i.e. relative to
                 * the address of the NEXT instruction (this 3-byte
                 * instruction's own address + 3). `pc` here is read after
                 * emit_byte(opcode) but before the operand bytes are
                 * emitted, so it's already (opcode address + 1); the
                 * reference point is therefore pc + 2, not pc + 1 - the old
                 * "+ 1" undershot by one byte, so a literal-address
                 * "PER $target" or "BRL $target" landed one byte before the
                 * intended target. See the matching fixup-path fix in
                 * resolve_fixups() below for the label-operand case. */
                int16_t offset = (int16_t)(value - (pc + 2));
                emit_word(offset);
            } else if (is_label && mode == AM_PC_RELATIVE_LONG) {
                /* Add fixup for relative address */
                add_fixup(pc, clean_val, AM_PC_RELATIVE_LONG, 2);
                emit_word(0);  /* Placeholder */
            } else {
                emit_word(value & 0xFFFF);
            }
        } else if (inst->bytes == 4) {
            emit_long(value & 0xFFFFFF);
        }
    }

    return 0;
}

/* Assemble a file */
void assemble_file(const char *filename) {
    FILE *f;
    char line[MAX_LINE];
    int line_num;

    f = fopen(filename, "r");
    if (!f) {
        error("Cannot open input file");
        return;
    }

    line_num = 0;

    while (fgets(line, sizeof(line), f)) {
        line_num++;
        assemble_line(line);
    }

    fclose(f);

    printf("Pass %d: Assembled %d bytes, %d labels, %d fixups\n",
           pass, pc, num_labels, num_fixups);
}

/* Main */
int main(int argc, char* argv[]) {
    const char *input;
    const char *output;
    FILE *out;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s input.asm [output.bin]\n", argv[0]);
        return 1;
    }

    input = argv[1];
    output = (argc > 2) ? argv[2] : "output.bin";

    printf("DEF88186 Assembler\n");
    printf("Input: %s\n", input);
    printf("Output: %s\n", output);

    init_instructions();

    /* Pass 1: Collect labels */
    pass = 1;
    pc = 0;
    assemble_file(input);

    /* Pass 2: Generate code */
    pass = 2;
    pc = 0;
    org = 0;
    num_fixups = 0;
    memset(code, 0, sizeof(code));
    assemble_file(input);

    /* Resolve forward references */
    resolve_fixups();

    if (errors > 0) {
        fprintf(stderr, "\n%d errors found, not writing output\n", errors);
        return 1;
    }

    /* Write output */
    out = fopen(output, "wb");
    if (!out) {
        error("Cannot open output file");
        return 1;
    }

    fwrite(code, 1, pc - org, out);
    fclose(out);

    printf("\nSuccess! Wrote %d bytes to %s\n", (int)(pc - org), output);

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "compat.h"

#define MAX_LABELS 256
#define MAX_LINE 512
#define MAX_TOKENS 10

typedef struct {
    char name[64];
    uint16_t address;
} Label;

static Label labels[MAX_LABELS];
static int label_count = 0;
static uint16_t current_address = 0;
static uint16_t* instructions = NULL;
static long instruction_count = 0;

/* Token parsing */
static char* tokens[MAX_TOKENS];
static int token_count = 0;

void error(const char* msg, int line) {
    fprintf(stderr, "Error on line %d: %s\n", line, msg);
    exit(1);
}

void trim(char* str) {
    char* start;
    int len;

    start = str;
    while (isspace(*start)) start++;
    if (start != str) memmove(str, start, strlen(start) + 1);

    len = strlen(str);
    while (len > 0 && isspace(str[len - 1])) {
        str[--len] = '\0';
    }
}

void tokenize(char* line) {
    char* token;

    token_count = 0;
    token = strtok(line, " \t,");
    while (token && token_count < MAX_TOKENS) {
        tokens[token_count++] = token;
        token = strtok(NULL, " \t,");
    }
}

int is_label_def(const char* str) {
    int len = strlen(str);
    return len > 0 && str[len - 1] == ':';
}

void add_label(const char* name, uint16_t address) {
    int len;

    if (label_count >= MAX_LABELS) {
        fprintf(stderr, "Too many labels\n");
        exit(1);
    }

    strncpy(labels[label_count].name, name, 63);
    labels[label_count].name[63] = '\0';

    /* Remove trailing colon if present */
    len = strlen(labels[label_count].name);
    if (len > 0 && labels[label_count].name[len - 1] == ':') {
        labels[label_count].name[len - 1] = '\0';
    }

    labels[label_count].address = address;
    label_count++;
}

int find_label(const char* name) {
    int i;
    for (i = 0; i < label_count; i++) {
        if (strcmp(labels[i].name, name) == 0) {
            return labels[i].address;
        }
    }
    return -1;
}

int parse_number(const char* str) {
    if (str[0] == '$') {
        return (int)strtol(str + 1, NULL, 16);
    } else if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        return (int)strtol(str + 2, NULL, 16);
    } else if (str[0] == '0' && (str[1] == 'b' || str[1] == 'B')) {
        return (int)strtol(str + 2, NULL, 2);
    } else {
        return atoi(str);
    }
}

int parse_register(const char* str) {
    if (str[0] == 'R' || str[0] == 'r') {
        return parse_number(str + 1);
    } else if (strcmp(str, "X") == 0 || strcmp(str, "x") == 0) {
        return 0;
    } else if (strcmp(str, "Y") == 0 || strcmp(str, "y") == 0) {
        return 1;
    }
    return parse_number(str);
}

void emit_instruction(uint16_t inst) {
    if (instruction_count >= 16384) {
        fprintf(stderr, "Too many instructions\n");
        exit(1);
    }
    instructions[instruction_count++] = inst;
    current_address += 2;
}

uint16_t encode_instruction(uint8_t opcode, uint16_t operand) {
    return ((opcode & 0x1F) << 11) | (operand & 0x7FF);
}

void assemble_line(char* line, int line_num) {
    char* comment;
    const char* mnemonic;
    char upper_mnemonic[64];
    int i;

    /* Remove comments */
    comment = strchr(line, ';');
    if (comment) *comment = '\0';

    trim(line);
    if (strlen(line) == 0) return;

    tokenize(line);
    if (token_count == 0) return;

    /* Check for label definition */
    if (is_label_def(tokens[0])) {
        add_label(tokens[0], current_address);

        /* Check if there's an instruction on the same line */
        if (token_count > 1) {
            /* Shift tokens left */
            for (i = 0; i < token_count - 1; i++) {
                tokens[i] = tokens[i + 1];
            }
            token_count--;
        } else {
            return;
        }
    }

    mnemonic = tokens[0];

    /* Convert to uppercase for comparison */
    strncpy(upper_mnemonic, mnemonic, 63);
    upper_mnemonic[63] = '\0';
    for (i = 0; upper_mnemonic[i]; i++) {
        upper_mnemonic[i] = toupper(upper_mnemonic[i]);
    }

    /* Instruction encoding */

    if (strcmp(upper_mnemonic, "NOP") == 0) {
        uint16_t stall = (token_count > 1) ? parse_number(tokens[1]) : 0;
        emit_instruction(encode_instruction(0x00, stall));
    }
    else if (strcmp(upper_mnemonic, "JMP") == 0) {
        /* Real encoding (APU::execJMP, apu.cpp): direction=bit9, mode=bit8,
         * offset=bits7-0 - not the doc's D=bit10/M=bit9/fixed-0=bit8 (off by
         * one bit; bit10 is unused dead weight in real hardware). Relative
         * is only taken when bits9-8 are both 0, which also means the
         * "backward" direction (bit9=1, bit8=0) is unreachable - that
         * combination decodes as absolute-BIOS instead. So: bare "JMP $ZZ"
         * is a forward-only relative jump (bits9-8=00); "JMP D, M, $ZZ"
         * (doc-style 3-arg form) selects absolute addressing via M
         * (0=$80ZZ BIOS, 1=$RPZZ) - D is accepted for doc compatibility but
         * has no reachable effect and is ignored, matching real hardware. */
        if (token_count >= 4) {
            int mode = parse_number(tokens[2]);
            int addr = parse_number(tokens[3]);
            if (mode) {
                emit_instruction(encode_instruction(0x01, 0x100 | (addr & 0xFF)));  /* $RPZZ */
            } else {
                emit_instruction(encode_instruction(0x01, 0x200 | (addr & 0xFF)));  /* $80ZZ (BIOS) */
            }
        } else if (token_count >= 2) {
            int addr = parse_number(tokens[1]);
            emit_instruction(encode_instruction(0x01, addr & 0xFF));
        }
    }
    else if (strcmp(upper_mnemonic, "JNZ") == 0) {
        if (token_count >= 3) {
            int reg = parse_register(tokens[1]);
            int addr = parse_number(tokens[2]);
            uint16_t operand = ((reg & 1) << 10) | (addr & 0xFF);
            emit_instruction(encode_instruction(0x02, operand));
        }
    }
    else if (strcmp(upper_mnemonic, "JZ") == 0) {
        if (token_count >= 4) {
            /* jz X, Y, $ZZ — test reg X, mode Y, jump to $ZZ */
            int rx   = parse_register(tokens[1]);
            int mode = parse_number(tokens[2]);
            int addr = parse_number(tokens[3]);
            emit_instruction(encode_instruction(0x02, 0x100 | ((rx&1)<<10) | ((mode&1)<<9) | (addr&0xFF)));
        } else if (token_count >= 3) {
            /* jz X, $ZZ — mode defaults to RP */
            int rx   = parse_register(tokens[1]);
            int addr = parse_number(tokens[2]);
            emit_instruction(encode_instruction(0x02, 0x100 | ((rx&1)<<10) | 0x200 | (addr&0xFF)));
        }
    }
    else if (strcmp(upper_mnemonic, "SRP") == 0) {
        if (token_count >= 2) {
            int page = parse_number(tokens[1]);
            emit_instruction(encode_instruction(0x03, page & 0xFF));
        }
    }
    else if (strcmp(upper_mnemonic, "SDP") == 0) {
        if (token_count >= 3) {
            /* sdp b,$XX — set byte b of data pointer, opcode 0x17 */
            int b = parse_number(tokens[1]);
            int value = parse_number(tokens[2]);
            emit_instruction(encode_instruction(0x17, 0x200 | ((b & 1) << 8) | (value & 0xFF)));
        } else if (token_count >= 2) {
            /* sdp $XX — legacy: set DP high byte, opcode 0x03 */
            int page = parse_number(tokens[1]);
            emit_instruction(encode_instruction(0x03, 0x400 | (page & 0xFF)));
        }
    }
    else if (strcmp(upper_mnemonic, "RET")  == 0) { emit_instruction(encode_instruction(0x17, 0x040)); }
    else if (strcmp(upper_mnemonic, "BSP")  == 0) { emit_instruction(encode_instruction(0x17, 0x000)); }
    else if (strcmp(upper_mnemonic, "PUDP") == 0) { emit_instruction(encode_instruction(0x17, 0x041)); }
    else if (strcmp(upper_mnemonic, "PODP") == 0) { emit_instruction(encode_instruction(0x17, 0x042)); }
    else if (strcmp(upper_mnemonic, "PUX")  == 0) { emit_instruction(encode_instruction(0x17, 0x400)); }
    else if (strcmp(upper_mnemonic, "PUY")  == 0) { emit_instruction(encode_instruction(0x17, 0x500)); }
    else if (strcmp(upper_mnemonic, "POX")  == 0) { emit_instruction(encode_instruction(0x17, 0x600)); }
    else if (strcmp(upper_mnemonic, "POY")  == 0) { emit_instruction(encode_instruction(0x17, 0x700)); }
    /* NOR/AND/ADD/SUB (opcodes 0x04-0x07) share one encoding shape:
     * toMem = !(operand & 0x100) in apu.cpp - bit8 CLEAR means "store the
     * word result to dp:$offset (bits7-0)", bit8 SET means "store to
     * register Z (bits6-0)". Same $-prefix-vs-register-name dispatch XOR/CRB
     * already use below to pick between their two forms. */
    else if (strcmp(upper_mnemonic, "NOR") == 0) {
        if (token_count >= 4) {
            int rx = parse_register(tokens[1]);
            int ry = parse_register(tokens[2]);
            char first = tokens[3][0];
            if (first == '$' || first == '0' || (first >= '1' && first <= '9')) {
                int offset = parse_number(tokens[3]);
                emit_instruction(encode_instruction(0x04, ((rx & 1) << 10) | ((ry & 1) << 9) | (offset & 0xFF)));
            } else {
                int rz = parse_register(tokens[3]);
                emit_instruction(encode_instruction(0x04, ((rx & 1) << 10) | ((ry & 1) << 9) | 0x100 | (rz & 0x7F)));
            }
        }
    }
    else if (strcmp(upper_mnemonic, "AND") == 0) {
        if (token_count >= 4) {
            int rx = parse_register(tokens[1]);
            int ry = parse_register(tokens[2]);
            char first = tokens[3][0];
            if (first == '$' || first == '0' || (first >= '1' && first <= '9')) {
                int offset = parse_number(tokens[3]);
                emit_instruction(encode_instruction(0x05, ((rx & 1) << 10) | ((ry & 1) << 9) | (offset & 0xFF)));
            } else {
                int rz = parse_register(tokens[3]);
                emit_instruction(encode_instruction(0x05, ((rx & 1) << 10) | ((ry & 1) << 9) | 0x100 | (rz & 0x7F)));
            }
        }
    }
    else if (strcmp(upper_mnemonic, "ADD") == 0) {
        if (token_count >= 4) {
            int rx = parse_register(tokens[1]);
            int ry = parse_register(tokens[2]);
            char first = tokens[3][0];
            if (first == '$' || first == '0' || (first >= '1' && first <= '9')) {
                int offset = parse_number(tokens[3]);
                emit_instruction(encode_instruction(0x06, ((rx & 1) << 10) | ((ry & 1) << 9) | (offset & 0xFF)));
            } else {
                int rz = parse_register(tokens[3]);
                emit_instruction(encode_instruction(0x06, ((rx & 1) << 10) | ((ry & 1) << 9) | 0x100 | (rz & 0x7F)));
            }
        }
    }
    else if (strcmp(upper_mnemonic, "SUB") == 0) {
        if (token_count >= 4) {
            int rx = parse_register(tokens[1]);
            int ry = parse_register(tokens[2]);
            char first = tokens[3][0];
            if (first == '$' || first == '0' || (first >= '1' && first <= '9')) {
                int offset = parse_number(tokens[3]);
                emit_instruction(encode_instruction(0x07, ((rx & 1) << 10) | ((ry & 1) << 9) | (offset & 0xFF)));
            } else {
                int rz = parse_register(tokens[3]);
                emit_instruction(encode_instruction(0x07, ((rx & 1) << 10) | ((ry & 1) << 9) | 0x100 | (rz & 0x7F)));
            }
        }
    }
    else if (strcmp(upper_mnemonic, "LDA") == 0) {
        if (token_count >= 3) {
            /* lda X/Y, $YY — load immediate (shorthand for SCR) */
            int reg = parse_register(tokens[1]);
            int value = parse_number(tokens[2]);
            uint16_t operand = ((reg & 1) << 10) | (value & 0xFF);
            emit_instruction(encode_instruction(0x0A, operand));
        } else if (token_count >= 2) {
            /* lda X/Y — load (dp:db) into register */
            int reg = parse_register(tokens[1]);
            emit_instruction(encode_instruction(0x09, ((reg & 1) << 10) | 0x000));
        }
    }
    else if (strcmp(upper_mnemonic, "STA") == 0) {
        if (token_count >= 3) {
            /* sta X, $offset — store to DP+offset, opcode 0x08 */
            int reg = parse_register(tokens[1]);
            int offset = parse_number(tokens[2]);
            uint16_t operand = ((reg & 1) << 10) | 0x200 | (offset & 0xFF);
            emit_instruction(encode_instruction(0x08, operand));
        } else if (token_count >= 2) {
            char first = tokens[1][0];
            if (first == 'X' || first == 'x' || first == 'Y' || first == 'y') {
                /* sta X/Y — store register to (dp:db), opcode 0x09 subop 01 */
                int reg = parse_register(tokens[1]);
                emit_instruction(encode_instruction(0x09, ((reg & 1) << 10) | 0x100));
            } else {
                /* sta $XX — store immediate to (dp:db), opcode 0x09 subop 10 */
                int value = parse_number(tokens[1]);
                emit_instruction(encode_instruction(0x09, 0x200 | (value & 0xFF)));
            }
        }
    }
    else if (strcmp(upper_mnemonic, "STR") == 0) {
        if (token_count >= 3) {
            int offset = parse_number(tokens[1]);
            int reg = parse_register(tokens[2]);
            uint16_t operand = ((reg & 1) << 10) | (offset & 0xFF);
            emit_instruction(encode_instruction(0x08, operand));
        }
    }
    else if (strcmp(upper_mnemonic, "SFR") == 0) {
        if (token_count >= 2) {
            int val = parse_number(tokens[1]);
            emit_instruction(encode_instruction(0x0B, 0x100 | ((val & 0xF) << 4)));
        }
    }
    else if (strcmp(upper_mnemonic, "CF") == 0) {
        if (token_count >= 2) {
            int val = parse_number(tokens[1]);
            emit_instruction(encode_instruction(0x0B, 0x200 | ((val & 0xF) << 4)));
        }
    }
    else if (strcmp(upper_mnemonic, "SF") == 0) {
        if (token_count >= 2) {
            int val = parse_number(tokens[1]);
            emit_instruction(encode_instruction(0x0B, 0x300 | ((val & 0xF) << 4)));
        }
    }
    else if (strcmp(upper_mnemonic, "STF") == 0) {
        if (token_count >= 2) {
            int reg = parse_register(tokens[1]);
            emit_instruction(encode_instruction(0x0B, 0x400 | ((reg & 1) << 8)));
        }
    }
    else if (strcmp(upper_mnemonic, "SCR") == 0) {
        if (token_count >= 3) {
            int reg = parse_register(tokens[1]);
            int value = parse_number(tokens[2]);
            uint16_t operand = ((reg & 1) << 10) | (value & 0xFF);
            emit_instruction(encode_instruction(0x0A, operand));
        }
    }
    else if (strcmp(upper_mnemonic, "ZOR") == 0) {
        if (token_count >= 2) {
            int reg = parse_register(tokens[1]);
            emit_instruction(encode_instruction(0x0C, 0x080 | (reg & 0x3F)));
        }
    }
    else if (strcmp(upper_mnemonic, "ZOA") == 0) {
        if (token_count >= 2) {
            if (strcmp(tokens[1], "DP") == 0 || strcmp(tokens[1], "dp") == 0) {
                emit_instruction(encode_instruction(0x0C, 0x300));
            } else {
                int offset = parse_number(tokens[1]);
                emit_instruction(encode_instruction(0x0C, 0x200 | (offset & 0xFF)));
            }
        }
    }
    else if (strcmp(upper_mnemonic, "LST") == 0) {
        if (token_count >= 3) {
            int loopId = parse_number(tokens[1]);
            int count = parse_number(tokens[2]);
            uint16_t operand = ((loopId & 0x0F) << 6) | (count & 0x3F);
            emit_instruction(encode_instruction(0x0D, operand));
        }
    }
    else if (strcmp(upper_mnemonic, "LFN") == 0) {
        if (token_count >= 2) {
            int loopId = parse_number(tokens[1]);
            uint16_t operand = 0x400 | ((loopId & 0x0F) << 6);
            emit_instruction(encode_instruction(0x0D, operand));
        }
    }
    else if (strcmp(upper_mnemonic, "BRT") == 0) {
        /* brt $WW - break from the highest-ID active loop, resuming at the
         * loop's endAddress + $WW*2 words forward (execBRT, apu.cpp). Raw
         * 11-bit operand, no dispatch bits. */
        if (token_count >= 2) {
            int offset = parse_number(tokens[1]);
            emit_instruction(encode_instruction(0x0E, offset & 0x7FF));
        }
    }
    else if (strcmp(upper_mnemonic, "BRP") == 0) {
        /* brp - break to re-enter; currently a no-op in the emulator
         * (execBRP discards its operand), but still a real, distinct
         * mnemonic that must assemble. */
        emit_instruction(encode_instruction(0x0F, 0));
    }
    /* ADC/SBC (opcodes 0x10-0x11) share NOR/AND/ADD/SUB's toMem polarity
     * (bit8 CLEAR = write byte to dp:$offset, bit8 SET = write register Z).
     * The old code never set bit8 for the register form, so "ADC X, Y, Z"
     * silently encoded as a memory write to dp:(Z&0x7F) instead of writing
     * register Z - a real runtime correctness bug, not just a missing mode. */
    else if (strcmp(upper_mnemonic, "ADC") == 0) {
        if (token_count >= 4) {
            int rx = parse_register(tokens[1]);
            int ry = parse_register(tokens[2]);
            char first = tokens[3][0];
            if (first == '$' || first == '0' || (first >= '1' && first <= '9')) {
                int offset = parse_number(tokens[3]);
                emit_instruction(encode_instruction(0x10, ((rx & 1) << 10) | ((ry & 1) << 9) | (offset & 0xFF)));
            } else {
                int rz = parse_register(tokens[3]);
                emit_instruction(encode_instruction(0x10, ((rx & 1) << 10) | ((ry & 1) << 9) | 0x100 | (rz & 0x7F)));
            }
        }
    }
    else if (strcmp(upper_mnemonic, "SBC") == 0) {
        if (token_count >= 4) {
            int rx = parse_register(tokens[1]);
            int ry = parse_register(tokens[2]);
            char first = tokens[3][0];
            if (first == '$' || first == '0' || (first >= '1' && first <= '9')) {
                int offset = parse_number(tokens[3]);
                emit_instruction(encode_instruction(0x11, ((rx & 1) << 10) | ((ry & 1) << 9) | (offset & 0xFF)));
            } else {
                int rz = parse_register(tokens[3]);
                emit_instruction(encode_instruction(0x11, ((rx & 1) << 10) | ((ry & 1) << 9) | 0x100 | (rz & 0x7F)));
            }
        }
    }
    else if (strcmp(upper_mnemonic, "CME") == 0 || strcmp(upper_mnemonic, "CMN") == 0 ||
             strcmp(upper_mnemonic, "CMG") == 0 || strcmp(upper_mnemonic, "CML") == 0) {
        /* opcode 0x19, subOp = bits7-6 selects CME(00)/CMN(01)/CMG(10)/CML(11)
         * (apu.cpp's case 0x19 dispatch). subOp's LSB doubles as branch
         * direction inside each execCM*: CME/CMG always branch forward,
         * CMN/CML always branch backward - direction is not independently
         * selectable despite the doc's 4-arg "X, Y, D, $WW" syntax, so this
         * only takes X, Y, $WW (offset is 6 bits, bits5-0). */
        if (token_count >= 4) {
            int rx = parse_register(tokens[1]);
            int ry = parse_register(tokens[2]);
            int offset = parse_number(tokens[3]);
            int subop;
            if (strcmp(upper_mnemonic, "CME") == 0) subop = 0;
            else if (strcmp(upper_mnemonic, "CMN") == 0) subop = 1;
            else if (strcmp(upper_mnemonic, "CMG") == 0) subop = 2;
            else subop = 3;
            emit_instruction(encode_instruction(0x19, ((rx & 1) << 10) | ((ry & 1) << 9) | (subop << 6) | (offset & 0x3F)));
        }
    }
    else if (strcmp(upper_mnemonic, "BEQ") == 0) {
        if (token_count >= 4) {
            int rx = parse_register(tokens[1]);
            int ry = parse_register(tokens[2]);
            int offset = parse_number(tokens[3]);
            uint16_t operand = ((rx & 1) << 10) | ((ry & 1) << 9) | (offset & 0xFF);
            emit_instruction(encode_instruction(0x12, operand));
        }
    }
    else if (strcmp(upper_mnemonic, "BNE") == 0) {
        if (token_count >= 4) {
            int rx = parse_register(tokens[1]);
            int ry = parse_register(tokens[2]);
            int offset = parse_number(tokens[3]);
            uint16_t operand = ((rx & 1) << 10) | ((ry & 1) << 9) | 0x100 | (offset & 0xFF);
            emit_instruction(encode_instruction(0x12, operand));
        }
    }
    else if (strcmp(upper_mnemonic, "BLT") == 0) {
        if (token_count >= 4) {
            int rx = parse_register(tokens[1]);
            int ry = parse_register(tokens[2]);
            int offset = parse_number(tokens[3]);
            uint16_t operand = ((rx & 1) << 10) | ((ry & 1) << 9) | (offset & 0xFF);
            emit_instruction(encode_instruction(0x13, operand));
        }
    }
    else if (strcmp(upper_mnemonic, "BGT") == 0) {
        if (token_count >= 4) {
            int rx = parse_register(tokens[1]);
            int ry = parse_register(tokens[2]);
            int offset = parse_number(tokens[3]);
            uint16_t operand = ((rx & 1) << 10) | ((ry & 1) << 9) | 0x100 | (offset & 0xFF);
            emit_instruction(encode_instruction(0x13, operand));
        }
    }
    else if (strcmp(upper_mnemonic, "SDB") == 0) {
        if (token_count >= 2) {
            int value = parse_number(tokens[1]);
            emit_instruction(encode_instruction(0x14, value & 0xFF));
        }
    }
    else if (strcmp(upper_mnemonic, "JMS") == 0) {
        if (token_count >= 2) {
            int addr = parse_number(tokens[1]);
            emit_instruction(encode_instruction(0x15, addr & 0xFF));
        }
    }
    else if (strcmp(upper_mnemonic, "JDP") == 0) {
        emit_instruction(encode_instruction(0x15, 0x400));
    }
    else if (strcmp(upper_mnemonic, "JSR") == 0) {
        if (token_count >= 2) {
            int addr = parse_number(tokens[1]);
            emit_instruction(encode_instruction(0x15, 0x200 | (addr & 0xFF)));
        }
    }
    else if (strcmp(upper_mnemonic, "JDPS") == 0) {
        emit_instruction(encode_instruction(0x15, 0x600));
    }
    else if (strcmp(upper_mnemonic, "INC") == 0 || strcmp(upper_mnemonic, "DEC") == 0) {
        if (token_count >= 2) {
            int dec = (strcmp(upper_mnemonic, "DEC") == 0) ? 1 : 0;
            int target;
            if (strcmp(tokens[1], "X") == 0 || strcmp(tokens[1], "x") == 0) target = 0;
            else if (strcmp(tokens[1], "Y") == 0 || strcmp(tokens[1], "y") == 0) target = 1;
            else if (strcmp(tokens[1], "DP") == 0 || strcmp(tokens[1], "dp") == 0) target = 2;
            else if (strcmp(tokens[1], "SP") == 0 || strcmp(tokens[1], "sp") == 0) target = 3;
            else target = parse_number(tokens[1]) & 0x03;
            emit_instruction(encode_instruction(0x16, (dec << 10) | ((target & 0x03) << 8)));
        }
    }
    else if (strcmp(upper_mnemonic, "MOV") == 0) {
        if (token_count >= 3) {
            int src = parse_register(tokens[1]);
            int dst = parse_register(tokens[2]);
            emit_instruction(encode_instruction(0x18, ((src&1)<<10) | ((dst&1)<<9)));
        }
    }
    else if (strcmp(upper_mnemonic, "EXC") == 0) {
        emit_instruction(encode_instruction(0x18, 0x100));
    }
    else if (strcmp(upper_mnemonic, "XOR") == 0) {
        if (token_count >= 4) {
            int rx  = parse_register(tokens[1]);
            int ry  = parse_register(tokens[2]);
            char first = tokens[3][0];
            if (first == '$' || first == '0' || (first >= '1' && first <= '9')) {
                int offset = parse_number(tokens[3]);
                emit_instruction(encode_instruction(0x1B, ((rx&1)<<10)|((ry&1)<<9)|0x100|(offset&0xFF)));
            } else {
                int rz = parse_register(tokens[3]);
                emit_instruction(encode_instruction(0x1B, ((rx&1)<<10)|((ry&1)<<9)|((rz&1)<<7)));
            }
        }
    }
    else if (strcmp(upper_mnemonic, "CRB") == 0) {
        if (token_count >= 4) {
            int rx  = parse_register(tokens[1]);  /* input */
            int ry  = parse_register(tokens[2]);  /* shift amount */
            char first = tokens[3][0];
            if (first == '$' || first == '0' || (first >= '1' && first <= '9')) {
                /* crb X, Y, $ZZ — store to dp:ZZ */
                int offset = parse_number(tokens[3]);
                emit_instruction(encode_instruction(0x1A, ((rx&1)<<10)|((ry&1)<<9)|0x100|(offset&0xFF)));
            } else {
                /* crb X, Y, Z — output to register */
                int rz = parse_register(tokens[3]);
                emit_instruction(encode_instruction(0x1A, ((rx&1)<<10)|((ry&1)<<9)|((rz&1)<<7)));
            }
        }
    }
    else if (strcmp(upper_mnemonic, "HLT") == 0) {
        /* Opcodes 0x1C-0x1F are reserved; the interpreter's default case
           treats any of them as an unknown opcode and halts. JMP 0 does NOT
           halt here: JMP is a *relative* jump and pc is already past this
           instruction by the time it executes (fetch does pc+=2 first), so
           offset 0 just falls through to the next instruction. */
        emit_instruction(encode_instruction(0x1C, 0));
    }
    else {
        char msg[128];
        sprintf(msg, "Unknown instruction: %s", mnemonic);
        error(msg, line_num);
    }
}

int main(int argc, char* argv[]) {
    const char* input_file;
    const char* output_file;
    char default_output[256];
    char* dot;
    FILE* fp;
    FILE* out;
    char line[512];
    int line_num;
    int i;
    uint16_t inst;
    uint8_t bytes[2];

    /* Allocate instruction buffer (reduced for 16-bit DOS) */
    instructions = (uint16_t*)malloc(16384 * sizeof(uint16_t));
    if (!instructions) {
        fprintf(stderr, "Out of memory\n");
        return 1;
    }

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input.asm> [output.bin]\n", argv[0]);
        free(instructions);
        return 1;
    }

    input_file = argv[1];
    output_file = (argc >= 3) ? argv[2] : NULL;

    /* Generate output filename if not provided */
    if (!output_file) {
        strncpy(default_output, input_file, 250);
        dot = strrchr(default_output, '.');
        if (dot) *dot = '\0';
        strcat(default_output, ".bin");
        output_file = default_output;
    }

    /* First pass: collect labels */
    fp = fopen(input_file, "r");
    if (!fp) {
        fprintf(stderr, "Cannot open input file: %s\n", input_file);
        free(instructions);
        return 1;
    }

    line_num = 0;

    while (fgets(line, sizeof(line), fp)) {
        line_num++;
        assemble_line(line, line_num);
    }

    fclose(fp);

    /* Write output */
    out = fopen(output_file, "wb");
    if (!out) {
        fprintf(stderr, "Cannot open output file: %s\n", output_file);
        free(instructions);
        return 1;
    }

    /* Write instructions as 16-bit big-endian */
    for (i = 0; i < instruction_count; i++) {
        inst = instructions[i];
        bytes[0] = (inst >> 8) & 0xFF;
        bytes[1] = inst & 0xFF;
        fwrite(bytes, 1, 2, out);
    }

    fclose(out);

    printf("Assembled %ld instructions (%ld bytes) to %s\n",
           instruction_count, instruction_count * 2, output_file);
    printf("Labels defined: %d\n", label_count);

    free(instructions);
    return 0;
}

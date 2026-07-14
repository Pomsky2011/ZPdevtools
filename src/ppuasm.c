/*
 * ZeroPoint PPU Assembler (ppuasm)
 * Assembles PPU microcode assembly into binary format
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 256
#define MAX_LABELS 512
#define MAX_INSTRUCTIONS 65536
#define MAX_LABEL_LENGTH 64
#define MAX_ALIASES 128
#define MAX_CONSTANTS 128
#define MAX_EXPANDED_LINES 65536

/* Instruction structure */
typedef struct {
    int line_num;
    char mnemonic[16];
    char operand1[64];
    char operand2[64];
    char operand3[64];
    int operand_count;
} Instruction;

/* Label structure */
typedef struct {
    char name[MAX_LABEL_LENGTH];
    int address;
} Label;

/* Alias structure */
typedef struct {
    char from[MAX_LABEL_LENGTH];
    char to[MAX_LABEL_LENGTH];
} Alias;

/* Constant structure (CONST NAME VALUE - resolved within expressions) */
typedef struct {
    char name[MAX_LABEL_LENGTH];
    int value;
} Constant;

/* Global tables - using pointers for malloc */
static Instruction* instructions = NULL;
static int instruction_count = 0;

static Label* labels = NULL;
static int label_count = 0;

static Alias* aliases = NULL;
static int alias_count = 0;

static Constant* constants = NULL;
static int constant_count = 0;

static char** expanded_lines = NULL;
static int expanded_line_count = 0;

static unsigned char* output_buffer = NULL;
static int output_size = 0;

/* Target register state tracking (for SETBYTE with labels) */
static int target_reg_byte_sel[4] = {0, 0, 0, 0};  /* 0=LSB, 1=MSB */

/* Opcode tables */
static const struct {
    const char *name;
    int opcode;
} basic_opcodes[] = {
    {"DEFCALL",    0x0},
    {"ENDDEFCALL", 0x1},
    {"SWAPREG",    0x2},
    {"CLR",        0x3},
    {"CMP",        0x4},
    {"CLRF",       0x5},
    {"JMZ",        0x6},
    {"JMG",        0x6},
    {"JNZ",        0x7},
    {"JNG",        0x7},
    {"JML",        0x7},
    {"INC",        0x8},
    {"DEC",        0x9},
    {"ADD",        0xA},
    {"SUB",        0xB},
    {"MUL",        0xC},
    {"INTDIV",     0xD},
    {NULL, 0}
};

static const struct {
    const char *name;
    int subop;
} preset_e_opcodes[] = {
    {"TARREG",     0x0},
    {"SETBYTE",    0x1},
    {"BUILD",      0x2},
    {"CPREG",      0x3},
    {NULL, 0}
};

static const struct {
    const char *name;
    int subop;
} preset_f_opcodes[] = {
    {"SETPOS",      0x0},
    {"SETTILE",     0x1},
    {"SETDP",       0x2},
    {"MOVDP",       0x3},
    {"SETRENDMOD",  0x4},
    {"PALETTE16",   0x5},
    {"PALETTE256",  0x6},
    {"JMR",         0x7},
    {"MOV",         0x8},
    {"SETREGBANK",  0x9},
    {"CLRTILE",     0xA},
    {"CLRPALETTE",  0xB},
    {"TILEDRAW",    0xC},
    {"CALL",        0xE},
    {"GBLS",        0xF},
    {NULL, 0}
};

/* Forward declarations */
static int parse_immediate(const char *str, int line_num);
static int parse_register(const char *str, int line_num);
static int eval_expr(const char *str, int line_num);

/* Error reporting */
static void error(int line_num, const char *message) {
    fprintf(stderr, "Error on line %d: %s\n", line_num, message);
    exit(1);
}

/* String utilities */
static void trim(char *str) {
    char *start = str;
    char *end;

    while (isspace(*start)) start++;

    if (*start == 0) {
        *str = 0;
        return;
    }

    end = start + strlen(start) - 1;
    while (end > start && isspace(*end)) end--;

    *(end + 1) = 0;
    memmove(str, start, strlen(start) + 1);
}

static void to_upper(char *str) {
    while (*str) {
        *str = toupper(*str);
        str++;
    }
}

/* Apply aliases to a string */
static void apply_aliases(char *str) {
    int i;
    for (i = 0; i < alias_count; i++) {
        if (strcmp(str, aliases[i].from) == 0) {
            strcpy(str, aliases[i].to);
            return;
        }
    }
}

/* Parse register name/number */
static int parse_register(const char *str, int line_num) {
    char temp[64];
    int reg_num;

    strcpy(temp, str);
    to_upper(temp);

    /* Special registers */
    if (strcmp(temp, "PC") == 0) return 62;
    if (strcmp(temp, "DP") == 0) return 63;
    if (strcmp(temp, "SP") == 0) return 61;

    /* R0-R63 format */
    if (temp[0] == 'R') {
        reg_num = atoi(temp + 1);
        if (reg_num >= 0 && reg_num <= 63) {
            return reg_num;
        }
    }

    /* Plain number */
    if (sscanf(temp, "%i", &reg_num) == 1) {
        if (reg_num >= 0 && reg_num <= 63) {
            return reg_num;
        }
    }

    error(line_num, "Invalid register");
    return 0;
}

/* Parse target register (0-3) */
static int parse_target_register(const char *str, int line_num) {
    int value = parse_immediate(str, line_num);
    if (value < 0 || value > 3) {
        error(line_num, "Target register must be 0-3");
    }
    return value;
}

/* Parse byte selector (LSB or MSB) */
static int parse_byte_selector(const char *str, int line_num) {
    char temp[64];
    int value;

    strcpy(temp, str);
    to_upper(temp);

    if (strcmp(temp, "LSB") == 0) return 0;
    if (strcmp(temp, "MSB") == 0) return 1;

    /* Try as number */
    value = parse_immediate(str, line_num);
    if (value == 0 || value == 1) {
        return value;
    }

    error(line_num, "Byte selector must be LSB/MSB or 0/1");
    return 0;
}

/* Parse immediate value (supports CONST names and +,-,*,/ expressions) */
static int parse_immediate(const char *str, int line_num) {
    return eval_expr(str, line_num);
}

/* Find label address */
static int find_label(const char *name, int line_num) {
    int i;
    for (i = 0; i < label_count; i++) {
        if (strcmp(labels[i].name, name) == 0) {
            return labels[i].address;
        }
    }

    error(line_num, "Undefined label");
    return 0;
}

/* Look up a CONST by exact name; returns 1 and sets *out if found */
static int find_constant(const char *name, int *out) {
    int i;
    for (i = 0; i < constant_count; i++) {
        if (strcmp(constants[i].name, name) == 0) {
            *out = constants[i].value;
            return 1;
        }
    }
    return 0;
}

/* Extract the leading identifier of str (up to the first operator/paren) */
static void leading_identifier(const char *str, char *out, size_t out_size) {
    size_t i = 0;
    while (str[i] && (isalnum((unsigned char)str[i]) || str[i] == '_' || str[i] == '@')
           && i < out_size - 1) {
        out[i] = str[i];
        i++;
    }
    out[i] = 0;
}

/* Check if string is a label reference (a CONST of the same name is not a
 * label - constants are always plain values, never code addresses). */
static int is_label(const char *str) {
    char ident[MAX_LABEL_LENGTH];
    int dummy;

    if (!(isalpha((unsigned char)str[0]) || str[0] == '_' || str[0] == '@')) {
        return 0;
    }
    leading_identifier(str, ident, sizeof(ident));
    if (find_constant(ident, &dummy)) {
        return 0;
    }
    return 1;
}

/* ---- Expression evaluator: +,-,*,/ with CONST names, labels, and 0x/$/decimal
 * numeric literals. Constants must be defined earlier in the file (CONST
 * lines are resolved in file order during shorthand expansion); labels are
 * resolved using whatever labels[] state exists at call time (empty during
 * CONST evaluation itself, fully populated by the time instructions are
 * assembled). */
static const char *expr_str;
static int expr_pos;
static int expr_line;

static void expr_skip_ws(void) {
    while (expr_str[expr_pos] == ' ' || expr_str[expr_pos] == '\t') expr_pos++;
}

static int expr_add(void);

static int expr_atom(void) {
    char tok[MAX_LABEL_LENGTH];
    int i = 0, value, negate = 0;

    expr_skip_ws();
    if (expr_str[expr_pos] == '-') {
        negate = 1;
        expr_pos++;
        expr_skip_ws();
    }

    if (expr_str[expr_pos] == '(') {
        expr_pos++;
        value = expr_add();
        expr_skip_ws();
        if (expr_str[expr_pos] == ')') {
            expr_pos++;
        } else {
            error(expr_line, "Expected ')' in expression");
        }
        return negate ? -value : value;
    }

    if (expr_str[expr_pos] == '$') {
        tok[i++] = '0';
        tok[i++] = 'x';
        expr_pos++;
    }
    while ((isalnum((unsigned char)expr_str[expr_pos]) || expr_str[expr_pos] == '_')
           && i < (int)sizeof(tok) - 1) {
        tok[i++] = expr_str[expr_pos];
        expr_pos++;
    }
    tok[i] = 0;

    if (tok[0] == 0) {
        error(expr_line, "Expected value in expression");
        return 0;
    }

    if (isdigit((unsigned char)tok[0]) || (tok[0] == '0' && tok[1] == 'x')) {
        if (sscanf(tok, "%i", &value) != 1) {
            error(expr_line, "Invalid numeric literal in expression");
        }
    } else if (find_constant(tok, &value)) {
        /* resolved via constants table */
    } else {
        value = find_label(tok, expr_line);  /* exits with error if undefined */
    }

    return negate ? -value : value;
}

static int expr_mul(void) {
    int value = expr_atom();
    for (;;) {
        expr_skip_ws();
        if (expr_str[expr_pos] == '*') {
            expr_pos++;
            value *= expr_atom();
        } else if (expr_str[expr_pos] == '/') {
            int rhs;
            expr_pos++;
            rhs = expr_atom();
            if (rhs == 0) error(expr_line, "Division by zero in expression");
            value /= rhs;
        } else {
            break;
        }
    }
    return value;
}

static int expr_add(void) {
    int value = expr_mul();
    for (;;) {
        expr_skip_ws();
        if (expr_str[expr_pos] == '+') {
            expr_pos++;
            value += expr_mul();
        } else if (expr_str[expr_pos] == '-') {
            expr_pos++;
            value -= expr_mul();
        } else {
            break;
        }
    }
    return value;
}

static int eval_expr(const char *str, int line_num) {
    int value;
    expr_str = str;
    expr_pos = 0;
    expr_line = line_num;
    value = expr_add();
    expr_skip_ws();
    if (expr_str[expr_pos] != 0) {
        error(line_num, "Unexpected trailing characters in expression");
    }
    return value;
}

/* Get operand value (label or immediate) */
static int get_operand_value(const char *str, int line_num) {
    if (is_label(str)) {
        return find_label(str, line_num);
    }
    return parse_immediate(str, line_num);
}

/* Look up basic opcode */
static int lookup_opcode(const char *mnemonic) {
    int i;
    for (i = 0; basic_opcodes[i].name != NULL; i++) {
        if (strcmp(basic_opcodes[i].name, mnemonic) == 0) {
            return basic_opcodes[i].opcode;
        }
    }
    return -1;
}

/* Look up preset E sub-opcode */
static int lookup_preset_e(const char *mnemonic) {
    int i;
    for (i = 0; preset_e_opcodes[i].name != NULL; i++) {
        if (strcmp(preset_e_opcodes[i].name, mnemonic) == 0) {
            return preset_e_opcodes[i].subop;
        }
    }
    return -1;
}

/* Look up preset F sub-opcode */
static int lookup_preset_f(const char *mnemonic) {
    int i;
    for (i = 0; preset_f_opcodes[i].name != NULL; i++) {
        if (strcmp(preset_f_opcodes[i].name, mnemonic) == 0) {
            return preset_f_opcodes[i].subop;
        }
    }
    return -1;
}

/* Assemble preset E instruction */
static unsigned short assemble_preset_e(Instruction *inst) {
    int subop = lookup_preset_e(inst->mnemonic);
    int suboperand = 0;
    int target_reg, byte_sel, source_reg, imm, dest_reg, value;

    if (strcmp(inst->mnemonic, "TARREG") == 0) {
        /* TARREG T, Y, X - encoding: 00 TT Y 0 XXXXXX */
        target_reg = parse_target_register(inst->operand1, inst->line_num);
        byte_sel = parse_byte_selector(inst->operand2, inst->line_num);
        source_reg = parse_register(inst->operand3, inst->line_num);

        /* Track target register byte selection for SETBYTE */
        target_reg_byte_sel[target_reg] = byte_sel;

        suboperand = (target_reg << 8) | (byte_sel << 7) | source_reg;
    }
    else if (strcmp(inst->mnemonic, "SETBYTE") == 0) {
        /* SETBYTE T, 0xXX - encoding: 01 TT XXXXXXXX */
        target_reg = parse_target_register(inst->operand1, inst->line_num);
        value = get_operand_value(inst->operand2, inst->line_num);

        /* Handle labels (addresses) vs literals (8-bit values) */
        if (is_label(inst->operand2)) {
            /* Label reference - extract appropriate byte based on target */
            if (target_reg_byte_sel[target_reg] == 1) {
                /* MSB - use high byte */
                imm = (value >> 8) & 0xFF;
            } else {
                /* LSB - use low byte */
                imm = value & 0xFF;
            }
        } else {
            /* 8-bit literal - use as-is */
            imm = value & 0xFF;
        }

        suboperand = (target_reg << 8) | imm;
    }
    else if (strcmp(inst->mnemonic, "BUILD") == 0) {
        /* BUILD T1, T2, X - encoding: 10 TT TT XXXXXX */
        int t1 = parse_target_register(inst->operand1, inst->line_num);
        int t2 = parse_target_register(inst->operand2, inst->line_num);
        dest_reg = parse_register(inst->operand3, inst->line_num);
        suboperand = (t1 << 8) | (t2 << 6) | (dest_reg & 0x3F);
    }
    else if (strcmp(inst->mnemonic, "CPREG") == 0) {
        /* CPREG X, Y - encoding: 11 00 XXXX YYYY */
        int reg_x = parse_register(inst->operand1, inst->line_num) & 0x0F;
        int reg_y = parse_register(inst->operand2, inst->line_num) & 0x0F;
        suboperand = (reg_x << 4) | reg_y;
    }

    return (0xE << 12) | (subop << 10) | (suboperand & 0x3FF);
}

/* Assemble preset F instruction */
static unsigned short assemble_preset_f(Instruction *inst) {
    int subop = lookup_preset_f(inst->mnemonic);
    int suboperand = 0;
    int reg_x, reg_y, mode, bank_x, bank_y;

    if (strcmp(inst->mnemonic, "SETPOS") == 0) {
        reg_x = parse_register(inst->operand1, inst->line_num) & 0x0F;
        reg_y = parse_register(inst->operand2, inst->line_num) & 0x0F;
        suboperand = (reg_x << 4) | reg_y;
    }
    else if (strcmp(inst->mnemonic, "SETTILE") == 0) {
        /* SETTILE X, Y - encoding: 0001 XXXXXX YY */
        reg_x = parse_register(inst->operand1, inst->line_num);
        mode = get_operand_value(inst->operand2, inst->line_num) & 0x03;
        suboperand = (reg_x << 2) | mode;
    }
    else if (strcmp(inst->mnemonic, "SETDP") == 0) {
        reg_x = parse_register(inst->operand1, inst->line_num);
        suboperand = (reg_x << 2);
    }
    else if (strcmp(inst->mnemonic, "MOVDP") == 0) {
        reg_x = parse_register(inst->operand1, inst->line_num);
        suboperand = (reg_x << 2);
    }
    else if (strcmp(inst->mnemonic, "SETRENDMOD") == 0) {
        mode = get_operand_value(inst->operand1, inst->line_num) & 0x01;
        suboperand = (mode << 7);
    }
    else if (strcmp(inst->mnemonic, "MOV") == 0) {
        reg_x = parse_register(inst->operand1, inst->line_num);
        suboperand = (reg_x << 2);
    }
    else if (strcmp(inst->mnemonic, "SETREGBANK") == 0) {
        bank_x = get_operand_value(inst->operand1, inst->line_num) & 0x03;
        bank_y = get_operand_value(inst->operand2, inst->line_num) & 0x03;
        suboperand = (bank_x << 4) | (bank_y << 2);
    }
    else if (strcmp(inst->mnemonic, "CALL") == 0) {
        suboperand = get_operand_value(inst->operand1, inst->line_num) & 0xFF;
    }
    else if (strcmp(inst->mnemonic, "GBLS") == 0) {
        reg_x = parse_register(inst->operand1, inst->line_num);
        suboperand = (reg_x << 2);
    }

    return (0xF << 12) | (subop << 8) | (suboperand & 0xFF);
}

/* Assemble single instruction */
static unsigned short assemble_instruction(Instruction *inst) {
    int opcode = lookup_opcode(inst->mnemonic);
    int operand = 0;
    int reg_x, reg_y;

    /* Check if it's a preset E instruction */
    if (opcode == -1) {
        if (lookup_preset_e(inst->mnemonic) != -1) {
            return assemble_preset_e(inst);
        }
        /* Check if it's a preset F instruction */
        if (lookup_preset_f(inst->mnemonic) != -1) {
            return assemble_preset_f(inst);
        }
        error(inst->line_num, "Unknown instruction");
    }

    /* Assemble based on mnemonic */
    if (strcmp(inst->mnemonic, "DEFCALL") == 0) {
        /* DEFCALL X, Y - encoding: 0000 XXXXXX YYYYYY */
        reg_x = parse_register(inst->operand1, inst->line_num);
        reg_y = parse_register(inst->operand2, inst->line_num);
        operand = (reg_x << 6) | reg_y;
    }
    else if (strcmp(inst->mnemonic, "ENDDEFCALL") == 0) {
        /* ENDDEFCALL X - encoding: 0001 000000 XXXXXX */
        operand = parse_register(inst->operand1, inst->line_num) & 0x3F;
    }
    else if (strcmp(inst->mnemonic, "SWAPREG") == 0) {
        reg_x = parse_register(inst->operand1, inst->line_num);
        reg_y = parse_register(inst->operand2, inst->line_num);
        operand = (reg_x << 6) | reg_y;
    }
    else if (strcmp(inst->mnemonic, "CLR") == 0) {
        operand = parse_register(inst->operand1, inst->line_num);
    }
    else if (strcmp(inst->mnemonic, "CMP") == 0) {
        reg_x = parse_register(inst->operand1, inst->line_num);
        reg_y = parse_register(inst->operand2, inst->line_num);
        operand = (reg_x << 6) | reg_y;
    }
    else if (strcmp(inst->mnemonic, "JMG") == 0 || strcmp(inst->mnemonic, "JNG") == 0 ||
             strcmp(inst->mnemonic, "JML") == 0) {
        operand = 0x800;  /* Bit 11 = 1 */
    }
    else if (strcmp(inst->mnemonic, "INC") == 0 || strcmp(inst->mnemonic, "DEC") == 0) {
        operand = parse_register(inst->operand1, inst->line_num);
    }
    else if (strcmp(inst->mnemonic, "ADD") == 0 || strcmp(inst->mnemonic, "SUB") == 0 ||
             strcmp(inst->mnemonic, "MUL") == 0 || strcmp(inst->mnemonic, "INTDIV") == 0) {
        reg_x = parse_register(inst->operand1, inst->line_num);
        reg_y = parse_register(inst->operand2, inst->line_num);
        operand = (reg_x << 6) | reg_y;
    }

    return (opcode << 12) | (operand & 0xFFF);
}

/* Expand shorthands before assembly */
static void expand_shorthands(FILE *fp) {
    char line[MAX_LINE_LENGTH];
    char *p;
    int line_num;
    int auto_label_counter;
    char left[MAX_LABEL_LENGTH], right[MAX_LABEL_LENGTH];
    char *left_ptr;
    char temp_line[MAX_LINE_LENGTH];
    char *label_end;
    char mnemonic[64], op1[64], op2[64];
    char *token;

    line_num = 0;
    auto_label_counter = 0;

    while (fgets(line, sizeof(line), fp)) {
        line_num++;

        /* Remove comments */
        p = strchr(line, ';');
        if (p) *p = 0;

        trim(line);
        if (strlen(line) == 0) {
            strcpy(expanded_lines[expanded_line_count++], "");
            continue;
        }

        /* Check for alias definition */
        p = strstr(line, "=");
        if (p) {
            *p = 0;
            strcpy(left, line);
            strcpy(right, p + 1);
            trim(left);
            trim(right);

            /* Remove $ prefix if present from left side */
            left_ptr = left;
            if (left[0] == '$') left_ptr = left + 1;

            /* Alias syntax: LEFT = RIGHT means "replace RIGHT with LEFT" */
            strcpy(aliases[alias_count].from, right);  /* what to replace */
            strcpy(aliases[alias_count].to, left_ptr); /* replace with this */
            alias_count++;
            strcpy(expanded_lines[expanded_line_count++], "");
            continue;
        }

        /* Parse instruction mnemonic */
        strcpy(temp_line, line);

        /* Handle labels */
        label_end = strchr(temp_line, ':');
        if (label_end) {
            strcpy(expanded_lines[expanded_line_count++], line);
            continue;
        }

        /* Get mnemonic and operands */
        mnemonic[0] = 0;
        op1[0] = 0;
        op2[0] = 0;
        token = strtok(temp_line, " \t,");
        if (token) {
            strcpy(mnemonic, token);
            to_upper(mnemonic);

            token = strtok(NULL, " \t,");
            if (token) {
                strcpy(op1, token);
                trim(op1);

                token = strtok(NULL, " \t,");
                if (token) {
                    strcpy(op2, token);
                    trim(op2);
                }
            }
        }

        /* CONST NAME VALUE - define a named constant usable in expressions
         * (SETBYTE 2, NAME, SETBYTE 2, NAME/2, etc). Must be defined before
         * use; VALUE may itself reference earlier constants. */
        if (strcmp(mnemonic, "CONST") == 0) {
            if (!*op1 || !*op2) {
                error(line_num, "CONST requires a name and a value");
            }
            if (constant_count >= MAX_CONSTANTS) {
                error(line_num, "Too many CONST definitions");
            }
            strcpy(constants[constant_count].name, op1);
            constants[constant_count].value = eval_expr(op2, line_num);
            constant_count++;
            strcpy(expanded_lines[expanded_line_count++], "");
            continue;
        }

        /* Expand HLT shorthand */
        if (strcmp(mnemonic, "HLT") == 0) {
            char label[64];
            sprintf(label, "@halt_%d", auto_label_counter++);
            sprintf(expanded_lines[expanded_line_count++], "%s:", label);
            sprintf(expanded_lines[expanded_line_count++], "TARREG 0, LSB, PC");
            sprintf(expanded_lines[expanded_line_count++], "TARREG 1, MSB, PC");
            sprintf(expanded_lines[expanded_line_count++], "SETBYTE 0, %s", label);
            sprintf(expanded_lines[expanded_line_count++], "SETBYTE 1, %s", label);
            sprintf(expanded_lines[expanded_line_count++], "JMR");
            continue;
        }

        /* Expand PUSH shorthand */
        if (strcmp(mnemonic, "PUSH") == 0) {
            sprintf(expanded_lines[expanded_line_count++], "SETDP SP");
            sprintf(expanded_lines[expanded_line_count++], "INC SP");
            sprintf(expanded_lines[expanded_line_count++], "INC SP");
            sprintf(expanded_lines[expanded_line_count++], "MOVDP %s", op1);
            continue;
        }

        /* Expand POP shorthand */
        if (strcmp(mnemonic, "POP") == 0) {
            sprintf(expanded_lines[expanded_line_count++], "SETDP SP");
            sprintf(expanded_lines[expanded_line_count++], "DEC SP");
            sprintf(expanded_lines[expanded_line_count++], "DEC SP");
            sprintf(expanded_lines[expanded_line_count++], "MOV %s", op1);
            continue;
        }

        /* Expand RET shorthand */
        if (strcmp(mnemonic, "RET") == 0) {
            sprintf(expanded_lines[expanded_line_count++], "SETDP SP");
            sprintf(expanded_lines[expanded_line_count++], "DEC SP");
            sprintf(expanded_lines[expanded_line_count++], "DEC SP");
            sprintf(expanded_lines[expanded_line_count++], "MOV PC");
            sprintf(expanded_lines[expanded_line_count++], "JMR");
            continue;
        }

        /* Expand JSR shorthand */
        if (strcmp(mnemonic, "JSR") == 0) {
            sprintf(expanded_lines[expanded_line_count++], "SETDP SP");
            sprintf(expanded_lines[expanded_line_count++], "INC SP");
            sprintf(expanded_lines[expanded_line_count++], "INC SP");
            sprintf(expanded_lines[expanded_line_count++], "MOVDP PC");
            sprintf(expanded_lines[expanded_line_count++], "TARREG 0, LSB, PC");
            sprintf(expanded_lines[expanded_line_count++], "TARREG 1, MSB, PC");
            sprintf(expanded_lines[expanded_line_count++], "SETBYTE 0, %s", op1);
            sprintf(expanded_lines[expanded_line_count++], "SETBYTE 1, %s", op1);
            sprintf(expanded_lines[expanded_line_count++], "JMR");
            continue;
        }

        /* Expand label-based jumps */
        if ((strcmp(mnemonic, "JMR") == 0 || strcmp(mnemonic, "JMZ") == 0 ||
             strcmp(mnemonic, "JNZ") == 0 || strcmp(mnemonic, "JMG") == 0 ||
             strcmp(mnemonic, "JNG") == 0 || strcmp(mnemonic, "JML") == 0) && strlen(op1) > 0) {
            sprintf(expanded_lines[expanded_line_count++], "TARREG 0, LSB, PC");
            sprintf(expanded_lines[expanded_line_count++], "TARREG 1, MSB, PC");
            sprintf(expanded_lines[expanded_line_count++], "SETBYTE 0, %s", op1);
            sprintf(expanded_lines[expanded_line_count++], "SETBYTE 1, %s", op1);
            sprintf(expanded_lines[expanded_line_count++], "%s", mnemonic);
            continue;
        }

        /* Default: copy line as-is */
        strcpy(expanded_lines[expanded_line_count++], line);
    }
}

/* First pass: collect labels and instructions */
static void first_pass(void) {
    int line_num = 0;
    int current_addr = 0;
    int i;

    for (i = 0; i < expanded_line_count; i++) {
        char line[MAX_LINE_LENGTH];
        char *p, *label_end;

        strcpy(line, expanded_lines[i]);
        line_num++;

        trim(line);
        if (strlen(line) == 0) continue;

        /* Check for label */
        label_end = strchr(line, ':');
        if (label_end) {
            char label_name[MAX_LABEL_LENGTH];
            int label_len = label_end - line;

            strncpy(label_name, line, label_len);
            label_name[label_len] = 0;
            trim(label_name);

            /* Add label */
            if (strlen(label_name) > 0) {
                strcpy(labels[label_count].name, label_name);
                labels[label_count].address = current_addr;
                label_count++;
            }

            /* Get rest of line */
            strcpy(line, label_end + 1);
            trim(line);
            if (strlen(line) == 0) continue;
        }

        /* Parse instruction */
        {
            char *token;
            Instruction *inst = &instructions[instruction_count];

            inst->line_num = line_num;
            inst->operand_count = 0;
            inst->operand1[0] = 0;
            inst->operand2[0] = 0;
            inst->operand3[0] = 0;

            /* Get mnemonic */
            token = strtok(line, " \t,");
            if (token) {
                strcpy(inst->mnemonic, token);
                to_upper(inst->mnemonic);

                /* Get operands */
                token = strtok(NULL, " \t,");
                if (token) {
                    strcpy(inst->operand1, token);
                    trim(inst->operand1);
                    apply_aliases(inst->operand1);
                    inst->operand_count = 1;

                    token = strtok(NULL, " \t,");
                    if (token) {
                        strcpy(inst->operand2, token);
                        trim(inst->operand2);
                        apply_aliases(inst->operand2);
                        inst->operand_count = 2;

                        token = strtok(NULL, " \t,");
                        if (token) {
                            strcpy(inst->operand3, token);
                            trim(inst->operand3);
                            apply_aliases(inst->operand3);
                            inst->operand_count = 3;
                        }
                    }
                }

                instruction_count++;
                current_addr += 2;
            }
        }
    }
}

/* Second pass: assemble instructions */
static void second_pass(void) {
    int i;

    for (i = 0; i < instruction_count; i++) {
        unsigned short word = assemble_instruction(&instructions[i]);

        /* Output as big-endian */
        output_buffer[output_size++] = (word >> 8) & 0xFF;
        output_buffer[output_size++] = word & 0xFF;
    }
}

/* Main */
int main(int argc, char *argv[]) {
    FILE *input_fp, *output_fp;
    char *input_file, *output_file;
    char default_output[256];
    int i;

    /* Allocate memory for arrays */
    instructions = (Instruction*)malloc(MAX_INSTRUCTIONS * sizeof(Instruction));
    labels = (Label*)malloc(MAX_LABELS * sizeof(Label));
    aliases = (Alias*)malloc(MAX_ALIASES * sizeof(Alias));
    constants = (Constant*)malloc(MAX_CONSTANTS * sizeof(Constant));
    output_buffer = (unsigned char*)malloc(65536);
    expanded_lines = (char**)malloc(MAX_EXPANDED_LINES * sizeof(char*));

    if (!instructions || !labels || !aliases || !constants || !output_buffer || !expanded_lines) {
        fprintf(stderr, "Error: Out of memory\n");
        return 1;
    }

    for (i = 0; i < MAX_EXPANDED_LINES; i++) {
        expanded_lines[i] = (char*)malloc(MAX_LINE_LENGTH);
        if (!expanded_lines[i]) {
            fprintf(stderr, "Error: Out of memory\n");
            return 1;
        }
    }

    if (argc < 2) {
        fprintf(stderr, "Usage: ppuasm <input.asm> [-o output.bin]\n");
        fprintf(stderr, "  Assembles ZeroPoint PPU microcode\n");
        free(instructions);
        free(labels);
        free(aliases);
        free(output_buffer);
        for (i = 0; i < MAX_EXPANDED_LINES; i++) free(expanded_lines[i]);
        free(expanded_lines);
        return 1;
    }

    input_file = argv[1];

    /* Determine output file */
    if (argc > 3 && strcmp(argv[2], "-o") == 0) {
        output_file = argv[3];
    } else {
        char *dot;
        strcpy(default_output, input_file);
        dot = strrchr(default_output, '.');
        if (dot) *dot = 0;
        strcat(default_output, ".bin");
        output_file = default_output;
    }

    /* Open input */
    input_fp = fopen(input_file, "r");
    if (!input_fp) {
        fprintf(stderr, "Error: Cannot open input file: %s\n", input_file);
        free(instructions);
        free(labels);
        free(aliases);
        free(output_buffer);
        for (i = 0; i < MAX_EXPANDED_LINES; i++) free(expanded_lines[i]);
        free(expanded_lines);
        return 1;
    }

    /* Expand shorthands */
    expand_shorthands(input_fp);
    fclose(input_fp);

    /* First pass */
    first_pass();

    /* Second pass */
    second_pass();

    /* Write output */
    output_fp = fopen(output_file, "wb");
    if (!output_fp) {
        fprintf(stderr, "Error: Cannot open output file: %s\n", output_file);
        free(instructions);
        free(labels);
        free(aliases);
        free(output_buffer);
        for (i = 0; i < MAX_EXPANDED_LINES; i++) free(expanded_lines[i]);
        free(expanded_lines);
        return 1;
    }

    fwrite(output_buffer, 1, output_size, output_fp);
    fclose(output_fp);

    printf("Assembled %d instructions -> %d bytes\n", instruction_count, output_size);
    printf("Output: %s\n", output_file);

    /* Free allocated memory */
    free(instructions);
    free(labels);
    free(aliases);
    free(output_buffer);
    for (i = 0; i < MAX_EXPANDED_LINES; i++) free(expanded_lines[i]);
    free(expanded_lines);

    return 0;
}

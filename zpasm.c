/*
 * ZeroPoint PPU Assembler (zpasm)
 * Assembles PPU microcode assembly into binary format
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 256
#define MAX_LABELS 1000
#define MAX_INSTRUCTIONS 32768
#define MAX_LABEL_LENGTH 64

/* Instruction structure */
typedef struct {
    int line_num;
    char mnemonic[16];
    char operand1[64];
    char operand2[64];
    int operand_count;
} Instruction;

/* Label structure */
typedef struct {
    char name[MAX_LABEL_LENGTH];
    int address;
} Label;

/* Global tables */
static Instruction instructions[MAX_INSTRUCTIONS];
static int instruction_count = 0;

static Label labels[MAX_LABELS];
static int label_count = 0;

static unsigned char output_buffer[65536];
static int output_size = 0;

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
    {"HALT",       0xE},
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
    {"STRTDEFTILE", 0xC},
    {"ENDDEFTILE",  0xD},
    {"CALL",        0xE},
    {"GBLS",        0xF},
    {NULL, 0}
};

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

/* Parse register name/number */
static int parse_register(const char *str, int line_num) {
    char temp[64];
    int reg_num;

    strcpy(temp, str);
    to_upper(temp);

    /* Special registers */
    if (strcmp(temp, "PC") == 0) return 62;
    if (strcmp(temp, "DP") == 0) return 63;

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

/* Parse immediate value */
static int parse_immediate(const char *str, int line_num) {
    int value;

    if (sscanf(str, "%i", &value) != 1) {
        error(line_num, "Invalid immediate value");
    }

    return value;
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

/* Check if string is a label reference */
static int is_label(const char *str) {
    /* Labels start with letter or underscore */
    if (isalpha(str[0]) || str[0] == '_') {
        return 1;
    }
    return 0;
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
        suboperand = get_operand_value(inst->operand1, inst->line_num) & 0xFF;
    }
    else if (strcmp(inst->mnemonic, "SETDP") == 0) {
        reg_x = parse_register(inst->operand1, inst->line_num);
        suboperand = (reg_x << 2);
    }
    else if (strcmp(inst->mnemonic, "MOVDP") == 0) {
        reg_x = parse_register(inst->operand1, inst->line_num);
        suboperand = (reg_x << 2) | 0x04;
    }
    else if (strcmp(inst->mnemonic, "SETRENDMOD") == 0) {
        mode = get_operand_value(inst->operand1, inst->line_num) & 0x01;
        suboperand = (mode << 7);
    }
    else if (strcmp(inst->mnemonic, "MOV") == 0) {
        reg_x = parse_register(inst->operand1, inst->line_num);
        suboperand = (reg_x << 2) | 0x08;
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

    /* Check if it's a preset F instruction */
    if (opcode == -1) {
        if (lookup_preset_f(inst->mnemonic) != -1) {
            return assemble_preset_f(inst);
        }
        error(inst->line_num, "Unknown instruction");
    }

    /* Assemble based on mnemonic */
    if (strcmp(inst->mnemonic, "DEFCALL") == 0) {
        operand = get_operand_value(inst->operand1, inst->line_num) & 0xFF;
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

/* First pass: collect labels and instructions */
static void first_pass(FILE *fp) {
    char line[MAX_LINE_LENGTH];
    char *p, *label_end;
    int line_num = 0;
    int current_addr = 0;

    while (fgets(line, sizeof(line), fp)) {
        line_num++;

        /* Remove comments */
        p = strchr(line, ';');
        if (p) *p = 0;

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
                    inst->operand_count = 1;

                    token = strtok(NULL, " \t,");
                    if (token) {
                        strcpy(inst->operand2, token);
                        trim(inst->operand2);
                        inst->operand_count = 2;
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

    if (argc < 2) {
        fprintf(stderr, "Usage: zpasm <input.asm> [-o output.bin]\n");
        fprintf(stderr, "  Assembles ZeroPoint PPU microcode\n");
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
        return 1;
    }

    /* First pass */
    first_pass(input_fp);
    fclose(input_fp);

    /* Second pass */
    second_pass();

    /* Write output */
    output_fp = fopen(output_file, "wb");
    if (!output_fp) {
        fprintf(stderr, "Error: Cannot open output file: %s\n", output_file);
        return 1;
    }

    fwrite(output_buffer, 1, output_size, output_fp);
    fclose(output_fp);

    printf("Assembled %d instructions -> %d bytes\n", instruction_count, output_size);
    printf("Output: %s\n", output_file);

    return 0;
}

/*
 * ppuasm - Simple assembler for ZeroPoint PPU
 *
 * Clean rewrite focusing on correctness over features
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#define MAX_LABELS 256
#define MAX_LINE 1024
#define MAX_PASSES 2

/* Label table */
typedef struct {
    char name[64];
    uint16_t address;
} Label;

Label labels[MAX_LABELS];
int label_count = 0;

/* Output buffer */
uint8_t output[65536];
uint16_t output_size = 0;

/* Current pass */
int current_pass = 0;

/* Add label */
int add_label(const char *name, uint16_t address) {
    if (label_count >= MAX_LABELS) {
        fprintf(stderr, "Error: Too many labels (max %d)\n", MAX_LABELS);
        return -1;
    }

    /* Check for duplicates */
    for (int i = 0; i < label_count; i++) {
        if (strcmp(labels[i].name, name) == 0) {
            if (current_pass == 0) {
                /* First pass: update address */
                labels[i].address = address;
                return i;
            } else {
                /* Second pass: verify address matches */
                if (labels[i].address != address) {
                    fprintf(stderr, "Error: Label '%s' has different address in pass 2\n", name);
                    return -1;
                }
                return i;
            }
        }
    }

    /* New label */
    strncpy(labels[label_count].name, name, 63);
    labels[label_count].name[63] = '\0';
    labels[label_count].address = address;
    return label_count++;
}

/* Find label */
int find_label(const char *name, uint16_t *address) {
    for (int i = 0; i < label_count; i++) {
        if (strcmp(labels[i].name, name) == 0) {
            *address = labels[i].address;
            return 0;
        }
    }
    return -1;
}

/* Parse register number */
int parse_register(const char *str, uint8_t *reg) {
    if (str[0] == 'R' || str[0] == 'r') {
        int num = atoi(str + 1);
        if (num >= 0 && num <= 63) {
            *reg = (uint8_t)num;
            return 0;
        }
    } else if (strcmp(str, "PC") == 0 || strcmp(str, "pc") == 0) {
        *reg = 62;
        return 0;
    } else if (strcmp(str, "DP") == 0 || strcmp(str, "dp") == 0) {
        *reg = 63;
        return 0;
    } else if (strcmp(str, "SP") == 0 || strcmp(str, "sp") == 0) {
        *reg = 61;
        return 0;
    } else if (isdigit(str[0])) {
        int num = atoi(str);
        if (num >= 0 && num <= 63) {
            *reg = (uint8_t)num;
            return 0;
        }
    }
    return -1;
}

/* Parse immediate value */
int parse_immediate(const char *str, uint16_t *value) {
    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        /* Hex */
        *value = (uint16_t)strtol(str + 2, NULL, 16);
        return 0;
    } else if (str[0] == '0' && (str[1] == 'b' || str[1] == 'B')) {
        /* Binary */
        *value = (uint16_t)strtol(str + 2, NULL, 2);
        return 0;
    } else if (isdigit(str[0]) || str[0] == '-') {
        /* Decimal */
        *value = (uint16_t)atoi(str);
        return 0;
    } else {
        /* Try as label */
        return find_label(str, value);
    }
}

/* Emit 16-bit instruction (big-endian) */
void emit16(uint16_t value) {
    output[output_size++] = (value >> 8) & 0xFF;  /* High byte */
    output[output_size++] = value & 0xFF;          /* Low byte */
}

/* Trim whitespace */
char *trim(char *str) {
    while (isspace(*str)) str++;
    char *end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) *end-- = '\0';
    return str;
}

/* Split line into tokens */
int tokenize(char *line, char **tokens, int max_tokens) {
    int count = 0;
    char *token = strtok(line, " ,\t");
    while (token && count < max_tokens) {
        tokens[count++] = token;
        token = strtok(NULL, " ,\t");
    }
    return count;
}

/* Assemble one instruction */
int assemble_instruction(char *line, int line_num) {
    char line_copy[MAX_LINE];
    strncpy(line_copy, line, MAX_LINE - 1);
    line_copy[MAX_LINE - 1] = '\0';

    char *tokens[8];
    int token_count = tokenize(line_copy, tokens, 8);

    if (token_count == 0) return 0;

    char *mnemonic = tokens[0];
    uint16_t instr = 0;
    uint8_t reg1, reg2;
    uint16_t imm;

    /* Basic instructions */
    if (strcmp(mnemonic, "CLR") == 0) {
        if (token_count != 2) goto error;
        if (parse_register(tokens[1], &reg1) < 0) goto error;
        instr = (0x3 << 12) | reg1;
        emit16(instr);
    }
    else if (strcmp(mnemonic, "INC") == 0) {
        if (token_count != 2) goto error;
        if (parse_register(tokens[1], &reg1) < 0) goto error;
        instr = (0x8 << 12) | reg1;
        emit16(instr);
    }
    else if (strcmp(mnemonic, "DEC") == 0) {
        if (token_count != 2) goto error;
        if (parse_register(tokens[1], &reg1) < 0) goto error;
        instr = (0x9 << 12) | reg1;
        emit16(instr);
    }
    else if (strcmp(mnemonic, "ADD") == 0) {
        if (token_count != 3) goto error;
        if (parse_register(tokens[1], &reg1) < 0) goto error;
        if (parse_register(tokens[2], &reg2) < 0) goto error;
        instr = (0xA << 12) | (reg1 << 6) | reg2;
        emit16(instr);
    }
    else if (strcmp(mnemonic, "SUB") == 0) {
        if (token_count != 3) goto error;
        if (parse_register(tokens[1], &reg1) < 0) goto error;
        if (parse_register(tokens[2], &reg2) < 0) goto error;
        instr = (0xB << 12) | (reg1 << 6) | reg2;
        emit16(instr);
    }
    else if (strcmp(mnemonic, "MUL") == 0) {
        if (token_count != 3) goto error;
        if (parse_register(tokens[1], &reg1) < 0) goto error;
        if (parse_register(tokens[2], &reg2) < 0) goto error;
        instr = (0xC << 12) | (reg1 << 6) | reg2;
        emit16(instr);
    }
    else if (strcmp(mnemonic, "CMP") == 0) {
        if (token_count != 3) goto error;
        if (parse_register(tokens[1], &reg1) < 0) goto error;
        if (parse_register(tokens[2], &reg2) < 0) goto error;
        instr = (0x4 << 12) | (reg1 << 6) | reg2;
        emit16(instr);
    }
    else if (strcmp(mnemonic, "SWAPREG") == 0) {
        if (token_count != 3) goto error;
        if (parse_register(tokens[1], &reg1) < 0) goto error;
        if (parse_register(tokens[2], &reg2) < 0) goto error;
        instr = (0x2 << 12) | (reg1 << 6) | reg2;
        emit16(instr);
    }
    else if (strcmp(mnemonic, "JMZ") == 0) {
        instr = (0x6 << 12) | 0x000;  /* Bit 11 = 0 for JMZ */
        emit16(instr);
    }
    else if (strcmp(mnemonic, "JMG") == 0) {
        instr = (0x6 << 12) | 0x800;  /* Bit 11 = 1 for JMG */
        emit16(instr);
    }
    else if (strcmp(mnemonic, "JNZ") == 0) {
        instr = (0x7 << 12) | 0x000;  /* Bit 11 = 0 for JNZ */
        emit16(instr);
    }
    else if (strcmp(mnemonic, "JNG") == 0) {
        instr = (0x7 << 12) | 0x800;  /* Bit 11 = 1 for JNG */
        emit16(instr);
    }
    /* Preset E instructions */
    else if (strcmp(mnemonic, "TARREG") == 0) {
        if (token_count != 4) goto error;
        uint8_t target_reg = atoi(tokens[1]);
        uint8_t byte_sel = (strcmp(tokens[2], "MSB") == 0) ? 1 : 0;
        if (parse_register(tokens[3], &reg1) < 0) goto error;

        /* Encoding: bits 11-10=00 (subop), 9-8=target_reg, 7=byte_sel, 6=0, 5-0=source_reg */
        uint16_t operand = (target_reg << 8) | (byte_sel << 7) | reg1;
        instr = (0xE << 12) | operand;
        emit16(instr);
    }
    else if (strcmp(mnemonic, "SETBYTE") == 0) {
        if (token_count != 3) goto error;
        uint8_t target_reg = atoi(tokens[1]);
        if (parse_immediate(tokens[2], &imm) < 0) goto error;

        /* Encoding: bits 11-10=01 (subop), 9-8=target_reg, 7-0=immediate */
        uint16_t operand = (1 << 10) | (target_reg << 8) | (imm & 0xFF);
        instr = (0xE << 12) | operand;
        emit16(instr);
    }
    /* Preset F instructions */
    else if (strcmp(mnemonic, "SETRENDMOD") == 0) {
        if (token_count != 2) goto error;
        uint8_t mode = atoi(tokens[1]);
        instr = (0xF << 12) | (0x4 << 8) | mode;
        emit16(instr);
    }
    else if (strcmp(mnemonic, "MOVDP") == 0) {
        if (token_count != 2) goto error;
        if (parse_register(tokens[1], &reg1) < 0) goto error;
        instr = (0xF << 12) | (0x3 << 8) | reg1;
        emit16(instr);
    }
    else if (strcmp(mnemonic, "MOV") == 0) {
        if (token_count != 2) goto error;
        if (parse_register(tokens[1], &reg1) < 0) goto error;
        instr = (0xF << 12) | (0x8 << 8) | reg1;
        emit16(instr);
    }
    else if (strcmp(mnemonic, "SETDP") == 0) {
        if (token_count != 2) goto error;
        if (parse_register(tokens[1], &reg1) < 0) goto error;
        instr = (0xF << 12) | (0x2 << 8) | reg1;
        emit16(instr);
    }
    else if (strcmp(mnemonic, "JMR") == 0) {
        instr = (0xF << 12) | (0x7 << 8);
        emit16(instr);
    }
    else {
        fprintf(stderr, "Line %d: Unknown instruction '%s'\n", line_num, mnemonic);
        return -1;
    }

    return 0;

error:
    fprintf(stderr, "Line %d: Invalid operands for '%s'\n", line_num, mnemonic);
    return -1;
}

/* Assemble one line */
int assemble_line(char *line, int line_num) {
    char *p = trim(line);

    /* Skip empty lines and comments */
    if (*p == '\0' || *p == ';') return 0;

    /* Check for label */
    char *colon = strchr(p, ':');
    if (colon) {
        *colon = '\0';
        char *label_name = trim(p);
        if (add_label(label_name, output_size) < 0) {
            fprintf(stderr, "Line %d: Failed to add label '%s'\n", line_num, label_name);
            return -1;
        }
        p = trim(colon + 1);
        if (*p == '\0') return 0;  /* Label only */
    }

    /* Remove comments */
    char *comment = strchr(p, ';');
    if (comment) *comment = '\0';
    p = trim(p);
    if (*p == '\0') return 0;

    /* Assemble instruction */
    return assemble_instruction(p, line_num);
}

/* Main assembler */
int assemble_file(const char *input_file, const char *output_file) {
    FILE *in = fopen(input_file, "r");
    if (!in) {
        fprintf(stderr, "Error: Cannot open input file '%s'\n", input_file);
        return -1;
    }

    char line[MAX_LINE];
    int line_num = 0;

    /* Two-pass assembly */
    for (current_pass = 0; current_pass < MAX_PASSES; current_pass++) {
        output_size = 0;
        line_num = 0;
        rewind(in);

        while (fgets(line, MAX_LINE, in)) {
            line_num++;
            if (assemble_line(line, line_num) < 0) {
                fclose(in);
                return -1;
            }
        }

        if (current_pass == 0) {
            printf("Pass 1: %d labels, %d bytes\n", label_count, output_size);
        }
    }

    fclose(in);

    /* Write output */
    FILE *out = fopen(output_file, "wb");
    if (!out) {
        fprintf(stderr, "Error: Cannot open output file '%s'\n", output_file);
        return -1;
    }

    fwrite(output, 1, output_size, out);
    fclose(out);

    printf("Pass 2: %d bytes written to '%s'\n", output_size, output_file);

    /* Print labels for debugging */
    printf("\nLabels:\n");
    for (int i = 0; i < label_count; i++) {
        printf("  %s = 0x%04X (%d)\n", labels[i].name, labels[i].address, labels[i].address);
    }

    return 0;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input.asm> <output.bin>\n", argv[0]);
        return 1;
    }

    return assemble_file(argv[1], argv[2]);
}

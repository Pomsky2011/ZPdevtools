/*
 * PPU Disassembler for ZeroPoint PPU
 *
 * Disassembles PPU microcode binaries into human-readable assembly.
 * Supports all 15 basic opcodes + Preset E/F extended instructions.
 *
 * Usage: ppudisasm <input.bin> [-o output.asm] [-s start] [-e end]
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

#undef MAX_FILE_SIZE
#define MAX_FILE_SIZE (64 * 1024)  /* 64 KB max (PPU memory) */

/* PPU instruction info */
typedef struct {
    const char* mnemonic;
    const char* description;
} InstrInfo;

/* Basic instructions (0x0-0xD). Mnemonic is NULL for opcodes 0x1/0x6/0x7,
 * which pick between two mnemonics via bit 11 of the operand at decode time
 * (see disassemble_instruction) and can't be printed from a static table. */
static const InstrInfo basic_instructions[14] = {
    {"DEFCALL", "Define call address"},
    {NULL, "Move execution pointer / NOP"},
    {"SWAPREG", "Swap two registers"},
    {"CLR", "Clear register"},
    {"CMP", "Compare registers"},
    {"CLRF", "Clear flags"},
    {NULL, "Jump if zero/greater"},
    {NULL, "Jump if not zero/not greater"},
    {"INC", "Increment register"},
    {"DEC", "Decrement register"},
    {"ADD", "Add registers"},
    {"SUB", "Subtract registers"},
    {"MUL", "Multiply registers"},
    {"INTDIV", "Integer divide registers"}
};

/* Preset E instructions (subop = bits 11-10 of the operand, 2 bits) */
static const char* preset_e_names[] = {
    "TARREG", "SETBYTE", "BUILD", "CPREG"
};

/* Preset F instructions (subop = bits 11-8 of the operand, 4 bits) */
static const char* preset_f_names[] = {
    "SETPOS", "SETTILE", "SETDP", "MOVDP", "SETRENDMOD",
    "PALETTE16", "PALETTE256", "JMR", "MOV", "SETREGBANK",
    "CLRTILE", "CLRPALETTE", "TILEDRAW", "SETTILEBANK", "CALL", "GBLS"
};

/* Read file */
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

/* Disassemble one PPU instruction (16-bit big-endian) */
static void disassemble_instruction(uint8_t* data, size_t offset, size_t max_size,
                                   FILE* out, int show_comments, int show_bytes, uint16_t addr) {
    uint8_t high;
    uint8_t low;
    uint16_t instruction;
    uint8_t opcode;
    uint16_t operand;

    if (offset + 1 >= max_size) return;

    /* PPU instructions are 16-bit big-endian */
    high = data[offset];
    low = data[offset + 1];
    instruction = (high << 8) | low;

    opcode = (instruction >> 12) & 0xF;  /* Top 4 bits */
    operand = instruction & 0xFFF;       /* Bottom 12 bits */

    /* Show address */
    fprintf(out, "%04X:  ", addr);

    /* Show bytes if requested */
    if (show_bytes) {
        fprintf(out, "%02X %02X   ", high, low);
    }

    /* Decode instruction */
    if (opcode <= 0xD) {
        /* Basic instruction. Encodings per ppu.cpp: DEFCALL/SWAPREG/CMP/
         * ADD/SUB/MUL/INTDIV pack two 6-bit registers as (X<<6)|Y; CLR/INC/
         * DEC take a single 6-bit register in the low 6 bits; CLRF and the
         * two branch opcodes take no register operand at all (branches jump
         * via REG_PC, not an operand field); opcode 0x1 and the two branch
         * opcodes (0x6/0x7) each pick their real mnemonic from bit 11. */
        const char *mnemonic;
        switch (opcode) {
            case 0x1: mnemonic = (operand & 0x800) ? "NOP" : "MOVXP"; break;
            case 0x6: mnemonic = (operand & 0x800) ? "JMG"  : "JMZ";  break;
            case 0x7: mnemonic = (operand & 0x800) ? "JNG"  : "JNZ"; break;
            default:  mnemonic = basic_instructions[opcode].mnemonic; break;
        }
        fprintf(out, "%-12s", mnemonic);

        switch (opcode) {
            case 0x0: case 0x2: case 0x4: case 0xA: case 0xB: case 0xC: case 0xD:
                fprintf(out, " R%d, R%d", (operand >> 6) & 0x3F, operand & 0x3F);
                break;
            case 0x1:
                /* MOVXP RY; NOP takes no operand */
                if (!(operand & 0x800)) {
                    fprintf(out, " R%d", operand & 0x3F);
                }
                break;
            case 0x3: case 0x8: case 0x9:
                fprintf(out, " R%d", operand & 0x3F);
                break;
            default:
                /* 0x5 CLRF, 0x6 JMZ/JMG, 0x7 JNZ/JNG - no operand */
                break;
        }

        if (show_comments) {
            fprintf(out, " ; %s", basic_instructions[opcode].description);
        }

    } else if (opcode == 0xE) {
        /* Preset E: subop = bits 11-10 (2 bits), suboperand = bits 9-0 (10 bits) */
        uint8_t subop = (operand >> 10) & 0x3;
        uint16_t suboperand = operand & 0x3FF;

        fprintf(out, "%-12s", preset_e_names[subop]);

        switch (subop) {
            case 0: {
                /* TARREG T, LSB/MSB, RX - encoding: TT Y 0 XXXXXX */
                uint8_t T = (suboperand >> 8) & 0x3;
                uint8_t Y = (suboperand >> 7) & 0x1;
                uint8_t X = suboperand & 0x3F;
                fprintf(out, " %d, %s, R%d", T, Y ? "MSB" : "LSB", X);
                break;
            }
            case 1: {
                /* SETBYTE T, $imm - encoding: TT XXXXXXXX */
                uint8_t T = (suboperand >> 8) & 0x3;
                uint8_t imm = suboperand & 0xFF;
                fprintf(out, " %d, $%02X", T, imm);
                break;
            }
            case 2: {
                /* BUILD T1, T2, Rdest - encoding: TT TT XXXXXX */
                uint8_t T1 = (suboperand >> 8) & 0x3;
                uint8_t T2 = (suboperand >> 6) & 0x3;
                uint8_t dest = suboperand & 0x3F;
                fprintf(out, " %d, %d, R%d", T1, T2, dest);
                break;
            }
            case 3: {
                /* CPREG RX, RY - encoding: 00 XXXX YYYY */
                uint8_t rx = (suboperand >> 4) & 0xF;
                uint8_t ry = suboperand & 0xF;
                fprintf(out, " R%d, R%d", rx, ry);
                break;
            }
        }

    } else if (opcode == 0xF) {
        /* Preset F: subop = bits 11-8 (4 bits), param = bits 7-0 (8 bits) */
        uint8_t subop = (operand >> 8) & 0xF;
        uint8_t param = operand & 0xFF;

        fprintf(out, "%-12s", preset_f_names[subop]);

        switch (subop) {
            case 0x0: /* SETPOS RX, RY */
                fprintf(out, " R%d, R%d", (param >> 4) & 0xF, param & 0xF);
                break;
            case 0x1: /* SETTILE RX, mode */
                fprintf(out, " R%d, %d", (param >> 2) & 0x3F, param & 0x3);
                break;
            case 0x2: /* SETDP RX */
            case 0x3: /* MOVDP RX */
            case 0x8: /* MOV RX */
            case 0xD: /* SETTILEBANK RX */
            case 0xF: /* GBLS RX */
                fprintf(out, " R%d", (param >> 2) & 0x3F);
                break;
            case 0x4: /* SETRENDMOD mode */
                fprintf(out, " %d", (param >> 7) & 0x1);
                break;
            case 0x9: /* SETREGBANK bankX, bankY */
                fprintf(out, " %d, %d", (param >> 4) & 0x3, (param >> 2) & 0x3);
                break;
            case 0xE: /* CALL $imm */
                fprintf(out, " $%02X", param);
                break;
            default:
                /* 0x5 PALETTE16, 0x6 PALETTE256, 0x7 JMR, 0xA CLRTILE,
                 * 0xB CLRPALETTE, 0xC TILEDRAW - no operand */
                break;
        }
    }

    fprintf(out, "\n");
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
    uint16_t addr;

    input_file = NULL;
    output_file = NULL;
    start_addr = 0;
    end_addr = 0xFFFF;
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
        }
    }

    if (!input_file) {
        printf("PPU Disassembler for ZeroPoint\n");
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
    fprintf(out, "; PPU Disassembly of %s\n", input_file);
    fprintf(out, "; Size: %zu bytes (0x%zX)\n", file_size, file_size);
    fprintf(out, "; Range: $%04X - $%04X\n\n", start_addr, end_addr);

    /* Disassemble (PPU instructions are 2 bytes each) */
    offset = start_addr;
    addr = start_addr;

    while (offset + 1 < end_addr && offset + 1 < file_size) {
        disassemble_instruction(data, offset, file_size, out,
                               show_comments, show_bytes, addr);
        offset += 2;
        addr += 2;
    }

    /* Cleanup */
    if (output_file) fclose(out);
    free(data);

    return 0;
}

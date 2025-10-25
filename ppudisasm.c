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
#include <stdint.h>
#include <string.h>

#define MAX_FILE_SIZE (64 * 1024)  // 64 KB max (PPU memory)

// PPU instruction info
typedef struct {
    const char* mnemonic;
    const char* description;
} InstrInfo;

// Basic instructions (0x0-0xD)
static const InstrInfo basic_instructions[14] = {
    {"DEFCALL", "Define call address"},
    {"MOVXP", "Move execution pointer (or NOP)"},
    {"SWAPREG", "Swap two registers"},
    {"CLR", "Clear register"},
    {"CMP", "Compare registers"},
    {"CLRF", "Clear flags"},
    {"JMZ/JMG", "Jump if zero/greater"},
    {"JNZ/JNG/JML", "Jump if not zero/not greater/less"},
    {"INC", "Increment register"},
    {"DEC", "Decrement register"},
    {"ADD", "Add registers"},
    {"SUB", "Subtract registers"},
    {"MUL", "Multiply registers"},
    {"INTDIV", "Integer divide registers"}
};

// Preset E instructions
static const char* preset_e_names[] = {
    "TARREG", "SETBYTE", "BUILD", "CPREG"
};

// Preset F instructions
static const char* preset_f_names[] = {
    "SETPOS", "SETTILE", "SETDP", "MOVDP", "SETRENDMOD",
    "PALETTE16", "PALETTE256", "JMR", "MOV", "SETREGBANK",
    "CLRTILE", "CLRPALETTE", "TILEDRAW", "RESERVED", "CALL", "GBLS"
};

// Read file
static uint8_t* read_file(const char* filename, size_t* size) {
    FILE* f = fopen(filename, "rb");
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

    uint8_t* buffer = malloc(*size);
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

// Parse hex address
static uint32_t parse_hex(const char* str) {
    uint32_t value = 0;
    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        str += 2;
    }
    sscanf(str, "%x", &value);
    return value;
}

// Disassemble one PPU instruction (16-bit big-endian)
static void disassemble_instruction(uint8_t* data, size_t offset, size_t max_size,
                                   FILE* out, int show_comments, int show_bytes, uint16_t addr) {
    if (offset + 1 >= max_size) return;

    // PPU instructions are 16-bit big-endian
    uint8_t high = data[offset];
    uint8_t low = data[offset + 1];
    uint16_t instruction = (high << 8) | low;

    uint8_t opcode = (instruction >> 12) & 0xF;  // Top 4 bits
    uint16_t operand = instruction & 0xFFF;       // Bottom 12 bits

    // Show address
    fprintf(out, "%04X:  ", addr);

    // Show bytes if requested
    if (show_bytes) {
        fprintf(out, "%02X %02X   ", high, low);
    }

    // Decode instruction
    if (opcode <= 0xD) {
        // Basic instruction
        fprintf(out, "%-12s", basic_instructions[opcode].mnemonic);

        // Show operand based on instruction
        if (opcode == 0x0) {
            // DEFCALL - operand is address
            fprintf(out, " $%03X", operand);
        } else if (opcode >= 0x2 && opcode <= 0xD) {
            // Most instructions use register operands
            uint8_t reg1 = (operand >> 6) & 0x3F;
            uint8_t reg2 = operand & 0x3F;
            fprintf(out, " R%d, R%d", reg1, reg2);
        } else if (opcode == 0x1) {
            // MOVXP/NOP
            if (operand == 0) {
                fprintf(out, " (NOP)");
            } else {
                fprintf(out, " $%03X", operand);
            }
        }

        if (show_comments) {
            fprintf(out, " ; %s", basic_instructions[opcode].description);
        }

    } else if (opcode == 0xE) {
        // Preset E
        uint8_t subop = (operand >> 8) & 0xF;
        uint8_t param = operand & 0xFF;

        if (subop < 4) {
            fprintf(out, "%-12s", preset_e_names[subop]);

            if (subop == 0) {
                // TARREG T, Y, X
                uint8_t T = (param >> 6) & 0x3;
                uint8_t Y = (param >> 3) & 0x7;
                uint8_t X = param & 0x7;
                fprintf(out, " %d, %s, R%d", T, Y ? "MSB" : "LSB", X);
            } else if (subop == 1) {
                // SETBYTE T, imm8
                uint8_t T = (param >> 6) & 0x3;
                uint8_t imm = param & 0x3F;
                fprintf(out, " %d, $%02X", T, imm);
            } else {
                fprintf(out, " $%02X", param);
            }
        } else {
            fprintf(out, "PRESET_E_%X  $%02X", subop, param);
        }

    } else if (opcode == 0xF) {
        // Preset F
        uint8_t subop = (operand >> 8) & 0xF;
        uint8_t param = operand & 0xFF;

        if (subop < 16) {
            fprintf(out, "%-12s", preset_f_names[subop]);
            fprintf(out, " $%02X", param);
        } else {
            fprintf(out, "PRESET_F_%X  $%02X", subop, param);
        }
    }

    fprintf(out, "\n");
}

int main(int argc, char* argv[]) {
    const char* input_file = NULL;
    const char* output_file = NULL;
    uint32_t start_addr = 0;
    uint32_t end_addr = 0xFFFF;
    int show_comments = 0;
    int show_bytes = 0;

    // Parse arguments
    for (int i = 1; i < argc; i++) {
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

    // Read input file
    size_t file_size;
    uint8_t* data = read_file(input_file, &file_size);
    if (!data) return 1;

    // Adjust end address
    if (end_addr > file_size) end_addr = file_size;
    if (start_addr > file_size) start_addr = file_size;

    // Open output
    FILE* out = stdout;
    if (output_file) {
        out = fopen(output_file, "w");
        if (!out) {
            fprintf(stderr, "Error: Cannot open output file '%s'\n", output_file);
            free(data);
            return 1;
        }
    }

    // Header
    fprintf(out, "; PPU Disassembly of %s\n", input_file);
    fprintf(out, "; Size: %zu bytes (0x%zX)\n", file_size, file_size);
    fprintf(out, "; Range: $%04X - $%04X\n\n", start_addr, end_addr);

    // Disassemble (PPU instructions are 2 bytes each)
    size_t offset = start_addr;
    uint16_t addr = start_addr;

    while (offset + 1 < end_addr && offset + 1 < file_size) {
        disassemble_instruction(data, offset, file_size, out,
                               show_comments, show_bytes, addr);
        offset += 2;
        addr += 2;
    }

    // Cleanup
    if (output_file) fclose(out);
    free(data);

    return 0;
}

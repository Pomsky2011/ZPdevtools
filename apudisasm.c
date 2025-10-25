/*
 * APU Disassembler for ZeroPoint APU/DSP
 *
 * Disassembles APU binary files into human-readable assembly.
 * Supports all 47 APU instructions (5-bit opcode + 11-bit operands).
 *
 * Usage: apudisasm <input.bin> [-o output.asm] [-s start] [-e end]
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

#define MAX_FILE_SIZE (64 * 1024)  // 64 KB max

// APU instruction info (47 instructions total)
static const char* apu_instructions[47] = {
    "MOV", "ADD", "SUB", "MUL", "AND", "OR", "XOR", "NOT",
    "SHL", "SHR", "CMP", "JMP", "JZ", "JNZ", "JG", "JL",
    "CALL", "RET", "PUX", "POX", "PUY", "POY", "NOP", "HLT",
    "LDI", "STI", "LDD", "STD", "LDM", "STM", "INC", "DEC",
    "CFN", "CCF", "SETDB", "SETBF", "GETBF", "SETRP", "GETRP",
    "SETDP", "GETDP", "GETPC", "IOI", "IOO", "BRP", "MMP", "SST"
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

// Disassemble one APU instruction (16-bit little-endian)
static void disassemble_instruction(uint8_t* data, size_t offset, size_t max_size,
                                   FILE* out, int show_comments, int show_bytes, uint16_t addr) {
    if (offset + 1 >= max_size) return;

    // APU instructions are 16-bit little-endian
    uint8_t low = data[offset];
    uint8_t high = data[offset + 1];
    uint16_t instruction = low | (high << 8);

    uint8_t opcode = (instruction >> 11) & 0x1F;  // Top 5 bits
    uint16_t operand = instruction & 0x7FF;        // Bottom 11 bits

    // Show address
    fprintf(out, "%04X:  ", addr);

    // Show bytes if requested
    if (show_bytes) {
        fprintf(out, "%02X %02X   ", low, high);
    }

    // Decode instruction
    if (opcode < 47) {
        fprintf(out, "%-10s", apu_instructions[opcode]);

        // Decode operands based on instruction type
        switch (opcode) {
            case 0:  // MOV X, Y
            case 1:  // ADD X, Y
            case 2:  // SUB X, Y
            case 3:  // MUL X, Y
            case 4:  // AND X, Y
            case 5:  // OR X, Y
            case 6:  // XOR X, Y
            case 10: // CMP X, Y
                {
                    uint8_t regX = (operand >> 5) & 0x1F;
                    uint8_t regY = operand & 0x1F;
                    fprintf(out, " R%d, R%d", regX, regY);
                }
                break;

            case 7:  // NOT X
            case 30: // INC X
            case 31: // DEC X
                {
                    uint8_t reg = operand & 0x1F;
                    fprintf(out, " R%d", reg);
                }
                break;

            case 8:  // SHL X, #imm
            case 9:  // SHR X, #imm
                {
                    uint8_t reg = (operand >> 5) & 0x1F;
                    uint8_t shift = operand & 0x1F;
                    fprintf(out, " R%d, #%d", reg, shift);
                }
                break;

            case 11: // JMP addr
            case 12: // JZ addr
            case 13: // JNZ addr
            case 14: // JG addr
            case 15: // JL addr
            case 16: // CALL addr
                fprintf(out, " $%04X", operand);
                break;

            case 17: // RET
            case 22: // NOP
            case 23: // HLT
                // No operands
                break;

            case 18: // PUX X
            case 19: // POX X
            case 20: // PUY Y
            case 21: // POY Y
                {
                    uint8_t reg = operand & 0xFF;
                    fprintf(out, " R%d", reg);
                }
                break;

            case 24: // LDI X, #imm
                {
                    uint8_t reg = (operand >> 8) & 0x7;
                    uint8_t imm = operand & 0xFF;
                    fprintf(out, " R%d, #$%02X", reg, imm);
                }
                break;

            case 25: // STI [imm], X
                {
                    uint8_t reg = (operand >> 8) & 0x7;
                    uint8_t addr = operand & 0xFF;
                    fprintf(out, " [$%02X], R%d", addr, reg);
                }
                break;

            case 26: // LDD X, [DP+imm]
            case 27: // STD [DP+imm], X
                {
                    uint8_t reg = (operand >> 8) & 0x7;
                    uint8_t offset = operand & 0xFF;
                    fprintf(out, " R%d, [DP+$%02X]", reg, offset);
                }
                break;

            case 28: // LDM X, [RP]
            case 29: // STM [RP], X
                {
                    uint8_t reg = operand & 0xFF;
                    fprintf(out, " R%d, [RP]", reg);
                }
                break;

            case 32: // CFN
            case 33: // CCF
                fprintf(out, " $%04X", operand);
                break;

            case 34: // SETDB
            case 35: // SETBF
            case 37: // SETRP
            case 39: // SETDP
                fprintf(out, " $%04X", operand);
                break;

            case 36: // GETBF X
            case 38: // GETRP X
            case 40: // GETDP X
            case 41: // GETPC X
                {
                    uint8_t reg = operand & 0xFF;
                    fprintf(out, " R%d", reg);
                }
                break;

            case 42: // IOI
            case 43: // IOO
            case 44: // BRP
                fprintf(out, " $%04X", operand);
                break;

            case 45: // MMP (Music Mixing Processor)
            case 46: // SST (Sample Storage)
                {
                    uint8_t channel = (operand >> 6) & 0x1F;
                    uint8_t param = operand & 0x3F;
                    fprintf(out, " CH%d, $%02X", channel, param);
                }
                break;

            default:
                fprintf(out, " $%04X", operand);
                break;
        }
    } else {
        fprintf(out, ".word      $%04X  ; Unknown opcode %d", instruction, opcode);
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
        printf("APU Disassembler for ZeroPoint\n");
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
    fprintf(out, "; APU Disassembly of %s\n", input_file);
    fprintf(out, "; Size: %zu bytes (0x%zX)\n", file_size, file_size);
    fprintf(out, "; Range: $%04X - $%04X\n\n", start_addr, end_addr);

    // Disassemble (APU instructions are 2 bytes each)
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

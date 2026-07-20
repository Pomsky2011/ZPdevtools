/*
 * APU Disassembler for ZeroPoint APU/DSP
 *
 * Disassembles APU binary files into human-readable assembly.
 * Decodes the real APU ISA: 16-bit instructions, opcode = bits 15-11
 * (5 bits, 0x00-0x1F), operand = bits 10-0 (11 bits). Cross-checked
 * against ZeroPoint/src/apu.cpp (APU::executeInstruction and friends,
 * the ground truth for behavior) and apuasm.c (the assembler-side
 * syntax this tool's output is meant to round-trip through). Opcodes
 * 0x1C-0x1F are reserved and halt the APU on execution; several defined
 * opcodes also have unused/reserved sub-encodings (e.g. 0x0B subop 0/6/7)
 * that no real mnemonic maps to - those print as ".WORD" with a comment
 * instead of a fabricated mnemonic.
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
#include "compat.h"
#include <string.h>

#undef MAX_FILE_SIZE
#define MAX_FILE_SIZE (64 * 1024)  /* 64 KB max */

/* 1-bit register selector (X=0/Y=1) -> name, matching apuasm.c's
 * parse_register() convention (X/Y accepted alongside R%d on input). */
static const char* reg1(int bit) {
    return bit ? "Y" : "X";
}

/* Emit a raw word for an opcode/sub-encoding with no real mnemonic,
 * rather than fabricating one or silently treating it as NOP. */
static void print_reserved(FILE* out, uint16_t instruction, const char* note) {
    fprintf(out, "%-10s$%04X  ; reserved (%s)", ".WORD", instruction, note);
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

/* Disassemble one APU instruction. ROM binaries are 16-bit big-endian
 * words (apuasm.c writes the high byte first) - read them the same way. */
static void disassemble_instruction(uint8_t* data, size_t offset, size_t max_size,
                                   FILE* out, int show_comments, int show_bytes, uint16_t addr) {
    uint8_t hi;
    uint8_t lo;
    uint16_t instruction;
    uint8_t opcode;
    uint16_t operand;
    char note[48];

    if (offset + 1 >= max_size) return;

    /* APU instructions are 16-bit big-endian (matches apuasm.c's output:
     * bytes[0] = high byte, bytes[1] = low byte). */
    hi = data[offset];
    lo = data[offset + 1];
    instruction = ((uint16_t)hi << 8) | lo;

    opcode = (instruction >> 11) & 0x1F;   /* bits 15-11 */
    operand = instruction & 0x7FF;         /* bits 10-0 */

    /* Show address */
    fprintf(out, "%04X:  ", addr);

    /* Show bytes if requested */
    if (show_bytes) {
        fprintf(out, "%02X %02X   ", hi, lo);
    }

    switch (opcode) {
        case 0x00: /* NOP - operand is a plain stall count */
            if (operand == 0) {
                fprintf(out, "%-10s", "NOP");
            } else {
                char buf[16];
                sprintf(buf, "NOP %u", (unsigned)operand);
                fprintf(out, "%-10s", buf);
            }
            break;

        case 0x01: { /* JMP - relative iff bits9-8==00 (backward relative is
                        dead: direction=bit9 can only be 0 when bits9-8==00) */
            char buf[24];
            if ((operand & 0x300) == 0) {
                sprintf(buf, "JMP $%02X", operand & 0xFF);
            } else {
                int mode = (operand >> 8) & 1;
                if (mode) {
                    sprintf(buf, "JMP 0, 1, $%02X", operand & 0xFF); /* $RPZZ */
                } else {
                    sprintf(buf, "JMP 0, 0, $%02X", operand & 0xFF); /* $80ZZ BIOS */
                }
            }
            fprintf(out, "%-10s", buf);
            break;
        }

        case 0x02: { /* bit8 selects JZ(1)/JNZ(0) */
            char buf[24];
            int reg = (operand >> 10) & 1;
            if (operand & 0x100) {
                int mode = (operand >> 9) & 1;
                sprintf(buf, "JZ %s, %d, $%02X", reg1(reg), mode, operand & 0xFF);
            } else {
                /* JNZ's mode bit is unreachable in practice - real JNZ
                 * always targets $80ZZ (BIOS). */
                sprintf(buf, "JNZ %s, $%02X", reg1(reg), operand & 0xFF);
            }
            fprintf(out, "%-10s", buf);
            break;
        }

        case 0x03: { /* bit10 selects SDP(1)/SRP(0) - "legacy" 2-arg SDP */
            char buf[24];
            if (operand & 0x400) {
                sprintf(buf, "SDP $%02X", operand & 0xFF);
            } else {
                sprintf(buf, "SRP $%02X", operand & 0xFF);
            }
            fprintf(out, "%-10s", buf);
            break;
        }

        case 0x04: /* NOR */
        case 0x05: /* AND */
        case 0x06: /* ADD */
        case 0x07: /* SUB */
        case 0x10: /* ADC */
        case 0x11: { /* SBC - all six share the same operand shape */
            static const char* names[0x12];
            char buf[32];
            int regX = (operand >> 10) & 1;
            int regY = (operand >> 9)  & 1;
            int toMem = !(operand & 0x100);
            const char* mnem;

            names[0x04] = "NOR"; names[0x05] = "AND"; names[0x06] = "ADD";
            names[0x07] = "SUB"; names[0x10] = "ADC"; names[0x11] = "SBC";
            mnem = names[opcode];

            if (toMem) {
                sprintf(buf, "%s %s, %s, $%02X", mnem, reg1(regX), reg1(regY), operand & 0xFF);
            } else {
                sprintf(buf, "%s %s, %s, R%d", mnem, reg1(regX), reg1(regY), operand & 0x7F);
            }
            fprintf(out, "%-10s", buf);
            break;
        }

        case 0x08: { /* bit9 selects STA(1)/STR(0) */
            char buf[24];
            int regX = (operand >> 10) & 1;
            int addrOff = operand & 0xFF;
            if (operand & 0x200) {
                sprintf(buf, "STA %s, $%02X", reg1(regX), addrOff);
            } else {
                sprintf(buf, "STR $%02X, %s", addrOff, reg1(regX));
            }
            fprintf(out, "%-10s", buf);
            break;
        }

        case 0x09: { /* LDA/STA via data pointer - subop = bits9-8 */
            char buf[24];
            int subop = (operand >> 8) & 0x03;
            int regSel = (operand >> 10) & 1;
            switch (subop) {
                case 0x00: sprintf(buf, "LDA %s", reg1(regSel)); break;
                case 0x01: sprintf(buf, "STA %s", reg1(regSel)); break;
                case 0x02: sprintf(buf, "STA $%02X", operand & 0xFF); break;
                default:
                    sprintf(note, "opcode 0x09 subop 3");
                    print_reserved(out, instruction, note);
                    goto done;
            }
            fprintf(out, "%-10s", buf);
            break;
        }

        case 0x0A: { /* SCR - also apuasm.c's "LDA reg, $imm" alias, same opcode */
            char buf[24];
            int reg = (operand >> 10) & 1;
            sprintf(buf, "SCR %s, $%02X", reg1(reg), operand & 0xFF);
            fprintf(out, "%-10s", buf);
            break;
        }

        case 0x0B: { /* subop = bits10-8, mask = bits7-4 */
            char buf[24];
            int subop = (operand >> 8) & 0x07;
            int mask = (operand >> 4) & 0x0F;
            switch (subop) {
                case 0x1: sprintf(buf, "SFR $%X", mask); break;
                case 0x2: sprintf(buf, "CF $%X", mask); break;
                case 0x3: sprintf(buf, "SF $%X", mask); break;
                case 0x4:
                case 0x5: {
                    int reg = (operand >> 8) & 1;
                    sprintf(buf, "STF %s", reg1(reg));
                    break;
                }
                default:
                    sprintf(note, "opcode 0x0B subop %d", subop);
                    print_reserved(out, instruction, note);
                    goto done;
            }
            fprintf(out, "%-10s", buf);
            break;
        }

        case 0x0C: { /* subop = bits10-8 */
            char buf[24];
            int subop = (operand >> 8) & 0x07;
            switch (subop) {
                case 0x0: sprintf(buf, "ZOR R%d", operand & 0x3F); break;
                case 0x2: sprintf(buf, "ZOA $%02X", operand & 0xFF); break;
                case 0x3: sprintf(buf, "ZOA DP"); break;
                default:
                    sprintf(note, "opcode 0x0C subop %d", subop);
                    print_reserved(out, instruction, note);
                    goto done;
            }
            fprintf(out, "%-10s", buf);
            break;
        }

        case 0x0D: { /* bit10 selects LFN(1)/LST(0) */
            char buf[24];
            int loopId = (operand >> 6) & 0x0F;
            if (operand & 0x400) {
                sprintf(buf, "LFN %d", loopId);
            } else {
                sprintf(buf, "LST %d, %d", loopId, operand & 0x3F);
            }
            fprintf(out, "%-10s", buf);
            break;
        }

        case 0x0E: { /* BRT - raw 11-bit forward word-offset, no dispatch bits */
            char buf[24];
            sprintf(buf, "BRT $%X", operand & 0x7FF);
            fprintf(out, "%-10s", buf);
            break;
        }

        case 0x0F: /* BRP - operand ignored by hardware */
            fprintf(out, "%-10s", "BRP");
            break;

        case 0x12: { /* bit8 selects BNE(1)/BEQ(0) */
            char buf[24];
            int regX = (operand >> 10) & 1;
            int regY = (operand >> 9)  & 1;
            const char* mnem = (operand & 0x100) ? "BNE" : "BEQ";
            sprintf(buf, "%s %s, %s, $%02X", mnem, reg1(regX), reg1(regY), operand & 0xFF);
            fprintf(out, "%-10s", buf);
            break;
        }

        case 0x13: { /* bit8 selects BGT(1)/BLT(0) */
            char buf[24];
            int regX = (operand >> 10) & 1;
            int regY = (operand >> 9)  & 1;
            const char* mnem = (operand & 0x100) ? "BGT" : "BLT";
            sprintf(buf, "%s %s, %s, $%02X", mnem, reg1(regX), reg1(regY), operand & 0xFF);
            fprintf(out, "%-10s", buf);
            break;
        }

        case 0x14: { /* SDB */
            char buf[24];
            sprintf(buf, "SDB $%02X", operand & 0xFF);
            fprintf(out, "%-10s", buf);
            break;
        }

        case 0x15: { /* JMS family: useDP=bit10, doCall=bit9. The emulator
                        dispatches all four combinations through a single
                        execJMS(), but apuasm.c exposes them as four
                        distinct mnemonics (JMS/JSR/JDP/JDPS) that encode to
                        this opcode - print those names for round-trip and
                        readability rather than collapsing to just "JMS". */
            char buf[24];
            int useDP = (operand >> 10) & 1;
            int doCall = (operand >> 9) & 1;
            if (!useDP && !doCall)      sprintf(buf, "JMS $%02X", operand & 0xFF);
            else if (!useDP && doCall)  sprintf(buf, "JSR $%02X", operand & 0xFF);
            else if (useDP && !doCall)  sprintf(buf, "JDP");
            else                        sprintf(buf, "JDPS");
            fprintf(out, "%-10s", buf);
            break;
        }

        case 0x16: { /* INC/DEC: bit10 = DEC(1)/INC(0), bits9-8 = target */
            static const char* targets[4] = { "X", "Y", "DP", "SP" };
            char buf[24];
            int dec = (operand >> 10) & 1;
            int target = (operand >> 8) & 0x03;
            sprintf(buf, "%s %s", dec ? "DEC" : "INC", targets[target]);
            fprintf(out, "%-10s", buf);
            break;
        }

        case 0x17: { /* Stack ops: D=bit10 */
            char buf[24];
            int D = (operand >> 10) & 1;
            if (!D) {
                if (operand & 0x200) {
                    int b = (operand >> 8) & 1;
                    sprintf(buf, "SDP %d, $%02X", b, operand & 0xFF);
                } else {
                    uint8_t low = operand & 0xFF;
                    if (low == 0x42)      sprintf(buf, "PODP");
                    else if (low == 0x41) sprintf(buf, "PUDP");
                    else if (low == 0x40) sprintf(buf, "RET");
                    else                  sprintf(buf, "BSP");
                }
            } else {
                int SS = (operand >> 8) & 0x03;
                switch (SS) {
                    case 0: sprintf(buf, "PUX"); break;
                    case 1: sprintf(buf, "PUY"); break;
                    case 2: sprintf(buf, "POX"); break;
                    default: sprintf(buf, "POY"); break;
                }
            }
            fprintf(out, "%-10s", buf);
            break;
        }

        case 0x18: { /* bit8 selects EXC(1)/MOV(0) */
            char buf[24];
            if (operand & 0x100) {
                sprintf(buf, "EXC");
            } else {
                int src = (operand >> 10) & 1;
                int dst = (operand >> 9)  & 1;
                sprintf(buf, "MOV %s, %s", reg1(src), reg1(dst));
            }
            fprintf(out, "%-10s", buf);
            break;
        }

        case 0x19: { /* CME/CMN/CMG/CML - subOp = bits7-6 selects mnemonic;
                        subOp's LSB (bit6) also drives branch direction
                        inside the emulator, so CME/CMG always branch
                        forward and CMN/CML always branch backward - there
                        is no independent direction operand to print. */
            static const char* names[4] = { "CME", "CMN", "CMG", "CML" };
            char buf[24];
            int regX = (operand >> 10) & 1;
            int regY = (operand >> 9)  & 1;
            int subOp = (operand >> 6) & 0x03;
            sprintf(buf, "%s %s, %s, $%02X", names[subOp], reg1(regX), reg1(regY), operand & 0x3F);
            fprintf(out, "%-10s", buf);
            break;
        }

        case 0x1A: /* CRB */
        case 0x1B: { /* XOR - same operand shape as CRB, opposite toMem
                        polarity from the NOR family: bit8 SET = memory. */
            char buf[24];
            const char* mnem = (opcode == 0x1A) ? "CRB" : "XOR";
            int rx = (operand >> 10) & 1;
            int ry = (operand >> 9)  & 1;
            int toMem = (operand >> 8) & 1;
            if (toMem) {
                sprintf(buf, "%s %s, %s, $%02X", mnem, reg1(rx), reg1(ry), operand & 0xFF);
            } else {
                int rz = (operand >> 7) & 1; /* 1-bit output selector, X/Y only */
                sprintf(buf, "%s %s, %s, %s", mnem, reg1(rx), reg1(ry), reg1(rz));
            }
            fprintf(out, "%-10s", buf);
            break;
        }

        case 0x1C:
            /* Opcodes 0x1C-0x1F are reserved and halt on execution; 0x1C
             * with a zero operand is exactly what apuasm.c's HLT mnemonic
             * assembles to, so print that recognizable form. Any other
             * word in this range has no defined mnemonic. */
            if (operand == 0) {
                fprintf(out, "%-10s", "HLT");
            } else {
                sprintf(note, "opcode 0x1C, halts");
                print_reserved(out, instruction, note);
                goto done;
            }
            break;

        case 0x1D:
        case 0x1E:
        case 0x1F:
            sprintf(note, "opcode 0x%02X, halts", opcode);
            print_reserved(out, instruction, note);
            goto done;

        default:
            sprintf(note, "unknown opcode 0x%02X", opcode);
            print_reserved(out, instruction, note);
            goto done;
    }

    if (show_comments) {
        fprintf(out, "  ; op=$%02X operand=$%03X", opcode, operand);
    }

done:
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

    /* Read input file */
    {
        FILE* f = fopen(input_file, "rb");
        if (!f) {
            fprintf(stderr, "Error: Cannot open file '%s'\n", input_file);
            return 1;
        }
        fseek(f, 0, SEEK_END);
        file_size = ftell(f);
        fseek(f, 0, SEEK_SET);

        if (file_size > MAX_FILE_SIZE) {
            fprintf(stderr, "Error: File too large (max %d bytes)\n", MAX_FILE_SIZE);
            fclose(f);
            return 1;
        }

        data = malloc(file_size);
        if (!data) {
            fprintf(stderr, "Error: Out of memory\n");
            fclose(f);
            return 1;
        }

        if (fread(data, 1, file_size, f) != file_size) {
            fprintf(stderr, "Error: Failed to read file\n");
            free(data);
            fclose(f);
            return 1;
        }
        fclose(f);
    }

    /* Adjust end address */
    if (end_addr > file_size) end_addr = (uint32_t)file_size;
    if (start_addr > file_size) start_addr = (uint32_t)file_size;

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
    fprintf(out, "; APU Disassembly of %s\n", input_file);
    fprintf(out, "; Size: %lu bytes (0x%lX)\n", (unsigned long)file_size, (unsigned long)file_size);
    fprintf(out, "; Range: $%04X - $%04X\n\n", start_addr, end_addr);

    /* Disassemble (APU instructions are 2 bytes each) */
    offset = start_addr;
    addr = (uint16_t)start_addr;

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

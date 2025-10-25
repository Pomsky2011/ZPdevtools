/*
 * Hex Viewer for ZeroPoint
 *
 * Smart hex dump utility with ASCII display and address offsets.
 * Similar to 'hexdump -C' but with ZeroPoint-specific features.
 *
 * Usage: hexview <file> [options]
 *
 * Options:
 *   -s <offset>   Start at offset (hex or decimal)
 *   -n <count>    Display N bytes
 *   -w <width>    Bytes per line (default: 16)
 *   -g <group>    Group bytes (1, 2, or 4, default: 1)
 *   -a            Show address in hex only (no decimal)
 *   -o <output>   Write to file instead of stdout
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#define DEFAULT_WIDTH 16
#define MAX_WIDTH 64

// Parse number (supports hex 0x prefix or decimal)
static long long parse_number(const char* str) {
    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        return strtoll(str + 2, NULL, 16);
    }
    return strtoll(str, NULL, 10);
}

// Print ASCII representation
static void print_ascii(uint8_t* data, size_t size) {
    printf(" |");
    for (size_t i = 0; i < size; i++) {
        char c = data[i];
        if (isprint(c) && c != '\n' && c != '\r' && c != '\t') {
            printf("%c", c);
        } else {
            printf(".");
        }
    }
    printf("|");
}

// Display hex dump
static void hexdump(FILE* in, FILE* out, size_t offset, size_t count,
                   int width, int group, int hex_only) {
    uint8_t buffer[MAX_WIDTH];
    size_t total_read = 0;

    // Seek to offset
    if (offset > 0) {
        fseek(in, offset, SEEK_SET);
    }

    // Read and display
    while (total_read < count || count == 0) {
        size_t to_read = width;
        if (count > 0 && total_read + to_read > count) {
            to_read = count - total_read;
        }

        size_t bytes_read = fread(buffer, 1, to_read, in);
        if (bytes_read == 0) break;

        // Print address
        if (hex_only) {
            fprintf(out, "%08zX: ", offset + total_read);
        } else {
            fprintf(out, "%08zX (%8zu): ", offset + total_read, offset + total_read);
        }

        // Print hex bytes
        for (size_t i = 0; i < width; i++) {
            if (i < bytes_read) {
                fprintf(out, "%02X", buffer[i]);

                // Add space after group
                if (group > 1 && (i + 1) % group == 0) {
                    fprintf(out, " ");
                } else if (group == 1) {
                    fprintf(out, " ");
                }
            } else {
                // Pad with spaces
                fprintf(out, "  ");
                if (group > 1 && (i + 1) % group == 0) {
                    fprintf(out, " ");
                } else if (group == 1) {
                    fprintf(out, " ");
                }
            }
        }

        // Print ASCII
        fprintf(out, " ");
        print_ascii(buffer, bytes_read);
        fprintf(out, "\n");

        total_read += bytes_read;
    }

    // Summary
    if (!hex_only) {
        fprintf(out, "\nTotal bytes displayed: %zu (0x%zX)\n", total_read, total_read);
    }
}

// Analyze file statistics
static void analyze_file(FILE* f) {
    uint8_t byte_counts[256] = {0};
    size_t total = 0;
    int c;

    rewind(f);
    while ((c = fgetc(f)) != EOF) {
        byte_counts[c]++;
        total++;
    }

    printf("=== File Statistics ===\n");
    printf("Total bytes: %zu (0x%zX)\n", total, total);
    printf("\nMost common bytes:\n");

    // Find top 10 most common bytes
    for (int pass = 0; pass < 10; pass++) {
        int max_byte = 0;
        size_t max_count = 0;

        for (int i = 0; i < 256; i++) {
            if (byte_counts[i] > max_count) {
                max_count = byte_counts[i];
                max_byte = i;
            }
        }

        if (max_count == 0) break;

        printf("  0x%02X (%3d): %zu times (%.2f%%)\n",
               max_byte, max_byte, max_count,
               (max_count * 100.0) / total);

        byte_counts[max_byte] = 0;  // Remove for next pass
    }

    printf("\n");
    rewind(f);
}

int main(int argc, char* argv[]) {
    const char* input_file = NULL;
    const char* output_file = NULL;
    size_t offset = 0;
    size_t count = 0;  // 0 = entire file
    int width = DEFAULT_WIDTH;
    int group = 1;
    int hex_only = 0;
    int analyze = 0;

    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            offset = parse_number(argv[++i]);
        } else if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
            count = parse_number(argv[++i]);
        } else if (strcmp(argv[i], "-w") == 0 && i + 1 < argc) {
            width = atoi(argv[++i]);
            if (width < 1 || width > MAX_WIDTH) {
                fprintf(stderr, "Error: Width must be 1-%d\n", MAX_WIDTH);
                return 1;
            }
        } else if (strcmp(argv[i], "-g") == 0 && i + 1 < argc) {
            group = atoi(argv[++i]);
            if (group != 1 && group != 2 && group != 4) {
                fprintf(stderr, "Error: Group must be 1, 2, or 4\n");
                return 1;
            }
        } else if (strcmp(argv[i], "-a") == 0) {
            hex_only = 1;
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output_file = argv[++i];
        } else if (strcmp(argv[i], "--analyze") == 0) {
            analyze = 1;
        } else if (argv[i][0] != '-') {
            input_file = argv[i];
        }
    }

    if (!input_file) {
        printf("Hex Viewer for ZeroPoint\n");
        printf("Usage: %s <file> [options]\n", argv[0]);
        printf("\nOptions:\n");
        printf("  -s <offset>   Start at offset (hex or decimal)\n");
        printf("  -n <count>    Display N bytes\n");
        printf("  -w <width>    Bytes per line (1-%d, default: 16)\n", MAX_WIDTH);
        printf("  -g <group>    Group bytes (1, 2, 4, default: 1)\n");
        printf("  -a            Hex addresses only\n");
        printf("  -o <file>     Write to file\n");
        printf("  --analyze     Show file statistics\n");
        printf("\nExamples:\n");
        printf("  %s program.bin\n", argv[0]);
        printf("  %s rom.zpb -s 0x100 -n 256\n", argv[0]);
        printf("  %s data.bin -w 8 -g 2\n", argv[0]);
        return 1;
    }

    // Open input file
    FILE* in = fopen(input_file, "rb");
    if (!in) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", input_file);
        return 1;
    }

    // Get file size
    fseek(in, 0, SEEK_END);
    size_t file_size = ftell(in);
    rewind(in);

    printf("File: %s\n", input_file);
    printf("Size: %zu bytes (0x%zX)\n\n", file_size, file_size);

    // Analyze if requested
    if (analyze) {
        analyze_file(in);
    }

    // Open output file if specified
    FILE* out = stdout;
    if (output_file) {
        out = fopen(output_file, "w");
        if (!out) {
            fprintf(stderr, "Error: Cannot create output file '%s'\n", output_file);
            fclose(in);
            return 1;
        }
    }

    // Perform hex dump
    hexdump(in, out, offset, count, width, group, hex_only);

    // Cleanup
    fclose(in);
    if (output_file) fclose(out);

    return 0;
}

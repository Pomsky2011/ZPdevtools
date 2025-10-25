/*
 * ROM Inspector for ZeroPoint ROM (.zpb) files
 *
 * Analyzes and displays information about ZPB ROM files including
 * header data, checksums, and component binaries.
 *
 * Usage: rominspect <rom.zpb> [options]
 *
 * Options:
 *   -v        Verbose mode (show more details)
 *   -x        Extract components to separate files
 *   -c        Verify checksums
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#pragma pack(push, 1)
typedef struct {
    char magic[4];           // "ZPB\0"
    uint8_t version;         // ROM format version
    uint8_t flags;           // ROM flags
    uint16_t reserved;       // Reserved for future use
    char title[64];          // ROM title (null-terminated)
    char developer[64];      // Developer name (null-terminated)
    uint32_t entryPoint;     // CPU entry point (24-bit address)
    uint32_t cpuSize;        // CPU binary size
    uint32_t ppuSize;        // PPU binary size
    uint32_t apuSize;        // APU binary size
    uint32_t cpuChecksum;    // CPU checksum (CRC32 or simple sum)
    uint32_t ppuChecksum;    // PPU checksum
    uint32_t apuChecksum;    // APU checksum
    uint32_t headerChecksum; // Header checksum
    uint8_t padding[96];     // Pad to 256 bytes
} ZPBHeader;
#pragma pack(pop)

// Simple checksum (32-bit sum)
static uint32_t calculate_checksum(uint8_t* data, size_t size) {
    uint32_t sum = 0;
    for (size_t i = 0; i < size; i++) {
        sum += data[i];
    }
    return sum;
}

// Read entire file
static uint8_t* read_file(const char* filename, size_t* size) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", filename);
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    *size = ftell(f);
    fseek(f, 0, SEEK_SET);

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

// Extract component to file
static int extract_component(const char* base_name, const char* suffix,
                            uint8_t* data, size_t size) {
    char filename[256];
    snprintf(filename, sizeof(filename), "%s_%s.bin", base_name, suffix);

    FILE* f = fopen(filename, "wb");
    if (!f) {
        fprintf(stderr, "Error: Cannot create file '%s'\n", filename);
        return 0;
    }

    if (fwrite(data, 1, size, f) != size) {
        fprintf(stderr, "Error: Failed to write file '%s'\n", filename);
        fclose(f);
        return 0;
    }

    fclose(f);
    printf("  Extracted: %s (%zu bytes)\n", filename, size);
    return 1;
}

int main(int argc, char* argv[]) {
    const char* input_file = NULL;
    int verbose = 0;
    int extract = 0;
    int verify_checksums = 0;

    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0) {
            verbose = 1;
        } else if (strcmp(argv[i], "-x") == 0) {
            extract = 1;
        } else if (strcmp(argv[i], "-c") == 0) {
            verify_checksums = 1;
        } else if (argv[i][0] != '-') {
            input_file = argv[i];
        }
    }

    if (!input_file) {
        printf("ROM Inspector for ZeroPoint\n");
        printf("Usage: %s <rom.zpb> [options]\n", argv[0]);
        printf("\nOptions:\n");
        printf("  -v    Verbose mode\n");
        printf("  -x    Extract components\n");
        printf("  -c    Verify checksums\n");
        return 1;
    }

    // Read ROM file
    size_t file_size;
    uint8_t* data = read_file(input_file, &file_size);
    if (!data) return 1;

    // Check minimum size
    if (file_size < sizeof(ZPBHeader)) {
        fprintf(stderr, "Error: File too small to be a valid ROM\n");
        free(data);
        return 1;
    }

    // Parse header
    ZPBHeader* header = (ZPBHeader*)data;

    // Verify magic
    if (memcmp(header->magic, "ZPB", 3) != 0) {
        fprintf(stderr, "Error: Invalid ROM file (bad magic number)\n");
        free(data);
        return 1;
    }

    // Display header information
    printf("=== ZeroPoint ROM Information ===\n\n");
    printf("File: %s\n", input_file);
    printf("Size: %zu bytes (%.2f KB)\n", file_size, file_size / 1024.0);
    printf("\n");

    printf("--- Header ---\n");
    printf("Magic:        %c%c%c (valid)\n", header->magic[0], header->magic[1], header->magic[2]);
    printf("Version:      %d\n", header->version);
    printf("Flags:        0x%02X\n", header->flags);
    printf("Title:        %s\n", header->title);
    printf("Developer:    %s\n", header->developer);
    printf("Entry Point:  $%06X\n", header->entryPoint);
    printf("\n");

    printf("--- Component Sizes ---\n");
    printf("CPU Binary:   %u bytes (%.2f KB)\n", header->cpuSize, header->cpuSize / 1024.0);
    printf("PPU Binary:   %u bytes (%.2f KB)\n", header->ppuSize, header->ppuSize / 1024.0);
    printf("APU Binary:   %u bytes (%.2f KB)\n", header->apuSize, header->apuSize / 1024.0);
    printf("Total:        %u bytes (%.2f KB)\n",
           header->cpuSize + header->ppuSize + header->apuSize,
           (header->cpuSize + header->ppuSize + header->apuSize) / 1024.0);
    printf("\n");

    // Calculate expected file size
    size_t expected_size = sizeof(ZPBHeader) + header->cpuSize +
                          header->ppuSize + header->apuSize;

    if (expected_size != file_size) {
        printf("WARNING: File size mismatch!\n");
        printf("  Expected: %zu bytes\n", expected_size);
        printf("  Actual:   %zu bytes\n", file_size);
        printf("  Difference: %zd bytes\n", (ssize_t)file_size - (ssize_t)expected_size);
        printf("\n");
    }

    // Verify checksums if requested
    if (verify_checksums) {
        printf("--- Checksum Verification ---\n");

        uint8_t* cpu_data = data + sizeof(ZPBHeader);
        uint8_t* ppu_data = cpu_data + header->cpuSize;
        uint8_t* apu_data = ppu_data + header->ppuSize;

        uint32_t cpu_sum = calculate_checksum(cpu_data, header->cpuSize);
        uint32_t ppu_sum = calculate_checksum(ppu_data, header->ppuSize);
        uint32_t apu_sum = calculate_checksum(apu_data, header->apuSize);

        printf("CPU: 0x%08X (header: 0x%08X) %s\n",
               cpu_sum, header->cpuChecksum,
               (cpu_sum == header->cpuChecksum) ? "✓" : "MISMATCH!");
        printf("PPU: 0x%08X (header: 0x%08X) %s\n",
               ppu_sum, header->ppuChecksum,
               (ppu_sum == header->ppuChecksum) ? "✓" : "MISMATCH!");
        printf("APU: 0x%08X (header: 0x%08X) %s\n",
               apu_sum, header->apuChecksum,
               (apu_sum == header->apuChecksum) ? "✓" : "MISMATCH!");
        printf("\n");
    }

    // Verbose mode
    if (verbose) {
        printf("--- Header Checksums ---\n");
        printf("CPU:     0x%08X\n", header->cpuChecksum);
        printf("PPU:     0x%08X\n", header->ppuChecksum);
        printf("APU:     0x%08X\n", header->apuChecksum);
        printf("Header:  0x%08X\n", header->headerChecksum);
        printf("\n");

        printf("--- Memory Layout ---\n");
        printf("Header:    0x%08zX - 0x%08zX (%zu bytes)\n",
               (size_t)0, sizeof(ZPBHeader), sizeof(ZPBHeader));
        printf("CPU Data:  0x%08zX - 0x%08zX (%u bytes)\n",
               sizeof(ZPBHeader), sizeof(ZPBHeader) + header->cpuSize, header->cpuSize);
        printf("PPU Data:  0x%08zX - 0x%08zX (%u bytes)\n",
               sizeof(ZPBHeader) + header->cpuSize,
               sizeof(ZPBHeader) + header->cpuSize + header->ppuSize,
               header->ppuSize);
        printf("APU Data:  0x%08zX - 0x%08zX (%u bytes)\n",
               sizeof(ZPBHeader) + header->cpuSize + header->ppuSize,
               sizeof(ZPBHeader) + header->cpuSize + header->ppuSize + header->apuSize,
               header->apuSize);
        printf("\n");
    }

    // Extract components if requested
    if (extract) {
        printf("--- Extracting Components ---\n");

        // Get base filename (remove extension)
        char base_name[256];
        strncpy(base_name, input_file, sizeof(base_name) - 1);
        char* dot = strrchr(base_name, '.');
        if (dot) *dot = '\0';

        uint8_t* cpu_data = data + sizeof(ZPBHeader);
        uint8_t* ppu_data = cpu_data + header->cpuSize;
        uint8_t* apu_data = ppu_data + header->ppuSize;

        extract_component(base_name, "cpu", cpu_data, header->cpuSize);
        extract_component(base_name, "ppu", ppu_data, header->ppuSize);
        extract_component(base_name, "apu", apu_data, header->apuSize);
        printf("\n");
    }

    printf("Inspection complete.\n");

    free(data);
    return 0;
}

/*
 * ZeroPoint ROM Builder (rombuilder)
 *
 * Combines CPU, PPU, and APU binaries into a single ROM file
 * with accompanying metadata file.
 *
 * Usage: rombuilder [options] -cpu <cpu.bin> -ppu <ppu.bin> -apu <apu.bin> -o <output.bin>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MAX_ROM_SIZE (16 * 1024 * 1024)  // 16 MB max ROM
#define MAX_COMPONENT_SIZE (512 * 1024)  // 512 KB max per component

// Default memory layout
#define DEFAULT_CPU_BASE 0x008000
#define DEFAULT_PPU_BASE 0x100000
#define DEFAULT_APU_BASE 0x200000

typedef struct {
    char* cpu_file;
    char* ppu_file;
    char* apu_file;
    char* output_file;
    uint32_t cpu_base;
    uint32_t ppu_base;
    uint32_t apu_base;
    uint32_t cpu_entry;
    int verbose;
} RomConfig;

void print_usage(const char* prog) {
    fprintf(stderr, "ZeroPoint ROM Builder\n\n");
    fprintf(stderr, "Usage: %s [options] -cpu <cpu.bin> -ppu <ppu.bin> -apu <apu.bin> -o <output.bin>\n\n", prog);
    fprintf(stderr, "Required:\n");
    fprintf(stderr, "  -cpu <file>       CPU binary (DEF88186 code)\n");
    fprintf(stderr, "  -ppu <file>       PPU binary (PPU microcode)\n");
    fprintf(stderr, "  -apu <file>       APU binary (APU code)\n");
    fprintf(stderr, "  -o <file>         Output ROM file\n\n");
    fprintf(stderr, "Optional:\n");
    fprintf(stderr, "  -cpu-base <addr>  CPU base address (default: 0x%06X)\n", DEFAULT_CPU_BASE);
    fprintf(stderr, "  -ppu-base <addr>  PPU base address (default: 0x%06X)\n", DEFAULT_PPU_BASE);
    fprintf(stderr, "  -apu-base <addr>  APU base address (default: 0x%06X)\n", DEFAULT_APU_BASE);
    fprintf(stderr, "  -entry <addr>     CPU entry point (default: cpu-base)\n");
    fprintf(stderr, "  -v                Verbose output\n");
    fprintf(stderr, "  -h, --help        Show this help\n\n");
    fprintf(stderr, "Examples:\n");
    fprintf(stderr, "  %s -cpu main.bin -ppu graphics.bin -apu audio.bin -o game.rom\n", prog);
    fprintf(stderr, "  %s -cpu main.bin -ppu gfx.bin -apu snd.bin -entry 0x8000 -o game.rom\n", prog);
}

uint32_t parse_address(const char* str) {
    if (strncmp(str, "0x", 2) == 0 || strncmp(str, "0X", 2) == 0) {
        return (uint32_t)strtoul(str, NULL, 16);
    } else {
        return (uint32_t)strtoul(str, NULL, 10);
    }
}

int parse_args(int argc, char** argv, RomConfig* config) {
    // Initialize with defaults
    memset(config, 0, sizeof(RomConfig));
    config->cpu_base = DEFAULT_CPU_BASE;
    config->ppu_base = DEFAULT_PPU_BASE;
    config->apu_base = DEFAULT_APU_BASE;
    config->cpu_entry = DEFAULT_CPU_BASE;  // Default to cpu_base
    config->verbose = 0;

    if (argc < 2) {
        print_usage(argv[0]);
        return 0;
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-cpu") == 0 && i + 1 < argc) {
            config->cpu_file = argv[++i];
        } else if (strcmp(argv[i], "-ppu") == 0 && i + 1 < argc) {
            config->ppu_file = argv[++i];
        } else if (strcmp(argv[i], "-apu") == 0 && i + 1 < argc) {
            config->apu_file = argv[++i];
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            config->output_file = argv[++i];
        } else if (strcmp(argv[i], "-cpu-base") == 0 && i + 1 < argc) {
            config->cpu_base = parse_address(argv[++i]);
        } else if (strcmp(argv[i], "-ppu-base") == 0 && i + 1 < argc) {
            config->ppu_base = parse_address(argv[++i]);
        } else if (strcmp(argv[i], "-apu-base") == 0 && i + 1 < argc) {
            config->apu_base = parse_address(argv[++i]);
        } else if (strcmp(argv[i], "-entry") == 0 && i + 1 < argc) {
            config->cpu_entry = parse_address(argv[++i]);
        } else if (strcmp(argv[i], "-v") == 0) {
            config->verbose = 1;
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            return 0;
        }
    }

    // Validate required arguments
    if (!config->cpu_file || !config->ppu_file || !config->apu_file || !config->output_file) {
        fprintf(stderr, "Error: Missing required arguments\n\n");
        print_usage(argv[0]);
        return 0;
    }

    return 1;
}

long get_file_size(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) return -1;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fclose(f);

    return size;
}

int load_binary(const char* filename, uint8_t* rom, uint32_t base_addr, uint32_t max_size, int verbose) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Error: Cannot open '%s'\n", filename);
        return 0;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size > max_size) {
        fprintf(stderr, "Error: '%s' is too large (%ld bytes, max %u)\n",
                filename, size, max_size);
        fclose(f);
        return 0;
    }

    if (base_addr + size > MAX_ROM_SIZE) {
        fprintf(stderr, "Error: '%s' would exceed ROM size (base: 0x%06X, size: %ld)\n",
                filename, base_addr, size);
        fclose(f);
        return 0;
    }

    size_t read = fread(rom + base_addr, 1, size, f);
    fclose(f);

    if (read != (size_t)size) {
        fprintf(stderr, "Error: Failed to read '%s'\n", filename);
        return 0;
    }

    if (verbose) {
        printf("  Loaded '%s': %ld bytes at 0x%06X\n", filename, size, base_addr);
    }

    return 1;
}

int write_metadata(const char* rom_file, RomConfig* config) {
    char meta_file[512];
    snprintf(meta_file, sizeof(meta_file), "%s", rom_file);

    // Replace .bin or .rom with .txt
    char* ext = strrchr(meta_file, '.');
    if (ext) {
        strcpy(ext, ".txt");
    } else {
        strcat(meta_file, ".txt");
    }

    FILE* f = fopen(meta_file, "w");
    if (!f) {
        fprintf(stderr, "Warning: Cannot create metadata file '%s'\n", meta_file);
        return 0;
    }

    fprintf(f, "# ZeroPoint ROM Metadata\n");
    fprintf(f, "# Generated by rombuilder\n\n");

    fprintf(f, "[ROM]\n");
    fprintf(f, "file = %s\n\n", rom_file);

    fprintf(f, "[CPU]\n");
    fprintf(f, "binary = %s\n", config->cpu_file);
    fprintf(f, "base = 0x%06X\n", config->cpu_base);
    fprintf(f, "size = %ld\n", get_file_size(config->cpu_file));
    fprintf(f, "entry = 0x%06X\n\n", config->cpu_entry);

    fprintf(f, "[PPU]\n");
    fprintf(f, "binary = %s\n", config->ppu_file);
    fprintf(f, "base = 0x%06X\n", config->ppu_base);
    fprintf(f, "size = %ld\n\n", get_file_size(config->ppu_file));

    fprintf(f, "[APU]\n");
    fprintf(f, "binary = %s\n", config->apu_file);
    fprintf(f, "base = 0x%06X\n", config->apu_base);
    fprintf(f, "size = %ld\n\n", get_file_size(config->apu_file));

    fprintf(f, "[Memory Layout]\n");
    fprintf(f, "# Component   Base        End         Size\n");
    fprintf(f, "# CPU         0x%06X    0x%06X    %ld bytes\n",
            config->cpu_base,
            config->cpu_base + (uint32_t)get_file_size(config->cpu_file),
            get_file_size(config->cpu_file));
    fprintf(f, "# PPU         0x%06X    0x%06X    %ld bytes\n",
            config->ppu_base,
            config->ppu_base + (uint32_t)get_file_size(config->ppu_file),
            get_file_size(config->ppu_file));
    fprintf(f, "# APU         0x%06X    0x%06X    %ld bytes\n",
            config->apu_base,
            config->apu_base + (uint32_t)get_file_size(config->apu_file),
            get_file_size(config->apu_file));

    fclose(f);

    if (config->verbose) {
        printf("\nMetadata written to '%s'\n", meta_file);
    }

    return 1;
}

int main(int argc, char** argv) {
    RomConfig config;

    if (!parse_args(argc, argv, &config)) {
        return 1;
    }

    printf("ZeroPoint ROM Builder\n");
    printf("=====================\n\n");

    // Allocate ROM buffer (zero-filled)
    uint8_t* rom = calloc(MAX_ROM_SIZE, 1);
    if (!rom) {
        fprintf(stderr, "Error: Cannot allocate ROM buffer\n");
        return 1;
    }

    if (config.verbose) {
        printf("ROM Configuration:\n");
        printf("  CPU Base:  0x%06X\n", config.cpu_base);
        printf("  PPU Base:  0x%06X\n", config.ppu_base);
        printf("  APU Base:  0x%06X\n", config.apu_base);
        printf("  CPU Entry: 0x%06X\n\n", config.cpu_entry);
        printf("Loading components:\n");
    }

    // Load binaries
    if (!load_binary(config.cpu_file, rom, config.cpu_base, MAX_COMPONENT_SIZE, config.verbose)) {
        free(rom);
        return 1;
    }

    if (!load_binary(config.ppu_file, rom, config.ppu_base, MAX_COMPONENT_SIZE, config.verbose)) {
        free(rom);
        return 1;
    }

    if (!load_binary(config.apu_file, rom, config.apu_base, MAX_COMPONENT_SIZE, config.verbose)) {
        free(rom);
        return 1;
    }

    // Calculate actual ROM size (highest byte written + 1)
    uint32_t rom_size = config.apu_base + get_file_size(config.apu_file);
    if (config.cpu_base + get_file_size(config.cpu_file) > rom_size) {
        rom_size = config.cpu_base + get_file_size(config.cpu_file);
    }
    if (config.ppu_base + get_file_size(config.ppu_file) > rom_size) {
        rom_size = config.ppu_base + get_file_size(config.ppu_file);
    }

    // Write ROM file
    FILE* out = fopen(config.output_file, "wb");
    if (!out) {
        fprintf(stderr, "Error: Cannot create output file '%s'\n", config.output_file);
        free(rom);
        return 1;
    }

    fwrite(rom, 1, rom_size, out);
    fclose(out);
    free(rom);

    printf("\nROM built successfully!\n");
    printf("  Output: %s\n", config.output_file);
    printf("  Size: %u bytes (%.2f KB)\n", rom_size, rom_size / 1024.0);

    // Write metadata file
    write_metadata(config.output_file, &config);

    printf("\nROM is ready to use with the ZeroPoint emulator.\n");

    return 0;
}

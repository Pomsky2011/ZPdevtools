/*
 * WAV to MMP Converter
 * Converts mono/stereo WAV files to ZeroPoint APU MMP format
 *
 * Usage: wav2mmp input.wav output.inc [options]
 *
 * Output format:
 *   - SST blocks (16 bytes each: 4-byte header + 12 PCM samples)
 *   - STL entry (4 bytes: sample address + loop address)
 *   - Assembly .inc file ready to include in APU programs
 */

#include <stdio.h>
#include <stdlib.h>
#include "compat.h"
#include <string.h>

#define MAX_SAMPLES 10000000  /* 10 million samples max */

typedef struct {
    char chunk_id[4];      /* "RIFF" */
    uint32_t chunk_size;
    char format[4];        /* "WAVE" */
} WAVHeader;

typedef struct {
    char subchunk_id[4];   /* "fmt " */
    uint32_t subchunk_size;
    uint16_t audio_format; /* 1 = PCM */
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
} WAVFormat;

typedef struct {
    char subchunk_id[4];   /* "data" */
    uint32_t subchunk_size;
} WAVDataHeader;

typedef struct {
    uint8_t data[16];
} SSTBlock;

void print_usage(const char *prog_name) {
    fprintf(stderr, "Usage: %s <input.wav> <output.inc> [options]\n", prog_name);
    fprintf(stderr, "\nOptions:\n");
    fprintf(stderr, "  --sst-addr <addr>  SST base address (default: 0x9000)\n");
    fprintf(stderr, "  --stl-addr <addr>  STL base address (default: 0x7000)\n");
    fprintf(stderr, "  --label <prefix>   Label prefix (default: sfx)\n");
    fprintf(stderr, "  --no-loop          Don't loop sample (play once)\n");
    fprintf(stderr, "\nConverts WAV files to ZeroPoint APU MMP format.\n");
}

uint16_t read_uint16_le(FILE *f) {
    uint8_t buf[2];
    fread(buf, 1, 2, f);
    return buf[0] | (buf[1] << 8);
}

uint32_t read_uint32_le(FILE *f) {
    uint8_t buf[4];
    fread(buf, 1, 4, f);
    return buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
}

int16_t clamp_8bit(int16_t value) {
    if (value > 127) return 127;
    if (value < -128) return -128;
    return value;
}

int read_wav(const char *filename, int16_t **samples, int *sample_count, int *sample_rate) {
    FILE *f;
    WAVHeader header;
    WAVFormat fmt;
    int found_fmt;
    WAVDataHeader data_hdr;
    int found_data;
    int bytes_per_sample;
    int num_samples;
    int i;
    int ch;
    int32_t sample_sum;
    int16_t sample;
    uint8_t u8_sample;

    f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Error: Cannot open file: %s\n", filename);
        return 0;
    }

    /* Read RIFF header */
    fread(&header.chunk_id, 1, 4, f);
    if (memcmp(header.chunk_id, "RIFF", 4) != 0) {
        fprintf(stderr, "Error: Not a valid WAV file (missing RIFF)\n");
        fclose(f);
        return 0;
    }

    header.chunk_size = read_uint32_le(f);
    fread(&header.format, 1, 4, f);
    if (memcmp(header.format, "WAVE", 4) != 0) {
        fprintf(stderr, "Error: Not a valid WAV file (missing WAVE)\n");
        fclose(f);
        return 0;
    }

    /* Find and read fmt chunk */
    found_fmt = 0;
    while (!found_fmt && !feof(f)) {
        fread(&fmt.subchunk_id, 1, 4, f);
        fmt.subchunk_size = read_uint32_le(f);

        if (memcmp(fmt.subchunk_id, "fmt ", 4) == 0) {
            found_fmt = 1;
            fmt.audio_format = read_uint16_le(f);
            fmt.num_channels = read_uint16_le(f);
            fmt.sample_rate = read_uint32_le(f);
            fmt.byte_rate = read_uint32_le(f);
            fmt.block_align = read_uint16_le(f);
            fmt.bits_per_sample = read_uint16_le(f);

            /* Skip any extra fmt data */
            if (fmt.subchunk_size > 16) {
                fseek(f, fmt.subchunk_size - 16, SEEK_CUR);
            }
        } else {
            /* Skip unknown chunk */
            fseek(f, fmt.subchunk_size, SEEK_CUR);
        }
    }

    if (!found_fmt) {
        fprintf(stderr, "Error: No fmt chunk found\n");
        fclose(f);
        return 0;
    }

    printf("Input WAV: %dch, %d-bit, %d Hz\n",
           fmt.num_channels, fmt.bits_per_sample, fmt.sample_rate);

    if (fmt.num_channels > 2) {
        fprintf(stderr, "Warning: Input is %d channels, will mix to mono\n", fmt.num_channels);
    }

    if (fmt.bits_per_sample != 16 && fmt.bits_per_sample != 8) {
        fprintf(stderr, "Error: Only 8-bit and 16-bit samples supported\n");
        fclose(f);
        return 0;
    }

    /* Find and read data chunk */
    found_data = 0;
    while (!found_data && !feof(f)) {
        fread(&data_hdr.subchunk_id, 1, 4, f);
        data_hdr.subchunk_size = read_uint32_le(f);

        if (memcmp(data_hdr.subchunk_id, "data", 4) == 0) {
            found_data = 1;
        } else {
            /* Skip unknown chunk */
            fseek(f, data_hdr.subchunk_size, SEEK_CUR);
        }
    }

    if (!found_data) {
        fprintf(stderr, "Error: No data chunk found\n");
        fclose(f);
        return 0;
    }

    /* Calculate number of samples */
    bytes_per_sample = fmt.bits_per_sample / 8;
    num_samples = data_hdr.subchunk_size / (bytes_per_sample * fmt.num_channels);

    if (num_samples > MAX_SAMPLES) {
        fprintf(stderr, "Error: Too many samples (%d, max %d)\n", num_samples, MAX_SAMPLES);
        fclose(f);
        return 0;
    }

    printf("  %d samples\n", num_samples);

    /* Allocate sample buffer */
    *samples = (int16_t *)malloc(num_samples * sizeof(int16_t));
    if (!*samples) {
        fprintf(stderr, "Error: Cannot allocate memory for samples\n");
        fclose(f);
        return 0;
    }

    /* Read and convert samples */
    for (i = 0; i < num_samples; i++) {
        sample_sum = 0;

        for (ch = 0; ch < fmt.num_channels; ch++) {
            if (fmt.bits_per_sample == 16) {
                sample = (int16_t)read_uint16_le(f);
            } else {  /* 8-bit */
                u8_sample = fgetc(f);
                sample = ((int16_t)u8_sample - 128) * 256;
            }

            sample_sum += sample;
        }

        /* Average if multiple channels */
        (*samples)[i] = sample_sum / fmt.num_channels;
    }

    *sample_count = num_samples;
    *sample_rate = fmt.sample_rate;

    fclose(f);
    return 1;
}

void downsample_to_8bit(int16_t *samples_16bit, int8_t *samples_8bit, int count) {
    int i;
    int16_t sample_8;

    for (i = 0; i < count; i++) {
        /* Use arithmetic right shift to match Python's floor division for signed values */
        sample_8 = samples_16bit[i] >> 8;
        samples_8bit[i] = clamp_8bit(sample_8);
    }
}

int create_sst_blocks(int8_t *samples_8bit, int sample_count, SSTBlock **blocks, int loop) {
    int padded_count;
    int8_t *padded_samples;
    int num_blocks;
    int block_idx;
    SSTBlock *block;
    int is_final;
    uint8_t Y;
    uint8_t L;
    int i;
    int sample_idx;
    int8_t sample_signed;
    uint8_t sample_unsigned;

    /* Pad to multiple of 12 */
    padded_count = ((sample_count + 11) / 12) * 12;
    padded_samples = (int8_t *)calloc(padded_count, sizeof(int8_t));
    memcpy(padded_samples, samples_8bit, sample_count);

    num_blocks = padded_count / 12;
    *blocks = (SSTBlock *)malloc(num_blocks * sizeof(SSTBlock));

    for (block_idx = 0; block_idx < num_blocks; block_idx++) {
        block = &(*blocks)[block_idx];
        memset(block->data, 0, 16);

        /* Header byte 0: Loop count */
        block->data[0] = loop ? 0xFF : 0x00;

        /* Header byte 1: Y (loop start) and L (config) */
        is_final = (block_idx == num_blocks - 1);
        Y = 0;  /* Loop from sample 0 */
        L = is_final ? 0x4 : 0x0;  /* W bit (bit 2) */
        block->data[1] = (Y << 4) | L;

        /* Header bytes 2-3: Clamping (disabled) */
        block->data[2] = 0x00;  /* V=0, U=0 */
        block->data[3] = 0x00;  /* T=0, S=0 */

        /* Sample data (bytes 4-15) */
        for (i = 0; i < 12; i++) {
            sample_idx = block_idx * 12 + i;
            sample_signed = padded_samples[sample_idx];
            sample_unsigned = (uint8_t)sample_signed;
            block->data[4 + i] = sample_unsigned;
        }
    }

    free(padded_samples);
    return num_blocks;
}

void generate_inc_file(const char *output_path, SSTBlock *blocks, int num_blocks,
                       uint16_t sst_address, uint16_t stl_address, const char *label_prefix) {
    FILE *f;
    int block_idx;
    SSTBlock *block;
    uint16_t block_offset;
    int i;

    f = fopen(output_path, "w");
    if (!f) {
        fprintf(stderr, "Error: Cannot open output file: %s\n", output_path);
        return;
    }

    /* Header comments */
    fprintf(f, "; Generated by wav2mmp\n");
    fprintf(f, "; SST blocks: %d (%d bytes)\n", num_blocks, num_blocks * 16);
    fprintf(f, "; SST address: $%04X\n", sst_address);
    fprintf(f, "; STL address: $%04X\n", stl_address);
    fprintf(f, "\n");

    /* SST data label */
    fprintf(f, "%s_sst_data:\n", label_prefix);

    /* Generate SST blocks */
    for (block_idx = 0; block_idx < num_blocks; block_idx++) {
        block = &blocks[block_idx];
        block_offset = block_idx * 16;

        fprintf(f, "    ; Block %d (offset $%02X)\n", block_idx, block_offset);
        fprintf(f, "    ; Header: loops=$%02X, Y/L=$%02X, V/U=$%02X, T/S=$%02X\n",
                block->data[0], block->data[1], block->data[2], block->data[3]);

        /* Set DB to block offset */
        fprintf(f, "    SDB $%02X\n", block_offset);
        fprintf(f, "    SBF 0\n");

        /* Write all 16 bytes using STA */
        for (i = 0; i < 16; i++) {
            fprintf(f, "    SCR X, %d\n", block->data[i]);
            fprintf(f, "    STA X, $%02X\n", block_offset + i);
        }

        fprintf(f, "\n");
    }

    /* STL entry */
    fprintf(f, "%s_stl_entry:\n", label_prefix);
    fprintf(f, "    ; Sample data address = $%04X\n", sst_address);
    fprintf(f, "    WRH $%02X\n", sst_address >> 8);
    fprintf(f, "    WRL $%02X\n", sst_address & 0xFF);
    fprintf(f, "    ; Loop address = 0 (loop entire sample)\n");
    fprintf(f, "    WRH $00\n");
    fprintf(f, "    WRL $00\n");
    fprintf(f, "\n");

    /* Usage example */
    fprintf(f, "; Usage example:\n");
    fprintf(f, ";   SDP $%02X      ; Set DP to SST page\n", sst_address >> 8);
    fprintf(f, ";   SDB $00\n");
    fprintf(f, ";   %s_sst_data        ; Write SST blocks\n", label_prefix);
    fprintf(f, ";\n");
    fprintf(f, ";   SDP $%02X      ; Set DP to STL page\n", stl_address >> 8);
    fprintf(f, ";   SDB $00\n");
    fprintf(f, ";   %s_stl_entry       ; Write STL entry\n", label_prefix);
    fprintf(f, ";\n");
    fprintf(f, ";   ; Configure channel 0 to play\n");
    fprintf(f, ";   SDP $00              ; MMP registers\n");
    fprintf(f, ";   SDB $00\n");
    fprintf(f, ";   WRH $10              ; Pitch = 0x1000 (1.0x)\n");
    fprintf(f, ";   WRL $00\n");
    fprintf(f, ";   SDB $20\n");
    fprintf(f, ";   SBF 0\n");
    fprintf(f, ";   SCR X, 128           ; Volume = 128\n");
    fprintf(f, ";   STA X, $20\n");
    fprintf(f, ";   SDB $54\n");
    fprintf(f, ";   WRH $%02X      ; STL address -> starts playback\n", stl_address >> 8);
    fprintf(f, ";   WRL $%02X\n", stl_address & 0xFF);

    fclose(f);

    printf("Generated %s\n", output_path);
    printf("  Blocks: %d\n", num_blocks);
    printf("  Total size: %d bytes\n", num_blocks * 16);
}

uint16_t parse_address(const char *str) {
    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        return (uint16_t)strtol(str, NULL, 16);
    }
    return (uint16_t)strtol(str, NULL, 10);
}

int main(int argc, char* argv[]) {
    const char *input_file;
    const char *output_file;
    uint16_t sst_addr;
    uint16_t stl_addr;
    const char *label;
    int loop;
    int i;
    int16_t *samples_16bit;
    int sample_count;
    int sample_rate;
    int8_t *samples_8bit;
    SSTBlock *blocks;
    int num_blocks;

    if (argc < 3) {
        print_usage(argv[0]);
        return 1;
    }

    input_file = argv[1];
    output_file = argv[2];
    sst_addr = 0x9000;
    stl_addr = 0x7000;
    label = "sfx";
    loop = 1;

    /* Parse optional arguments */
    for (i = 3; i < argc; i++) {
        if (strcmp(argv[i], "--sst-addr") == 0 && i + 1 < argc) {
            sst_addr = parse_address(argv[++i]);
        } else if (strcmp(argv[i], "--stl-addr") == 0 && i + 1 < argc) {
            stl_addr = parse_address(argv[++i]);
        } else if (strcmp(argv[i], "--label") == 0 && i + 1 < argc) {
            label = argv[++i];
        } else if (strcmp(argv[i], "--no-loop") == 0) {
            loop = 0;
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }

    printf("Converting %s -> %s\n", input_file, output_file);

    /* Read WAV file */
    samples_16bit = NULL;
    sample_count = 0;
    sample_rate = 0;

    if (!read_wav(input_file, &samples_16bit, &sample_count, &sample_rate)) {
        return 1;
    }

    printf("Read %d samples\n", sample_count);

    /* Convert to 8-bit */
    samples_8bit = (int8_t *)malloc(sample_count * sizeof(int8_t));
    downsample_to_8bit(samples_16bit, samples_8bit, sample_count);
    printf("Converted to 8-bit: %d samples\n", sample_count);

    /* Create SST blocks */
    blocks = NULL;
    num_blocks = create_sst_blocks(samples_8bit, sample_count, &blocks, loop);
    printf("Created %d SST blocks\n", num_blocks);

    /* Generate .inc file */
    generate_inc_file(output_file, blocks, num_blocks, sst_addr, stl_addr, label);

    printf("\nConversion complete!\n");

    /* Cleanup */
    free(samples_16bit);
    free(samples_8bit);
    free(blocks);

    return 0;
}

/*
 * rominspect - inspector for ZeroPoint build artifacts.
 *
 * Auto-detects and reports on the three formats the toolchain produces:
 *
 *   ELF32   (magic 7f 'E' 'L' 'F')  - zplink output.  Lists PT_LOAD segments
 *                                     and section headers (.text/.ppu/.apu/
 *                                     .data/...), even when a section is
 *                                     unnamed.  This is where per-component
 *                                     breakdown lives.
 *   ZPB ROM (magic "ZPB")           - zpbuild output, matching the emulator's
 *                                     include/rom.h: 64-byte header + flat
 *                                     payload + optional "ZPSG" signed trailer.
 *   raw                             - anything else: a headerless blob such as
 *                                     a rombuilder flat image or a bare
 *                                     cpuasm/ppuasm/apuasm object (even unnamed).
 *
 * Usage: rominspect <file> [-v] [-x] [-c]
 *   -v   Verbose (offsets, flags, digests)
 *   -x   Extract components (ELF sections / ZPB payload) to files
 *   -c   Check integrity (ZPB: recompute SHA-256 vs the signed digest;
 *        ELF/raw: 32-bit checksum)
 */

#include <stdio.h>
#include <stdlib.h>
#include "compat.h"
#include <string.h>
#include "zpsha256.h"

/* ---- little-endian readers (format-agnostic; no packed structs) ------- */
static unsigned int get16(const uint8_t *p)
{ return (unsigned int)p[0] | ((unsigned int)p[1] << 8); }

static uint32_t get32(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8)
         | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

typedef struct {
    int verbose;
    int extract;
    int check;
    const char *input;
    char base[256];
} Opts;

static uint32_t checksum(const uint8_t *data, size_t size)
{
    uint32_t sum = 0; size_t i;
    for (i = 0; i < size; i++) sum += data[i];
    return sum;
}

static uint8_t *read_file(const char *filename, size_t *size)
{
    FILE *f; uint8_t *buf;
    f = fopen(filename, "rb");
    if (!f) { fprintf(stderr, "Error: cannot open '%s'\n", filename); return NULL; }
    fseek(f, 0, SEEK_END); *size = (size_t)ftell(f); fseek(f, 0, SEEK_SET);
    buf = (uint8_t*)malloc(*size ? *size : 1);
    if (!buf) { fprintf(stderr, "Error: out of memory\n"); fclose(f); return NULL; }
    if (*size && fread(buf, 1, *size, f) != *size) {
        fprintf(stderr, "Error: failed to read '%s'\n", filename); free(buf); fclose(f); return NULL;
    }
    fclose(f);
    return buf;
}

/* Write a region to <base>_<tag>.bin (tag sanitized: no leading dot/slashes). */
static void extract_region(const char *base, const char *tag,
                           const uint8_t *data, size_t size)
{
    char name[320]; const char *t = tag; FILE *f;
    while (*t == '.' || *t == '/' || *t == '\\') t++;      /* drop leading .  */
    if (!*t) t = "blob";
    sprintf(name, "%s_%.200s.bin", base, t);
    f = fopen(name, "wb");
    if (!f) { fprintf(stderr, "  Error: cannot create '%s'\n", name); return; }
    if (size) fwrite(data, 1, size, f);
    fclose(f);
    printf("  Extracted: %s (%lu bytes)\n", name, (unsigned long)size);
}

static void print_hex_preview(const uint8_t *d, size_t size)
{
    size_t n = size < 16 ? size : 16, i;
    printf("First bytes:  ");
    for (i = 0; i < n; i++) printf("%02X ", d[i]);
    printf("\n");
}

/* ====================================================================== */
/* ZPB ROM (authoritative layout = ZeroPoint/include/rom.h)               */
/* ====================================================================== */
static int inspect_zpb(const uint8_t *d, size_t size, Opts *o)
{
    unsigned int version, flags, headerSize;
    uint32_t romSize, entry;
    char title[33], dev[17];
    size_t payload_off, trailer_off, expected;
    int signed_rom = 0;

    if (size < 64) { fprintf(stderr, "Error: file too small for a ZPB header\n"); return 1; }

    version    = d[4];
    flags      = d[5];
    headerSize = get16(d + 6);
    romSize    = get32(d + 8);
    entry      = get32(d + 12);
    memcpy(title, d + 16, 32); title[32] = '\0';
    memcpy(dev,   d + 48, 16); dev[16]   = '\0';
    if (headerSize == 0) headerSize = 64;
    payload_off = headerSize;

    printf("=== ZeroPoint ROM (ZPB) ===\n\n");
    printf("File:         %s\n", o->input);
    printf("Size:         %lu bytes (%.2f KB)\n\n", (unsigned long)size, size / 1024.0);
    printf("--- Header ---\n");
    printf("Magic:        ZPB (valid)\n");
    printf("Version:      %u\n", version);
    printf("Flags:        0x%02X%s\n", flags, (flags & 1) ? " (signed)" : "");
    printf("Header size:  %u bytes\n", headerSize);
    printf("Title:        \"%s\"\n", title);
    printf("Developer:    \"%s\"\n", dev);
    printf("Entry point:  $%06lX\n", (unsigned long)entry);
    printf("Payload:      %lu bytes @ offset %lu\n\n",
           (unsigned long)romSize, (unsigned long)payload_off);

    /* Signed trailer, if any, sits right after the payload. */
    trailer_off = payload_off + romSize;
    if (size >= trailer_off + 8 &&
        d[trailer_off]=='Z' && d[trailer_off+1]=='P' &&
        d[trailer_off+2]=='S' && d[trailer_off+3]=='G') {
        unsigned int keybits = (unsigned int)d[trailer_off+5] * 64;
        unsigned int siglen  = get16(d + trailer_off + 6);
        signed_rom = 1;
        printf("--- Signature (ZPSG) ---\n");
        printf("Present:      yes\n");
        printf("Algorithm:    RSA-%u PKCS#1 v1.5 / SHA-256\n", keybits ? keybits : 2048);
        printf("Signature:    %u bytes\n", siglen);
        if (o->verbose) {
            int i; const uint8_t *dg = d + trailer_off + 8;
            printf("Stored digest: ");
            for (i = 0; i < 8; i++) printf("%02x", dg[i]);
            printf("...\n");
        }
        printf("\n");
        expected = trailer_off + 8 + 32 + siglen;
    } else {
        printf("--- Signature ---\n");
        printf("Present:      no (unsigned payload)\n\n");
        expected = trailer_off;
    }

    if (expected != size) {
        printf("WARNING: size mismatch (header says %lu, file is %lu)\n\n",
               (unsigned long)expected, (unsigned long)size);
    }

    printf("NOTE: the payload is a flat located image; per-segment (.text/.ppu/\n");
    printf("      .apu) breakdown is in the matching .elf, not the ROM header.\n\n");

    if (o->check && signed_rom) {
        unsigned char digest[32]; int i, ok = 1;
        const uint8_t *stored = d + trailer_off + 8;
        /* Header and payload are contiguous in the file, so hash them in one go
           (identical bytes to zpbuild's header||payload digest). */
        zp_sha256_buf(d, (unsigned long)(headerSize + romSize), digest);
        for (i = 0; i < 32; i++) if (digest[i] != stored[i]) { ok = 0; break; }
        printf("--- Integrity ---\n");
        printf("SHA-256(header+payload) vs stored digest: %s\n\n", ok ? "MATCH" : "MISMATCH!");
    } else if (o->check) {
        printf("--- Integrity ---\n");
        printf("Payload checksum: 0x%08lX (unsigned ROM, no stored digest)\n\n",
               (unsigned long)checksum(d + payload_off, romSize));
    }

    if (o->extract) {
        printf("--- Extracting ---\n");
        extract_region(o->base, "payload", d + payload_off, romSize);
        printf("\n");
    }
    return 0;
}

/* ====================================================================== */
/* ELF32 (zplink output)                                                  */
/* ====================================================================== */
static const char *sh_type_name(uint32_t t)
{
    switch (t) {
        case 0:  return "NULL";
        case 1:  return "PROGBITS";
        case 3:  return "STRTAB";
        default: return "?";
    }
}

static void flags_str(uint32_t f, char *out, int is_section)
{
    /* Segment p_flags: R=4 W=2 X=1.  Section sh_flags: WRITE=1 ALLOC=2 EXEC=4. */
    int r, w, x;
    if (is_section) { w = (f & 1); r = (f & 2); x = (f & 4); }
    else            { x = (f & 1); w = (f & 2); r = (f & 4); }
    out[0] = r ? 'R' : '-';
    out[1] = w ? 'W' : '-';
    out[2] = x ? 'X' : '-';
    out[3] = '\0';
}

static int inspect_elf(const uint8_t *d, size_t size, Opts *o)
{
    unsigned int e_type, e_machine, e_phentsize, e_phnum, e_shentsize, e_shnum, e_shstrndx;
    uint32_t e_entry, e_phoff, e_shoff, strtab_off = 0;
    unsigned int i;
    char fl[4];

    if (size < 52) { fprintf(stderr, "Error: file too small for an ELF header\n"); return 1; }
    if (d[4] != 1 || d[5] != 1) {
        fprintf(stderr, "Error: only 32-bit little-endian ELF is supported\n"); return 1;
    }
    e_type      = get16(d + 16);
    e_machine   = get16(d + 18);
    e_entry     = get32(d + 24);
    e_phoff     = get32(d + 28);
    e_shoff     = get32(d + 32);
    e_phentsize = get16(d + 42);
    e_phnum     = get16(d + 44);
    e_shentsize = get16(d + 46);
    e_shnum     = get16(d + 48);
    e_shstrndx  = get16(d + 50);

    printf("=== ZeroPoint image (ELF32) ===\n\n");
    printf("File:         %s\n", o->input);
    printf("Size:         %lu bytes (%.2f KB)\n\n", (unsigned long)size, size / 1024.0);
    printf("--- ELF header ---\n");
    printf("Type:         %s\n", e_type == 2 ? "EXEC" : "?");
    if (e_machine == 0x5A50) printf("Machine:      0x5A50 (ZP, ZeroPoint)\n");
    else                     printf("Machine:      0x%04X\n", e_machine);
    printf("Entry point:  $%06lX\n", (unsigned long)e_entry);
    printf("Segments:     %u   Sections: %u\n\n", e_phnum, e_shnum);

    /* Program headers (load segments). */
    printf("--- Load segments (program headers) ---\n");
    printf("  #  type  flags  vaddr      offset     filesz\n");
    for (i = 0; i < e_phnum; i++) {
        const uint8_t *ph = d + e_phoff + (size_t)i * e_phentsize;
        uint32_t p_type, p_offset, p_vaddr, p_filesz, p_flags;
        if ((size_t)(ph - d) + 32 > size) { printf("  (truncated program headers)\n"); break; }
        p_type   = get32(ph + 0);
        p_offset = get32(ph + 4);
        p_vaddr  = get32(ph + 8);
        p_filesz = get32(ph + 16);
        p_flags  = get32(ph + 24);
        flags_str(p_flags, fl, 0);
        printf("  %-2u %-5s %-5s  0x%06lX   0x%06lX   %lu\n", i,
               p_type == 1 ? "LOAD" : "?", fl,
               (unsigned long)p_vaddr, (unsigned long)p_offset, (unsigned long)p_filesz);
    }
    printf("\n");

    /* Locate the section-header string table. */
    if (e_shnum && e_shstrndx < e_shnum && e_shoff) {
        const uint8_t *sh = d + e_shoff + (size_t)e_shstrndx * e_shentsize;
        if ((size_t)(sh - d) + 40 <= size) strtab_off = get32(sh + 16);
    }

    /* Section headers - name each, even if the name index is bogus/zero. */
    if (e_shnum && e_shoff) {
        printf("--- Sections ---\n");
        printf("  #  name          type      addr       offset     size    flags\n");
        for (i = 0; i < e_shnum; i++) {
            const uint8_t *sh = d + e_shoff + (size_t)i * e_shentsize;
            uint32_t sh_name, sh_type, sh_flags, sh_addr, sh_offset, sh_size;
            const char *nm = "(unnamed)";
            if ((size_t)(sh - d) + 40 > size) { printf("  (truncated section headers)\n"); break; }
            sh_name   = get32(sh + 0);
            sh_type   = get32(sh + 4);
            sh_flags  = get32(sh + 8);
            sh_addr   = get32(sh + 12);
            sh_offset = get32(sh + 16);
            sh_size   = get32(sh + 20);
            if (strtab_off && sh_name && (size_t)strtab_off + sh_name < size &&
                d[strtab_off + sh_name] != '\0')
                nm = (const char *)(d + strtab_off + sh_name);
            flags_str(sh_flags, fl, 1);
            printf("  %-2u %-13s %-9s 0x%06lX   0x%06lX   %-7lu %s\n", i, nm,
                   sh_type_name(sh_type), (unsigned long)sh_addr,
                   (unsigned long)sh_offset, (unsigned long)sh_size, fl);
        }
        printf("\n");
    } else {
        printf("--- Sections ---\n  (none; inspecting load segments only)\n\n");
    }

    if (o->check) {
        printf("--- Integrity ---\n");
        printf("File checksum: 0x%08lX\n\n", (unsigned long)checksum(d, size));
    }

    /* Extract: prefer named PROGBITS sections; fall back to load segments. */
    if (o->extract) {
        int wrote = 0;
        printf("--- Extracting ---\n");
        if (e_shnum && e_shoff && strtab_off) {
            for (i = 0; i < e_shnum; i++) {
                const uint8_t *sh = d + e_shoff + (size_t)i * e_shentsize;
                uint32_t sh_name, sh_type, sh_offset, sh_size; char tag[32];
                if ((size_t)(sh - d) + 40 > size) break;
                sh_name = get32(sh + 0); sh_type = get32(sh + 4);
                sh_offset = get32(sh + 16); sh_size = get32(sh + 20);
                if (sh_type != 1 || sh_size == 0) continue;          /* PROGBITS only */
                if ((size_t)sh_offset + sh_size > size) continue;
                if (sh_name && (size_t)strtab_off + sh_name < size)
                    extract_region(o->base, (const char *)(d + strtab_off + sh_name),
                                   d + sh_offset, sh_size);
                else { sprintf(tag, "sec%u", i); extract_region(o->base, tag, d + sh_offset, sh_size); }
                wrote = 1;
            }
        }
        if (!wrote) {                                                /* unnamed: by segment */
            for (i = 0; i < e_phnum; i++) {
                const uint8_t *ph = d + e_phoff + (size_t)i * e_phentsize;
                uint32_t p_type, p_offset, p_filesz; char tag[32];
                if ((size_t)(ph - d) + 32 > size) break;
                p_type = get32(ph + 0); p_offset = get32(ph + 4); p_filesz = get32(ph + 16);
                if (p_type != 1 || p_filesz == 0) continue;
                if ((size_t)p_offset + p_filesz > size) continue;
                sprintf(tag, "seg%u", i);
                extract_region(o->base, tag, d + p_offset, p_filesz);
            }
        }
        printf("\n");
    }
    return 0;
}

/* ====================================================================== */
/* Raw / unnamed blob (rombuilder flat image, bare asm object, ...)       */
/* ====================================================================== */
static int inspect_raw(const uint8_t *d, size_t size, Opts *o)
{
    printf("=== Raw binary (no ZPB/ELF header) ===\n\n");
    printf("File:         %s\n", o->input);
    printf("Size:         %lu bytes (%.2f KB)\n", (unsigned long)size, size / 1024.0);
    printf("Checksum:     0x%08lX\n", (unsigned long)checksum(d, size));
    if (size) print_hex_preview(d, size);
    printf("\n");
    printf("This looks like an unnamed object: a rombuilder flat image, or a bare\n");
    printf("cpuasm/ppuasm/apuasm output blob. There is no header to describe its\n");
    printf("layout - load address and role come from how it was assembled/linked.\n\n");

    if (o->check) {
        printf("--- Integrity ---\n");
        printf("Checksum: 0x%08lX\n\n", (unsigned long)checksum(d, size));
    }
    if (o->extract) {
        printf("--- Extracting ---\n");
        extract_region(o->base, "raw", d, size);
        printf("\n");
    }
    return 0;
}

int main(int argc, char *argv[])
{
    Opts o; size_t size; uint8_t *data; char *dot; int i, rc;

    memset(&o, 0, sizeof(o));
    for (i = 1; i < argc; i++) {
        if      (!strcmp(argv[i], "-v")) o.verbose = 1;
        else if (!strcmp(argv[i], "-x")) o.extract = 1;
        else if (!strcmp(argv[i], "-c")) o.check   = 1;
        else if (argv[i][0] != '-')      o.input   = argv[i];
    }

    if (!o.input) {
        printf("rominspect - inspect ZeroPoint build artifacts (ELF / ZPB / raw)\n");
        printf("Usage: %s <file> [-v] [-x] [-c]\n", argv[0]);
        printf("  -v  verbose    -x  extract components    -c  check integrity\n");
        return 1;
    }

    data = read_file(o.input, &size);
    if (!data) return 1;

    strncpy(o.base, o.input, sizeof(o.base) - 1);
    dot = strrchr(o.base, '.');
    if (dot) *dot = '\0';

    if (size >= 4 && data[0]==0x7f && data[1]=='E' && data[2]=='L' && data[3]=='F')
        rc = inspect_elf(data, size, &o);
    else if (size >= 4 && data[0]=='Z' && data[1]=='P' && data[2]=='B')
        rc = inspect_zpb(data, size, &o);
    else
        rc = inspect_raw(data, size, &o);

    free(data);
    return rc;
}

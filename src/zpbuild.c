/*
 * zpbuild - ZeroPoint ROM signer (the HQ / mastering step).
 *
 * In the ZeroPoint toolchain the developer-side linker (zplink) has no signing
 * key: it only emits an ELF32 image.  zpbuild is the step that *follows* zplink
 * on the trusted side - it reads that ELF, reconstructs the flat ROM payload
 * from its PT_LOAD segments, and wraps it in a signed .zpb ROM (ZPB header +
 * payload + RSA-2048 / SHA-256 trailer).
 *
 * The RSA private key lives *here*, not in zplink, so it never ships to
 * developers.  The key compiled in from zpkey.h is only the bundled DEVELOPMENT
 * key (good enough for a locally-loadable ROM the emulator will accept, since
 * the base emulator ignores the trailer); a real console vendor builds zpbuild
 * against their own secret key, kept on the mastering machine at HQ.
 *
 * Usage:
 *   zpbuild IN.elf -o OUT.zpb [--title NAME] [--dev AUTHOR] [--entry ADDR]
 *           [--codesize N] [--selftest] [-v]
 *
 * The payload and header are byte-for-byte identical to what zplink's old
 * built-in "rom" mode produced, so existing .zpb ROMs are unaffected.
 *
 * --codesize N opts into code/data-split signing (v2 trailer): the first N
 * bytes of the payload are treated as "code" (hashed with SHA-256, same as
 * the whole-payload path below) and the remaining bytes as "data" (hashed
 * with BLAKE2s, much cheaper per byte on the boot ROM's 16-bit CPU - see
 * ZPbootROM/def88186/blake2s.def). The two digests are folded into one
 * composite SHA-256 that's what actually gets RSA-signed
 * (SHA256(header||code_digest||data_digest)) - BLAKE2s(data) being fast is
 * only useful if the boot ROM's signature genuinely depends on it, not if
 * it's computed and compared unsigned (see rsa.def's rsa_verify_composite
 * comment for why that would give zero tamper resistance).
 *
 * zpbuild does not try to auto-detect the code/data boundary from ELF
 * PT_LOAD segment flags: zplink's CPU/PPU/APU/DATA roles can be placed at
 * addresses that interleave in the flattened image (see zplink.c's
 * DEFAULT_*_BASE constants), so "highest PF_X segment's end" is not
 * reliably the same as "end of the code prefix" the way it would be for a
 * plain single-segment ELF. The caller (build script) is expected to know
 * its own memory layout and pass an explicit --codesize; omit the flag
 * entirely for the original single-region SHA-256 path (v1 trailer,
 * unchanged, fully backward compatible).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compat.h"
#include "zpsha256.h"
#include "zpblake2s.h"
#include "zprsa.h"

#define ZPB_HDR_SIZE 64
#define MAX_IMAGE    (16UL*1024UL*1024UL)
#define EM_ZP        0x5A50      /* private machine id: 'ZP' */
#define PT_LOAD      1

/* ---- little-endian helpers ------------------------------------------- */
static unsigned int get16(const unsigned char *p)
{ return (unsigned int)p[0] | ((unsigned int)p[1] << 8); }

static unsigned long get32(const unsigned char *p)
{
    return (unsigned long)p[0] | ((unsigned long)p[1] << 8)
         | ((unsigned long)p[2] << 16) | ((unsigned long)p[3] << 24);
}

static void put16(unsigned char *p, unsigned int v)
{ p[0]=(unsigned char)(v&0xFF); p[1]=(unsigned char)((v>>8)&0xFF); }

static void put32(unsigned char *p, unsigned long v)
{ p[0]=(unsigned char)(v&0xFF); p[1]=(unsigned char)((v>>8)&0xFF);
  p[2]=(unsigned char)((v>>16)&0xFF); p[3]=(unsigned char)((v>>24)&0xFF); }

static unsigned long parse_addr(const char *s)
{
    if (s[0]=='0' && (s[1]=='x'||s[1]=='X')) return strtoul(s,NULL,16);
    if (s[0]=='$') return strtoul(s+1,NULL,16);
    return strtoul(s,NULL,10);
}

/* Read a whole file into a freshly malloc'd buffer. */
static unsigned char *read_file(const char *path, unsigned long *out_len)
{
    FILE *f; long sz; unsigned char *buf;
    f = fopen(path,"rb");
    if (!f) { fprintf(stderr,"zpbuild: cannot open '%s'\n",path); return NULL; }
    fseek(f,0,SEEK_END); sz=ftell(f); fseek(f,0,SEEK_SET);
    if (sz<0) { fclose(f); return NULL; }
    buf=(unsigned char*)malloc((size_t)sz?(size_t)sz:1);
    if (!buf) { fclose(f); return NULL; }
    if (sz>0 && fread(buf,1,(size_t)sz,f)!=(size_t)sz) { fclose(f); free(buf); return NULL; }
    fclose(f);
    *out_len=(unsigned long)sz;
    return buf;
}

/*
 * Rebuild the flat ROM payload from a zplink ELF's PT_LOAD program headers.
 * This reproduces zplink's build_image(): each segment is placed at its load
 * address in a zero-filled buffer sized to the highest segment end.
 * Returns the payload (caller frees) and fills *out_size and *out_entry.
 */
static unsigned char *image_from_elf(const unsigned char *elf, unsigned long elf_len,
                                     unsigned long *out_size, unsigned long *out_entry)
{
    unsigned long e_entry, e_phoff, hi;
    unsigned int e_phentsize, e_phnum, i;
    unsigned char *img;

    if (elf_len < 52 ||
        elf[0]!=0x7f || elf[1]!='E' || elf[2]!='L' || elf[3]!='F') {
        fprintf(stderr,"zpbuild: not an ELF file\n"); return NULL;
    }
    if (elf[4]!=1 || elf[5]!=1) {
        fprintf(stderr,"zpbuild: expected 32-bit little-endian ELF\n"); return NULL;
    }
    if (get16(elf+18) != EM_ZP)
        fprintf(stderr,"zpbuild: warning: e_machine is not 'ZP' (0x%04X)\n", get16(elf+18));

    e_entry     = get32(elf+24);
    e_phoff     = get32(elf+28);
    e_phentsize = get16(elf+42);
    e_phnum     = get16(elf+44);
    if (e_phentsize < 32) { fprintf(stderr,"zpbuild: bad program-header size\n"); return NULL; }

    /* First pass: find the highest end address. */
    hi = 0;
    for (i=0; i<e_phnum; i++) {
        const unsigned char *ph = elf + e_phoff + (unsigned long)i*e_phentsize;
        unsigned long p_vaddr, p_filesz, end;
        if ((unsigned long)(ph - elf) + 32 > elf_len) {
            fprintf(stderr,"zpbuild: truncated program headers\n"); return NULL;
        }
        if (get32(ph+0) != PT_LOAD) continue;
        p_vaddr  = get32(ph+8);
        p_filesz = get32(ph+16);
        end = p_vaddr + p_filesz;
        if (end > hi) hi = end;
    }
    if (hi > MAX_IMAGE) { fprintf(stderr,"zpbuild: image too large (0x%06lX)\n",hi); return NULL; }

    img = (unsigned char*)calloc((size_t)(hi?hi:1),1);
    if (!img) { fprintf(stderr,"zpbuild: out of memory\n"); return NULL; }

    /* Second pass: place each PT_LOAD segment's bytes at its load address. */
    for (i=0; i<e_phnum; i++) {
        const unsigned char *ph = elf + e_phoff + (unsigned long)i*e_phentsize;
        unsigned long p_offset, p_vaddr, p_filesz;
        if (get32(ph+0) != PT_LOAD) continue;
        p_offset = get32(ph+4);
        p_vaddr  = get32(ph+8);
        p_filesz = get32(ph+16);
        if (p_offset + p_filesz > elf_len) {
            fprintf(stderr,"zpbuild: segment data outside file\n"); free(img); return NULL;
        }
        memcpy(img + p_vaddr, elf + p_offset, (size_t)p_filesz);
    }

    *out_size  = hi;
    *out_entry = e_entry;
    return img;
}

static int run_selftest(void)
{
    unsigned char h[32], sig[ZP_RSA_BYTES];
    zp_sha256_buf((const unsigned char*)"abc",3,h);
    if (h[0]!=0xba||h[31]!=0xad){ printf("selftest: SHA-256 FAIL\n"); return 1; }
    zp_rsa_sign(h,sig);
    if (!zp_rsa_verify(h,sig)){ printf("selftest: RSA verify FAIL\n"); return 1; }
    sig[10]^=0x80;
    if (zp_rsa_verify(h,sig)){ printf("selftest: RSA tamper NOT detected\n"); return 1; }
    printf("selftest: OK (SHA-256 vector + RSA-2048 sign/verify/tamper)\n");
    return 0;
}

static void usage(const char *p)
{
    fprintf(stderr, "zpbuild - ZeroPoint ROM signer (HQ mastering step)\n\n");
    fprintf(stderr, "Usage: %s IN.elf -o OUT.zpb\n", p);
    fprintf(stderr, "       [--title NAME] [--dev AUTHOR] [--entry ADDR] [--codesize N]\n");
    fprintf(stderr, "       [--selftest] [-v]\n\n");
    fprintf(stderr, "Reads a zplink ELF, reconstructs the ROM payload, and writes a\n");
    fprintf(stderr, "signed .zpb ROM (ZPB header + payload + RSA-2048/SHA-256 trailer).\n");
    fprintf(stderr, "Entry defaults to the ELF entry point; override with --entry.\n");
    fprintf(stderr, "--codesize N opts into code/data-split signing: the first N payload\n");
    fprintf(stderr, "bytes are SHA-256-signed as code, the rest BLAKE2s-hashed as data,\n");
    fprintf(stderr, "both folded into one signed composite digest. Omit for the original\n");
    fprintf(stderr, "single-region SHA-256-over-the-whole-payload path.\n");
}

int main(int argc, char **argv)
{
    const char *in = NULL, *out = NULL;
    char title[32], dev[16];
    unsigned long entry = 0, img_size = 0, elf_entry = 0, elf_len = 0, total;
    unsigned long code_size = 0;
    int have_entry = 0, have_codesize = 0, verbose = 0, i;
    unsigned char *elf, *img;
    unsigned char hdr[ZPB_HDR_SIZE], digest[32], sig[ZP_RSA_BYTES];
    zp_sha256 sh;
    FILE *f;

    memset(title,0,sizeof(title)); strncpy(title,"ZeroPoint Game",sizeof(title)-1);
    memset(dev,0,sizeof(dev));     strncpy(dev,"unknown",sizeof(dev)-1);

    for (i=1;i<argc;i++){
        if (!strcmp(argv[i],"-h")||!strcmp(argv[i],"--help")){ usage(argv[0]); return 0; }
        else if (!strcmp(argv[i],"--selftest")) return run_selftest();
        else if (!strcmp(argv[i],"-v")) verbose=1;
        else if (!strcmp(argv[i],"-o") && i+1<argc) out=argv[++i];
        else if (!strcmp(argv[i],"--entry") && i+1<argc){ entry=parse_addr(argv[++i]); have_entry=1; }
        else if (!strcmp(argv[i],"--codesize") && i+1<argc){ code_size=parse_addr(argv[++i]); have_codesize=1; }
        else if (!strcmp(argv[i],"--title") && i+1<argc){ memset(title,0,32); strncpy(title,argv[++i],31); }
        else if (!strcmp(argv[i],"--dev") && i+1<argc){ memset(dev,0,16); strncpy(dev,argv[++i],15); }
        else if (argv[i][0]=='-'){ fprintf(stderr,"zpbuild: unknown option '%s'\n",argv[i]); usage(argv[0]); return 1; }
        else if (!in) in=argv[i];
        else { fprintf(stderr,"zpbuild: unexpected argument '%s'\n",argv[i]); return 1; }
    }

    if (!in){ fprintf(stderr,"zpbuild: input ELF is required\n"); usage(argv[0]); return 1; }
    if (!out){ fprintf(stderr,"zpbuild: -o OUT.zpb is required\n"); return 1; }

    elf = read_file(in,&elf_len);
    if (!elf) return 1;
    img = image_from_elf(elf,elf_len,&img_size,&elf_entry);
    free(elf);
    if (!img) return 1;
    if (!have_entry) entry = elf_entry;
    if (have_codesize && code_size > img_size) {
        fprintf(stderr,"zpbuild: --codesize %lu exceeds payload size %lu\n",code_size,img_size);
        free(img); return 1;
    }
    /* sha256_hash_code (ZPbootROM/def88186/rsa.def) reads the code region
     * with fixed $000000,X addressing - single-bank only, unlike the data
     * region's blake2s_hash_multibank. A codeSize past 64KB would silently
     * false-reject an otherwise-valid ROM at verify time; reject it here
     * instead, at signing time, where the failure is loud and immediate. */
    if (have_codesize && code_size > 65536UL) {
        fprintf(stderr,"zpbuild: --codesize %lu exceeds 64KB (code region is single-bank)\n",code_size);
        free(img); return 1;
    }

    /* ZPB header (identical layout to the old zplink rom mode). */
    memset(hdr,0,sizeof(hdr));
    hdr[0]='Z'; hdr[1]='P'; hdr[2]='B'; hdr[3]=0;
    hdr[4]=2;                                   /* version 2: signed */
    hdr[5]=0x01;                                /* flags: bit0 = signed */
    put16(hdr+6, ZPB_HDR_SIZE);
    put32(hdr+8, img_size);
    put32(hdr+12, entry);
    memcpy(hdr+16, title, 32);
    memcpy(hdr+48, dev, 16);

    if (have_codesize) {
        /* Code/data split: code_digest = SHA256(header||code), data_digest
           = BLAKE2s(data), and what actually gets RSA-signed is the
           composite SHA256(header||code_digest||data_digest) - matching
           rsa_verify_composite in ZPbootROM/def88186/rsa.def exactly (that
           file's comment is the authoritative spec; keep both in sync). */
        unsigned char code_digest[32], data_digest[32];
        zp_sha256_init(&sh);
        zp_sha256_update(&sh, hdr, ZPB_HDR_SIZE);
        zp_sha256_update(&sh, img, code_size);
        zp_sha256_final(&sh, code_digest);

        zp_blake2s_buf(img + code_size, img_size - code_size, data_digest);

        zp_sha256_init(&sh);
        zp_sha256_update(&sh, hdr, ZPB_HDR_SIZE);
        zp_sha256_update(&sh, code_digest, 32);
        zp_sha256_update(&sh, data_digest, 32);
        zp_sha256_final(&sh, digest);
    } else {
        /* Hash covers header + payload so entry/title/size are authenticated. */
        zp_sha256_init(&sh);
        zp_sha256_update(&sh, hdr, ZPB_HDR_SIZE);
        zp_sha256_update(&sh, img, img_size);
        zp_sha256_final(&sh, digest);
    }
    zp_rsa_sign(digest, sig);

    f=fopen(out,"wb");
    if(!f){ fprintf(stderr,"zpbuild: cannot create '%s'\n",out); free(img); return 1; }
    fwrite(hdr,1,ZPB_HDR_SIZE,f);
    fwrite(img,1,img_size,f);
    {
        unsigned char tr[8+32];
        memset(tr,0,sizeof(tr));
        tr[0]='Z'; tr[1]='P'; tr[2]='S'; tr[3]='G';
        tr[4]=(unsigned char)(have_codesize?2:1);/* trailer version */
        tr[5]=(unsigned char)(ZP_RSA_BITS/64);   /* 2048/64 = 32 */
        put16(tr+6, ZP_RSA_BYTES);
        memcpy(tr+8, digest, 32);
        fwrite(tr,1,sizeof(tr),f);
        fwrite(sig,1,ZP_RSA_BYTES,f);
        if (have_codesize) {
            unsigned char cs[4];
            put32(cs, code_size);
            fwrite(cs,1,4,f);
        }
    }
    total=ZPB_HDR_SIZE+img_size+8+32+ZP_RSA_BYTES+(have_codesize?4:0);
    fclose(f);
    free(img);

    printf("ROM written: %s\n", out);
    printf("  header    : 64 bytes (ZPB v2, signed)\n");
    printf("  payload   : %lu bytes  (entry 0x%06lX)\n", img_size, entry);
    if (have_codesize) {
        printf("  split     : %lu code bytes (SHA-256) + %lu data bytes (BLAKE2s)\n",
               code_size, img_size-code_size);
    }
    printf("  signature : RSA-2048 PKCS#1 v1.5 / SHA-256 (256 bytes)\n");
    printf("  total     : %lu bytes\n", total);
    if (verbose){
        printf("  title     : \"%s\"\n", title);
        printf("  developer : \"%s\"\n", dev);
        printf("  source    : %s\n", in);
    }
    return 0;
}

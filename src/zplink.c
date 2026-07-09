/*
 * zplink - ZeroPoint locating linker (DEF88186 / PPU / APU).
 *
 * The ZeroPoint toolchain has no relocatable object format: the assemblers
 * (cpuasm/ppuasm/apuasm) and the C compilers (zpcc/def88186cc) emit flat
 * binaries that are already assembled for a fixed load address.  zplink takes
 * those flat segments, places each at its declared 24-bit address, checks for
 * overlap, resolves the entry point, and emits a proper ELF32 executable
 * (custom e_machine 'ZP'), readable by readelf/objdump for debugging and
 * inspection.  That ELF is the developer-side deliverable.
 *
 * zplink deliberately holds NO signing key: turning the ELF into a signed .zpb
 * ROM is a separate, trusted step performed by zpbuild (the mastering tool),
 * so the private key never ships to developers.  For local testing, feed the
 * ELF to zpbuild (which carries the bundled development key) to get a .zpb the
 * emulator will load.
 *
 * Usage:
 *   zplink -o OUT.elf
 *          --cpu FILE[@ADDR] [--ppu FILE[@ADDR]] [--apu FILE[@ADDR]]
 *          [--data FILE@ADDR ...] [--entry ADDR]
 *
 * Default addresses match rombuilder/CLAUDE.md:
 *   cpu  @ 0x008000   ppu @ 0x100000   apu @ 0x200000
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compat.h"

#define MAX_SEG        32
#define MAX_IMAGE      (16UL*1024UL*1024UL)

#define DEFAULT_CPU_BASE 0x008000UL
#define DEFAULT_PPU_BASE 0x100000UL
#define DEFAULT_APU_BASE 0x200000UL

/* ELF constants we emit. */
#define ET_EXEC   2
#define EM_ZP     0x5A50      /* private machine id: 'ZP' */
#define PT_LOAD   1
#define PF_X      1
#define PF_W      2
#define PF_R      4
#define SHT_PROGBITS 1
#define SHT_STRTAB   3
#define SHF_WRITE     1
#define SHF_ALLOC     2
#define SHF_EXECINSTR 4

enum { ROLE_CPU, ROLE_PPU, ROLE_APU, ROLE_DATA };

typedef struct {
    char          name[16];
    int           role;
    unsigned long addr;
    unsigned char *data;
    unsigned long size;
    int           explicit_addr;
} Segment;

typedef struct {
    const char   *out;
    unsigned long entry;
    int           have_entry;
    Segment       seg[MAX_SEG];
    int           nseg;
} Config;

/* ---- little-endian byte emitters ------------------------------------- */
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
    if (!f) { fprintf(stderr,"zplink: cannot open '%s'\n",path); return NULL; }
    fseek(f,0,SEEK_END); sz=ftell(f); fseek(f,0,SEEK_SET);
    if (sz<0) { fclose(f); return NULL; }
    buf=(unsigned char*)malloc((size_t)sz?(size_t)sz:1);
    if (!buf) { fclose(f); return NULL; }
    if (sz>0 && fread(buf,1,(size_t)sz,f)!=(size_t)sz) { fclose(f); free(buf); return NULL; }
    fclose(f);
    *out_len=(unsigned long)sz;
    return buf;
}

/* Add a segment "file[@addr]".  default_addr used when @addr is absent. */
static int add_segment(Config *c, const char *spec, int role,
                       const char *nm, unsigned long default_addr)
{
    Segment *s; char path[512]; const char *at;
    unsigned long addr = default_addr; int expl = 0;
    if (c->nseg >= MAX_SEG) { fprintf(stderr,"zplink: too many segments\n"); return 0; }
    at = strchr(spec,'@');
    if (at) {
        size_t n=(size_t)(at-spec);
        if (n>=sizeof(path)) n=sizeof(path)-1;
        memcpy(path,spec,n); path[n]='\0';
        addr = parse_addr(at+1); expl=1;
    } else {
        strncpy(path,spec,sizeof(path)-1); path[sizeof(path)-1]='\0';
    }
    s=&c->seg[c->nseg];
    memset(s,0,sizeof(*s));
    s->role=role; s->addr=addr; s->explicit_addr=expl;
    strncpy(s->name,nm,sizeof(s->name)-1);
    s->data=read_file(path,&s->size);
    if (!s->data) return 0;
    c->nseg++;
    return 1;
}

static void seg_flags(int role, unsigned long *pf, unsigned long *shf)
{
    if (role==ROLE_CPU) { *pf=PF_R|PF_X; *shf=SHF_ALLOC|SHF_EXECINSTR; }
    else                { *pf=PF_R|PF_W; *shf=SHF_ALLOC|SHF_WRITE; }
}

/* Check that no two segments overlap. */
static int check_overlap(Config *c)
{
    int i,j;
    for (i=0;i<c->nseg;i++) for (j=i+1;j<c->nseg;j++) {
        unsigned long a0=c->seg[i].addr, a1=a0+c->seg[i].size;
        unsigned long b0=c->seg[j].addr, b1=b0+c->seg[j].size;
        if (a0<b1 && b0<a1) {
            fprintf(stderr,"zplink: segments '%s' (0x%06lX..0x%06lX) and "
                    "'%s' (0x%06lX..0x%06lX) overlap\n",
                    c->seg[i].name,a0,a1,c->seg[j].name,b0,b1);
            return 0;
        }
    }
    return 1;
}

/* ---- ELF32 writer ---------------------------------------------------- */
static int write_elf(Config *c, const char *path)
{
    FILE *f;
    int n=c->nseg, i;
    int nsec = n + 2;                 /* NULL + per-seg + .shstrtab */
    unsigned long phoff = 52;
    unsigned long dataoff = phoff + 32UL*n;
    unsigned long shstr_off, shoff, off;
    unsigned char eh[52], ph[32], sh[40];
    unsigned char *shstr; unsigned long shstr_len, shstr_cap;
    unsigned long *seg_off;
    unsigned long *name_off;

    seg_off=(unsigned long*)malloc(sizeof(unsigned long)*(n>0?n:1));
    name_off=(unsigned long*)malloc(sizeof(unsigned long)*(nsec>0?nsec:1));
    if (!seg_off||!name_off){ free(seg_off); free(name_off); return 0; }

    /* Assign file offsets to each segment's bytes. */
    off=dataoff;
    for (i=0;i<n;i++){ seg_off[i]=off; off+=c->seg[i].size; }
    shstr_off=off;

    /* Build .shstrtab: "\0<segnames>\0.shstrtab\0" */
    shstr_cap=64; for(i=0;i<n;i++) shstr_cap+=strlen(c->seg[i].name)+2;
    shstr=(unsigned char*)malloc(shstr_cap);
    if(!shstr){ free(seg_off); free(name_off); return 0; }
    shstr_len=0; shstr[shstr_len++]=0;      /* index 0 = empty name */
    name_off[0]=0;                          /* NULL section */
    for (i=0;i<n;i++){
        char nm[24]; nm[0]='.';
        strncpy(nm+1,c->seg[i].name,sizeof(nm)-2); nm[sizeof(nm)-1]='\0';
        name_off[i+1]=shstr_len;
        memcpy(shstr+shstr_len,nm,strlen(nm)+1); shstr_len+=strlen(nm)+1;
    }
    name_off[n+1]=shstr_len;
    memcpy(shstr+shstr_len,".shstrtab",10); shstr_len+=10;

    shoff=shstr_off+shstr_len;

    f=fopen(path,"wb");
    if(!f){ fprintf(stderr,"zplink: cannot create '%s'\n",path);
            free(seg_off); free(name_off); free(shstr); return 0; }

    /* ELF header */
    memset(eh,0,sizeof(eh));
    eh[0]=0x7f; eh[1]='E'; eh[2]='L'; eh[3]='F';
    eh[4]=1;  /* ELFCLASS32 */
    eh[5]=1;  /* ELFDATA2LSB */
    eh[6]=1;  /* EV_CURRENT */
    put16(eh+16, ET_EXEC);
    put16(eh+18, EM_ZP);
    put32(eh+20, 1);
    put32(eh+24, c->entry);
    put32(eh+28, phoff);
    put32(eh+32, shoff);
    put32(eh+36, 0);
    put16(eh+40, 52);
    put16(eh+42, 32);
    put16(eh+44, (unsigned int)n);
    put16(eh+46, 40);
    put16(eh+48, (unsigned int)nsec);
    put16(eh+50, (unsigned int)(nsec-1));   /* shstrndx = last section */
    fwrite(eh,1,52,f);

    /* Program headers */
    for (i=0;i<n;i++){
        unsigned long pf,shf; seg_flags(c->seg[i].role,&pf,&shf);
        memset(ph,0,sizeof(ph));
        put32(ph+0,  PT_LOAD);
        put32(ph+4,  seg_off[i]);
        put32(ph+8,  c->seg[i].addr);
        put32(ph+12, c->seg[i].addr);
        put32(ph+16, c->seg[i].size);
        put32(ph+20, c->seg[i].size);
        put32(ph+24, pf);
        put32(ph+28, 1);
        fwrite(ph,1,32,f);
    }

    /* Segment data */
    for (i=0;i<n;i++) fwrite(c->seg[i].data,1,c->seg[i].size,f);

    /* .shstrtab */
    fwrite(shstr,1,shstr_len,f);

    /* Section headers */
    memset(sh,0,sizeof(sh)); fwrite(sh,1,40,f);   /* NULL section */
    for (i=0;i<n;i++){
        unsigned long pf,shf; seg_flags(c->seg[i].role,&pf,&shf);
        memset(sh,0,sizeof(sh));
        put32(sh+0,  name_off[i+1]);
        put32(sh+4,  SHT_PROGBITS);
        put32(sh+8,  shf);
        put32(sh+12, c->seg[i].addr);
        put32(sh+16, seg_off[i]);
        put32(sh+20, c->seg[i].size);
        put32(sh+32, 1);
        fwrite(sh,1,40,f);
    }
    memset(sh,0,sizeof(sh));                        /* .shstrtab section */
    put32(sh+0,  name_off[n+1]);
    put32(sh+4,  SHT_STRTAB);
    put32(sh+16, shstr_off);
    put32(sh+20, shstr_len);
    put32(sh+32, 1);
    fwrite(sh,1,40,f);

    fclose(f);
    free(seg_off); free(name_off); free(shstr);
    return 1;
}

static void usage(const char *p)
{
    fprintf(stderr, "zplink - ZeroPoint locating linker (emits ELF)\n\n");
    fprintf(stderr, "Usage: %s -o OUT.elf --cpu FILE[@ADDR]\n", p);
    fprintf(stderr, "       [--ppu FILE[@ADDR]] [--apu FILE[@ADDR]] [--data FILE@ADDR]...\n");
    fprintf(stderr, "       [--entry ADDR]\n\n");
    fprintf(stderr, "Places each flat segment at its 24-bit load address and writes an\n");
    fprintf(stderr, "ELF32 executable (e_machine 'ZP').  Sign it into a .zpb ROM with zpbuild.\n");
    fprintf(stderr, "Defaults: cpu@0x008000 ppu@0x100000 apu@0x200000; entry = cpu base.\n");
}

int main(int argc, char **argv)
{
    Config c; int i;
    memset(&c,0,sizeof(c));
    c.entry=DEFAULT_CPU_BASE;

    for (i=1;i<argc;i++){
        if (!strcmp(argv[i],"-h")||!strcmp(argv[i],"--help")){ usage(argv[0]); return 0; }
        else if (!strcmp(argv[i],"-o") && i+1<argc) c.out=argv[++i];
        else if (!strcmp(argv[i],"--entry") && i+1<argc){ c.entry=parse_addr(argv[++i]); c.have_entry=1; }
        else if (!strcmp(argv[i],"--cpu") && i+1<argc){ if(!add_segment(&c,argv[++i],ROLE_CPU,"text",DEFAULT_CPU_BASE)) return 1; }
        else if (!strcmp(argv[i],"--ppu") && i+1<argc){ if(!add_segment(&c,argv[++i],ROLE_PPU,"ppu",DEFAULT_PPU_BASE)) return 1; }
        else if (!strcmp(argv[i],"--apu") && i+1<argc){ if(!add_segment(&c,argv[++i],ROLE_APU,"apu",DEFAULT_APU_BASE)) return 1; }
        else if (!strcmp(argv[i],"--data") && i+1<argc){ if(!add_segment(&c,argv[++i],ROLE_DATA,"data",0)) return 1; }
        else { fprintf(stderr,"zplink: unknown option '%s'\n",argv[i]); usage(argv[0]); return 1; }
    }

    if (!c.out){ fprintf(stderr,"zplink: -o OUT.elf is required\n"); return 1; }
    if (c.nseg==0){ fprintf(stderr,"zplink: at least one segment (--cpu) is required\n"); return 1; }
    if (!check_overlap(&c)) return 1;

    /* Default entry = base of the first CPU segment. */
    if (!c.have_entry){
        for (i=0;i<c.nseg;i++) if (c.seg[i].role==ROLE_CPU){ c.entry=c.seg[i].addr; break; }
    }

    printf("zplink: %d segment(s), entry 0x%06lX\n", c.nseg, c.entry);
    for (i=0;i<c.nseg;i++)
        printf("  [%d] %-4s 0x%06lX  %lu bytes\n",
               i, c.seg[i].name, c.seg[i].addr, c.seg[i].size);

    if (!write_elf(&c,c.out)){ return 1; }
    printf("ELF written: %s\n", c.out);

    for (i=0;i<c.nseg;i++) free(c.seg[i].data);
    return 0;
}

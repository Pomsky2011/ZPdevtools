/* C89 preprocessor for def88186cc.
 *
 * Handles: #include, #define/#undef (object- and function-like), #if/#ifdef/#ifndef/#elif/#else/#endif with
 * a full integer constant-expression evaluator, #error, #line, #pragma (ignored), __LINE__, __FILE__, __STDC__.
 * Macro names are never expanded inside string/character literals or comments; a macro is not re-expanded inside
 * its own expansion.  Everything is bounded: nothing here writes past a fixed-size buffer, and every limit that
 * can be hit (line length, nesting, macro depth) is reported as an error instead.
 *
 * Not supported (reported as errors, never silently ignored): the # and ## operators, variadic macros.
 * A macro invocation must be on one logical line (backslash-newline continues a line).
 * __DATE__ and __TIME__ are not defined (output must be reproducible).
 */
#include "preprocessor.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "zpstrdup.h"

#define PP_MAX_LINE   30000   /* longest logical line or macro expansion, in bytes */
#define PP_MAX_DEPTH  64      /* include nesting and macro rescan depth */
#define PP_MAX_INCLUDES 5000  /* #include directives per translation unit */
#define PP_MAX_COND   256     /* #if nesting */
#define PP_MAX_PARAMS 64      /* parameters of one function-like macro */
#define PP_EV_DEPTH   200     /* nesting inside one #if expression */

#define UC(c) ((unsigned char)(c))

/* ------------------------------------------------------------------ */
/* growable, bounded string buffer */

typedef struct {
    char* s;
    int len;
    int cap;
    int bad;        /* overflowed PP_MAX_LINE or ran out of memory; contents are truncated */
} Buf;

static void buf_init(Buf* b) {
    b->cap = 64;
    b->len = 0;
    b->bad = 0;
    b->s = (char*)malloc(b->cap);
    if (!b->s) {
        b->cap = 0;
        b->bad = 1;
        return;
    }
    b->s[0] = '\0';
}

static void buf_free(Buf* b) {
    free(b->s);
    b->s = NULL;
    b->len = 0;
    b->cap = 0;
}

static void buf_putc(Buf* b, char c) {
    if (b->bad) return;
    if (b->len + 1 >= PP_MAX_LINE) {
        b->bad = 1;
        return;
    }
    if (b->len + 2 > b->cap) {
        int ncap = b->cap * 2;
        char* ns;
        if (ncap > PP_MAX_LINE + 2) ncap = PP_MAX_LINE + 2;
        ns = (char*)realloc(b->s, ncap);
        if (!ns) {
            b->bad = 1;
            return;
        }
        b->s = ns;
        b->cap = ncap;
    }
    b->s[b->len++] = c;
    b->s[b->len] = '\0';
}

static void buf_putn(Buf* b, const char* s, int n) {
    while (n-- > 0) buf_putc(b, *s++);
}

static void buf_puts(Buf* b, const char* s) {
    while (*s) buf_putc(b, *s++);
}

static int is_space(int c);

/* drop leading and trailing whitespace (an argument's surrounding blanks are not part of it) */
static void buf_trim(Buf* b) {
    int lead = 0;
    if (!b->s) return;
    while (b->len > 0 && is_space(b->s[b->len - 1])) b->s[--b->len] = '\0';
    while (lead < b->len && is_space(b->s[lead])) lead++;
    if (lead > 0) {
        memmove(b->s, b->s + lead, b->len - lead + 1);
        b->len -= lead;
    }
}

/* ------------------------------------------------------------------ */
/* diagnostics and character classes */

static void pp_error(PreprocessorContext* ctx, const char* fmt, ...) {
    va_list ap;
    fprintf(stderr, "%s:%d: error: ", ctx->file_name ? ctx->file_name : "<input>", ctx->line_num);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
    ctx->errors++;
}

static void pp_warning(PreprocessorContext* ctx, const char* fmt, ...) {
    va_list ap;
    fprintf(stderr, "%s:%d: warning: ", ctx->file_name ? ctx->file_name : "<input>", ctx->line_num);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
}

/* logical lines contain no newline, so '\n' is not whitespace here */
static int is_space(int c) {
    return c == ' ' || c == '\t' || c == '\f' || c == '\v' || c == '\r';
}

static int is_ident_start(int c) {
    return isalpha(UC(c)) || c == '_';
}

static int is_ident_char(int c) {
    return isalnum(UC(c)) || c == '_';
}

static const char* skip_ws(const char* p) {
    while (*p && is_space(*p)) p++;
    return p;
}

/* ------------------------------------------------------------------ */
/* macro table */

static Macro* find_macro_n(PreprocessorContext* ctx, const char* name, int len) {
    Macro* m;
    for (m = ctx->macros; m; m = m->next) {
        if ((int)strlen(m->name) == len && memcmp(m->name, name, len) == 0) return m;
    }
    return NULL;
}

Macro* preprocessor_find_macro(PreprocessorContext* ctx, const char* name) {
    return find_macro_n(ctx, name, (int)strlen(name));
}

static void macro_free(Macro* m) {
    int i;
    free(m->name);
    free(m->replacement);
    if (m->params) {
        for (i = 0; i < m->param_count; i++) free(m->params[i]);
        free(m->params);
    }
    free(m);
}

static void macro_unlink(PreprocessorContext* ctx, Macro* victim) {
    Macro** pp = &ctx->macros;
    while (*pp) {
        if (*pp == victim) {
            *pp = victim->next;
            macro_free(victim);
            return;
        }
        pp = &(*pp)->next;
    }
}

static void define_macro(PreprocessorContext* ctx, const char* name, MacroType type,
                         char** params, int nparams, const char* replacement) {
    Macro* old = preprocessor_find_macro(ctx, name);
    Macro* m;
    int i;

    if (old) {
        int same = (old->type == type && old->param_count == nparams &&
                    strcmp(old->replacement, replacement) == 0);
        for (i = 0; same && i < nparams; i++) {
            if (strcmp(old->params[i], params[i]) != 0) same = 0;
        }
        if (same) return;
        pp_warning(ctx, "'%s' redefined", name);
        macro_unlink(ctx, old);
    }

    m = (Macro*)malloc(sizeof(Macro));
    if (!m) {
        pp_error(ctx, "out of memory");
        return;
    }
    m->name = zp_strdup(name);
    m->replacement = zp_strdup(replacement);
    m->type = type;
    m->params = NULL;
    m->param_count = 0;
    if (type == MACRO_FUNCTION) {
        m->params = (char**)malloc(sizeof(char*) * (nparams > 0 ? nparams : 1));
        if (!m->params) {
            pp_error(ctx, "out of memory");
            free(m->name);
            free(m->replacement);
            free(m);
            return;
        }
        for (i = 0; i < nparams; i++) m->params[i] = zp_strdup(params[i]);
        m->param_count = nparams;
    }
    m->next = ctx->macros;
    ctx->macros = m;
}

void preprocessor_define(PreprocessorContext* ctx, const char* name, const char* replacement) {
    define_macro(ctx, name, MACRO_SIMPLE, NULL, 0, replacement);
}

void preprocessor_define_function(PreprocessorContext* ctx, const char* name,
                                   char** params, int param_count, const char* replacement) {
    define_macro(ctx, name, MACRO_FUNCTION, params, param_count, replacement);
}

void preprocessor_undef(PreprocessorContext* ctx, const char* name) {
    Macro* m = preprocessor_find_macro(ctx, name);
    if (m) macro_unlink(ctx, m);
}

/* ------------------------------------------------------------------ */
/* context */

static void set_dir_from_path(PreprocessorContext* ctx, const char* path) {
    const char* slash = strrchr(path, '/');
    const char* bslash = strrchr(path, '\\');
    int n;
    char* dir;
    if (bslash && (!slash || bslash > slash)) slash = bslash;
    n = slash ? (int)(slash - path) + 1 : 0;      /* keep the separator */
    dir = (char*)malloc(n + 1);
    if (!dir) return;
    memcpy(dir, path, n);
    dir[n] = '\0';
    free(ctx->file_dir);
    ctx->file_dir = dir;
}

void preprocessor_set_file(PreprocessorContext* ctx, const char* name) {
    free(ctx->file_name);
    ctx->file_name = zp_strdup(name);
    set_dir_from_path(ctx, name);
}

PreprocessorContext* preprocessor_create(FILE* input, FILE* output) {
    PreprocessorContext* ctx = (PreprocessorContext*)malloc(sizeof(PreprocessorContext));
    if (!ctx) return NULL;
    ctx->input = input;
    ctx->output = output;
    ctx->macros = NULL;
    ctx->include_paths = NULL;
    ctx->include_path_count = 0;
    ctx->include_path_cap = 0;
    ctx->file_name = zp_strdup("<input>");
    ctx->file_dir = zp_strdup("");
    ctx->line_num = 1;
    ctx->conds = NULL;
    ctx->cond_depth = 0;
    ctx->cond_cap = 0;
    ctx->include_depth = 0;
    ctx->include_total = 0;
    ctx->errors = 0;

    preprocessor_add_include_path(ctx, ".");
    preprocessor_define(ctx, "__STDC__", "1");
    return ctx;
}

void preprocessor_add_include_path(PreprocessorContext* ctx, const char* path) {
    if (ctx->include_path_count >= ctx->include_path_cap) {
        int ncap = ctx->include_path_cap ? ctx->include_path_cap * 2 : 8;
        char** np = (char**)realloc(ctx->include_paths, sizeof(char*) * ncap);
        if (!np) return;
        ctx->include_paths = np;
        ctx->include_path_cap = ncap;
    }
    ctx->include_paths[ctx->include_path_count++] = zp_strdup(path);
}

void preprocessor_destroy(PreprocessorContext* ctx) {
    int i;
    Macro* m;
    if (!ctx) return;
    m = ctx->macros;
    while (m) {
        Macro* next = m->next;
        macro_free(m);
        m = next;
    }
    for (i = 0; i < ctx->include_path_count; i++) free(ctx->include_paths[i]);
    free(ctx->include_paths);
    free(ctx->conds);
    free(ctx->file_name);
    free(ctx->file_dir);
    free(ctx);
}

/* ------------------------------------------------------------------ */
/* conditional stack */

static int cond_active(PreprocessorContext* ctx) {
    return ctx->cond_depth == 0 || ctx->conds[ctx->cond_depth - 1].active;
}

static void cond_push(PreprocessorContext* ctx, int parent, int value) {
    PPCond* c;
    if (ctx->cond_depth >= PP_MAX_COND) {
        pp_error(ctx, "#if nested more than %d deep", PP_MAX_COND);
        return;
    }
    if (ctx->cond_depth >= ctx->cond_cap) {
        int ncap = ctx->cond_cap ? ctx->cond_cap * 2 : 16;
        PPCond* nc = (PPCond*)realloc(ctx->conds, sizeof(PPCond) * ncap);
        if (!nc) {
            pp_error(ctx, "out of memory");
            return;
        }
        ctx->conds = nc;
        ctx->cond_cap = ncap;
    }
    c = &ctx->conds[ctx->cond_depth++];
    c->parent = parent;
    c->active = parent && value;
    c->taken = value || !parent;      /* inside a dead region no branch may ever be taken */
    c->seen_else = 0;
}

/* ------------------------------------------------------------------ */
/* reading logical lines */

/* One logical line: backslash-newline spliced, comments replaced by a single space, string and character
   literals left alone.  Returns 0 at end of input.  *nphys = physical lines consumed. */
static int read_logical_line(PreprocessorContext* ctx, FILE* f, Buf* out, int* nphys) {
    int c, d;
    int got = 0;
    int quote = 0;

    *nphys = 0;
    out->len = 0;
    out->bad = (out->s == NULL);
    if (out->s) out->s[0] = '\0';

    for (;;) {
        c = getc(f);
        if (c == EOF) {
            if (!got) return 0;
            (*nphys)++;
            break;
        }
        got = 1;

        if (c == '\r') {
            d = getc(f);
            if (d == '\n') {
                c = '\n';
            } else {
                if (d != EOF) ungetc(d, f);
                c = ' ';
            }
        }

        if (c == '\\') {
            d = getc(f);
            if (d == '\r') {
                int e = getc(f);
                if (e == '\n') {
                    d = '\n';
                } else if (e != EOF) {
                    ungetc(e, f);
                }
            }
            if (d == '\n') {            /* line splice */
                (*nphys)++;
                continue;
            }
            if (d != EOF) ungetc(d, f);
        }

        if (c == '\n') {
            (*nphys)++;
            break;
        }

        if (quote) {
            buf_putc(out, (char)c);
            if (c == '\\') {
                d = getc(f);
                if (d != EOF) {
                    if (d == '\n') ungetc(d, f);       /* unterminated literal: the line ends here */
                    else buf_putc(out, (char)d);
                }
            } else if (c == quote) {
                quote = 0;
            }
            continue;
        }

        if (c == '"' || c == '\'') {
            quote = c;
            buf_putc(out, (char)c);
            continue;
        }

        if (c == '/') {
            d = getc(f);
            if (d == '*') {
                int prev = 0;
                for (;;) {
                    d = getc(f);
                    if (d == EOF) {
                        pp_error(ctx, "unterminated comment");
                        break;
                    }
                    if (d == '\n') (*nphys)++;
                    if (prev == '*' && d == '/') break;
                    prev = d;
                }
                buf_putc(out, ' ');
                continue;
            }
            if (d == '/') {
                /* not C89, but the lexer accepts // comments, so drop them here too */
                for (;;) {
                    d = getc(f);
                    if (d == EOF) break;
                    if (d == '\n') {
                        ungetc(d, f);
                        break;
                    }
                }
                continue;
            }
            if (d != EOF) ungetc(d, f);
            buf_putc(out, '/');
            continue;
        }

        buf_putc(out, (char)c);
    }

    if (out->bad) pp_error(ctx, "line too long (limit is %d bytes)", PP_MAX_LINE);
    return 1;
}

/* ------------------------------------------------------------------ */
/* macro expansion */

static int is_hidden(Macro** hide, int nhide, Macro* m) {
    int i;
    for (i = 0; i < nhide; i++) {
        if (hide[i] == m) return 1;
    }
    return 0;
}

static void copy_literal(const char** pp, Buf* out) {
    const char* p = *pp;
    char q = *p++;
    buf_putc(out, q);
    while (*p && *p != q) {
        if (*p == '\\' && p[1]) buf_putc(out, *p++);
        buf_putc(out, *p++);
    }
    if (*p) buf_putc(out, *p++);
    *pp = p;
}

/* a preprocessing number (so that 0x1F or 1e5 is not mistaken for an identifier) */
static void copy_number(const char** pp, Buf* out) {
    const char* p = *pp;
    while (*p) {
        if (isalnum(UC(*p)) || *p == '_' || *p == '.') {
            buf_putc(out, *p++);
        } else if ((*p == '+' || *p == '-') && (p[-1] == 'e' || p[-1] == 'E')) {
            buf_putc(out, *p++);
        } else {
            break;
        }
    }
    *pp = p;
}

/* p points just after the '('.  Fills args[0..maxargs-1], counts every argument in *nargs and returns the
   position after the closing ')', or NULL if the list is not terminated on this line. */
static const char* collect_args(const char* p, Buf* args, int maxargs, int* nargs) {
    int depth = 0;
    int n = 0;
    Buf* cur = (maxargs > 0) ? &args[0] : NULL;
    *nargs = 1;
    while (*p) {
        if (*p == '"' || *p == '\'') {
            char q = *p;
            if (cur) buf_putc(cur, *p);
            p++;
            while (*p && *p != q) {
                if (*p == '\\' && p[1]) {
                    if (cur) buf_putc(cur, *p);
                    p++;
                }
                if (cur) buf_putc(cur, *p);
                p++;
            }
            if (*p) {
                if (cur) buf_putc(cur, *p);
                p++;
            }
            continue;
        }
        if (*p == '(') {
            depth++;
        } else if (*p == ')') {
            if (depth == 0) return p + 1;
            depth--;
        } else if (*p == ',' && depth == 0) {
            (*nargs)++;
            n++;
            cur = (n < maxargs) ? &args[n] : NULL;
            p++;
            continue;
        }
        if (cur) buf_putc(cur, *p);
        p++;
    }
    return NULL;
}

static void expand(PreprocessorContext* ctx, const char* in, Buf* out, Macro** hide, int nhide, int depth) {
    const char* p = in;

    while (*p && !out->bad) {
        if (*p == '"' || *p == '\'') {
            copy_literal(&p, out);
            continue;
        }
        if (isdigit(UC(*p)) || (*p == '.' && isdigit(UC(p[1])))) {
            copy_number(&p, out);
            continue;
        }
        if (is_ident_start(*p)) {
            const char* start = p;
            int len;
            Macro* m;

            while (is_ident_char(*p)) p++;
            len = (int)(p - start);

            if (len == 8 && memcmp(start, "__LINE__", 8) == 0) {
                char nb[24];
                sprintf(nb, "%d", ctx->line_num);
                buf_puts(out, nb);
                continue;
            }
            if (len == 8 && memcmp(start, "__FILE__", 8) == 0) {
                const char* f = ctx->file_name ? ctx->file_name : "";
                buf_putc(out, '"');
                for (; *f; f++) {
                    if (*f == '"' || *f == '\\') buf_putc(out, '\\');
                    buf_putc(out, *f);
                }
                buf_putc(out, '"');
                continue;
            }

            m = find_macro_n(ctx, start, len);
            if (!m || is_hidden(hide, nhide, m)) {
                buf_putn(out, start, len);
                continue;
            }
            if (depth >= PP_MAX_DEPTH) {
                pp_error(ctx, "macro '%s' nested more than %d deep", m->name, PP_MAX_DEPTH);
                buf_putn(out, start, len);
                continue;
            }

            if (m->type == MACRO_SIMPLE) {
                hide[nhide] = m;
                expand(ctx, m->replacement, out, hide, nhide + 1, depth + 1);
                continue;
            }

            {
                const char* q = skip_ws(p);
                Buf* args;
                Buf sub;
                const char* after;
                const char* r;
                int maxargs, nargs, i, ok;

                if (*q != '(') {          /* a function-like macro's name alone is just an identifier */
                    buf_putn(out, start, len);
                    continue;
                }

                maxargs = m->param_count > 0 ? m->param_count : 1;
                args = (Buf*)malloc(sizeof(Buf) * maxargs);
                if (!args) {
                    pp_error(ctx, "out of memory");
                    buf_putn(out, start, len);
                    continue;
                }
                for (i = 0; i < maxargs; i++) buf_init(&args[i]);

                ok = 1;
                after = collect_args(q + 1, args, maxargs, &nargs);
                if (!after) {
                    pp_error(ctx, "unterminated argument list invoking macro '%s'", m->name);
                    ok = 0;
                } else {
                    if (m->param_count == 0 && nargs == 1 && *skip_ws(args[0].s ? args[0].s : "") == '\0') nargs = 0;
                    if (nargs != m->param_count) {
                        pp_error(ctx, "macro '%s' requires %d argument(s), but %d given",
                                 m->name, m->param_count, nargs);
                        ok = 0;
                    }
                    for (i = 0; i < maxargs; i++) {
                        if (args[i].bad) {
                            pp_error(ctx, "argument %d of macro '%s' is too long", i + 1, m->name);
                            ok = 0;
                        }
                    }
                }

                if (!ok) {
                    for (i = 0; i < maxargs; i++) buf_free(&args[i]);
                    free(args);
                    buf_putn(out, start, len);
                    continue;
                }

                /* arguments are fully macro-expanded before they are substituted */
                for (i = 0; i < m->param_count; i++) {
                    Buf ea;
                    buf_init(&ea);
                    expand(ctx, args[i].s, &ea, hide, nhide, depth + 1);
                    buf_free(&args[i]);
                    args[i] = ea;
                    buf_trim(&args[i]);
                }

                buf_init(&sub);
                r = m->replacement;
                while (*r) {
                    if (*r == '"' || *r == '\'') {
                        copy_literal(&r, &sub);
                    } else if (isdigit(UC(*r)) || (*r == '.' && isdigit(UC(r[1])))) {
                        copy_number(&r, &sub);
                    } else if (is_ident_start(*r)) {
                        const char* s0 = r;
                        int l0, k, hit = 0;
                        while (is_ident_char(*r)) r++;
                        l0 = (int)(r - s0);
                        for (k = 0; k < m->param_count; k++) {
                            if ((int)strlen(m->params[k]) == l0 && memcmp(m->params[k], s0, l0) == 0) {
                                if (args[k].s) buf_puts(&sub, args[k].s);
                                hit = 1;
                                break;
                            }
                        }
                        if (!hit) buf_putn(&sub, s0, l0);
                    } else {
                        buf_putc(&sub, *r++);
                    }
                }
                if (sub.bad) out->bad = 1;

                if (!out->bad) {
                    hide[nhide] = m;
                    expand(ctx, sub.s, out, hide, nhide + 1, depth + 1);
                }

                buf_free(&sub);
                for (i = 0; i < maxargs; i++) buf_free(&args[i]);
                free(args);
                p = after;
            }
            continue;
        }
        buf_putc(out, *p++);
    }
}

/* Expands every macro in line.  The result is malloc'd (the caller frees it); on overflow it is truncated and an
   error is recorded. */
char* preprocessor_expand_macros(PreprocessorContext* ctx, const char* line) {
    Buf out;
    Macro* hide[PP_MAX_DEPTH + 1];
    buf_init(&out);
    expand(ctx, line, &out, hide, 0, 0);
    if (out.bad) pp_error(ctx, "macro expansion too long (limit is %d bytes) or out of memory", PP_MAX_LINE);
    if (!out.s) return zp_strdup("");
    return out.s;
}

/* ------------------------------------------------------------------ */
/* #if expressions */

typedef struct {
    const char* p;
    PreprocessorContext* ctx;
    int err;
    int noeval;      /* inside an operand that short-circuiting or ?: does not evaluate */
    int depth;
} Ev;

enum { OP_NONE, OP_LOR, OP_LAND, OP_OR, OP_XOR, OP_AND, OP_EQ, OP_NE, OP_LT, OP_LE, OP_GT, OP_GE,
       OP_SHL, OP_SHR, OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD };

static const int op_prec[] = { 0, 1, 2, 3, 4, 5, 6, 6, 7, 7, 7, 7, 8, 8, 9, 9, 10, 10, 10 };

static void ev_fail(Ev* e, const char* msg) {
    if (!e->err) pp_error(e->ctx, "%s in #if expression", msg);
    e->err = 1;
}

static void ev_ws(Ev* e) {
    while (is_space(*e->p)) e->p++;
}

static int peek_op(const char* p, int* len) {
    *len = 1;
    switch (*p) {
    case '|': if (p[1] == '|') { *len = 2; return OP_LOR; } return OP_OR;
    case '&': if (p[1] == '&') { *len = 2; return OP_LAND; } return OP_AND;
    case '^': return OP_XOR;
    case '=': if (p[1] == '=') { *len = 2; return OP_EQ; } return OP_NONE;
    case '!': if (p[1] == '=') { *len = 2; return OP_NE; } return OP_NONE;
    case '<':
        if (p[1] == '=') { *len = 2; return OP_LE; }
        if (p[1] == '<') { *len = 2; return OP_SHL; }
        return OP_LT;
    case '>':
        if (p[1] == '=') { *len = 2; return OP_GE; }
        if (p[1] == '>') { *len = 2; return OP_SHR; }
        return OP_GT;
    case '+': return OP_ADD;
    case '-': return OP_SUB;
    case '*': return OP_MUL;
    case '/': return OP_DIV;
    case '%': return OP_MOD;
    }
    return OP_NONE;
}

static long ev_ternary(Ev* e);

/* one (possibly escaped) character of a character constant */
static long ev_char(Ev* e) {
    long v;
    if (*e->p != '\\') return (unsigned char)*e->p++;
    e->p++;
    switch (*e->p) {
    case 'n': e->p++; return 10;
    case 't': e->p++; return 9;
    case 'r': e->p++; return 13;
    case 'a': e->p++; return 7;
    case 'b': e->p++; return 8;
    case 'f': e->p++; return 12;
    case 'v': e->p++; return 11;
    case 'x':
        e->p++;
        v = 0;
        if (!isxdigit(UC(*e->p))) ev_fail(e, "bad \\x escape");
        while (isxdigit(UC(*e->p))) {
            int d = isdigit(UC(*e->p)) ? *e->p - '0' : (tolower(UC(*e->p)) - 'a' + 10);
            v = (long)(((unsigned long)v << 4) | (unsigned long)d);
            e->p++;
        }
        return v;
    default:
        if (*e->p >= '0' && *e->p <= '7') {
            int n = 0;
            v = 0;
            while (n < 3 && *e->p >= '0' && *e->p <= '7') {
                v = v * 8 + (*e->p - '0');
                e->p++;
                n++;
            }
            return v;
        }
        if (*e->p == '\0') {
            ev_fail(e, "unterminated character constant");
            return 0;
        }
        return (unsigned char)*e->p++;      /* \\ \' \" \? */
    }
}

static long ev_primary(Ev* e) {
    long v = 0;

    ev_ws(e);
    if (e->err) return 0;

    if (isdigit(UC(*e->p))) {
        unsigned long u = 0;
        int base = 10;
        if (e->p[0] == '0' && (e->p[1] == 'x' || e->p[1] == 'X')) {
            base = 16;
            e->p += 2;
            if (!isxdigit(UC(*e->p))) ev_fail(e, "invalid hexadecimal constant");
        } else if (e->p[0] == '0') {
            base = 8;
        }
        while (!e->err) {
            int d;
            if (isdigit(UC(*e->p))) d = *e->p - '0';
            else if (base == 16 && isxdigit(UC(*e->p))) d = tolower(UC(*e->p)) - 'a' + 10;
            else break;
            if (d >= base) {
                ev_fail(e, "invalid digit in constant");
                break;
            }
            u = u * (unsigned long)base + (unsigned long)d;
            e->p++;
        }
        while (*e->p == 'u' || *e->p == 'U' || *e->p == 'l' || *e->p == 'L') e->p++;
        if (is_ident_char(*e->p) || *e->p == '.') ev_fail(e, "invalid number");
        return (long)u;
    }

    if (*e->p == '\'') {
        e->p++;
        if (*e->p == '\'' || *e->p == '\0') {
            ev_fail(e, "empty character constant");
            return 0;
        }
        v = ev_char(e);
        if (*e->p != '\'') {
            ev_fail(e, "unterminated or multi-character constant");
            return 0;
        }
        e->p++;
        return v;
    }

    if (*e->p == '(') {
        e->p++;
        if (++e->depth > PP_EV_DEPTH) {
            ev_fail(e, "expression nested too deeply");
            return 0;
        }
        v = ev_ternary(e);
        e->depth--;
        ev_ws(e);
        if (*e->p != ')') {
            ev_fail(e, "missing ')'");
            return 0;
        }
        e->p++;
        return v;
    }

    if (is_ident_start(*e->p)) {
        /* identifiers left after expansion are 0; calling one is not allowed */
        while (is_ident_char(*e->p)) e->p++;
        ev_ws(e);
        if (*e->p == '(') ev_fail(e, "function call");
        return 0;
    }

    ev_fail(e, *e->p ? "unexpected token" : "unexpected end of expression");
    return 0;
}

static long ev_unary(Ev* e) {
    long v;
    ev_ws(e);
    if (e->err) return 0;
    if (*e->p == '+' || *e->p == '-' || *e->p == '!' || *e->p == '~') {
        char op = *e->p++;
        if (++e->depth > PP_EV_DEPTH) {
            ev_fail(e, "expression nested too deeply");
            return 0;
        }
        v = ev_unary(e);
        e->depth--;
        switch (op) {
        case '-': return (long)(0UL - (unsigned long)v);
        case '!': return v == 0;
        case '~': return (long)(~(unsigned long)v);
        }
        return v;
    }
    return ev_primary(e);
}

static long ev_apply(Ev* e, int op, long l, long r) {
    unsigned long ul = (unsigned long)l;
    unsigned long ur = (unsigned long)r;
    switch (op) {
    case OP_OR:  return (long)(ul | ur);
    case OP_XOR: return (long)(ul ^ ur);
    case OP_AND: return (long)(ul & ur);
    case OP_EQ:  return l == r;
    case OP_NE:  return l != r;
    case OP_LT:  return l < r;
    case OP_LE:  return l <= r;
    case OP_GT:  return l > r;
    case OP_GE:  return l >= r;
    case OP_SHL:
    case OP_SHR:
        if (r < 0 || r >= (long)(sizeof(long) * 8)) return 0;
        return op == OP_SHL ? (long)(ul << r) : (l >> r);
    case OP_ADD: return (long)(ul + ur);
    case OP_SUB: return (long)(ul - ur);
    case OP_MUL: return (long)(ul * ur);
    case OP_DIV:
    case OP_MOD:
        if (r == 0) {
            if (!e->noeval) ev_fail(e, "division by zero");
            return 0;
        }
        if (l == (long)(~0UL ^ (~0UL >> 1)) && r == -1) return op == OP_DIV ? l : 0;   /* LONG_MIN / -1 */
        return op == OP_DIV ? l / r : l % r;
    }
    return 0;
}

static long ev_bin(Ev* e, int minprec) {
    long l = ev_unary(e);
    while (!e->err) {
        int len, op, prec;
        long r;
        ev_ws(e);
        op = peek_op(e->p, &len);
        if (op == OP_NONE) break;
        prec = op_prec[op];
        if (prec < minprec) break;
        e->p += len;
        if (++e->depth > PP_EV_DEPTH) {
            ev_fail(e, "expression nested too deeply");
            return 0;
        }
        if (op == OP_LAND || op == OP_LOR) {
            int skip = (op == OP_LAND) ? (l == 0) : (l != 0);
            if (skip) e->noeval++;
            r = ev_bin(e, prec + 1);
            if (skip) e->noeval--;
            l = (op == OP_LAND) ? (l != 0 && r != 0) : (l != 0 || r != 0);
        } else {
            r = ev_bin(e, prec + 1);
            l = ev_apply(e, op, l, r);
        }
        e->depth--;
    }
    return l;
}

static long ev_ternary(Ev* e) {
    long c = ev_bin(e, 1);
    ev_ws(e);
    if (!e->err && *e->p == '?') {
        long a, b;
        e->p++;
        if (++e->depth > PP_EV_DEPTH) {
            ev_fail(e, "expression nested too deeply");
            return 0;
        }
        if (c == 0) e->noeval++;
        a = ev_ternary(e);
        if (c == 0) e->noeval--;
        ev_ws(e);
        if (*e->p != ':') {
            ev_fail(e, "missing ':'");
            return 0;
        }
        e->p++;
        if (c != 0) e->noeval++;
        b = ev_ternary(e);
        if (c != 0) e->noeval--;
        e->depth--;
        return c ? a : b;
    }
    return c;
}

static int macro_defined(PreprocessorContext* ctx, const char* name, int len) {
    if (len == 8 && (memcmp(name, "__LINE__", 8) == 0 || memcmp(name, "__FILE__", 8) == 0)) return 1;
    return find_macro_n(ctx, name, len) != NULL;
}

/* Evaluates the controlling expression of #if / #elif.  Errors are reported and count as false. */
static int eval_if(PreprocessorContext* ctx, const char* text) {
    Buf pre;
    char* expanded;
    Ev e;
    long v;
    const char* p = text;

    /* 1. defined X / defined(X) become 1 or 0 before anything is macro-expanded */
    buf_init(&pre);
    while (*p && !pre.bad) {
        if (*p == '\'' ) {
            copy_literal(&p, &pre);
        } else if (isdigit(UC(*p)) || (*p == '.' && isdigit(UC(p[1])))) {
            copy_number(&p, &pre);
        } else if (is_ident_start(*p)) {
            const char* s = p;
            while (is_ident_char(*p)) p++;
            if ((int)(p - s) == 7 && memcmp(s, "defined", 7) == 0) {
                int paren = 0;
                const char* n;
                p = skip_ws(p);
                if (*p == '(') {
                    paren = 1;
                    p = skip_ws(p + 1);
                }
                n = p;
                while (is_ident_char(*p)) p++;
                if (p == n || !is_ident_start(*n)) {
                    pp_error(ctx, "'defined' needs a macro name in #if expression");
                    buf_free(&pre);
                    return 0;
                }
                buf_puts(&pre, macro_defined(ctx, n, (int)(p - n)) ? " 1 " : " 0 ");
                if (paren) {
                    p = skip_ws(p);
                    if (*p != ')') {
                        pp_error(ctx, "missing ')' after 'defined' in #if expression");
                        buf_free(&pre);
                        return 0;
                    }
                    p++;
                }
            } else {
                buf_putn(&pre, s, (int)(p - s));
            }
        } else {
            buf_putc(&pre, *p++);
        }
    }
    if (pre.bad) {
        pp_error(ctx, "#if expression too long");
        buf_free(&pre);
        return 0;
    }

    /* 2. macro expansion */
    expanded = preprocessor_expand_macros(ctx, pre.s);
    buf_free(&pre);

    /* 3. evaluation */
    e.p = expanded;
    e.ctx = ctx;
    e.err = 0;
    e.noeval = 0;
    e.depth = 0;
    ev_ws(&e);
    if (*e.p == '\0') {
        pp_error(ctx, "#if with no expression");
        free(expanded);
        return 0;
    }
    v = ev_ternary(&e);
    ev_ws(&e);
    if (!e.err && *e.p != '\0') ev_fail(&e, "extra tokens at end of expression");
    free(expanded);
    return e.err ? 0 : (v != 0);
}

/* ------------------------------------------------------------------ */
/* directives */

/* the unsupported operators, outside string/character literals */
static const char* find_unsupported_op(const char* r, int function_like) {
    while (*r) {
        if (*r == '"' || *r == '\'') {
            char q = *r++;
            while (*r && *r != q) {
                if (*r == '\\' && r[1]) r++;
                r++;
            }
            if (*r) r++;
        } else if (r[0] == '#' && r[1] == '#') {
            return "'##' (token pasting)";
        } else if (r[0] == '#' && function_like) {
            return "'#' (stringizing)";
        } else {
            r++;
        }
    }
    return NULL;
}

static void do_define(PreprocessorContext* ctx, const char* p) {
    char* name;
    const char* s;
    char* params[PP_MAX_PARAMS];
    int np = 0;
    int fn = 0;
    int ok = 1;
    int i, n;
    char* repl;
    const char* bad;

    p = skip_ws(p);
    if (!is_ident_start(*p)) {
        pp_error(ctx, "macro name missing in #define");
        return;
    }
    s = p;
    while (is_ident_char(*p)) p++;
    n = (int)(p - s);
    name = (char*)malloc(n + 1);
    if (!name) {
        pp_error(ctx, "out of memory");
        return;
    }
    memcpy(name, s, n);
    name[n] = '\0';

    if (strcmp(name, "defined") == 0 || strcmp(name, "__LINE__") == 0 || strcmp(name, "__FILE__") == 0) {
        pp_error(ctx, "'%s' cannot be defined as a macro", name);
        free(name);
        return;
    }

    if (*p == '(') {            /* function-like only when the '(' follows the name directly */
        fn = 1;
        p = skip_ws(p + 1);
        if (*p == ')') {
            p++;
        } else {
            for (;;) {
                const char* id;
                char* copy;
                p = skip_ws(p);
                if (p[0] == '.' && p[1] == '.' && p[2] == '.') {
                    pp_error(ctx, "variadic macros are not supported (#define %s)", name);
                    ok = 0;
                    break;
                }
                if (!is_ident_start(*p)) {
                    pp_error(ctx, "invalid parameter list in #define %s", name);
                    ok = 0;
                    break;
                }
                id = p;
                while (is_ident_char(*p)) p++;
                n = (int)(p - id);
                for (i = 0; i < np; i++) {
                    if ((int)strlen(params[i]) == n && memcmp(params[i], id, n) == 0) {
                        pp_error(ctx, "duplicate parameter in #define %s", name);
                        ok = 0;
                    }
                }
                if (!ok) break;
                if (np >= PP_MAX_PARAMS) {
                    pp_error(ctx, "more than %d parameters in #define %s", PP_MAX_PARAMS, name);
                    ok = 0;
                    break;
                }
                copy = (char*)malloc(n + 1);
                if (!copy) {
                    pp_error(ctx, "out of memory");
                    ok = 0;
                    break;
                }
                memcpy(copy, id, n);
                copy[n] = '\0';
                params[np++] = copy;
                p = skip_ws(p);
                if (*p == ',') {
                    p++;
                    continue;
                }
                if (*p == ')') {
                    p++;
                    break;
                }
                pp_error(ctx, "expected ',' or ')' in parameter list of #define %s", name);
                ok = 0;
                break;
            }
        }
    }

    if (ok) {
        int len;
        p = skip_ws(p);
        repl = zp_strdup(p);
        len = (int)strlen(repl);
        while (len > 0 && is_space(repl[len - 1])) repl[--len] = '\0';
        bad = find_unsupported_op(repl, fn);
        if (bad) {
            pp_error(ctx, "%s is not supported (#define %s)", bad, name);
        } else {
            define_macro(ctx, name, fn ? MACRO_FUNCTION : MACRO_SIMPLE, params, np, repl);
        }
        free(repl);
    }

    for (i = 0; i < np; i++) free(params[i]);
    free(name);
}

static FILE* try_open(const char* dir, const char* name, char** opened) {
    FILE* f;
    char* path;
    int dl = (int)strlen(dir);
    int nl = (int)strlen(name);
    int absolute = (name[0] == '/' || name[0] == '\\' || (name[0] && name[1] == ':'));

    if (absolute || dl == 0) {
        path = zp_strdup(name);
    } else {
        path = (char*)malloc(dl + nl + 2);
        if (!path) return NULL;
        memcpy(path, dir, dl);
        if (dir[dl - 1] != '/' && dir[dl - 1] != '\\') path[dl++] = '/';
        memcpy(path + dl, name, nl + 1);
    }
    if (!path) return NULL;
    f = fopen(path, "r");
    if (f) *opened = path;
    else free(path);
    return f;
}

static int pp_stream(PreprocessorContext* ctx, FILE* f);

static void do_include(PreprocessorContext* ctx, const char* rest) {
    char* text = NULL;
    const char* p;
    char close;
    const char* start;
    char* name;
    char* path = NULL;
    FILE* f = NULL;
    int i, n, quoted;

    p = skip_ws(rest);
    if (*p != '"' && *p != '<') {
        /* computed include: macro-expand and look again */
        text = preprocessor_expand_macros(ctx, p);
        p = skip_ws(text);
    }
    if (*p != '"' && *p != '<') {
        pp_error(ctx, "#include expects \"FILE\" or <FILE>");
        free(text);
        return;
    }
    quoted = (*p == '"');
    close = quoted ? '"' : '>';
    start = ++p;
    while (*p && *p != close) p++;
    if (*p != close) {
        pp_error(ctx, "missing terminating %c in #include", close);
        free(text);
        return;
    }
    n = (int)(p - start);
    if (n == 0) {
        pp_error(ctx, "empty file name in #include");
        free(text);
        return;
    }
    name = (char*)malloc(n + 1);
    if (!name) {
        pp_error(ctx, "out of memory");
        free(text);
        return;
    }
    memcpy(name, start, n);
    name[n] = '\0';
    free(text);

    if (ctx->include_depth >= PP_MAX_DEPTH) {
        pp_error(ctx, "#include nested more than %d deep (recursive include?)", PP_MAX_DEPTH);
        free(name);
        return;
    }

    if (++ctx->include_total > PP_MAX_INCLUDES) {
        pp_error(ctx, "more than %d #include directives (recursive include?)", PP_MAX_INCLUDES);
        free(name);
        return;
    }

    /* "file": the including file's directory first; then every -I path; then (leniently) that directory for <file> */
    if (quoted) f = try_open(ctx->file_dir ? ctx->file_dir : "", name, &path);
    for (i = 0; !f && i < ctx->include_path_count; i++) f = try_open(ctx->include_paths[i], name, &path);
    if (!f && !quoted) f = try_open(ctx->file_dir ? ctx->file_dir : "", name, &path);

    if (!f) {
        pp_error(ctx, "cannot open include file '%s'", name);
        free(name);
        return;
    }
    free(name);

    {
        char* saved_name = ctx->file_name;
        char* saved_dir = ctx->file_dir;
        int saved_line = ctx->line_num;

        ctx->file_name = path;
        ctx->file_dir = NULL;
        set_dir_from_path(ctx, path);
        ctx->include_depth++;
        pp_stream(ctx, f);
        ctx->include_depth--;
        free(ctx->file_name);
        free(ctx->file_dir);
        ctx->file_name = saved_name;
        ctx->file_dir = saved_dir;
        ctx->line_num = saved_line;
    }
    fclose(f);
}

/* #line N ["file"]: returns the number the next line gets, or -1 on error */
static int do_line(PreprocessorContext* ctx, const char* rest) {
    char* text = preprocessor_expand_macros(ctx, rest);
    const char* p = skip_ws(text);
    long n = 0;
    int digits = 0;

    while (isdigit(UC(*p))) {
        n = n * 10 + (*p - '0');
        if (n > 2147483647L) break;
        p++;
        digits++;
    }
    if (!digits || n < 1 || n > 2147483647L) {
        pp_error(ctx, "#line needs a line number");
        free(text);
        return -1;
    }
    p = skip_ws(p);
    if (*p == '"') {
        const char* s = ++p;
        char* name;
        int len;
        while (*p && *p != '"') p++;
        if (*p != '"') {
            pp_error(ctx, "missing terminating \" in #line");
            free(text);
            return -1;
        }
        len = (int)(p - s);
        name = (char*)malloc(len + 1);
        if (name) {
            memcpy(name, s, len);
            name[len] = '\0';
            free(ctx->file_name);
            ctx->file_name = name;
        }
    } else if (*p) {
        pp_error(ctx, "invalid #line directive");
        free(text);
        return -1;
    }
    free(text);
    return (int)n;
}

static void do_directive(PreprocessorContext* ctx, const char* p, int* next_line) {
    const char* name;
    int nl;
    int active = cond_active(ctx);

#define IS(s) (nl == (int)sizeof(s) - 1 && memcmp(name, s, nl) == 0)

    p = skip_ws(p);
    if (*p == '\0') return;                       /* null directive */
    name = p;
    while (is_ident_char(*p)) p++;
    nl = (int)(p - name);
    if (nl == 0) {
        if (active) pp_error(ctx, "invalid preprocessing directive");
        return;
    }

    /* conditionals are tracked even inside a skipped region, so that nesting stays balanced */
    if (IS("if")) {
        if (!active) cond_push(ctx, 0, 0);
        else cond_push(ctx, 1, eval_if(ctx, p));
        return;
    }
    if (IS("ifdef") || IS("ifndef")) {
        int want = IS("ifdef");
        int def = 0;
        p = skip_ws(p);
        if (!active) {
            cond_push(ctx, 0, 0);
            return;
        }
        if (!is_ident_start(*p)) {
            pp_error(ctx, "macro name missing in #%s", want ? "ifdef" : "ifndef");
        } else {
            const char* s = p;
            while (is_ident_char(*p)) p++;
            def = macro_defined(ctx, s, (int)(p - s));
        }
        cond_push(ctx, 1, def == want);
        return;
    }
    if (IS("elif")) {
        PPCond* c;
        if (ctx->cond_depth == 0) {
            pp_error(ctx, "#elif without #if");
            return;
        }
        c = &ctx->conds[ctx->cond_depth - 1];
        if (c->seen_else) {
            pp_error(ctx, "#elif after #else");
            return;
        }
        if (!c->parent || c->taken) {
            c->active = 0;
        } else {
            int v = eval_if(ctx, p);
            c->active = v;
            c->taken = v;
        }
        return;
    }
    if (IS("else")) {
        PPCond* c;
        if (ctx->cond_depth == 0) {
            pp_error(ctx, "#else without #if");
            return;
        }
        c = &ctx->conds[ctx->cond_depth - 1];
        if (c->seen_else) {
            pp_error(ctx, "#else after #else");
            return;
        }
        c->seen_else = 1;
        c->active = c->parent && !c->taken;
        c->taken = 1;
        return;
    }
    if (IS("endif")) {
        if (ctx->cond_depth == 0) pp_error(ctx, "#endif without #if");
        else ctx->cond_depth--;
        return;
    }

    if (!active) return;                          /* everything else is ignored in a skipped region */

    if (IS("include")) {
        do_include(ctx, p);
    } else if (IS("define")) {
        do_define(ctx, p);
    } else if (IS("undef")) {
        p = skip_ws(p);
        if (!is_ident_start(*p)) {
            pp_error(ctx, "macro name missing in #undef");
        } else {
            const char* s = p;
            Macro* m;
            while (is_ident_char(*p)) p++;
            m = find_macro_n(ctx, s, (int)(p - s));
            if (m) macro_unlink(ctx, m);
        }
    } else if (IS("error")) {
        p = skip_ws(p);
        pp_error(ctx, "#error %s", p);
    } else if (IS("pragma")) {
        /* unknown pragmas are ignored, as the standard allows */
    } else if (IS("line")) {
        int n = do_line(ctx, p);
        if (n > 0) *next_line = n;
    } else {
        pp_error(ctx, "invalid preprocessing directive #%.*s", nl, name);
    }
#undef IS
}

/* ------------------------------------------------------------------ */
/* driver */

static int pp_stream(PreprocessorContext* ctx, FILE* f) {
    Buf line;
    int nphys;
    int next_line = 1;
    int base = ctx->cond_depth;

    buf_init(&line);
    for (;;) {
        const char* t;
        ctx->line_num = next_line;
        if (!read_logical_line(ctx, f, &line, &nphys)) break;
        next_line += nphys;
        t = skip_ws(line.s ? line.s : "");
        if (*t == '#') {
            do_directive(ctx, t + 1, &next_line);
        } else if (cond_active(ctx)) {
            char* expanded = preprocessor_expand_macros(ctx, line.s ? line.s : "");
            fputs(expanded, ctx->output);
            fputc('\n', ctx->output);
            free(expanded);
        }
    }
    buf_free(&line);

    if (ctx->cond_depth > base) {
        pp_error(ctx, "unterminated #if/#ifdef/#ifndef");
        ctx->cond_depth = base;
    }
    return ctx->errors == 0;
}

int preprocessor_run(PreprocessorContext* ctx) {
    ctx->include_depth = 0;
    ctx->include_total = 0;
    pp_stream(ctx, ctx->input);
    return ctx->errors == 0;
}

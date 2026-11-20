#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <stdio.h>

/* Macro definition types */
typedef enum {
    MACRO_SIMPLE,      /* #define FOO 123 */
    MACRO_FUNCTION     /* #define ADD(a,b) ((a)+(b)) */
} MacroType;

/* Macro definition structure */
typedef struct Macro {
    char* name;
    char* replacement;
    MacroType type;
    char** params;      /* For function-like macros */
    int param_count;
    struct Macro* next;
} Macro;

/* One level of #if nesting */
typedef struct {
    int active;         /* lines are being emitted */
    int taken;          /* some branch of this #if has already been taken */
    int parent;         /* the enclosing region was active when this #if was reached */
    int seen_else;
} PPCond;

/* Preprocessor context */
typedef struct {
    FILE* input;
    FILE* output;
    Macro* macros;
    char** include_paths;
    int include_path_count;
    int include_path_cap;
    char* file_name;       /* current file, for __FILE__ and diagnostics */
    char* file_dir;        /* directory of the current file ("" = working directory) */
    int line_num;          /* line of the directive / text being processed */
    PPCond* conds;
    int cond_depth;
    int cond_cap;
    int include_depth;
    int include_total;     /* #include directives processed so far (bounds recursive fan-out) */
    int errors;            /* diagnostics reported so far; preprocessor_run() fails if any */
} PreprocessorContext;

/* Preprocessor functions */
PreprocessorContext* preprocessor_create(FILE* input, FILE* output);
void preprocessor_set_file(PreprocessorContext* ctx, const char* name);
void preprocessor_add_include_path(PreprocessorContext* ctx, const char* path);
int preprocessor_run(PreprocessorContext* ctx);   /* 1 = ok, 0 = errors (already reported on stderr) */
void preprocessor_destroy(PreprocessorContext* ctx);

/* Macro functions */
void preprocessor_define(PreprocessorContext* ctx, const char* name, const char* replacement);
void preprocessor_define_function(PreprocessorContext* ctx, const char* name,
                                   char** params, int param_count, const char* replacement);
void preprocessor_undef(PreprocessorContext* ctx, const char* name);
Macro* preprocessor_find_macro(PreprocessorContext* ctx, const char* name);
char* preprocessor_expand_macros(PreprocessorContext* ctx, const char* line);

#endif

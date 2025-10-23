#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <stdio.h>

// Macro definition types
typedef enum {
    MACRO_SIMPLE,      // #define FOO 123
    MACRO_FUNCTION     // #define ADD(a,b) ((a)+(b))
} MacroType;

// Macro definition structure
typedef struct Macro {
    char* name;
    char* replacement;
    MacroType type;
    char** params;      // For function-like macros
    int param_count;
    struct Macro* next;
} Macro;

// Preprocessor context
typedef struct {
    FILE* input;
    FILE* output;
    Macro* macros;
    char** include_paths;
    int include_path_count;
    int line_num;
    int if_stack[256];   // Stack for #if/#ifdef nesting
    int if_depth;
    int skip_mode;       // 0 = not skipping, 1 = skipping due to #if
} PreprocessorContext;

// Preprocessor functions
PreprocessorContext* preprocessor_create(FILE* input, FILE* output);
void preprocessor_add_include_path(PreprocessorContext* ctx, const char* path);
int preprocessor_run(PreprocessorContext* ctx);
void preprocessor_destroy(PreprocessorContext* ctx);

// Macro functions
void preprocessor_define(PreprocessorContext* ctx, const char* name, const char* replacement);
void preprocessor_define_function(PreprocessorContext* ctx, const char* name,
                                   char** params, int param_count, const char* replacement);
void preprocessor_undef(PreprocessorContext* ctx, const char* name);
Macro* preprocessor_find_macro(PreprocessorContext* ctx, const char* name);
char* preprocessor_expand_macros(PreprocessorContext* ctx, const char* line);

#endif

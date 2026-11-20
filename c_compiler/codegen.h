#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include <stdio.h>

#define MAX_DIMS 4

/* Struct member information */
typedef struct {
    char* name;
    DataType type;
    int offset;
    int pointer_level;
    char* struct_name;
    TypeModifier modifiers;
    int ndim;             /* array member: number of dimensions and their sizes */
    int dims[MAX_DIMS];
} StructMember;

/* Struct definition */
typedef struct {
    char* name;
    StructMember* members;
    int member_count;
    int total_size;  /* Total size in bytes */
    int is_union;
    int align;       /* alignment in bytes: 1 for a struct of chars only, else 2 */
} StructDef;

/* Symbol table entry */
typedef struct {
    char* name;
    DataType type;
    int offset;       /* Stack offset for local variables (or array base) */
    int is_param;     /* 1 if parameter, 0 if local */
    int param_index;  /* Index in parameter list (for register allocation) */
    int is_array;     /* 1 if this is an array */
    int* array_sizes;  /* Array of sizes for each dimension */
    int array_dimensions; /* Number of dimensions (0 = not array, 1 = arr[N], 2 = arr[N][M]) */
    int pointer_level; /* 0 = not a pointer, 1 = *, 2 = **, etc. */
    char* struct_name; /* For struct types */
    TypeModifier modifiers; /* signed/unsigned */
} Symbol;

/* Loop context for break/continue */
typedef struct {
    int break_label;
    int continue_label;
    int break_depth;     /* ctx.depth a break must unwind to (the target may pop one more word itself) */
    int continue_depth;
} LoopContext;

/* Code generator context */
typedef struct {
    FILE* output;
    Symbol* symbols;
    int symbol_count;
    int symbol_capacity;
    int stack_offset;
    int label_counter;
    int string_counter;   /* Counter for string literals */
    char* current_function;
    StructDef* structs;
    int struct_count;
    int struct_capacity;
    LoopContext* loop_stack;
    int loop_depth;
    int loop_capacity;
    int depth;            /* bytes pushed on top of the frame while evaluating an expression */
    int scratch;          /* frame offset of the three scratch words */
    int frame_total;      /* bytes between S+1 and the return address */
    int ret_words;        /* return type of the function being generated */
    int ret_uns;
    int ret_char;         /* the function returns a char: values are narrowed to 8 bits on the way out */
    int ret_signed;       /* ... and sign-extended (signed char) rather than zero-extended */
    int scope_base;       /* first symbol of the innermost block (redeclarations are errors only within it) */
    char** addr_taken;    /* variables whose address is taken somewhere in the function being generated */
    int addr_taken_count;
} CodegenContext;

/* Code generation functions */
extern int codegen_no_hwloop;   /* set by --no-hwloop: counted loops always use compare-and-branch */
void codegen_program(ASTNode* node, FILE* output);

#endif

#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include <stdio.h>

// Struct member information
typedef struct {
    char* name;
    DataType type;
    int offset;
    int pointer_level;
    char* struct_name;
} StructMember;

// Struct definition
typedef struct {
    char* name;
    StructMember* members;
    int member_count;
    int total_size;  // Total size in bytes
} StructDef;

// Symbol table entry
typedef struct {
    char* name;
    DataType type;
    int offset;       // Stack offset for local variables (or array base)
    int is_param;     // 1 if parameter, 0 if local
    int param_index;  // Index in parameter list (for register allocation)
    int is_array;     // 1 if this is an array
    int* array_sizes;  // Array of sizes for each dimension
    int array_dimensions; // Number of dimensions (0 = not array, 1 = arr[N], 2 = arr[N][M])
    int pointer_level; // 0 = not a pointer, 1 = *, 2 = **, etc.
    char* struct_name; // For struct types
} Symbol;

// Loop context for break/continue
typedef struct {
    int break_label;
    int continue_label;
} LoopContext;

// Code generator context
typedef struct {
    FILE* output;
    Symbol* symbols;
    int symbol_count;
    int symbol_capacity;
    int stack_offset;
    int label_counter;
    int string_counter;   // Counter for string literals
    char* current_function;
    StructDef* structs;
    int struct_count;
    int struct_capacity;
    LoopContext* loop_stack;
    int loop_depth;
    int loop_capacity;
} CodegenContext;

// Code generation functions
void codegen_program(ASTNode* node, FILE* output);

#endif

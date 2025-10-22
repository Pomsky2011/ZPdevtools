#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include <stdio.h>

// Symbol table entry
typedef struct {
    char* name;
    DataType type;
    int offset;       // Stack offset for local variables
    int is_param;     // 1 if parameter, 0 if local
    int param_index;  // Index in parameter list (for register allocation)
} Symbol;

// Code generator context
typedef struct {
    FILE* output;
    Symbol* symbols;
    int symbol_count;
    int symbol_capacity;
    int stack_offset;
    int label_counter;
    char* current_function;
} CodegenContext;

// Code generation functions
void codegen_program(ASTNode* node, FILE* output);

#endif

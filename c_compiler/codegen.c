#include "codegen.h"
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

static CodegenContext ctx;

// Forward declarations
static void codegen_function(ASTNode* node);
static void codegen_statement(ASTNode* node);
static void codegen_expression(ASTNode* node);
static void codegen_member_address(ASTNode* node);
static void codegen_member_assign(ASTNode* node);
static void codegen_ptr_member_assign(ASTNode* node);
static void codegen_addr_of(ASTNode* node);
static void codegen_deref(ASTNode* node);

// Struct management functions
static void add_struct(StructDef def) {
    if (ctx.struct_count >= ctx.struct_capacity) {
        ctx.struct_capacity = ctx.struct_capacity == 0 ? 16 : ctx.struct_capacity * 2;
        ctx.structs = realloc(ctx.structs, ctx.struct_capacity * sizeof(StructDef));
    }
    ctx.structs[ctx.struct_count++] = def;
}

static StructDef* find_struct(const char* name) {
    for (int i = 0; i < ctx.struct_count; i++) {
        if (strcmp(ctx.structs[i].name, name) == 0) {
            return &ctx.structs[i];
        }
    }
    return NULL;
}

// Loop management functions
static void push_loop(int break_label, int continue_label) {
    if (ctx.loop_depth >= ctx.loop_capacity) {
        ctx.loop_capacity = ctx.loop_capacity == 0 ? 8 : ctx.loop_capacity * 2;
        ctx.loop_stack = realloc(ctx.loop_stack, ctx.loop_capacity * sizeof(LoopContext));
    }
    ctx.loop_stack[ctx.loop_depth].break_label = break_label;
    ctx.loop_stack[ctx.loop_depth].continue_label = continue_label;
    ctx.loop_depth++;
}

static void pop_loop() {
    if (ctx.loop_depth > 0) {
        ctx.loop_depth--;
    }
}

static LoopContext* current_loop() {
    if (ctx.loop_depth > 0) {
        return &ctx.loop_stack[ctx.loop_depth - 1];
    }
    return NULL;
}

// Symbol table functions
static void init_context(FILE* output) {
    ctx.output = output;
    ctx.symbols = NULL;
    ctx.symbol_count = 0;
    ctx.symbol_capacity = 0;
    ctx.stack_offset = 0;
    ctx.label_counter = 0;
    ctx.string_counter = 0;
    ctx.current_function = NULL;
    ctx.structs = NULL;
    ctx.struct_count = 0;
    ctx.struct_capacity = 0;
    ctx.loop_stack = NULL;
    ctx.loop_depth = 0;
    ctx.loop_capacity = 0;
}

static void clear_symbols() {
    for (int i = 0; i < ctx.symbol_count; i++) {
        free(ctx.symbols[i].name);
        if (ctx.symbols[i].array_sizes) {
            free(ctx.symbols[i].array_sizes);
        }
        if (ctx.symbols[i].struct_name) {
            free(ctx.symbols[i].struct_name);
        }
    }
    free(ctx.symbols);
    ctx.symbols = NULL;
    ctx.symbol_count = 0;
    ctx.symbol_capacity = 0;
}

static int local_var_index = 0;

static int get_type_size(DataType type, int pointer_level, const char* struct_name) {
    // Pointers are always 2 bytes (16-bit addresses)
    if (pointer_level > 0) {
        return 2;
    }

    switch (type) {
        case TYPE_INT:
            return 2;
        case TYPE_CHAR:
            return 1;
        case TYPE_STRUCT:
            if (struct_name) {
                StructDef* sdef = find_struct(struct_name);
                if (sdef) {
                    return sdef->total_size;
                }
            }
            return 2;  // Default fallback
        default:
            return 2;
    }
}

static void add_symbol(const char* name, DataType type, int is_param, int param_index, int is_array, int array_size, int pointer_level, const char* struct_name) {
    if (ctx.symbol_count >= ctx.symbol_capacity) {
        ctx.symbol_capacity = ctx.symbol_capacity == 0 ? 16 : ctx.symbol_capacity * 2;
        ctx.symbols = realloc(ctx.symbols, ctx.symbol_capacity * sizeof(Symbol));
    }

    ctx.symbols[ctx.symbol_count].name = strdup(name);
    ctx.symbols[ctx.symbol_count].type = type;
    ctx.symbols[ctx.symbol_count].is_param = is_param;
    ctx.symbols[ctx.symbol_count].param_index = param_index;
    ctx.symbols[ctx.symbol_count].is_array = is_array;
    if (is_array) {
        ctx.symbols[ctx.symbol_count].array_sizes = malloc(sizeof(int));
        ctx.symbols[ctx.symbol_count].array_sizes[0] = array_size;
        ctx.symbols[ctx.symbol_count].array_dimensions = 1;
    } else {
        ctx.symbols[ctx.symbol_count].array_sizes = NULL;
        ctx.symbols[ctx.symbol_count].array_dimensions = 0;
    }
    ctx.symbols[ctx.symbol_count].pointer_level = pointer_level;
    ctx.symbols[ctx.symbol_count].struct_name = struct_name ? strdup(struct_name) : NULL;

    if (is_param) {
        // Parameters are accessed via stack relative addressing
        // After JSR: [ret_addr:2] [saved_regs...] [params...]
        // We'll calculate actual offset later
        ctx.symbols[ctx.symbol_count].offset = 0;
    } else {
        // Local variables - positive offset from SP after allocation
        int elem_size = get_type_size(type, pointer_level, struct_name);
        if (is_array) {
            // Arrays take multiple slots
            ctx.symbols[ctx.symbol_count].offset = local_var_index * 2;
            local_var_index += array_size * (elem_size / 2);  // Reserve space for all elements
        } else {
            // Regular variables take one slot
            ctx.symbols[ctx.symbol_count].offset = local_var_index * 2;
            local_var_index += (elem_size + 1) / 2;  // Round up to word boundary
        }
    }

    ctx.symbol_count++;
}

// Add symbol with multi-dimensional array support
static void add_symbol_multidim(const char* name, DataType type, int is_param, int param_index, int* array_sizes, int dimensions, int pointer_level, const char* struct_name) {
    if (ctx.symbol_count >= ctx.symbol_capacity) {
        ctx.symbol_capacity = ctx.symbol_capacity == 0 ? 16 : ctx.symbol_capacity * 2;
        ctx.symbols = realloc(ctx.symbols, ctx.symbol_capacity * sizeof(Symbol));
    }

    ctx.symbols[ctx.symbol_count].name = strdup(name);
    ctx.symbols[ctx.symbol_count].type = type;
    ctx.symbols[ctx.symbol_count].is_param = is_param;
    ctx.symbols[ctx.symbol_count].param_index = param_index;
    ctx.symbols[ctx.symbol_count].is_array = (dimensions > 0);
    if (dimensions > 0) {
        ctx.symbols[ctx.symbol_count].array_sizes = malloc(sizeof(int) * dimensions);
        memcpy(ctx.symbols[ctx.symbol_count].array_sizes, array_sizes, sizeof(int) * dimensions);
        ctx.symbols[ctx.symbol_count].array_dimensions = dimensions;
    } else {
        ctx.symbols[ctx.symbol_count].array_sizes = NULL;
        ctx.symbols[ctx.symbol_count].array_dimensions = 0;
    }
    ctx.symbols[ctx.symbol_count].pointer_level = pointer_level;
    ctx.symbols[ctx.symbol_count].struct_name = struct_name ? strdup(struct_name) : NULL;

    if (is_param) {
        ctx.symbols[ctx.symbol_count].offset = 0;
    } else {
        int elem_size = get_type_size(type, pointer_level, struct_name);
        if (dimensions > 0) {
            // Calculate total array size
            int total_elements = 1;
            for (int i = 0; i < dimensions; i++) {
                total_elements *= array_sizes[i];
            }
            ctx.symbols[ctx.symbol_count].offset = local_var_index * 2;
            local_var_index += total_elements * (elem_size / 2);
        } else {
            ctx.symbols[ctx.symbol_count].offset = local_var_index * 2;
            local_var_index += (elem_size + 1) / 2;
        }
    }

    ctx.symbol_count++;
}

static Symbol* find_symbol(const char* name) {
    for (int i = ctx.symbol_count - 1; i >= 0; i--) {
        if (strcmp(ctx.symbols[i].name, name) == 0) {
            return &ctx.symbols[i];
        }
    }
    return NULL;
}

static int new_label() {
    return ctx.label_counter++;
}

// Emit functions
static void emit(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(ctx.output, fmt, args);
    va_end(args);
}

// Code generation for expressions
static void codegen_number(ASTNode* node) {
    // Load immediate into A
    emit("    LDA #$%04X\n", node->number & 0xFFFF);
}

static void codegen_identifier(ASTNode* node) {
    Symbol* sym = find_symbol(node->identifier);
    if (!sym) {
        fprintf(stderr, "Error: Undefined variable '%s'\n", node->identifier);
        return;
    }

    if (sym->is_param) {
        // Parameter - might be in register or on stack
        if (sym->param_index == 0) {
            // First parameter is already in A
            // Nothing to do if it's already there
        } else if (sym->param_index == 1) {
            // Second parameter is in X
            emit("    TXA\n");  // Transfer X to A
        } else if (sym->param_index == 2) {
            // Third parameter is in Y
            emit("    TYA\n");  // Transfer Y to A
        } else {
            // Stack parameter
            // Calculate offset: after saved registers
            int offset = 3 + (sym->param_index - 3) * 2;
            emit("    LDA %d,S\n", offset);
        }
    } else {
        // Local variable
        emit("    LDA %d,S\n", sym->offset);
    }
}

static void codegen_string_literal(ASTNode* node) {
    // Generate unique label for this string
    int str_id = ctx.string_counter++;

    // Load address of string into A
    emit("    LDA #.STR%d\n", str_id);

    // Store string in data section (will be emitted at end)
    // For now, we'll emit it inline with a comment
    emit("    ; String literal: %s\n", node->string_literal);

    // Note: Full implementation would collect strings and emit them in .rodata section
}

static void codegen_sizeof(ASTNode* node) {
    int size = 0;

    if (node->sizeof_expr.expr) {
        // sizeof(expression) - determine type of expression
        // For now, simplified: assume int
        size = 2;  // int is 2 bytes
    } else {
        // sizeof(type)
        size = get_type_size(node->sizeof_expr.size_type,
                            node->sizeof_expr.pointer_level,
                            node->sizeof_expr.type_name);
    }

    // Load size as immediate
    emit("    LDA #$%04X\n", size);
}

static void codegen_ternary(ASTNode* node) {
    int else_label = new_label();
    int done_label = new_label();

    // Evaluate condition
    codegen_expression(node->ternary.condition);

    // Branch if false
    emit("    CMP #$0000\n");
    emit("    BEQ .L%d\n", else_label);

    // True branch
    codegen_expression(node->ternary.then_expr);
    emit("    BRA .L%d\n", done_label);

    // False branch
    emit(".L%d:\n", else_label);
    codegen_expression(node->ternary.else_expr);

    emit(".L%d:\n", done_label);
}

static void codegen_binop(ASTNode* node) {
    // Generate code for left operand (result in A)
    codegen_expression(node->binop.left);

    // Save A to stack
    emit("    PHA\n");

    // Generate code for right operand (result in A)
    codegen_expression(node->binop.right);

    // Move right operand to X
    emit("    TAX\n");

    // Restore left operand to A
    emit("    PLA\n");

    // Perform operation
    switch (node->binop.op) {
        case OP_ADD:
            emit("    CLC\n");
            emit("    STX temp\n");
            emit("    ADC temp\n");
            break;
        case OP_SUB:
            emit("    SEC\n");
            emit("    STX temp\n");
            emit("    SBC temp\n");
            break;
        case OP_MUL:
            emit("    MUL X\n");  // Hardware multiply
            break;
        case OP_DIV:
            emit("    DIV X\n");  // Hardware divide
            break;
        case OP_MOD:
            // A % X: compute A - (A/X)*X
            emit("    PHA\n");           // Save A
            emit("    DIV X\n");         // A = A / X
            emit("    MUL X\n");         // A = (A/X) * X
            emit("    STA temp\n");      // Save result
            emit("    PLA\n");           // Restore original A
            emit("    SEC\n");
            emit("    SBC temp\n");      // A = A - (A/X)*X
            break;
        case OP_BIT_AND:
            emit("    STX temp\n");
            emit("    AND temp\n");
            break;
        case OP_BIT_OR:
            emit("    STX temp\n");
            emit("    ORA temp\n");
            break;
        case OP_BIT_XOR:
            emit("    STX temp\n");
            emit("    EOR temp\n");
            break;
        case OP_SHL:
            // Shift A left by X times
            emit("    CPX #$00\n");
            int shl_loop = new_label();
            int shl_done = new_label();
            emit("    BEQ .L%d\n", shl_done);
            emit(".L%d:\n", shl_loop);
            emit("    ASL A\n");
            emit("    DEX\n");
            emit("    BNE .L%d\n", shl_loop);
            emit(".L%d:\n", shl_done);
            break;
        case OP_SHR:
            // Shift A right by X times
            emit("    CPX #$00\n");
            int shr_loop = new_label();
            int shr_done = new_label();
            emit("    BEQ .L%d\n", shr_done);
            emit(".L%d:\n", shr_loop);
            emit("    LSR A\n");
            emit("    DEX\n");
            emit("    BNE .L%d\n", shr_loop);
            emit(".L%d:\n", shr_done);
            break;
        case OP_EQ:
        case OP_NE:
        case OP_LT:
        case OP_GT:
        case OP_LE:
        case OP_GE:
            // Comparison: A CMP X
            emit("    STX temp\n");
            emit("    CMP temp\n");

            int true_label = new_label();
            int done_label = new_label();

            switch (node->binop.op) {
                case OP_EQ:
                    emit("    BEQ .L%d\n", true_label);
                    break;
                case OP_NE:
                    emit("    BNE .L%d\n", true_label);
                    break;
                case OP_LT:
                    emit("    BMI .L%d\n", true_label);
                    break;
                case OP_GT:
                    // A > X: not equal and not less
                    emit("    BEQ .L%d\n", done_label);
                    emit("    BMI .L%d\n", done_label);
                    emit("    BRA .L%d\n", true_label);
                    break;
                case OP_LE:
                    // A <= X: less or equal
                    emit("    BEQ .L%d\n", true_label);
                    emit("    BMI .L%d\n", true_label);
                    break;
                case OP_GE:
                    // A >= X: not less (equal or greater)
                    emit("    BEQ .L%d\n", true_label);
                    emit("    BPL .L%d\n", true_label);
                    break;
                default:
                    break;
            }

            // False
            emit("    LDA #$0000\n");
            emit("    BRA .L%d\n", done_label);
            // True
            emit(".L%d:\n", true_label);
            emit("    LDA #$0001\n");
            emit(".L%d:\n", done_label);
            break;
        case OP_AND:
        case OP_OR:
            // Logical operators: treat non-zero as true
            // Already have left in A, right in X

            // Convert A to boolean (0 or 1)
            emit("    CMP #$0000\n");
            int a_true = new_label();
            int a_done = new_label();
            emit("    BNE .L%d\n", a_true);
            emit("    LDA #$0000\n");
            emit("    BRA .L%d\n", a_done);
            emit(".L%d:\n", a_true);
            emit("    LDA #$0001\n");
            emit(".L%d:\n", a_done);

            emit("    PHA\n");  // Save boolean A

            // Convert X to boolean
            emit("    TXA\n");
            emit("    CMP #$0000\n");
            int x_true = new_label();
            int x_done = new_label();
            emit("    BNE .L%d\n", x_true);
            emit("    LDA #$0000\n");
            emit("    BRA .L%d\n", x_done);
            emit(".L%d:\n", x_true);
            emit("    LDA #$0001\n");
            emit(".L%d:\n", x_done);

            emit("    TAX\n");  // X = boolean right
            emit("    PLA\n");  // A = boolean left

            if (node->binop.op == OP_AND) {
                emit("    STX temp\n");
                emit("    AND temp\n");
            } else {  // OP_OR
                emit("    STX temp\n");
                emit("    ORA temp\n");
            }
            break;
    }
}

static void codegen_unop(ASTNode* node) {
    switch (node->unop.op) {
        case OP_ADDR_OF:
            // Handled by codegen_addr_of
            codegen_addr_of(node);
            return;
        case OP_DEREF:
            // Handled by codegen_deref
            codegen_deref(node);
            return;
        default:
            break;
    }

    codegen_expression(node->unop.operand);

    switch (node->unop.op) {
        case OP_NEG:
            // Negate: 0 - A
            emit("    EOR #$FFFF\n");
            emit("    INC A\n");
            break;
        case OP_NOT:
            // Logical not: A = (A == 0) ? 1 : 0
            emit("    CMP #$0000\n");
            int not_true = new_label();
            int not_done = new_label();
            emit("    BEQ .L%d\n", not_true);
            emit("    LDA #$0000\n");
            emit("    BRA .L%d\n", not_done);
            emit(".L%d:\n", not_true);
            emit("    LDA #$0001\n");
            emit(".L%d:\n", not_done);
            break;
        case OP_BIT_NOT:
            emit("    EOR #$FFFF\n");
            break;
        case OP_ADDR_OF:
        case OP_DEREF:
            // Already handled above
            break;
    }
}

static void codegen_call(ASTNode* node) {
    // Generate code for arguments in reverse order (push right to left)
    // First 3 go in A, X, Y; rest on stack

    // Push arguments > 3 onto stack (in reverse)
    for (int i = node->call.arg_count - 1; i >= 3; i--) {
        codegen_expression(node->call.args[i]);
        emit("    PHA\n");
    }

    // Load first 3 arguments into registers
    if (node->call.arg_count >= 3) {
        codegen_expression(node->call.args[2]);
        emit("    TAY\n");  // Third arg in Y
    }

    if (node->call.arg_count >= 2) {
        codegen_expression(node->call.args[1]);
        emit("    TAX\n");  // Second arg in X
    }

    if (node->call.arg_count >= 1) {
        codegen_expression(node->call.args[0]);
        // First arg already in A
    }

    // Call function
    emit("    JSR %s\n", node->call.name);

    // Clean up stack if we pushed any arguments
    if (node->call.arg_count > 3) {
        int stack_bytes = (node->call.arg_count - 3) * 2;
        emit("    TSC\n");
        emit("    CLC\n");
        emit("    ADC #$%04X\n", stack_bytes);
        emit("    TCS\n");
    }

    // Result is in A
}

static void codegen_assign(ASTNode* node) {
    Symbol* sym = find_symbol(node->assign.var_name);
    if (!sym) {
        fprintf(stderr, "Error: Undefined variable '%s'\n", node->assign.var_name);
        return;
    }

    // Generate code for value (result in A)
    codegen_expression(node->assign.value);

    // Store to variable
    if (sym->is_param) {
        if (sym->param_index == 0) {
            // First parameter - just leave in A
            // But also need to update stack location if there is one
        } else if (sym->param_index == 1) {
            emit("    TAX\n");  // Update X register
        } else if (sym->param_index == 2) {
            emit("    TAY\n");  // Update Y register
        } else {
            // Stack parameter
            int offset = 3 + (sym->param_index - 3) * 2;
            emit("    STA %d,S\n", offset);
        }
    } else {
        // Local variable
        emit("    STA %d,S\n", sym->offset);
    }
}

static void codegen_array_subscript(ASTNode* node) {
    Symbol* sym = find_symbol(node->array_subscript.array_name);
    if (!sym) {
        fprintf(stderr, "Error: Undefined array '%s'\n", node->array_subscript.array_name);
        return;
    }

    if (!sym->is_array) {
        fprintf(stderr, "Error: '%s' is not an array\n", node->array_subscript.array_name);
        return;
    }

    // Calculate address: base + (index * 2)
    // First, evaluate index expression
    codegen_expression(node->array_subscript.index);

    // A now contains the index
    // Multiply by 2 (shift left once for 16-bit elements)
    emit("    ASL A\n");  // A = index * 2

    // Add base offset
    emit("    CLC\n");
    emit("    ADC #$%04X\n", sym->offset);  // A = base + (index * 2)

    // Now A contains the offset from SP
    // Calculate effective address: SP + A
    emit("    STA temp\n");      // Save offset
    emit("    TSC\n");           // A = SP
    emit("    CLC\n");
    emit("    ADC temp\n");      // A = SP + offset
    emit("    TAX\n");           // X = effective address

    // Load from address in X (using absolute indexed by 0)
    // DEF88186 doesn't have (X) addressing, so we need to use DP
    // Set DP = X, then load from (DP)
    emit("    TXA\n");
    emit("    TCD\n");           // DP = address
    emit("    LDA $00\n");       // Load from (DP)

    // Restore DP to 0 (or previous value)
    emit("    PHD\n");           // Will need to restore later - skip for now
}

static void codegen_array_assign(ASTNode* node) {
    Symbol* sym = find_symbol(node->array_assign.array_name);
    if (!sym) {
        fprintf(stderr, "Error: Undefined array '%s'\n", node->array_assign.array_name);
        return;
    }

    if (!sym->is_array) {
        fprintf(stderr, "Error: '%s' is not an array\n", node->array_assign.array_name);
        return;
    }

    // Generate code for value (result in A)
    codegen_expression(node->array_assign.value);
    emit("    PHA\n");  // Save value on stack

    // Calculate address: base + (index * 2)
    codegen_expression(node->array_assign.index);

    // A now contains the index
    emit("    ASL A\n");  // A = index * 2

    // Add base offset
    emit("    CLC\n");
    emit("    ADC #$%04X\n", sym->offset);  // A = base + (index * 2)

    // Now A contains the offset from SP
    emit("    TAX\n");           // X = offset

    // Restore value from stack
    emit("    PLA\n");           // A = value

    // Store to SP + X
    emit("    STA 0,S,X\n");     // Store to [SP + X]
}

static void codegen_addr_of(ASTNode* node) {
    // Calculate address of operand and leave it in A
    ASTNode* operand = node->unop.operand;

    if (operand->type == AST_IDENTIFIER) {
        Symbol* sym = find_symbol(operand->identifier);
        if (!sym) {
            fprintf(stderr, "Error: Undefined variable '%s'\n", operand->identifier);
            return;
        }

        // Calculate address: SP + offset
        emit("    TSC\n");           // A = SP
        if (sym->offset != 0) {
            emit("    CLC\n");
            emit("    ADC #$%04X\n", sym->offset);  // A = SP + offset
        }
        // A now contains the address
    } else if (operand->type == AST_ARRAY_SUBSCRIPT) {
        // Address of array element - similar to array subscript code
        Symbol* sym = find_symbol(operand->array_subscript.array_name);
        if (!sym) {
            fprintf(stderr, "Error: Undefined array '%s'\n", operand->array_subscript.array_name);
            return;
        }

        codegen_expression(operand->array_subscript.index);
        emit("    ASL A\n");  // A = index * 2
        emit("    CLC\n");
        emit("    ADC #$%04X\n", sym->offset);
        emit("    STA temp\n");
        emit("    TSC\n");
        emit("    CLC\n");
        emit("    ADC temp\n");  // A = SP + offset + (index * 2)
    } else if (operand->type == AST_DEREF) {
        // &(*ptr) = ptr, so just evaluate the pointer
        codegen_expression(operand->unop.operand);
    } else if (operand->type == AST_MEMBER_ACCESS) {
        // Address of struct member
        codegen_member_address(operand);
    } else {
        fprintf(stderr, "Error: Cannot take address of this expression\n");
    }
}

static void codegen_deref(ASTNode* node) {
    // Evaluate pointer expression (result in A = address)
    codegen_expression(node->unop.operand);

    // Load value from address in A
    emit("    TAX\n");           // X = address
    emit("    TXA\n");
    emit("    TCD\n");           // DP = address
    emit("    LDA $00\n");       // Load from (DP)
}

static void codegen_member_address(ASTNode* node) {
    // Calculate address of struct member
    if (node->type == AST_MEMBER_ACCESS) {
        // Get base object address
        if (node->member_access.object->type == AST_IDENTIFIER) {
            Symbol* sym = find_symbol(node->member_access.object->identifier);
            if (!sym || sym->type != TYPE_STRUCT) {
                fprintf(stderr, "Error: Not a struct variable\n");
                return;
            }

            StructDef* sdef = find_struct(sym->struct_name);
            if (!sdef) {
                fprintf(stderr, "Error: Struct type '%s' not found\n", sym->struct_name);
                return;
            }

            // Find member offset
            int member_offset = -1;
            for (int i = 0; i < sdef->member_count; i++) {
                if (strcmp(sdef->members[i].name, node->member_access.member_name) == 0) {
                    member_offset = sdef->members[i].offset;
                    break;
                }
            }

            if (member_offset < 0) {
                fprintf(stderr, "Error: Member '%s' not found in struct\n", node->member_access.member_name);
                return;
            }

            // Calculate address: SP + var_offset + member_offset
            emit("    TSC\n");
            emit("    CLC\n");
            emit("    ADC #$%04X\n", sym->offset + member_offset);
        } else {
            fprintf(stderr, "Error: Complex member access not yet supported\n");
        }
    }
}

static void codegen_member_access(ASTNode* node) {
    // Get address of member, then load value
    codegen_member_address(node);
    emit("    TAX\n");
    emit("    TXA\n");
    emit("    TCD\n");
    emit("    LDA $00\n");
}

static void codegen_ptr_member_access(ASTNode* node) {
    // ptr->member is equivalent to (*ptr).member
    // First evaluate pointer to get address
    codegen_expression(node->ptr_member_access.pointer);

    // A now contains the base address of the struct
    // Need to add member offset

    // Get struct type from pointer expression
    // This is simplified - in full implementation would need type tracking
    fprintf(stderr, "Warning: Pointer member access not fully implemented\n");

    // For now, just load from the address (member offset 0)
    emit("    TAX\n");
    emit("    TXA\n");
    emit("    TCD\n");
    emit("    LDA $00\n");
}

static void codegen_member_assign(ASTNode* node) {
    // Evaluate the value to assign (result in A)
    codegen_expression(node->member_assign.value);

    // Push value onto stack to preserve it
    emit("    PHA\n");

    // Calculate address of member
    if (node->member_assign.object->type == AST_IDENTIFIER) {
        Symbol* sym = find_symbol(node->member_assign.object->identifier);
        if (!sym || sym->type != TYPE_STRUCT) {
            fprintf(stderr, "Error: Not a struct variable\n");
            return;
        }

        StructDef* sdef = find_struct(sym->struct_name);
        if (!sdef) {
            fprintf(stderr, "Error: Struct type '%s' not found\n", sym->struct_name);
            return;
        }

        // Find member offset
        int member_offset = -1;
        for (int i = 0; i < sdef->member_count; i++) {
            if (strcmp(sdef->members[i].name, node->member_assign.member_name) == 0) {
                member_offset = sdef->members[i].offset;
                break;
            }
        }

        if (member_offset < 0) {
            fprintf(stderr, "Error: Member '%s' not found in struct\n", node->member_assign.member_name);
            return;
        }

        // Calculate address: SP + var_offset + member_offset
        emit("    TSC\n");
        emit("    CLC\n");
        emit("    ADC #$%04X\n", sym->offset + member_offset);

        // Store address in X, then use it via direct page
        emit("    TAX\n");
        emit("    TXA\n");
        emit("    TCD\n");

        // Pop value and store it
        emit("    PLA\n");
        emit("    STA $00\n");
    } else {
        fprintf(stderr, "Error: Complex member assignment not yet supported\n");
    }
}

static void codegen_ptr_member_assign(ASTNode* node) {
    // ptr->member = value
    // Evaluate the value to assign (result in A)
    codegen_expression(node->ptr_member_assign.value);

    // Push value onto stack to preserve it
    emit("    PHA\n");

    // Evaluate pointer to get base address
    codegen_expression(node->ptr_member_assign.pointer);

    // For now, assume member offset is 0 (simplified)
    fprintf(stderr, "Warning: Pointer member assignment not fully implemented\n");

    // Store address in X, then use it via direct page
    emit("    TAX\n");
    emit("    TXA\n");
    emit("    TCD\n");

    // Pop value and store it
    emit("    PLA\n");
    emit("    STA $00\n");
}

static void codegen_inc_dec(ASTNode* node) {
    Symbol* sym = find_symbol(node->inc_dec.var_name);
    if (!sym) {
        fprintf(stderr, "Error: Undefined variable '%s'\n", node->inc_dec.var_name);
        return;
    }

    switch (node->type) {
        case AST_PRE_INC:
            // Increment first, then load value
            if (sym->is_param && sym->param_index < 3) {
                // Parameter in register
                if (sym->param_index == 0) {
                    emit("    INC A\n");
                } else if (sym->param_index == 1) {
                    emit("    INX\n");
                    emit("    TXA\n");
                } else if (sym->param_index == 2) {
                    emit("    INY\n");
                    emit("    TYA\n");
                }
            } else {
                // Local variable or stack parameter
                emit("    LDA %d,S\n", sym->offset);
                emit("    INC A\n");
                emit("    STA %d,S\n", sym->offset);
                // Result is already in A
            }
            break;

        case AST_POST_INC:
            // Load value first, then increment
            if (sym->is_param && sym->param_index < 3) {
                // Parameter in register
                if (sym->param_index == 0) {
                    emit("    PHA\n");           // Save old value
                    emit("    INC A\n");
                    emit("    PLA\n");           // Restore old value to A
                } else if (sym->param_index == 1) {
                    emit("    TXA\n");
                    emit("    PHA\n");
                    emit("    INX\n");
                    emit("    PLA\n");
                } else if (sym->param_index == 2) {
                    emit("    TYA\n");
                    emit("    PHA\n");
                    emit("    INY\n");
                    emit("    PLA\n");
                }
            } else {
                // Local variable or stack parameter
                emit("    LDA %d,S\n", sym->offset);
                emit("    PHA\n");              // Save old value
                emit("    INC A\n");
                emit("    STA %d,S\n", sym->offset);
                emit("    PLA\n");              // Restore old value to A
            }
            break;

        case AST_PRE_DEC:
            // Decrement first, then load value
            if (sym->is_param && sym->param_index < 3) {
                if (sym->param_index == 0) {
                    emit("    DEC A\n");
                } else if (sym->param_index == 1) {
                    emit("    DEX\n");
                    emit("    TXA\n");
                } else if (sym->param_index == 2) {
                    emit("    DEY\n");
                    emit("    TYA\n");
                }
            } else {
                emit("    LDA %d,S\n", sym->offset);
                emit("    DEC A\n");
                emit("    STA %d,S\n", sym->offset);
            }
            break;

        case AST_POST_DEC:
            // Load value first, then decrement
            if (sym->is_param && sym->param_index < 3) {
                if (sym->param_index == 0) {
                    emit("    PHA\n");
                    emit("    DEC A\n");
                    emit("    PLA\n");
                } else if (sym->param_index == 1) {
                    emit("    TXA\n");
                    emit("    PHA\n");
                    emit("    DEX\n");
                    emit("    PLA\n");
                } else if (sym->param_index == 2) {
                    emit("    TYA\n");
                    emit("    PHA\n");
                    emit("    DEY\n");
                    emit("    PLA\n");
                }
            } else {
                emit("    LDA %d,S\n", sym->offset);
                emit("    PHA\n");
                emit("    DEC A\n");
                emit("    STA %d,S\n", sym->offset);
                emit("    PLA\n");
            }
            break;

        default:
            break;
    }
}

static void codegen_expression(ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case AST_NUMBER:
            codegen_number(node);
            break;
        case AST_IDENTIFIER:
            codegen_identifier(node);
            break;
        case AST_STRING_LITERAL:
            codegen_string_literal(node);
            break;
        case AST_SIZEOF:
            codegen_sizeof(node);
            break;
        case AST_TERNARY:
            codegen_ternary(node);
            break;
        case AST_BINOP:
            codegen_binop(node);
            break;
        case AST_UNOP:
            codegen_unop(node);
            break;
        case AST_CALL:
            codegen_call(node);
            break;
        case AST_ASSIGN:
            codegen_assign(node);
            break;
        case AST_ARRAY_SUBSCRIPT:
            codegen_array_subscript(node);
            break;
        case AST_ARRAY_ASSIGN:
            codegen_array_assign(node);
            break;
        case AST_ADDR_OF:
            codegen_addr_of(node);
            break;
        case AST_DEREF:
            codegen_deref(node);
            break;
        case AST_MEMBER_ACCESS:
            codegen_member_access(node);
            break;
        case AST_PTR_MEMBER_ACCESS:
            codegen_ptr_member_access(node);
            break;
        case AST_MEMBER_ASSIGN:
            codegen_member_assign(node);
            break;
        case AST_PTR_MEMBER_ASSIGN:
            codegen_ptr_member_assign(node);
            break;
        case AST_PRE_INC:
        case AST_POST_INC:
        case AST_PRE_DEC:
        case AST_POST_DEC:
            codegen_inc_dec(node);
            break;
        default:
            fprintf(stderr, "Error: Unsupported expression type %d\n", node->type);
            break;
    }
}

// Code generation for statements
static void codegen_return(ASTNode* node) {
    if (node->ret.value) {
        codegen_expression(node->ret.value);
        // Result is in A
    }

    // Epilogue - deallocate locals
    if (ctx.stack_offset < 0) {
        emit("    TSC\n");
        emit("    CLC\n");
        emit("    ADC #$%04X\n", -ctx.stack_offset);
        emit("    TCS\n");
    }

    emit("    RTS\n");
}

static void codegen_if(ASTNode* node) {
    int else_label = new_label();
    int done_label = new_label();

    // Evaluate condition
    codegen_expression(node->if_stmt.condition);

    // Branch if false
    emit("    CMP #$0000\n");
    if (node->if_stmt.else_stmt) {
        emit("    BEQ .L%d\n", else_label);
    } else {
        emit("    BEQ .L%d\n", done_label);
    }

    // Then branch
    codegen_statement(node->if_stmt.then_stmt);

    if (node->if_stmt.else_stmt) {
        emit("    BRA .L%d\n", done_label);
        emit(".L%d:\n", else_label);
        codegen_statement(node->if_stmt.else_stmt);
    }

    emit(".L%d:\n", done_label);
}

static void codegen_while(ASTNode* node) {
    int loop_label = new_label();
    int done_label = new_label();

    // Push loop context for break/continue
    push_loop(done_label, loop_label);

    emit(".L%d:\n", loop_label);

    // Evaluate condition
    codegen_expression(node->while_stmt.condition);

    // Branch if false
    emit("    CMP #$0000\n");
    emit("    BEQ .L%d\n", done_label);

    // Loop body
    codegen_statement(node->while_stmt.body);

    // Jump back to condition
    emit("    BRA .L%d\n", loop_label);

    emit(".L%d:\n", done_label);

    // Pop loop context
    pop_loop();
}

// Helper to detect simple counted loop suitable for LOOP/LPEND
static int is_simple_counted_loop(ASTNode* init, ASTNode* cond, ASTNode* incr,
                                   char** loop_var, int* loop_count) {
    if (!init || !cond || !incr) return 0;

    // Check init: must be "var = 0" or similar assignment
    if (init->type != AST_EXPR_STMT || !init->expr_stmt.expr) return 0;
    ASTNode* init_expr = init->expr_stmt.expr;
    if (init_expr->type != AST_ASSIGN) return 0;

    // Check that init is "var = 0"
    if (init_expr->assign.value->type != AST_NUMBER) return 0;
    if (init_expr->assign.value->number != 0) return 0;

    *loop_var = init_expr->assign.var_name;

    // Check condition: must be "var < N" where N is constant
    if (cond->type != AST_EXPR_STMT || !cond->expr_stmt.expr) return 0;
    ASTNode* cond_expr = cond->expr_stmt.expr;
    if (cond_expr->type != AST_BINOP) return 0;
    if (cond_expr->binop.op != OP_LT) return 0;

    // Left side must be same variable
    if (cond_expr->binop.left->type != AST_IDENTIFIER) return 0;
    if (strcmp(cond_expr->binop.left->identifier, *loop_var) != 0) return 0;

    // Right side must be constant
    if (cond_expr->binop.right->type != AST_NUMBER) return 0;
    *loop_count = cond_expr->binop.right->number;

    // Check increment: must be "var = var + 1" or simple increment
    if (incr->type != AST_EXPR_STMT || !incr->expr_stmt.expr) return 0;
    ASTNode* incr_expr = incr->expr_stmt.expr;
    if (incr_expr->type != AST_ASSIGN) return 0;

    // Must be same variable
    if (strcmp(incr_expr->assign.var_name, *loop_var) != 0) return 0;

    // Value must be "var + 1"
    if (incr_expr->assign.value->type != AST_BINOP) return 0;
    if (incr_expr->assign.value->binop.op != OP_ADD) return 0;

    ASTNode* add_left = incr_expr->assign.value->binop.left;
    ASTNode* add_right = incr_expr->assign.value->binop.right;

    if (add_left->type != AST_IDENTIFIER) return 0;
    if (strcmp(add_left->identifier, *loop_var) != 0) return 0;

    if (add_right->type != AST_NUMBER) return 0;
    if (add_right->number != 1) return 0;

    return 1;  // This is a simple counted loop!
}

static void codegen_for(ASTNode* node) {
    char* loop_var = NULL;
    int loop_count = 0;

    // Try to use hardware LOOP/LPEND for simple counted loops
    if (is_simple_counted_loop(node->for_stmt.init, node->for_stmt.condition,
                                node->for_stmt.increment, &loop_var, &loop_count)) {

        emit("    ; Hardware loop: for (%s = 0; %s < %d; %s++)\n",
             loop_var, loop_var, loop_count, loop_var);

        int loop_start = new_label();
        int done_label = new_label();

        // Initialize loop variable to 0
        codegen_statement(node->for_stmt.init);

        // Use hardware LOOP instruction
        emit("    LOOP #$%04X\n", loop_count);

        emit(".L%d:\n", loop_start);

        // Push loop context
        push_loop(done_label, loop_start);

        // Loop body
        codegen_statement(node->for_stmt.body);

        // Increment the loop variable
        codegen_statement(node->for_stmt.increment);

        // LPEND decrements hardware counter and loops back if non-zero
        emit("    LPEND\n");

        emit(".L%d:\n", done_label);

        // Pop loop context
        pop_loop();

        return;
    }

    // Fall back to manual loop for complex cases
    emit("    ; Manual loop (complex condition)\n");

    int loop_label = new_label();
    int done_label = new_label();
    int increment_label = new_label();

    // Initialization
    if (node->for_stmt.init) {
        codegen_statement(node->for_stmt.init);
    }

    // Push loop context (continue goes to increment, break goes to done)
    push_loop(done_label, increment_label);

    emit(".L%d:\n", loop_label);

    // Condition
    if (node->for_stmt.condition && node->for_stmt.condition->expr_stmt.expr) {
        codegen_expression(node->for_stmt.condition->expr_stmt.expr);
        emit("    CMP #$0000\n");
        emit("    BEQ .L%d\n", done_label);
    }

    // Body
    codegen_statement(node->for_stmt.body);

    // Increment label (for continue)
    emit(".L%d:\n", increment_label);

    // Increment
    if (node->for_stmt.increment) {
        codegen_statement(node->for_stmt.increment);
    }

    // Jump back to condition
    emit("    BRA .L%d\n", loop_label);

    emit(".L%d:\n", done_label);

    // Pop loop context
    pop_loop();
}

static void codegen_block(ASTNode* node) {
    for (int i = 0; i < node->block.stmt_count; i++) {
        codegen_statement(node->block.statements[i]);
    }
}

static void codegen_var_decl(ASTNode* node) {
    // Add variable to symbol table
    if (node->var_decl.array_dimensions > 0) {
        // Multi-dimensional array
        add_symbol_multidim(node->var_decl.var_name, node->var_decl.var_type, 0, 0,
                           node->var_decl.array_sizes, node->var_decl.array_dimensions,
                           node->var_decl.pointer_level, node->var_decl.struct_name);
    } else {
        // Regular variable or 1D array (backwards compatibility)
        int array_size = (node->var_decl.array_sizes && node->var_decl.is_array) ?
                         node->var_decl.array_sizes[0] : 0;
        add_symbol(node->var_decl.var_name, node->var_decl.var_type, 0, 0,
                   node->var_decl.is_array, array_size,
                   node->var_decl.pointer_level, node->var_decl.struct_name);
    }

    // Initialize if there's an initial value (not for arrays yet)
    if (node->var_decl.init_value && !node->var_decl.is_array) {
        codegen_expression(node->var_decl.init_value);

        Symbol* sym = find_symbol(node->var_decl.var_name);
        if (sym) {
            emit("    STA %d,S\n", sym->offset);
        }
    }
}

static void codegen_switch(ASTNode* node) {
    int end_label = new_label();

    // Push loop context for break statements
    push_loop(end_label, end_label);

    // Evaluate switch expression once and save to stack
    codegen_expression(node->switch_stmt.expr);
    emit("    PHA\n");  // Save switch value

    // Generate labels for each case
    int* case_labels = malloc(sizeof(int) * node->switch_stmt.case_count);
    int default_label = -1;

    for (int i = 0; i < node->switch_stmt.case_count; i++) {
        case_labels[i] = new_label();
        if (node->switch_stmt.cases[i]->type == AST_DEFAULT) {
            default_label = case_labels[i];
        }
    }

    // Generate comparisons for each case
    for (int i = 0; i < node->switch_stmt.case_count; i++) {
        ASTNode* case_node = node->switch_stmt.cases[i];
        if (case_node->type == AST_CASE) {
            emit("    LDA 1,S\n");  // Load switch value from stack
            emit("    CMP #$%04X\n", case_node->case_stmt.case_value);
            emit("    BEQ .L%d\n", case_labels[i]);
        }
    }

    // If no case matched, jump to default or end
    if (default_label >= 0) {
        emit("    BRA .L%d\n", default_label);
    } else {
        emit("    BRA .L%d\n", end_label);
    }

    // Generate code for each case
    for (int i = 0; i < node->switch_stmt.case_count; i++) {
        ASTNode* case_node = node->switch_stmt.cases[i];
        emit(".L%d:\n", case_labels[i]);

        if (case_node->type == AST_CASE) {
            for (int j = 0; j < case_node->case_stmt.stmt_count; j++) {
                codegen_statement(case_node->case_stmt.statements[j]);
            }
        } else if (case_node->type == AST_DEFAULT) {
            for (int j = 0; j < case_node->default_stmt.stmt_count; j++) {
                codegen_statement(case_node->default_stmt.statements[j]);
            }
        }
    }

    emit(".L%d:\n", end_label);
    emit("    PLA\n");  // Clean up stack (remove switch value)

    free(case_labels);

    // Pop loop context
    pop_loop();
}

static void codegen_expr_stmt(ASTNode* node) {
    if (node->expr_stmt.expr) {
        codegen_expression(node->expr_stmt.expr);
    }
}

static void codegen_break(ASTNode* node) {
    LoopContext* loop = current_loop();
    if (!loop) {
        fprintf(stderr, "Error: break statement outside of loop\n");
        return;
    }
    emit("    BRA .L%d\n", loop->break_label);
}

static void codegen_continue(ASTNode* node) {
    LoopContext* loop = current_loop();
    if (!loop) {
        fprintf(stderr, "Error: continue statement outside of loop\n");
        return;
    }
    emit("    BRA .L%d\n", loop->continue_label);
}

static void codegen_statement(ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case AST_RETURN:
            codegen_return(node);
            break;
        case AST_IF:
            codegen_if(node);
            break;
        case AST_WHILE:
            codegen_while(node);
            break;
        case AST_FOR:
            codegen_for(node);
            break;
        case AST_SWITCH:
            codegen_switch(node);
            break;
        case AST_BLOCK:
            codegen_block(node);
            break;
        case AST_VAR_DECL:
            codegen_var_decl(node);
            break;
        case AST_EXPR_STMT:
            codegen_expr_stmt(node);
            break;
        case AST_BREAK:
            codegen_break(node);
            break;
        case AST_CONTINUE:
            codegen_continue(node);
            break;
        default:
            fprintf(stderr, "Error: Unsupported statement type\n");
            break;
    }
}

// Helper to count local variables in a statement
static int count_locals(ASTNode* node);

static int count_locals_block(ASTNode* node) {
    int count = 0;
    for (int i = 0; i < node->block.stmt_count; i++) {
        count += count_locals(node->block.statements[i]);
    }
    return count;
}

static int count_locals(ASTNode* node) {
    if (!node) return 0;

    switch (node->type) {
        case AST_VAR_DECL:
            // Arrays count as multiple locals
            if (node->var_decl.is_array) {
                return (node->var_decl.array_sizes && node->var_decl.array_dimensions > 0) ?
                       node->var_decl.array_sizes[0] : 0;
            }
            return 1;
        case AST_BLOCK:
            return count_locals_block(node);
        case AST_IF:
            return count_locals(node->if_stmt.then_stmt) + count_locals(node->if_stmt.else_stmt);
        case AST_WHILE:
            return count_locals(node->while_stmt.body);
        case AST_FOR:
            return count_locals(node->for_stmt.body);
        default:
            return 0;
    }
}

// Code generation for functions
static void codegen_function(ASTNode* node) {
    ctx.current_function = node->func_decl.func_name;
    ctx.stack_offset = 0;
    local_var_index = 0;

    emit("\n; Function: %s\n", node->func_decl.func_name);
    emit("%s:\n", node->func_decl.func_name);

    // Set to 16-bit mode
    emit("    REP #$30        ; 16-bit A, X, Y\n");

    // Add parameters to symbol table
    for (int i = 0; i < node->func_decl.param_count; i++) {
        ASTNode* param = node->func_decl.params[i];
        add_symbol(param->param.param_name, param->param.param_type, 1, i, 0, 0,
                   param->param.pointer_level, param->param.struct_name);
    }

    // Count local variables to allocate stack space
    int num_locals = count_locals(node->func_decl.body);

    // Allocate stack space for locals if needed
    if (num_locals > 0) {
        int stack_size = num_locals * 2;  // 2 bytes per variable
        emit("    TSC\n");
        emit("    SEC\n");
        emit("    SBC #$%04X\n", stack_size);
        emit("    TCS\n");
        ctx.stack_offset = -stack_size;
    }

    // Generate code for function body
    codegen_statement(node->func_decl.body);

    // Add default return if needed
    if (num_locals > 0) {
        emit("    ; Epilogue\n");
        emit("    TSC\n");
        emit("    CLC\n");
        emit("    ADC #$%04X\n", num_locals * 2);
        emit("    TCS\n");
    }
    emit("    RTS\n");

    // Clear symbols for next function
    clear_symbols();
    ctx.current_function = NULL;
}

static void codegen_struct_decl(ASTNode* node) {
    StructDef sdef;
    sdef.name = strdup(node->struct_decl.struct_name);
    sdef.member_count = node->struct_decl.member_count;
    sdef.members = malloc(sizeof(StructMember) * sdef.member_count);

    int offset = 0;
    for (int i = 0; i < sdef.member_count; i++) {
        ASTNode* member = node->struct_decl.members[i];
        sdef.members[i].name = strdup(member->var_decl.var_name);
        sdef.members[i].type = member->var_decl.var_type;
        sdef.members[i].pointer_level = member->var_decl.pointer_level;
        sdef.members[i].struct_name = member->var_decl.struct_name ? strdup(member->var_decl.struct_name) : NULL;
        sdef.members[i].offset = offset;

        int size = get_type_size(member->var_decl.var_type, member->var_decl.pointer_level, member->var_decl.struct_name);
        offset += size;
    }

    sdef.total_size = offset;
    add_struct(sdef);

    emit("; Struct definition: %s (size = %d bytes)\n", sdef.name, sdef.total_size);
}

// Code generation for program
void codegen_program(ASTNode* node, FILE* output) {
    init_context(output);

    emit("; Generated by DEF88186 C Compiler\n");
    emit("; ZeroPoint Fantasy Console\n\n");

    // Emit data section for temporary variables
    emit(".data\n");
    emit("temp: .word 0\n");
    emit("\n.code\n");

    // First pass: process struct declarations
    for (int i = 0; i < node->program.decl_count; i++) {
        ASTNode* decl = node->program.decls[i];
        if (decl->type == AST_STRUCT_DECL) {
            codegen_struct_decl(decl);
        }
    }

    // Second pass: generate code for functions and variables
    for (int i = 0; i < node->program.decl_count; i++) {
        ASTNode* decl = node->program.decls[i];

        if (decl->type == AST_FUNC_DECL) {
            codegen_function(decl);
        } else if (decl->type == AST_VAR_DECL) {
            // Global variable
            emit("\n; Global variable: %s\n", decl->var_decl.var_name);
            int size = get_type_size(decl->var_decl.var_type, decl->var_decl.pointer_level, decl->var_decl.struct_name);
            if (size == 1) {
                emit("%s: .byte 0\n", decl->var_decl.var_name);
            } else {
                emit("%s: .word 0\n", decl->var_decl.var_name);
            }
        }
    }
}

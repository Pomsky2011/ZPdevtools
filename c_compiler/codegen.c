#include "codegen.h"
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

static CodegenContext ctx;

// Forward declarations
static void codegen_function(ASTNode* node);
static void codegen_statement(ASTNode* node);
static void codegen_expression(ASTNode* node);

// Symbol table functions
static void init_context(FILE* output) {
    ctx.output = output;
    ctx.symbols = NULL;
    ctx.symbol_count = 0;
    ctx.symbol_capacity = 0;
    ctx.stack_offset = 0;
    ctx.label_counter = 0;
    ctx.current_function = NULL;
}

static void clear_symbols() {
    for (int i = 0; i < ctx.symbol_count; i++) {
        free(ctx.symbols[i].name);
    }
    free(ctx.symbols);
    ctx.symbols = NULL;
    ctx.symbol_count = 0;
    ctx.symbol_capacity = 0;
}

static int local_var_index = 0;

static void add_symbol(const char* name, DataType type, int is_param, int param_index) {
    if (ctx.symbol_count >= ctx.symbol_capacity) {
        ctx.symbol_capacity = ctx.symbol_capacity == 0 ? 16 : ctx.symbol_capacity * 2;
        ctx.symbols = realloc(ctx.symbols, ctx.symbol_capacity * sizeof(Symbol));
    }

    ctx.symbols[ctx.symbol_count].name = strdup(name);
    ctx.symbols[ctx.symbol_count].type = type;
    ctx.symbols[ctx.symbol_count].is_param = is_param;
    ctx.symbols[ctx.symbol_count].param_index = param_index;

    if (is_param) {
        // Parameters are accessed via stack relative addressing
        // After JSR: [ret_addr:2] [saved_regs...] [params...]
        // We'll calculate actual offset later
        ctx.symbols[ctx.symbol_count].offset = 0;
    } else {
        // Local variables - positive offset from SP after allocation
        ctx.symbols[ctx.symbol_count].offset = local_var_index * 2;
        local_var_index++;
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

static void codegen_expression(ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case AST_NUMBER:
            codegen_number(node);
            break;
        case AST_IDENTIFIER:
            codegen_identifier(node);
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
        default:
            fprintf(stderr, "Error: Unsupported expression type\n");
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

        // Initialize loop variable to 0
        codegen_statement(node->for_stmt.init);

        // Use hardware LOOP instruction
        emit("    LOOP #$%04X\n", loop_count);

        // Loop body
        codegen_statement(node->for_stmt.body);

        // Increment the loop variable
        codegen_statement(node->for_stmt.increment);

        // LPEND decrements hardware counter and loops back if non-zero
        emit("    LPEND\n");

        return;
    }

    // Fall back to manual loop for complex cases
    emit("    ; Manual loop (complex condition)\n");

    int loop_label = new_label();
    int done_label = new_label();

    // Initialization
    if (node->for_stmt.init) {
        codegen_statement(node->for_stmt.init);
    }

    emit(".L%d:\n", loop_label);

    // Condition
    if (node->for_stmt.condition && node->for_stmt.condition->expr_stmt.expr) {
        codegen_expression(node->for_stmt.condition->expr_stmt.expr);
        emit("    CMP #$0000\n");
        emit("    BEQ .L%d\n", done_label);
    }

    // Body
    codegen_statement(node->for_stmt.body);

    // Increment
    if (node->for_stmt.increment) {
        codegen_statement(node->for_stmt.increment);
    }

    // Jump back to condition
    emit("    BRA .L%d\n", loop_label);

    emit(".L%d:\n", done_label);
}

static void codegen_block(ASTNode* node) {
    for (int i = 0; i < node->block.stmt_count; i++) {
        codegen_statement(node->block.statements[i]);
    }
}

static void codegen_var_decl(ASTNode* node) {
    // Add variable to symbol table
    add_symbol(node->var_decl.var_name, node->var_decl.var_type, 0, 0);

    // Initialize if there's an initial value
    if (node->var_decl.init_value) {
        codegen_expression(node->var_decl.init_value);

        Symbol* sym = find_symbol(node->var_decl.var_name);
        if (sym) {
            emit("    STA %d,S\n", sym->offset);
        }
    }
}

static void codegen_expr_stmt(ASTNode* node) {
    if (node->expr_stmt.expr) {
        codegen_expression(node->expr_stmt.expr);
    }
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
        case AST_BLOCK:
            codegen_block(node);
            break;
        case AST_VAR_DECL:
            codegen_var_decl(node);
            break;
        case AST_EXPR_STMT:
            codegen_expr_stmt(node);
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
        add_symbol(param->param.param_name, param->param.param_type, 1, i);
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

// Code generation for program
void codegen_program(ASTNode* node, FILE* output) {
    init_context(output);

    emit("; Generated by DEF88186 C Compiler\n");
    emit("; ZeroPoint Fantasy Console\n\n");

    // Emit data section for temporary variables
    emit(".data\n");
    emit("temp: .word 0\n");
    emit("\n.code\n");

    // Generate code for each declaration
    for (int i = 0; i < node->program.decl_count; i++) {
        ASTNode* decl = node->program.decls[i];

        if (decl->type == AST_FUNC_DECL) {
            codegen_function(decl);
        } else if (decl->type == AST_VAR_DECL) {
            // Global variable
            emit("\n; Global variable: %s\n", decl->var_decl.var_name);
            emit("%s: .word 0\n", decl->var_decl.var_name);
        }
    }
}

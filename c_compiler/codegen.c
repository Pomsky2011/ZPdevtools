#include "codegen.h"
#include <string.h>
#include "zpstrdup.h"
#include <stdlib.h>
#include <stdarg.h>

static CodegenContext ctx;

/* Forward declarations */
static void codegen_function(ASTNode* node);
static void codegen_statement(ASTNode* node);
static void codegen_expression(ASTNode* node);
static void codegen_member_address(ASTNode* node);
static void codegen_member_assign(ASTNode* node);
static void codegen_ptr_member_assign(ASTNode* node);
static void codegen_addr_of(ASTNode* node);
static void codegen_deref(ASTNode* node);

/* Struct management functions */
static void add_struct(StructDef def) {
    if (ctx.struct_count >= ctx.struct_capacity) {
        ctx.struct_capacity = ctx.struct_capacity == 0 ? 16 : ctx.struct_capacity * 2;
        ctx.structs = realloc(ctx.structs, ctx.struct_capacity * sizeof(StructDef));
    }
    ctx.structs[ctx.struct_count++] = def;
}

static StructDef* find_struct(const char* name) {
    int i;
    for (i = 0; i < ctx.struct_count; i++) {
        if (strcmp(ctx.structs[i].name, name) == 0) {
            return &ctx.structs[i];
        }
    }
    return NULL;
}

/* Loop management functions */
static void push_loop(int break_label, int continue_label) {
    if (ctx.loop_depth >= ctx.loop_capacity) {
        ctx.loop_capacity = ctx.loop_capacity == 0 ? 8 : ctx.loop_capacity * 2;
        ctx.loop_stack = realloc(ctx.loop_stack, ctx.loop_capacity * sizeof(LoopContext));
    }
    ctx.loop_stack[ctx.loop_depth].break_label = break_label;
    ctx.loop_stack[ctx.loop_depth].continue_label = continue_label;
    ctx.loop_stack[ctx.loop_depth].break_depth = ctx.depth;
    ctx.loop_stack[ctx.loop_depth].continue_depth = ctx.depth;
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

/* Symbol table functions */
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
    int i;
    for (i = 0; i < ctx.symbol_count; i++) {
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
    /* Pointers are always 2 bytes (16-bit addresses) */
    if (pointer_level > 0) {
        return 2;
    }

    switch (type) {
        case TYPE_INT:
            return 2;
        case TYPE_SHORT:
            return 2;
        case TYPE_LONG:
            return 4;
        case TYPE_CHAR:
            return 1;
        case TYPE_BOOL:
            return 1;
        case TYPE_STRUCT:
            if (struct_name) {
                StructDef* sdef = find_struct(struct_name);
                if (sdef) {
                    return sdef->total_size;
                }
            }
            return 2;  /* Default fallback */
        default:
            return 2;
    }
}

static void add_symbol(const char* name, DataType type, int is_param, int param_index, int is_array, int array_size, int pointer_level, const char* struct_name) {
    if (ctx.symbol_count >= ctx.symbol_capacity) {
        ctx.symbol_capacity = ctx.symbol_capacity == 0 ? 16 : ctx.symbol_capacity * 2;
        ctx.symbols = realloc(ctx.symbols, ctx.symbol_capacity * sizeof(Symbol));
    }

    ctx.symbols[ctx.symbol_count].name = zp_strdup(name);
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
    ctx.symbols[ctx.symbol_count].struct_name = struct_name ? zp_strdup(struct_name) : NULL;
    ctx.symbols[ctx.symbol_count].modifiers = MOD_NONE;

    if (is_param) {
        /* Parameters are accessed via stack relative addressing */
        /* After JSR: [ret_addr:2] [saved_regs...] [params...] */
        /* We'll calculate actual offset later */
        ctx.symbols[ctx.symbol_count].offset = 0;
    } else {
        /* Local variables - positive offset from SP after allocation */
        int elem_size = get_type_size(type, pointer_level, struct_name);
        if (is_array) {
            /* Arrays take multiple slots */
            ctx.symbols[ctx.symbol_count].offset = local_var_index * 2;
            local_var_index += array_size * ((elem_size + 1) / 2);  /* Reserve space for all elements */
        } else {
            /* Regular variables take one slot */
            ctx.symbols[ctx.symbol_count].offset = local_var_index * 2;
            local_var_index += (elem_size + 1) / 2;  /* Round up to word boundary */
        }
    }

    ctx.symbol_count++;
}

/* Add symbol with multi-dimensional array support */
static void add_symbol_multidim(const char* name, DataType type, int is_param, int param_index, int* array_sizes, int dimensions, int pointer_level, const char* struct_name) {
    int i;
    if (ctx.symbol_count >= ctx.symbol_capacity) {
        ctx.symbol_capacity = ctx.symbol_capacity == 0 ? 16 : ctx.symbol_capacity * 2;
        ctx.symbols = realloc(ctx.symbols, ctx.symbol_capacity * sizeof(Symbol));
    }

    ctx.symbols[ctx.symbol_count].name = zp_strdup(name);
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
    ctx.symbols[ctx.symbol_count].struct_name = struct_name ? zp_strdup(struct_name) : NULL;
    ctx.symbols[ctx.symbol_count].modifiers = MOD_NONE;

    if (is_param) {
        ctx.symbols[ctx.symbol_count].offset = 0;
    } else {
        int elem_size = get_type_size(type, pointer_level, struct_name);
        if (dimensions > 0) {
            /* Calculate total array size */
            int total_elements = 1;
            for (i = 0; i < dimensions; i++) {
                total_elements *= array_sizes[i];
            }
            ctx.symbols[ctx.symbol_count].offset = local_var_index * 2;
            local_var_index += total_elements * ((elem_size + 1) / 2);
        } else {
            ctx.symbols[ctx.symbol_count].offset = local_var_index * 2;
            local_var_index += (elem_size + 1) / 2;
        }
    }

    ctx.symbol_count++;
}

static Symbol* find_symbol(const char* name) {
    int i;
    for (i = ctx.symbol_count - 1; i >= 0; i--) {
        if (strcmp(ctx.symbols[i].name, name) == 0) {
            return &ctx.symbols[i];
        }
    }
    return NULL;
}

static int new_label() {
    return ctx.label_counter++;
}

/* Emit functions */
static void emit(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(ctx.output, fmt, args);
    va_end(args);
}


/* ====================================================================================================
 * Stack discipline
 *
 * S points at the next free byte, so the byte at offset k from the lowest address of the frame is at S+1+k
 * (the old code used S+k, which put the first local on the free slot a JSR push overwrites).  ctx.depth counts
 * the bytes pushed on top of the frame while an expression is being evaluated (PHA temporaries, call
 * arguments), so SPO() gives the right "n,S" operand at every point.
 *
 * Frame, lowest address first:   locals | three scratch words | register-argument slots | return address |
 * stack arguments.  Register arguments are pushed by the prologue (PHY/PHX/PHA) before anything touches A.
 * ==================================================================================================== */
static int SPO(int frame_off) { return frame_off + 1 + ctx.depth; }
#define SCRATCH0 (ctx.scratch)
#define SCRATCH1 (ctx.scratch + 2)
#define SCRATCH2 (ctx.scratch + 4)

static void push_a(void) { emit("    PHA\n"); ctx.depth += 2; }
static void pop_a(void)  { emit("    PLA\n"); ctx.depth -= 2; }
/* A:X pair (lo, hi): lo ends up at the lower address, so a PLA pops the low word first. */
static void push_ax(void) { emit("    PHX\n    PHA\n"); ctx.depth += 4; }

/* S += n without disturbing A or X: A is parked in Y, which carries nothing across a call or return. */
static void emit_drop_bytes(int n) {
    if (n <= 0) return;
    emit("    TAY\n    TSC\n    CLC\n    ADC #$%04X\n    TCS\n    TYA\n", n & 0xFFFF);
}

/* ====================================================================================================
 * Expression types.  A value is one word (A) or two (A = low, X = high); the only other thing the code
 * generator has to know is whether it is unsigned (compares, shifts right, divide, widening).
 * ==================================================================================================== */
typedef struct { int words; int uns; } CType;
static CType ctype(int words, int uns) { CType t; t.words = words; t.uns = uns; return t; }

static CType type_ctype(DataType t, TypeModifier m, int ptr) {
    if (ptr > 0) return ctype(1, 1);
    if (t == TYPE_LONG) return ctype(2, (m & MOD_UNSIGNED) != 0);
    if (t == TYPE_CHAR) return ctype(1, !(m & MOD_SIGNED));   /* plain char is unsigned here */
    if (t == TYPE_BOOL) return ctype(1, 1);
    return ctype(1, (m & MOD_UNSIGNED) != 0);
}

static CType usual(CType a, CType b) {
    if (a.words == 2 || b.words == 2)
        return ctype(2, (a.words == 2 && a.uns) || (b.words == 2 && b.uns));
    return ctype(1, a.uns || b.uns);
}

#define MAX_FUNCS 512
#define MAX_PARAMS 16
typedef struct { char* name; CType ret; int nparams; CType params[MAX_PARAMS]; } FuncInfo;
static FuncInfo func_table[MAX_FUNCS];
static int func_count = 0;

static FuncInfo* find_func(const char* name) {
    int i;
    for (i = 0; i < func_count; i++)
        if (strcmp(func_table[i].name, name) == 0) return &func_table[i];
    return NULL;
}

static void register_func(ASTNode* node) {
    FuncInfo* f;
    int i;
    if (func_count >= MAX_FUNCS) return;
    f = &func_table[func_count++];
    f->name = node->u.func_decl.func_name;
    f->ret = type_ctype(node->u.func_decl.return_type, node->u.func_decl.return_modifiers, 0);
    f->nparams = node->u.func_decl.param_count < MAX_PARAMS ? node->u.func_decl.param_count : MAX_PARAMS;
    for (i = 0; i < f->nparams; i++) {
        ASTNode* p = node->u.func_decl.params[i];
        f->params[i] = type_ctype(p->u.param.param_type, p->u.param.modifiers, p->u.param.pointer_level);
    }
}

static CType sym_ctype(Symbol* sym) {
    if (sym->is_array) return ctype(1, 1);
    return type_ctype(sym->type, sym->modifiers, sym->pointer_level);
}

static CType expr_type(ASTNode* n) {
    Symbol* sym;
    FuncInfo* f;
    unsigned int v;
    CType a, b;
    if (!n) return ctype(1, 0);
    switch (n->type) {
        case AST_NUMBER:
            v = (unsigned int)n->u.number;
            if (v <= 0x7FFFu) return ctype(1, 0);
            if (v <= 0xFFFFu) return ctype(1, 1);
            if (v <= 0x7FFFFFFFu) return ctype(2, 0);
            return ctype(2, 1);
        case AST_IDENTIFIER:
            sym = find_symbol(n->u.identifier);
            return sym ? sym_ctype(sym) : ctype(1, 0);
        case AST_STRING_LITERAL:
        case AST_SIZEOF:
        case AST_ADDR_OF:
            return ctype(1, 1);
        case AST_CAST:
            if (n->u.cast.pointer_level > 0) return ctype(1, 1);
            return type_ctype(n->u.cast.target_type, n->mods, 0);
        case AST_COMMA:
            return n->u.comma.expr_count > 0 ? expr_type(n->u.comma.expressions[n->u.comma.expr_count - 1]) : ctype(1, 0);
        case AST_TERNARY:
            return usual(expr_type(n->u.ternary.then_expr), expr_type(n->u.ternary.else_expr));
        case AST_BINOP:
            switch (n->u.binop.op) {
                case OP_EQ: case OP_NE: case OP_LT: case OP_GT: case OP_LE: case OP_GE:
                case OP_AND: case OP_OR:
                    return ctype(1, 0);
                case OP_SHL: case OP_SHR:
                    a = expr_type(n->u.binop.left);
                    return ctype(a.words, a.uns);
                default:
                    a = expr_type(n->u.binop.left);
                    b = expr_type(n->u.binop.right);
                    return usual(a, b);
            }
        case AST_UNOP:
            if (n->u.unop.op == OP_NOT) return ctype(1, 0);
            if (n->u.unop.op == OP_ADDR_OF) return ctype(1, 1);
            if (n->u.unop.op == OP_DEREF) {
                if (n->u.unop.operand->type == AST_IDENTIFIER) {
                    sym = find_symbol(n->u.unop.operand->u.identifier);
                    if (sym && sym->pointer_level > 0)
                        return type_ctype(sym->type, sym->modifiers, sym->pointer_level - 1);
                }
                return ctype(1, 0);
            }
            return expr_type(n->u.unop.operand);
        case AST_DEREF:
            if (n->u.unop.operand->type == AST_IDENTIFIER) {
                sym = find_symbol(n->u.unop.operand->u.identifier);
                if (sym && sym->pointer_level > 0)
                    return type_ctype(sym->type, sym->modifiers, sym->pointer_level - 1);
            }
            return ctype(1, 0);
        case AST_CALL:
            f = find_func(n->u.call.name);
            return f ? f->ret : ctype(1, 0);
        case AST_ASSIGN:
            sym = find_symbol(n->u.assign.var_name);
            return sym ? sym_ctype(sym) : ctype(1, 0);
        case AST_ARRAY_SUBSCRIPT:
            sym = find_symbol(n->u.array_subscript.array_name);
            return sym ? type_ctype(sym->type, sym->modifiers, sym->pointer_level) : ctype(1, 0);
        case AST_PRE_INC: case AST_POST_INC: case AST_PRE_DEC: case AST_POST_DEC:
            sym = find_symbol(n->u.inc_dec.var_name);
            return sym ? sym_ctype(sym) : ctype(1, 0);
        default:
            return ctype(1, 0);
    }
}

/* A (and X) hold a value of type 'from'; make them hold it as 'to'. */
static void emit_convert(CType from, CType to) {
    int pos, done;
    if (from.words == to.words || to.words == 1) return;   /* long -> int: the low word is already in A */
    if (from.uns) { emit("    LDX #$0000\n"); return; }       /* int -> long: X = zero / sign extension of A */
    pos = new_label();
    done = new_label();
    emit("    CMP #$0000\n");
    emit("    BPL .L%d\n", pos);
    emit("    LDX #$FFFF\n");
    emit("    BRA .L%d\n", done);
    emit(".L%d:\n", pos);
    emit("    LDX #$0000\n");
    emit(".L%d:\n", done);
}

static void codegen_expr_to(ASTNode* n, CType to) {
    CType from = expr_type(n);
    codegen_expression(n);
    emit_convert(from, to);
}

/* Evaluate n for a truth test: A is nonzero exactly when n is true (a long is or'd down to one word). */
static void codegen_truth(ASTNode* n) {
    CType t = expr_type(n);
    codegen_expression(n);
    if (t.words == 2)
        emit("    PHA\n    TXA\n    ORA 1,S\n    TAY\n    PLA\n    TYA\n");
}

static void load_sym(Symbol* sym) {
    CType t = sym_ctype(sym);
    if (t.words == 2) {
        emit("    LDA %d,S\n", SPO(sym->offset + 2));
        emit("    TAX\n");
    }
    emit("    LDA %d,S\n", SPO(sym->offset));
}

/* Store A (and X) into sym, converted by the caller to its type; A (and X) still hold the value afterwards. */
static void store_sym(Symbol* sym) {
    CType t = sym_ctype(sym);
    emit("    STA %d,S\n", SPO(sym->offset));
    if (t.words == 2) {
        emit("    TXA\n");
        emit("    STA %d,S\n", SPO(sym->offset + 2));
        emit("    LDA %d,S\n", SPO(sym->offset));
    }
}

/* Code generation for expressions */
static void codegen_number(ASTNode* node) {
    CType t = expr_type(node);
    unsigned int v = (unsigned int)node->u.number;
    emit("    LDA #$%04X\n", (unsigned)(v & 0xFFFF));
    if (t.words == 2) emit("    LDX #$%04X\n", (unsigned)((v >> 16) & 0xFFFF));
}

static void codegen_identifier(ASTNode* node) {
    Symbol* sym = find_symbol(node->u.identifier);
    if (!sym) {
        fprintf(stderr, "Error: Undefined variable '%s'\n", node->u.identifier);
        return;
    }
    load_sym(sym);
}

static void codegen_string_literal(ASTNode* node) {
    /* Generate unique label for this string */
    int str_id = ctx.string_counter++;

    /* Load address of string into A */
    emit("    LDA #.STR%d\n", str_id);

    /* Store string in data section (will be emitted at end) */
    /* For now, we'll emit it inline with a comment */
    emit("    ; String literal: %s\n", node->u.string_literal);

    /* Note: Full implementation would collect strings and emit them in .rodata section */
}

static void codegen_sizeof(ASTNode* node) {
    int size = 0;

    if (node->u.sizeof_expr.expr) {
        CType t = expr_type(node->u.sizeof_expr.expr);
        size = t.words * 2;
    } else {
        size = get_type_size(node->u.sizeof_expr.size_type,
                            node->u.sizeof_expr.pointer_level,
                            node->u.sizeof_expr.type_name);
    }

    emit("    LDA #$%04X\n", size);
}

/* Code generation for cast expressions */
static void codegen_cast(ASTNode* node) {
    CType from = expr_type(node->u.cast.expr);
    CType to;
    int ok, done;

    codegen_expression(node->u.cast.expr);

    if (node->u.cast.pointer_level > 0) {
        return;                      /* pointers are one word: the low word is already in A */
    }
    to = type_ctype(node->u.cast.target_type, node->mods, 0);

    switch (node->u.cast.target_type) {
        case TYPE_CHAR:
            emit("    AND #$00FF    ; Cast to char (truncate to 8 bits)\n");
            if (node->mods & MOD_SIGNED) {          /* signed char: sign-extend bit 7 */
                ok = new_label();
                emit("    CMP #$0080\n");
                emit("    BCC .L%d\n", ok);
                emit("    ORA #$FF00\n");
                emit(".L%d:\n", ok);
            }
            break;

        case TYPE_BOOL:
            if (from.words == 2) emit("    PHA\n    TXA\n    ORA 1,S\n    TAY\n    PLA\n    TYA\n");
            ok = new_label();
            done = new_label();
            emit("    CMP #$0000    ; Cast to bool (normalize to 0/1)\n");
            emit("    BEQ .L%d\n", ok);
            emit("    LDA #$0001\n");
            emit("    BRA .L%d\n", done);
            emit(".L%d:\n", ok);
            emit("    LDA #$0000\n");
            emit(".L%d:\n", done);
            break;

        case TYPE_LONG:
            emit_convert(from, to);
            break;

        default:                     /* int, short, void, struct, union: nothing to convert */
            break;
    }
}

/* Code generation for comma expressions */
static void codegen_comma(ASTNode* node) {
    int i;
    /* Evaluate all expressions left to right */
    /* Only the last expression's result remains in A */
    for (i = 0; i < node->u.comma.expr_count; i++) {
        codegen_expression(node->u.comma.expressions[i]);
        /* Result of all but last expression is discarded */
    }
    /* Final result is in A register (from last expression) */
}

static void codegen_ternary(ASTNode* node) {
    int else_label = new_label();
    int done_label = new_label();
    CType t = expr_type(node);

    codegen_truth(node->u.ternary.condition);
    emit("    CMP #$0000\n");
    emit("    BEQ .L%d\n", else_label);

    codegen_expr_to(node->u.ternary.then_expr, t);
    emit("    BRA .L%d\n", done_label);

    emit(".L%d:\n", else_label);
    codegen_expr_to(node->u.ternary.else_expr, t);

    emit(".L%d:\n", done_label);
}

/* After SEC/SBC (signed) or CMP (unsigned) with A = left: branch on the relation and leave 0 / 1 in A. */
static void emit_relation(BinOpType op, int uns) {
    int t = new_label(), f = new_label(), done = new_label(), ovf;
    /* BGE is only an alias for BCS (unsigned >=), and CMP does not set V: signed relations are N != V after SBC. */
    switch (op) {
        case OP_EQ: emit("    BEQ .L%d\n    BRA .L%d\n", t, f); break;
        case OP_NE: emit("    BEQ .L%d\n    BRA .L%d\n", f, t); break;
        default:
            if (uns) {
                switch (op) {
                    case OP_GE: emit("    BCS .L%d\n    BRA .L%d\n", t, f); break;
                    case OP_LT: emit("    BCS .L%d\n    BRA .L%d\n", f, t); break;
                    case OP_GT: emit("    BEQ .L%d\n    BCS .L%d\n    BRA .L%d\n", f, t, f); break;
                    default:    emit("    BEQ .L%d\n    BCS .L%d\n    BRA .L%d\n", t, f, t); break;   /* LE */
                }
            } else {
                int less_true = (op == OP_LT || op == OP_LE);
                if (op == OP_GT) emit("    BEQ .L%d\n", f);
                if (op == OP_LE) emit("    BEQ .L%d\n", t);
                ovf = new_label();
                emit("    BVS .L%d\n", ovf);
                emit("    BMI .L%d\n", less_true ? t : f);
                emit("    BRA .L%d\n", less_true ? f : t);
                emit(".L%d:\n", ovf);
                emit("    BMI .L%d\n", less_true ? f : t);
                emit("    BRA .L%d\n", less_true ? t : f);
            }
    }
    emit(".L%d:\n    LDA #$0001\n    BRA .L%d\n", t, done);
    emit(".L%d:\n    LDA #$0000\n", f);
    emit(".L%d:\n", done);
}

/* A = -1 / 0 / 1 from a long compare helper: turn it into the relation's 0 / 1. */
static void emit_cmp_result(BinOpType op) {
    int t = new_label(), f = new_label(), done = new_label();
    emit("    CMP #$0000\n");
    switch (op) {
        case OP_EQ: emit("    BEQ .L%d\n    BRA .L%d\n", t, f); break;
        case OP_NE: emit("    BEQ .L%d\n    BRA .L%d\n", f, t); break;
        case OP_LT: emit("    BMI .L%d\n    BRA .L%d\n", t, f); break;
        case OP_LE: emit("    BMI .L%d\n    BEQ .L%d\n    BRA .L%d\n", t, t, f); break;
        case OP_GT: emit("    BMI .L%d\n    BEQ .L%d\n    BRA .L%d\n", f, f, t); break;
        default:    emit("    BMI .L%d\n    BRA .L%d\n", f, t); break;     /* GE */
    }
    emit(".L%d:\n    LDA #$0001\n    BRA .L%d\n", t, done);
    emit(".L%d:\n    LDA #$0000\n", f);
    emit(".L%d:\n", done);
}

static void emit_abs(void) {
    int neg = new_label(), done = new_label();
    emit("    BMI .L%d\n    BRA .L%d\n.L%d:\n    EOR #$FFFF\n    INC A\n.L%d:\n", neg, done, neg, done);
}

/* Signed 16-bit divide / remainder on top of the unsigned DIV: divide the magnitudes, then negate the result when
 * its sign calls for it (quotient: operand signs differ; remainder: the dividend's sign). The right operand has
 * been pushed and A holds the left. */
static void emit_sdivmod(int rem) {
    int neg = new_label(), pos = new_label(), done = new_label();
    emit("    TAY\n");
    pop_a();
    emit("    STA %d,S\n", SPO(SCRATCH1));           /* divisor */
    emit("    TYA\n");
    emit("    STA %d,S\n", SPO(SCRATCH0));           /* dividend */
    if (!rem) emit("    XOR %d,S\n", SPO(SCRATCH1));  /* bit 15 = signs differ */
    emit("    STA %d,S\n", SPO(SCRATCH2));           /* sign of the result */
    emit("    LDA %d,S\n", SPO(SCRATCH1));
    emit_abs();
    emit("    TAY\n");
    emit("    LDA %d,S\n", SPO(SCRATCH0));
    emit_abs();
    emit("    TAX\n");
    emit("    DIV X,Y\n");
    if (rem) emit("    TXA\n");
    emit("    STA %d,S\n", SPO(SCRATCH0));
    emit("    LDA %d,S\n", SPO(SCRATCH2));
    emit("    BMI .L%d\n    BRA .L%d\n", neg, pos);
    emit(".L%d:\n    LDA %d,S\n    EOR #$FFFF\n    INC A\n    BRA .L%d\n", neg, SPO(SCRATCH0), done);
    emit(".L%d:\n    LDA %d,S\n", pos, SPO(SCRATCH0));
    emit(".L%d:\n", done);
}

/* Short-circuit && and ||. */
static void codegen_logical(ASTNode* node) {
    int fl = new_label(), end = new_label();
    if (node->u.binop.op == OP_AND) {
        codegen_truth(node->u.binop.left);
        emit("    CMP #$0000\n    BEQ .L%d\n", fl);
        codegen_truth(node->u.binop.right);
        emit("    CMP #$0000\n    BEQ .L%d\n", fl);
        emit("    LDA #$0001\n    BRA .L%d\n", end);
    } else {
        int tr = new_label(), rhs = new_label();
        codegen_truth(node->u.binop.left);
        emit("    CMP #$0000\n    BEQ .L%d\n    BRA .L%d\n", rhs, tr);
        emit(".L%d:\n", rhs);
        codegen_truth(node->u.binop.right);
        emit("    CMP #$0000\n    BEQ .L%d\n", fl);
        emit(".L%d:\n    LDA #$0001\n    BRA .L%d\n", tr, end);
    }
    emit(".L%d:\n    LDA #$0000\n", fl);
    emit(".L%d:\n", end);
}

/* Long scratch block used by the zp_long.asm helpers: left $802C00, right $802C04 (low word first). */
static void stage_long(void) {
    /* right operand pushed (low word on top), left in A:X */
    emit("    STA $802C00\n    TXA\n    STA $802C02\n");
    emit("    PLA\n    STA $802C04\n");
    ctx.depth -= 2;
    emit("    PLA\n    STA $802C06\n");
    ctx.depth -= 2;
}

static const char* long_helper(BinOpType op, int uns) {
    switch (op) {
        case OP_ADD: return "__zp_ladd";
        case OP_SUB: return "__zp_lsub";
        case OP_MUL: return "__zp_lmul";
        case OP_DIV: return uns ? "__zp_ludiv" : "__zp_lsdiv";
        case OP_MOD: return uns ? "__zp_lumod" : "__zp_lsmod";
        case OP_BIT_AND: return "__zp_land";
        case OP_BIT_OR: return "__zp_lor";
        case OP_BIT_XOR: return "__zp_lxor";
        case OP_SHL: return "__zp_lshl";
        case OP_SHR: return uns ? "__zp_llsr" : "__zp_lasr";
        default: return NULL;
    }
}

static void codegen_binop(ASTNode* node) {
    BinOpType op = node->u.binop.op;
    CType lt, rt, c;
    int is_cmp, is_shift;
    int loop, done;

    if (op == OP_AND || op == OP_OR) { codegen_logical(node); return; }

    lt = expr_type(node->u.binop.left);
    rt = expr_type(node->u.binop.right);
    is_cmp = (op == OP_EQ || op == OP_NE || op == OP_LT || op == OP_GT || op == OP_LE || op == OP_GE);
    is_shift = (op == OP_SHL || op == OP_SHR);
    c = is_shift ? ctype(lt.words, lt.uns) : usual(lt, rt);

    if (c.words == 2) {
        /* ---- 32-bit: right operand (or the shift count) first, then the left; runtime helper does the work ---- */
        if (is_shift) {
            codegen_expr_to(node->u.binop.right, ctype(1, 1));
            push_a();
            codegen_expr_to(node->u.binop.left, c);
            emit("    STA $802C00\n    TXA\n    STA $802C02\n");
            pop_a();
            emit("    STA $802C04\n");
        } else {
            codegen_expr_to(node->u.binop.right, c);
            push_ax();
            codegen_expr_to(node->u.binop.left, c);
            stage_long();
        }
        if (is_cmp) {
            emit("    JSR %s\n", c.uns ? "__zp_lcmpu" : "__zp_lcmps");
            emit_cmp_result(op);
        } else {
            emit("    JSR %s\n", long_helper(op, c.uns));
        }
        return;
    }

    /* ---- 16-bit: right operand pushed, left in A ---- */
    if (is_shift) {
        codegen_expr_to(node->u.binop.right, ctype(1, 1));
        push_a();
        codegen_expr_to(node->u.binop.left, c);
        emit("    TAY\n");
        pop_a();
        emit("    TAX\n    TYA\n");               /* X = count, A = value */
        loop = new_label();
        done = new_label();
        emit("    INX\n");
        emit(".L%d:\n", loop);
        emit("    DEX\n");
        emit("    BEQ .L%d\n", done);
        if (op == OP_SHL) emit("    ASL A\n");
        else if (c.uns) emit("    LSR A\n");
        else emit("    CMP #$8000\n    ROR A\n");   /* arithmetic: C = sign bit rotates back in */
        emit("    BRA .L%d\n", loop);
        emit(".L%d:\n", done);
        return;
    }

    codegen_expr_to(node->u.binop.right, c);
    push_a();
    codegen_expr_to(node->u.binop.left, c);

    if (op == OP_DIV || op == OP_MOD) {
        if (c.uns) {
            emit("    TAX\n");
            pop_a();
            emit("    TAY\n");
            emit("    DIV X,Y\n");
            if (op == OP_MOD) emit("    TXA\n");
        } else {
            emit_sdivmod(op == OP_MOD);
        }
        return;
    }

    /* A = left, right in scratch 0 */
    emit("    TAY\n");
    pop_a();
    emit("    STA %d,S\n", SPO(SCRATCH0));
    emit("    TYA\n");

    switch (op) {
        case OP_ADD: emit("    CLC\n    ADC %d,S\n", SPO(SCRATCH0)); break;
        case OP_SUB: emit("    SEC\n    SBC %d,S\n", SPO(SCRATCH0)); break;
        case OP_MUL: emit("    MUL %d,S\n", SPO(SCRATCH0)); break;
        case OP_BIT_AND: emit("    AND %d,S\n", SPO(SCRATCH0)); break;
        case OP_BIT_OR:  emit("    ORA %d,S\n", SPO(SCRATCH0)); break;
        case OP_BIT_XOR: emit("    XOR %d,S\n", SPO(SCRATCH0)); break;   /* the stack-relative form is spelled XOR */
        default:
            if (is_cmp) {
                if (c.uns || op == OP_EQ || op == OP_NE) emit("    CMP %d,S\n", SPO(SCRATCH0));
                else emit("    SEC\n    SBC %d,S\n", SPO(SCRATCH0));
                emit_relation(op, c.uns);
            }
            break;
    }
}

static void codegen_unop(ASTNode* node) {
    CType t;
    int one, done;
    switch (node->u.unop.op) {
        case OP_ADDR_OF:
            codegen_addr_of(node);
            return;
        case OP_DEREF:
            codegen_deref(node);
            return;
        default:
            break;
    }

    t = expr_type(node->u.unop.operand);

    if (node->u.unop.op == OP_NOT) {
        codegen_truth(node->u.unop.operand);
        one = new_label();
        done = new_label();
        emit("    CMP #$0000\n");
        emit("    BEQ .L%d\n", one);
        emit("    LDA #$0000\n");
        emit("    BRA .L%d\n", done);
        emit(".L%d:\n", one);
        emit("    LDA #$0001\n");
        emit(".L%d:\n", done);
        return;
    }

    codegen_expression(node->u.unop.operand);
    switch (node->u.unop.op) {
        case OP_NEG:
            emit("    EOR #$FFFF\n");
            if (t.words == 2)
                emit("    TAY\n    TXA\n    EOR #$FFFF\n    TAX\n    TYA\n    CLC\n    ADC #$0001\n    TAY\n    TXA\n    ADC #$0000\n    TAX\n    TYA\n");
            else
                emit("    INC A\n");
            break;
        case OP_BIT_NOT:
            emit("    EOR #$FFFF\n");
            if (t.words == 2) emit("    TAY\n    TXA\n    EOR #$FFFF\n    TAX\n    TYA\n");
            break;
        default:
            break;
    }
}

static int arg_is_simple(ASTNode* a) {
    if (a->type == AST_NUMBER) return 1;
    if (a->type == AST_IDENTIFIER) {
        Symbol* sym = find_symbol(a->u.identifier);
        return sym && !sym->is_array;
    }
    return 0;
}

/* Calls: the first three register words (A, X, Y) carry the leading arguments - a 32-bit argument takes two
 * (A:X or X:Y) - and everything from the first argument that does not fit goes on the stack, right to left.
 * Arguments that need real evaluation are computed into stack temporaries first, so evaluating one cannot
 * clobber a register another has already been loaded into; constants and variables are loaded directly. */
static void codegen_call(ASTNode* node) {
    int n = node->u.call.arg_count;
    FuncInfo* f = find_func(node->u.call.name);
    CType at[MAX_PARAMS];
    int reg_of[MAX_PARAMS];
    int temp_at[MAX_PARAMS];
    int regs_used = 0, stack_from, i, base_depth;

    if (n > MAX_PARAMS) {
        fprintf(stderr, "Error: too many arguments in call to '%s'\n", node->u.call.name);
        n = MAX_PARAMS;
    }
    for (i = 0; i < n; i++) {
        CType from = expr_type(node->u.call.args[i]);
        at[i] = (f && i < f->nparams) ? f->params[i] : from;   /* unknown callee: pass each argument as it is */
    }
    stack_from = n;
    for (i = 0; i < n; i++) {
        if (stack_from == n && regs_used + at[i].words <= 3) {
            reg_of[i] = regs_used;
            regs_used += at[i].words;
        } else {
            if (stack_from == n) stack_from = i;
            reg_of[i] = -1;
        }
    }

    base_depth = ctx.depth;

    for (i = 0; i < stack_from; i++) {
        temp_at[i] = -1;
        if (arg_is_simple(node->u.call.args[i])) continue;
        codegen_expr_to(node->u.call.args[i], at[i]);
        if (at[i].words == 2) push_ax(); else push_a();
        temp_at[i] = ctx.depth;
    }
    for (i = n - 1; i >= stack_from; i--) {
        codegen_expr_to(node->u.call.args[i], at[i]);
        if (at[i].words == 2) push_ax(); else push_a();
    }

    /* load the registers, highest first so the lower ones (A last) are never disturbed */
    for (i = stack_from - 1; i >= 0; i--) {
        int r = reg_of[i];
        if (temp_at[i] < 0) {
            codegen_expr_to(node->u.call.args[i], at[i]);      /* leaves the value in A (and X) */
            if (at[i].words == 1) {
                if (r == 1) emit("    TAX\n");
                else if (r == 2) emit("    TAY\n");
            } else if (r == 1) {
                emit("    TXY\n    TAX\n");                    /* X:Y = lo:hi */
            }
        } else {
            int lo = ctx.depth - temp_at[i] + 1;
            if (at[i].words == 1) {
                emit("    LDA %d,S\n", lo);
                if (r == 1) emit("    TAX\n");
                else if (r == 2) emit("    TAY\n");
            } else if (r == 0) {
                emit("    LDA %d,S\n    TAX\n    LDA %d,S\n", lo + 2, lo);
            } else {
                emit("    LDA %d,S\n    TAY\n    LDA %d,S\n    TAX\n", lo + 2, lo);
            }
        }
    }

    emit("    JSR %s\n", node->u.call.name);

    emit_drop_bytes(ctx.depth - base_depth);       /* temporaries and stack arguments, keeping A and X */
    ctx.depth = base_depth;
}

static void codegen_assign(ASTNode* node) {
    Symbol* sym = find_symbol(node->u.assign.var_name);
    if (!sym) {
        fprintf(stderr, "Error: Undefined variable '%s'\n", node->u.assign.var_name);
        return;
    }
    codegen_expr_to(node->u.assign.value, sym_ctype(sym));
    store_sym(sym);
}

static void codegen_array_subscript(ASTNode* node) {
    Symbol* sym = find_symbol(node->u.array_subscript.array_name);
    if (!sym) {
        fprintf(stderr, "Error: Undefined array '%s'\n", node->u.array_subscript.array_name);
        return;
    }

    if (!sym->is_array) {
        fprintf(stderr, "Error: '%s' is not an array\n", node->u.array_subscript.array_name);
        return;
    }

    /* Calculate address: base + (index * 2) */
    /* First, evaluate index expression */
    codegen_expression(node->u.array_subscript.index);

    /* A now contains the index */
    /* Multiply by 2 (shift left once for 16-bit elements) */
    emit("    ASL A\n");  /* A = index * 2 */

    /* Add base offset */
    emit("    CLC\n");
    emit("    ADC #$%04X\n", SPO(sym->offset));  /* A = base + (index * 2) */

    /* Now A contains the offset from SP */
    /* Calculate effective address: SP + A */
    emit("    STA %d,S\n", SPO(SCRATCH0));      /* Save offset */
    emit("    TSC\n");           /* A = SP */
    emit("    CLC\n");
    emit("    ADC %d,S\n", SPO(SCRATCH0));      /* A = SP + offset */
    emit("    TAX\n");           /* X = effective address */

    /* Load from address in X (using absolute indexed by 0) */
    /* DEF88186 doesn't have (X) addressing, so we need to use DP */
    /* Set DP = X, then load from (DP) */
    emit("    TXA\n");
    emit("    TCD\n");           /* DP = address */
    emit("    LDA $00\n");       /* Load from (DP) */

    /* (D is left pointing at the element: arrays and pointers still go through the direct page - see docs) */
}

static void codegen_array_assign(ASTNode* node) {
    Symbol* sym = find_symbol(node->u.array_assign.array_name);
    if (!sym) {
        fprintf(stderr, "Error: Undefined array '%s'\n", node->u.array_assign.array_name);
        return;
    }

    if (!sym->is_array) {
        fprintf(stderr, "Error: '%s' is not an array\n", node->u.array_assign.array_name);
        return;
    }

    /* Generate code for value (result in A) */
    codegen_expression(node->u.array_assign.value);
    push_a();  /* Save value on stack */

    /* Calculate address: base + (index * 2) */
    codegen_expression(node->u.array_assign.index);

    /* A now contains the index */
    emit("    ASL A\n");  /* A = index * 2 */

    /* Add base offset */
    emit("    CLC\n");
    emit("    ADC #$%04X\n", SPO(sym->offset));  /* A = base + (index * 2) */

    /* Now A contains the offset from SP */
    emit("    TAX\n");           /* X = offset */

    /* Restore value from stack */
    pop_a();           /* A = value */

    /* Store to SP + X */
    emit("    STA 0,S,X\n");     /* Store to [SP + X] */
}

static void codegen_addr_of(ASTNode* node) {
    /* Calculate address of operand and leave it in A */
    ASTNode* operand = node->u.unop.operand;

    if (operand->type == AST_IDENTIFIER) {
        Symbol* sym = find_symbol(operand->u.identifier);
        if (!sym) {
            fprintf(stderr, "Error: Undefined variable '%s'\n", operand->u.identifier);
            return;
        }

        /* Calculate address: SP + 1 + offset (+ anything pushed) */
        emit("    TSC\n");           /* A = SP */
        emit("    CLC\n");
        emit("    ADC #$%04X\n", SPO(sym->offset));
        /* A now contains the address */
    } else if (operand->type == AST_ARRAY_SUBSCRIPT) {
        /* Address of array element - similar to array subscript code */
        Symbol* sym = find_symbol(operand->u.array_subscript.array_name);
        if (!sym) {
            fprintf(stderr, "Error: Undefined array '%s'\n", operand->u.array_subscript.array_name);
            return;
        }

        codegen_expression(operand->u.array_subscript.index);
        emit("    ASL A\n");  /* A = index * 2 */
        emit("    CLC\n");
        emit("    ADC #$%04X\n", SPO(sym->offset));
        emit("    STA %d,S\n", SPO(SCRATCH0));
        emit("    TSC\n");
        emit("    CLC\n");
        emit("    ADC %d,S\n", SPO(SCRATCH0));  /* A = SP + offset + (index * 2) */
    } else if (operand->type == AST_DEREF) {
        /* &(*ptr) = ptr, so just evaluate the pointer */
        codegen_expression(operand->u.unop.operand);
    } else if (operand->type == AST_MEMBER_ACCESS) {
        /* Address of struct member */
        codegen_member_address(operand);
    } else {
        fprintf(stderr, "Error: Cannot take address of this expression\n");
    }
}

static void codegen_deref(ASTNode* node) {
    /* Evaluate pointer expression (result in A = address) */
    codegen_expression(node->u.unop.operand);

    /* Load value from address in A */
    emit("    TAX\n");           /* X = address */
    emit("    TXA\n");
    emit("    TCD\n");           /* DP = address */
    emit("    LDA $00\n");       /* Load from (DP) */
}

static void codegen_member_address(ASTNode* node) {
    int i;
    /* Calculate address of struct member */
    if (node->type == AST_MEMBER_ACCESS) {
        /* Get base object address */
        if (node->u.member_access.object->type == AST_IDENTIFIER) {
            StructDef* sdef;
            int member_offset;
            Symbol* sym = find_symbol(node->u.member_access.object->u.identifier);
            if (!sym || sym->type != TYPE_STRUCT) {
                fprintf(stderr, "Error: Not a struct variable\n");
                return;
            }

            sdef = find_struct(sym->struct_name);
            if (!sdef) {
                fprintf(stderr, "Error: Struct type '%s' not found\n", sym->struct_name);
                return;
            }

            /* Find member offset */
            member_offset = -1;
            for (i = 0; i < sdef->member_count; i++) {
                if (strcmp(sdef->members[i].name, node->u.member_access.member_name) == 0) {
                    member_offset = sdef->members[i].offset;
                    break;
                }
            }

            if (member_offset < 0) {
                fprintf(stderr, "Error: Member '%s' not found in struct\n", node->u.member_access.member_name);
                return;
            }

            /* Calculate address: SP + var_offset + member_offset */
            emit("    TSC\n");
            emit("    CLC\n");
            emit("    ADC #$%04X\n", SPO(sym->offset + member_offset));
        } else {
            fprintf(stderr, "Error: Complex member access not yet supported\n");
        }
    }
}

static void codegen_member_access(ASTNode* node) {
    /* Get address of member, then load value */
    codegen_member_address(node);
    emit("    TAX\n");
    emit("    TXA\n");
    emit("    TCD\n");
    emit("    LDA $00\n");
}

static void codegen_ptr_member_access(ASTNode* node) {
    /* ptr->member is equivalent to (*ptr).member */
    /* First evaluate pointer to get address */
    codegen_expression(node->u.ptr_member_access.pointer);

    /* A now contains the base address of the struct */
    /* Need to add member offset */

    /* Get struct type from pointer expression */
    /* This is simplified - in full implementation would need type tracking */
    fprintf(stderr, "Warning: Pointer member access not fully implemented\n");

    /* For now, just load from the address (member offset 0) */
    emit("    TAX\n");
    emit("    TXA\n");
    emit("    TCD\n");
    emit("    LDA $00\n");
}

static void codegen_member_assign(ASTNode* node) {
    int i;
    /* Evaluate the value to assign (result in A) */
    codegen_expression(node->u.member_assign.value);

    /* Push value onto stack to preserve it */
    push_a();

    /* Calculate address of member */
    if (node->u.member_assign.object->type == AST_IDENTIFIER) {
        StructDef* sdef;
        int member_offset;
        Symbol* sym = find_symbol(node->u.member_assign.object->u.identifier);
        if (!sym || sym->type != TYPE_STRUCT) {
            fprintf(stderr, "Error: Not a struct variable\n");
            return;
        }

        sdef = find_struct(sym->struct_name);
        if (!sdef) {
            fprintf(stderr, "Error: Struct type '%s' not found\n", sym->struct_name);
            return;
        }

        /* Find member offset */
        member_offset = -1;
        for (i = 0; i < sdef->member_count; i++) {
            if (strcmp(sdef->members[i].name, node->u.member_assign.member_name) == 0) {
                member_offset = sdef->members[i].offset;
                break;
            }
        }

        if (member_offset < 0) {
            fprintf(stderr, "Error: Member '%s' not found in struct\n", node->u.member_assign.member_name);
            return;
        }

        /* Calculate address: SP + var_offset + member_offset */
        emit("    TSC\n");
        emit("    CLC\n");
        emit("    ADC #$%04X\n", SPO(sym->offset + member_offset));

        /* Store address in X, then use it via direct page */
        emit("    TAX\n");
        emit("    TXA\n");
        emit("    TCD\n");

        /* Pop value and store it */
        pop_a();
        emit("    STA $00\n");
    } else {
        fprintf(stderr, "Error: Complex member assignment not yet supported\n");
    }
}

static void codegen_ptr_member_assign(ASTNode* node) {
    /* ptr->member = value */
    /* Evaluate the value to assign (result in A) */
    codegen_expression(node->u.ptr_member_assign.value);

    /* Push value onto stack to preserve it */
    push_a();

    /* Evaluate pointer to get base address */
    codegen_expression(node->u.ptr_member_assign.pointer);

    /* For now, assume member offset is 0 (simplified) */
    fprintf(stderr, "Warning: Pointer member assignment not fully implemented\n");

    /* Store address in X, then use it via direct page */
    emit("    TAX\n");
    emit("    TXA\n");
    emit("    TCD\n");

    /* Pop value and store it */
    pop_a();
    emit("    STA $00\n");
}

static void codegen_inc_dec(ASTNode* node) {
    Symbol* sym = find_symbol(node->u.inc_dec.var_name);
    CType t;
    int inc = (node->type == AST_PRE_INC || node->type == AST_POST_INC);
    int post = (node->type == AST_POST_INC || node->type == AST_POST_DEC);
    if (!sym) {
        fprintf(stderr, "Error: Undefined variable '%s'\n", node->u.inc_dec.var_name);
        return;
    }
    t = sym_ctype(sym);
    load_sym(sym);
    if (t.words == 1) {
        if (post) emit("    TAY\n");
        emit(inc ? "    INC A\n" : "    DEC A\n");
        emit("    STA %d,S\n", SPO(sym->offset));
        if (post) emit("    TYA\n");
        return;
    }
    if (post) push_ax();
    if (inc) emit("    CLC\n    ADC #$0001\n    TAY\n    TXA\n    ADC #$0000\n    TAX\n    TYA\n");
    else     emit("    SEC\n    SBC #$0001\n    TAY\n    TXA\n    SBC #$0000\n    TAX\n    TYA\n");
    store_sym(sym);
    if (post) {
        /* the old value is back on the stack, low word on top */
        emit("    PLA\n    TAY\n    PLA\n    TAX\n    TYA\n");
        ctx.depth -= 4;
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
        case AST_CAST:
            codegen_cast(node);
            break;
        case AST_COMMA:
            codegen_comma(node);
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

/* Code generation for statements */
static void codegen_return(ASTNode* node) {
    if (node->u.ret.value)
        codegen_expr_to(node->u.ret.value, ctype(ctx.ret_words, ctx.ret_uns));

    /* Epilogue: pop the frame and anything still pushed (a switch value), keeping A and X */
    emit_drop_bytes(ctx.frame_total + ctx.depth);
    emit("    RTS\n");
}

static void codegen_if(ASTNode* node) {
    int else_label = new_label();
    int done_label = new_label();

    /* Evaluate condition */
    codegen_truth(node->u.if_stmt.condition);

    /* Branch if false */
    emit("    CMP #$0000\n");
    if (node->u.if_stmt.else_stmt) {
        emit("    BEQ .L%d\n", else_label);
    } else {
        emit("    BEQ .L%d\n", done_label);
    }

    /* Then branch */
    codegen_statement(node->u.if_stmt.then_stmt);

    if (node->u.if_stmt.else_stmt) {
        emit("    BRA .L%d\n", done_label);
        emit(".L%d:\n", else_label);
        codegen_statement(node->u.if_stmt.else_stmt);
    }

    emit(".L%d:\n", done_label);
}

static void codegen_while(ASTNode* node) {
    int loop_label = new_label();
    int done_label = new_label();

    /* Push loop context for break/continue */
    push_loop(done_label, loop_label);

    emit(".L%d:\n", loop_label);

    /* Evaluate condition */
    codegen_truth(node->u.while_stmt.condition);

    /* Branch if false */
    emit("    CMP #$0000\n");
    emit("    BEQ .L%d\n", done_label);

    /* Loop body */
    codegen_statement(node->u.while_stmt.body);

    /* Jump back to condition */
    emit("    BRA .L%d\n", loop_label);

    emit(".L%d:\n", done_label);

    /* Pop loop context */
    pop_loop();
}

/* Code generation for do-while statements */
static void codegen_do_while(ASTNode* node) {
    int loop_label = new_label();
    int done_label = new_label();

    /* Push loop context for break/continue */
    push_loop(done_label, loop_label);

    emit(".L%d:\n", loop_label);

    /* Loop body (executes at least once) */
    codegen_statement(node->u.do_while_stmt.body);

    /* Evaluate condition */
    codegen_truth(node->u.do_while_stmt.condition);

    /* Branch back if true (using BEQ + BRA since DEF88186 has no BNE) */
    emit("    CMP #$0000\n");
    emit("    BEQ .L%d\n", done_label);  /* If false (zero), exit */
    emit("    BRA .L%d\n", loop_label);  /* Otherwise loop */

    emit(".L%d:\n", done_label);

    /* Pop loop context */
    pop_loop();
}

/* Helper to detect simple counted loop suitable for LOOP/LPEND */
static int is_simple_counted_loop(ASTNode* init, ASTNode* cond, ASTNode* incr,
                                   char** loop_var, int* loop_count) {
                                       ASTNode* add_right;
                                       ASTNode* init_expr;
                                       ASTNode* cond_expr;
                                       ASTNode* incr_expr;
                                       ASTNode* add_left;
    if (!init || !cond || !incr) return 0;

    /* Check init: must be "var = 0" or similar assignment */
    if (init->type != AST_EXPR_STMT || !init->u.expr_stmt.expr) return 0;
    init_expr = init->u.expr_stmt.expr;
    if (init_expr->type != AST_ASSIGN) return 0;

    /* Check that init is "var = 0" */
    if (init_expr->u.assign.value->type != AST_NUMBER) return 0;
    if (init_expr->u.assign.value->u.number != 0) return 0;

    *loop_var = init_expr->u.assign.var_name;

    /* Check condition: must be "var < N" where N is constant */
    if (cond->type != AST_EXPR_STMT || !cond->u.expr_stmt.expr) return 0;
    cond_expr = cond->u.expr_stmt.expr;
    if (cond_expr->type != AST_BINOP) return 0;
    if (cond_expr->u.binop.op != OP_LT) return 0;

    /* Left side must be same variable */
    if (cond_expr->u.binop.left->type != AST_IDENTIFIER) return 0;
    if (strcmp(cond_expr->u.binop.left->u.identifier, *loop_var) != 0) return 0;

    /* Right side must be constant */
    if (cond_expr->u.binop.right->type != AST_NUMBER) return 0;
    *loop_count = cond_expr->u.binop.right->u.number;

    /* Check increment: must be "var = var + 1" or simple increment */
    if (incr->type != AST_EXPR_STMT || !incr->u.expr_stmt.expr) return 0;
    incr_expr = incr->u.expr_stmt.expr;
    if (incr_expr->type != AST_ASSIGN) return 0;

    /* Must be same variable */
    if (strcmp(incr_expr->u.assign.var_name, *loop_var) != 0) return 0;

    /* Value must be "var + 1" */
    if (incr_expr->u.assign.value->type != AST_BINOP) return 0;
    if (incr_expr->u.assign.value->u.binop.op != OP_ADD) return 0;

    add_left = incr_expr->u.assign.value->u.binop.left;
    add_right = incr_expr->u.assign.value->u.binop.right;

    if (add_left->type != AST_IDENTIFIER) return 0;
    if (strcmp(add_left->u.identifier, *loop_var) != 0) return 0;

    if (add_right->type != AST_NUMBER) return 0;
    if (add_right->u.number != 1) return 0;

    return 1;  /* This is a simple counted loop! */
}

static void codegen_for(ASTNode* node) {
    int done_label;
    int loop_label;
    int increment_label;
    char* loop_var = NULL;
    int loop_count = 0;

    /* Try to use hardware LOOP/LPEND for simple counted loops */
    if (is_simple_counted_loop(node->u.for_stmt.init, node->u.for_stmt.condition,
                                node->u.for_stmt.increment, &loop_var, &loop_count)) {
                                    int done_label;
                                    int loop_start;

        emit("    ; Hardware loop: for (%s = 0; %s < %d; %s++)\n",
             loop_var, loop_var, loop_count, loop_var);

        loop_start = new_label();
        done_label = new_label();

        /* Initialize loop variable to 0 */
        codegen_statement(node->u.for_stmt.init);

        /* Use hardware LOOP instruction */
        emit("    LOOP #$%04X\n", loop_count);

        emit(".L%d:\n", loop_start);

        /* Push loop context */
        push_loop(done_label, loop_start);

        /* Loop body */
        codegen_statement(node->u.for_stmt.body);

        /* Increment the loop variable */
        codegen_statement(node->u.for_stmt.increment);

        /* LPEND decrements hardware counter and loops back if non-zero */
        emit("    LPEND\n");

        emit(".L%d:\n", done_label);

        /* Pop loop context */
        pop_loop();

        return;
    }

    /* Fall back to manual loop for complex cases */
    emit("    ; Manual loop (complex condition)\n");

    loop_label = new_label();
    done_label = new_label();
    increment_label = new_label();

    /* Initialization */
    if (node->u.for_stmt.init) {
        codegen_statement(node->u.for_stmt.init);
    }

    /* Push loop context (continue goes to increment, break goes to done) */
    push_loop(done_label, increment_label);

    emit(".L%d:\n", loop_label);

    /* Condition */
    if (node->u.for_stmt.condition && node->u.for_stmt.condition->u.expr_stmt.expr) {
        codegen_truth(node->u.for_stmt.condition->u.expr_stmt.expr);
        emit("    CMP #$0000\n");
        emit("    BEQ .L%d\n", done_label);
    }

    /* Body */
    codegen_statement(node->u.for_stmt.body);

    /* Increment label (for continue) */
    emit(".L%d:\n", increment_label);

    /* Increment */
    if (node->u.for_stmt.increment) {
        codegen_statement(node->u.for_stmt.increment);
    }

    /* Jump back to condition */
    emit("    BRA .L%d\n", loop_label);

    emit(".L%d:\n", done_label);

    /* Pop loop context */
    pop_loop();
}

static void codegen_block(ASTNode* node) {
    int i;
    for (i = 0; i < node->u.block.stmt_count; i++) {
        codegen_statement(node->u.block.statements[i]);
    }
}

static void codegen_var_decl(ASTNode* node) {
    Symbol* sym;
    /* Add variable to symbol table */
    if (node->u.var_decl.array_dimensions > 0) {
        /* Multi-dimensional array */
        add_symbol_multidim(node->u.var_decl.var_name, node->u.var_decl.var_type, 0, 0,
                           node->u.var_decl.array_sizes, node->u.var_decl.array_dimensions,
                           node->u.var_decl.pointer_level, node->u.var_decl.struct_name);
    } else {
        /* Regular variable or 1D array (backwards compatibility) */
        int array_size = (node->u.var_decl.array_sizes && node->u.var_decl.is_array) ?
                         node->u.var_decl.array_sizes[0] : 0;
        add_symbol(node->u.var_decl.var_name, node->u.var_decl.var_type, 0, 0,
                   node->u.var_decl.is_array, array_size,
                   node->u.var_decl.pointer_level, node->u.var_decl.struct_name);
    }
    ctx.symbols[ctx.symbol_count - 1].modifiers = node->u.var_decl.modifiers;

    /* Initialize if there's an initial value */
    if (node->u.var_decl.init_value) {
        sym = find_symbol(node->u.var_decl.var_name);
        if (!sym) return;

        if (node->u.var_decl.init_value->type == AST_INIT_LIST) {
            /* Array initialization: int arr[3] = {1, 2, 3}; */
            ASTNode* init_list = node->u.var_decl.init_value;
            int elem_size = (node->u.var_decl.var_type == TYPE_CHAR) ? 1 : 2;
            int i;

            for (i = 0; i < init_list->u.init_list.value_count; i++) {
                int offset;
                /* Evaluate the expression for this element */
                codegen_expression(init_list->u.init_list.values[i]);

                /* Store to array element */
                offset = sym->offset + (i * elem_size);
                if (node->u.var_decl.var_type == TYPE_CHAR) {
                    /* For char arrays, store byte */
                    emit("    SEP #$20        ; 8-bit A\n");
                    emit("    STA %d,S\n", SPO(offset));
                    emit("    REP #$20        ; 16-bit A\n");
                } else {
                    /* For int arrays, store word */
                    emit("    STA %d,S\n", SPO(offset));
                }
            }
        } else if (!node->u.var_decl.is_array) {
            /* Regular variable initialization */
            codegen_expr_to(node->u.var_decl.init_value, sym_ctype(sym));
            store_sym(sym);
        }
    }
}

static void codegen_switch(ASTNode* node) {
    int default_label;
    int* case_labels;
    int end_label = new_label();
    int i;

    /* Evaluate switch expression once and save to stack */
    codegen_expr_to(node->u.switch_stmt.expr, ctype(1, 0));
    push_a();

    /* break leaves through end_label, which pops the switch value itself; continue belongs to an enclosing loop */
    push_loop(end_label, -1);

    /* Generate labels for each case */
    case_labels = malloc(sizeof(int) * node->u.switch_stmt.case_count);
    default_label = -1;

    for (i = 0; i < node->u.switch_stmt.case_count; i++) {
        case_labels[i] = new_label();
        if (node->u.switch_stmt.cases[i]->type == AST_DEFAULT) {
            default_label = case_labels[i];
        }
    }

    /* Generate comparisons for each case */
    for (i = 0; i < node->u.switch_stmt.case_count; i++) {
        ASTNode* case_node = node->u.switch_stmt.cases[i];
        if (case_node->type == AST_CASE) {
            emit("    LDA 1,S\n");  /* Load switch value from stack */
            emit("    CMP #$%04X\n", case_node->u.case_stmt.case_value);
            emit("    BEQ .L%d\n", case_labels[i]);
        }
    }

    /* If no case matched, jump to default or end */
    if (default_label >= 0) {
        emit("    BRA .L%d\n", default_label);
    } else {
        emit("    BRA .L%d\n", end_label);
    }

    /* Generate code for each case */
    for (i = 0; i < node->u.switch_stmt.case_count; i++) {
        ASTNode* case_node = node->u.switch_stmt.cases[i];
        int j;
        emit(".L%d:\n", case_labels[i]);

        if (case_node->type == AST_CASE) {
            for (j = 0; j < case_node->u.case_stmt.stmt_count; j++) {
                codegen_statement(case_node->u.case_stmt.statements[j]);
            }
        } else if (case_node->type == AST_DEFAULT) {
            for (j = 0; j < case_node->u.default_stmt.stmt_count; j++) {
                codegen_statement(case_node->u.default_stmt.statements[j]);
            }
        }
    }

    emit(".L%d:\n", end_label);
    pop_loop();
    pop_a();                /* Clean up stack (remove switch value) */

    free(case_labels);
}

static void codegen_expr_stmt(ASTNode* node) {
    if (node->u.expr_stmt.expr) {
        codegen_expression(node->u.expr_stmt.expr);
    }
}

static void codegen_break(ASTNode* node) {
    LoopContext* loop = current_loop();
    if (!loop) {
        fprintf(stderr, "Error: break statement outside of loop\n");
        return;
    }
    if (ctx.depth > loop->break_depth) {       /* leave a switch value etc. behind: the target expects this depth */
        emit("    TSC\n    CLC\n    ADC #$%04X\n    TCS\n", ctx.depth - loop->break_depth);
    }
    emit("    BRA .L%d\n", loop->break_label);
}

static void codegen_continue(ASTNode* node) {
    LoopContext* loop = current_loop();
    int i;
    /* a switch has no continue of its own: use the innermost real loop's */
    for (i = ctx.loop_depth - 1; i >= 0 && ctx.loop_stack[i].continue_label < 0; i--) { }
    if (i < 0) {
        fprintf(stderr, "Error: continue statement outside of loop\n");
        return;
    }
    loop = &ctx.loop_stack[i];
    if (ctx.depth > loop->continue_depth) {
        emit("    TSC\n    CLC\n    ADC #$%04X\n    TCS\n", ctx.depth - loop->continue_depth);
    }
    emit("    BRA .L%d\n", loop->continue_label);
}

/* Code generation for goto statements */
static void codegen_goto(ASTNode* node) {
    /* Emit unconditional branch to user label */
    emit("    BRA %s\n", node->u.goto_stmt.label);
}

/* Code generation for label statements */
static void codegen_label(ASTNode* node) {
    /* Emit label */
    emit("%s:\n", node->u.label_stmt.label);
    /* Generate code for statement following the label */
    codegen_statement(node->u.label_stmt.statement);
}

static void codegen_statement(ASTNode* node) {
    int i;
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
        case AST_DO_WHILE:
            codegen_do_while(node);
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
        case AST_VAR_DECL_LIST:
            /* Process each declaration in the list */
            for (i = 0; i < node->u.var_decl_list.decl_count; i++) {
                codegen_var_decl(node->u.var_decl_list.declarations[i]);
            }
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
        case AST_GOTO:
            codegen_goto(node);
            break;
        case AST_LABEL:
            codegen_label(node);
            break;
        case AST_INLINE_ASM:
            /* Emit inline assembly directly */
            emit("    %s\n", node->u.inline_asm.asm_code);
            break;
        default:
            fprintf(stderr, "Error: Unsupported statement type\n");
            break;
    }
}


/* (count_locals_block and the original count_locals live below, rewritten) */



/* Code generation for functions */
/* Words a declaration occupies in the frame (must mirror add_symbol / add_symbol_multidim). */
static int decl_words(ASTNode* d) {
    int elem_size = get_type_size(d->u.var_decl.var_type, d->u.var_decl.pointer_level, d->u.var_decl.struct_name);
    int elems = 1, i;
    if (d->u.var_decl.array_dimensions > 0) {
        for (i = 0; i < d->u.var_decl.array_dimensions; i++) elems *= d->u.var_decl.array_sizes[i];
    } else if (d->u.var_decl.is_array && d->u.var_decl.array_sizes) {
        elems = d->u.var_decl.array_sizes[0];
    }
    return elems * ((elem_size + 1) / 2);
}

static int count_locals(ASTNode* node) {
    int i, count = 0;
    if (!node) return 0;

    switch (node->type) {
        case AST_VAR_DECL:
            return decl_words(node);
        case AST_VAR_DECL_LIST:
            for (i = 0; i < node->u.var_decl_list.decl_count; i++)
                count += count_locals(node->u.var_decl_list.declarations[i]);
            return count;
        case AST_BLOCK:
            for (i = 0; i < node->u.block.stmt_count; i++)
                count += count_locals(node->u.block.statements[i]);
            return count;
        case AST_IF:
            return count_locals(node->u.if_stmt.then_stmt) + count_locals(node->u.if_stmt.else_stmt);
        case AST_WHILE:
            return count_locals(node->u.while_stmt.body);
        case AST_DO_WHILE:
            return count_locals(node->u.do_while_stmt.body);
        case AST_FOR:
            return count_locals(node->u.for_stmt.init) + count_locals(node->u.for_stmt.body);
        case AST_SWITCH:
            for (i = 0; i < node->u.switch_stmt.case_count; i++) {
                ASTNode* c = node->u.switch_stmt.cases[i];
                int j;
                if (c->type == AST_CASE)
                    for (j = 0; j < c->u.case_stmt.stmt_count; j++) count += count_locals(c->u.case_stmt.statements[j]);
                else if (c->type == AST_DEFAULT)
                    for (j = 0; j < c->u.default_stmt.stmt_count; j++) count += count_locals(c->u.default_stmt.statements[j]);
            }
            return count;
        case AST_LABEL:
            return count_locals(node->u.label_stmt.statement);
        default:
            return 0;
    }
}

static int param_words(ASTNode* p) {
    return type_ctype(p->u.param.param_type, p->u.param.modifiers, p->u.param.pointer_level).words;
}

static void codegen_function(ASTNode* node) {
    int nparams = node->u.func_decl.param_count;
    int reg_of[MAX_PARAMS];
    int nregs = 0, stack_from, i, locals_bytes, below, stack_words = 0;
    CType rt;

    ctx.current_function = node->u.func_decl.func_name;
    ctx.stack_offset = 0;
    ctx.depth = 0;
    local_var_index = 0;
    rt = type_ctype(node->u.func_decl.return_type, node->u.func_decl.return_modifiers, 0);
    ctx.ret_words = rt.words;
    ctx.ret_uns = rt.uns;

    emit("\n; Function: %s\n", node->u.func_decl.func_name);
    emit("%s:\n", node->u.func_decl.func_name);

    /* Set to 16-bit mode */
    emit("    REP #$30        ; 16-bit A, X, Y\n");

    if (nparams > MAX_PARAMS) nparams = MAX_PARAMS;

    /* where each parameter arrives: register words (A, X, Y) first, a 32-bit one taking two; once one does not
     * fit, it and everything after it is on the stack */
    stack_from = nparams;
    for (i = 0; i < nparams; i++) {
        int w = param_words(node->u.func_decl.params[i]);
        if (stack_from == nparams && nregs + w <= 3) {
            reg_of[i] = nregs;
            nregs += w;
        } else {
            if (stack_from == nparams) stack_from = i;
            reg_of[i] = -1;
        }
    }

    locals_bytes = count_locals(node->u.func_decl.body) * 2;
    ctx.scratch = locals_bytes;
    below = locals_bytes + 6;                 /* locals + three scratch words */
    ctx.frame_total = below + nregs * 2;

    /* Spill the register arguments first (PHY/PHX/PHA): TSC/SBC/TCS below would destroy A */
    if (nregs >= 3) emit("    PHY\n");
    if (nregs >= 2) emit("    PHX\n");
    if (nregs >= 1) emit("    PHA\n");
    emit("    TSC\n");
    emit("    SEC\n");
    emit("    SBC #$%04X\n", below);
    emit("    TCS\n");

    /* Add parameters to symbol table */
    for (i = 0; i < nparams; i++) {
        ASTNode* param = node->u.func_decl.params[i];
        Symbol* sym;
        add_symbol(param->u.param.param_name, param->u.param.param_type, 1, i, 0, 0,
                   param->u.param.pointer_level, param->u.param.struct_name);
        sym = &ctx.symbols[ctx.symbol_count - 1];
        sym->modifiers = param->u.param.modifiers;
        if (reg_of[i] >= 0) {
            sym->offset = below + reg_of[i] * 2;
        } else {
            sym->offset = ctx.frame_total + 2 + stack_words * 2;    /* above the 2-byte return address */
            stack_words += param_words(param);
        }
    }

    /* Generate code for function body */
    codegen_statement(node->u.func_decl.body);

    /* Add default return if needed */
    emit("    ; Epilogue\n");
    emit_drop_bytes(ctx.frame_total);
    emit("    RTS\n");

    /* Clear symbols for next function */
    clear_symbols();
    ctx.current_function = NULL;
}

static void codegen_struct_decl(ASTNode* node) {
    int i;
    int offset;
    StructDef sdef;
    sdef.name = zp_strdup(node->u.struct_decl.struct_name);
    sdef.member_count = node->u.struct_decl.member_count;
    sdef.members = malloc(sizeof(StructMember) * sdef.member_count);

    offset = 0;
    for (i = 0; i < sdef.member_count; i++) {
        int size;
        ASTNode* member = node->u.struct_decl.members[i];
        sdef.members[i].name = zp_strdup(member->u.var_decl.var_name);
        sdef.members[i].type = member->u.var_decl.var_type;
        sdef.members[i].pointer_level = member->u.var_decl.pointer_level;
        sdef.members[i].struct_name = member->u.var_decl.struct_name ? zp_strdup(member->u.var_decl.struct_name) : NULL;
        sdef.members[i].offset = offset;

        size = get_type_size(member->u.var_decl.var_type, member->u.var_decl.pointer_level, member->u.var_decl.struct_name);
        offset += size;
    }

    sdef.total_size = offset;
    add_struct(sdef);

    emit("; Struct definition: %s (size = %d bytes)\n", sdef.name, sdef.total_size);
}

static void codegen_union_decl(ASTNode* node) {
    int i;
    int max_size;
    StructDef udef;  /* Use StructDef for unions too (same structure) */
    udef.name = zp_strdup(node->u.union_decl.union_name);
    udef.member_count = node->u.union_decl.member_count;
    udef.members = malloc(sizeof(StructMember) * udef.member_count);

    max_size = 0;
    for (i = 0; i < udef.member_count; i++) {
        int size;
        ASTNode* member = node->u.union_decl.members[i];
        udef.members[i].name = zp_strdup(member->u.var_decl.var_name);
        udef.members[i].type = member->u.var_decl.var_type;
        udef.members[i].pointer_level = member->u.var_decl.pointer_level;
        udef.members[i].struct_name = member->u.var_decl.struct_name ? zp_strdup(member->u.var_decl.struct_name) : NULL;
        udef.members[i].offset = 0;  /* All union members start at offset 0 */

        size = get_type_size(member->u.var_decl.var_type, member->u.var_decl.pointer_level, member->u.var_decl.struct_name);
        if (size > max_size) {
            max_size = size;
        }
    }

    udef.total_size = max_size;  /* Union size is the maximum member size */
    add_struct(udef);  /* Use same struct table for unions */

    emit("; Union definition: %s (size = %d bytes)\n", udef.name, udef.total_size);
}

/* Code generation for program */
void codegen_program(ASTNode* node, FILE* output) {
    int i;
    int j;
    init_context(output);

    emit("; Generated by DEF88186 C Compiler\n");
    emit("; ZeroPoint Fantasy Console\n\n");

    /* Emit data section for temporary variables */
    emit(".data\n");
    emit("temp: .word 0\n");
    emit("\n.code\n");

    /* First pass: process struct, union, enum, and typedef declarations */
    for (i = 0; i < node->u.program.decl_count; i++) {
        ASTNode* decl = node->u.program.decls[i];
        if (decl->type == AST_STRUCT_DECL) {
            codegen_struct_decl(decl);
        } else if (decl->type == AST_UNION_DECL) {
            codegen_union_decl(decl);
        } else if (decl->type == AST_ENUM_DECL) {
            /* Enum declarations don't generate code, but we could add enum constants as comments */
            emit("\n; Enum: %s\n", decl->u.enum_decl.enum_name);
            for (j = 0; j < decl->u.enum_decl.enumerator_count; j++) {
                emit(";   %s = %d\n", decl->u.enum_decl.enumerator_names[j], decl->u.enum_decl.enumerator_values[j]);
            }
        } else if (decl->type == AST_TYPEDEF) {
            const char* type_str;
            /* Typedef declarations don't generate code, but we add them as comments for documentation */
            emit("\n; Typedef: %s = ", decl->u.typedef_decl.type_name);
            type_str = "";
            switch (decl->u.typedef_decl.base_type) {
                case TYPE_INT: type_str = "int"; break;
                case TYPE_CHAR: type_str = "char"; break;
                case TYPE_BOOL: type_str = "bool"; break;
                case TYPE_VOID: type_str = "void"; break;
                default: type_str = "unknown"; break;
            }
            emit("%s", type_str);
            for (j = 0; j < decl->u.typedef_decl.pointer_level; j++) {
                emit("*");
            }
            if (decl->u.typedef_decl.array_dim_count > 0) {
                for (j = 0; j < decl->u.typedef_decl.array_dim_count; j++) {
                    emit("[%d]", decl->u.typedef_decl.array_sizes[j]);
                }
            }
            emit("\n");
        }
    }

    /* Register every function's signature (calls convert their arguments to the parameter types) */
    func_count = 0;
    for (i = 0; i < node->u.program.decl_count; i++)
        if (node->u.program.decls[i]->type == AST_FUNC_DECL) register_func(node->u.program.decls[i]);

    /* Second pass: generate code for functions and variables */
    for (i = 0; i < node->u.program.decl_count; i++) {
        ASTNode* decl = node->u.program.decls[i];

        if (decl->type == AST_FUNC_DECL) {
            if (decl->u.func_decl.body) codegen_function(decl);   /* a prototype only registers the signature */
        } else if (decl->type == AST_VAR_DECL) {
            int size;
            /* Global variable */
            emit("\n; Global variable: %s\n", decl->u.var_decl.var_name);
            size = get_type_size(decl->u.var_decl.var_type, decl->u.var_decl.pointer_level, decl->u.var_decl.struct_name);
            if (size == 1) {
                emit("%s: .byte 0\n", decl->u.var_decl.var_name);
            } else {
                emit("%s: .word 0\n", decl->u.var_decl.var_name);
            }
        }
    }
}

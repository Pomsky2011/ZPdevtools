#include "codegen.h"
#include <string.h>
#include "zpstrdup.h"
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

static CodegenContext ctx;

/* Forward declarations */
static void codegen_function(ASTNode* node);
static void codegen_statement(ASTNode* node);
static void codegen_expression(ASTNode* node);
static void codegen_addr_of(ASTNode* node);
static void codegen_deref(ASTNode* node);
static void codegen_ptr_arith(ASTNode* node);
static void codegen_lvalue_load(ASTNode* node);
static void codegen_assign_lv(ASTNode* node);
static void codegen_incdec_lv(ASTNode* node);
static void codegen_addr(ASTNode* node);
static void pop_symbols_to(int mark);

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


/* ====================================================================================================
 * Full C types (what a value or object *is*; CType below is only what the code generator needs of a value).
 * Pointers are 16-bit offsets into bank $80, the bank the stack and all data live in; memory behind a pointer is
 * reached with LDA/STA $800000,X, which does not depend on the data bank register.
 * ==================================================================================================== */
typedef struct {
    DataType base;        /* int, short, char, long, void, struct/union, bool */
    TypeModifier mods;
    int ptr;              /* levels of pointer */
    const char* sname;    /* struct/union name */
    int ndim;             /* array: number of dimensions ... */
    int dims[MAX_DIMS];   /* ... and their sizes */
} Ty;

static Ty ty_make(DataType base, TypeModifier mods, int ptr, const char* sname) {
    Ty t;
    int i;
    t.base = base;
    t.mods = mods;
    t.ptr = ptr;
    t.sname = sname;
    t.ndim = 0;
    for (i = 0; i < MAX_DIMS; i++) t.dims[i] = 0;
    return t;
}

static int ty_ptrlike(const Ty* t) { return t->ptr > 0 || t->ndim > 0; }

static int ty_is_struct(const Ty* t) { return t->base == TYPE_STRUCT && t->ptr == 0 && t->ndim == 0; }

/* size in bytes of an object of this type (an array: all its elements) */
static int ty_size(const Ty* t) {
    long n = 1;
    int i, elem;
    if (t->ptr > 0) elem = 2;
    else if (t->base == TYPE_VOID) elem = 1;
    else elem = get_type_size(t->base, 0, t->sname);
    for (i = 0; i < t->ndim; i++) {
        n *= t->dims[i];
        if (n > 32767L) return 32767;
    }
    n *= elem;
    return n > 32767L ? 32767 : (int)n;
}

/* the type of *p, p[i], or the element of an array */
static Ty ty_deref(Ty t) {
    int i;
    if (t.ndim > 0) {
        for (i = 1; i < t.ndim; i++) t.dims[i - 1] = t.dims[i];
        t.ndim--;
    } else if (t.ptr > 0) {
        t.ptr--;
    }
    return t;
}

/* an array used as a value is a pointer to its first element */
static Ty ty_decay(Ty t) {
    if (t.ndim > 0) {
        t.ndim = 0;
        t.ptr++;
    }
    return t;
}

static StructMember* find_member(StructDef* sd, const char* name) {
    int i;
    for (i = 0; i < sd->member_count; i++)
        if (strcmp(sd->members[i].name, name) == 0) return &sd->members[i];
    return NULL;
}

static Ty member_ty(const StructMember* m) {
    Ty t = ty_make(m->type, m->modifiers, m->pointer_level, m->struct_name);
    int i;
    t.ndim = m->ndim;
    for (i = 0; i < m->ndim; i++) t.dims[i] = m->dims[i];
    return t;
}

static Ty sym_ty_raw(const Symbol* s) {
    Ty t = ty_make(s->type, s->modifiers, s->pointer_level, s->struct_name);
    int i;
    if (s->is_array) {
        t.ndim = s->array_dimensions > MAX_DIMS ? MAX_DIMS : s->array_dimensions;
        for (i = 0; i < t.ndim; i++) t.dims[i] = s->array_sizes[i];
    }
    return t;
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
        /* every local starts on a word boundary and takes a whole number of words */
        Ty lt = sym_ty_raw(&ctx.symbols[ctx.symbol_count]);
        ctx.symbols[ctx.symbol_count].offset = local_var_index * 2;
        local_var_index += (ty_size(&lt) + 1) / 2;
    }

    ctx.symbol_count++;
}

/* Add symbol with multi-dimensional array support */
static void add_symbol_multidim(const char* name, DataType type, int is_param, int param_index, int* array_sizes, int dimensions, int pointer_level, const char* struct_name) {
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
        Ty lt = sym_ty_raw(&ctx.symbols[ctx.symbol_count]);
        ctx.symbols[ctx.symbol_count].offset = local_var_index * 2;
        local_var_index += (ty_size(&lt) + 1) / 2;
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
typedef struct { char* name; CType ret; Ty ret_ty; int nparams; CType params[MAX_PARAMS]; } FuncInfo;
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
    f->ret = type_ctype(node->u.func_decl.return_type, node->u.func_decl.return_modifiers, node->u.func_decl.return_pointer_level);
    f->ret_ty = ty_make(node->u.func_decl.return_type, node->u.func_decl.return_modifiers, node->u.func_decl.return_pointer_level, NULL);
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

/* the CType of a value of this type, after the integer promotions: char (any signedness) and bool become signed int,
   unsigned short/int stay unsigned (same width), every pointer and array is an unsigned word */
static CType ty_ctype(const Ty* t) {
    if (ty_ptrlike(t)) return ctype(1, 1);
    if (t->base == TYPE_LONG) return ctype(2, (t->mods & MOD_UNSIGNED) != 0);
    if (t->base == TYPE_SHORT || t->base == TYPE_INT) return ctype(1, (t->mods & MOD_UNSIGNED) != 0);
    return ctype(1, 0);
}

static Ty ty_from_ctype(CType c) {
    return ty_make(c.words == 2 ? TYPE_LONG : TYPE_INT, c.uns ? MOD_UNSIGNED : MOD_NONE, 0, NULL);
}

static Ty sym_ty(Symbol* s) { return sym_ty_raw(s); }

static Ty type_of(ASTNode* n);

static CType expr_type(ASTNode* n) {
    Ty t = type_of(n);
    return ty_ctype(&t);
}

static Ty type_of(ASTNode* n) {
    Symbol* sym;
    FuncInfo* f;
    StructDef* sd;
    StructMember* m;
    Ty t, u;
    CType a, b;
    unsigned int v;
    if (!n) return ty_make(TYPE_INT, MOD_NONE, 0, NULL);
    switch (n->type) {
        case AST_NUMBER:
            v = (unsigned int)n->u.number;
            if (v <= 0x7FFFu) return ty_make(TYPE_INT, MOD_NONE, 0, NULL);
            if (v <= 0xFFFFu) return ty_make(TYPE_INT, MOD_UNSIGNED, 0, NULL);
            if (v <= 0x7FFFFFFFu) return ty_make(TYPE_LONG, MOD_NONE, 0, NULL);
            return ty_make(TYPE_LONG, MOD_UNSIGNED, 0, NULL);
        case AST_IDENTIFIER:
            sym = find_symbol(n->u.identifier);
            return sym ? sym_ty(sym) : ty_make(TYPE_INT, MOD_NONE, 0, NULL);
        case AST_STRING_LITERAL:
            return ty_make(TYPE_CHAR, MOD_NONE, 1, NULL);
        case AST_SIZEOF:
            return ty_make(TYPE_INT, MOD_UNSIGNED, 0, NULL);
        case AST_CAST:
            return ty_make(n->u.cast.target_type, n->mods, n->u.cast.pointer_level, n->u.cast.type_name);
        case AST_COMMA:
            return n->u.comma.expr_count > 0 ? type_of(n->u.comma.expressions[n->u.comma.expr_count - 1])
                                             : ty_make(TYPE_INT, MOD_NONE, 0, NULL);
        case AST_TERNARY:
            t = type_of(n->u.ternary.then_expr);
            u = type_of(n->u.ternary.else_expr);
            if (ty_ptrlike(&t)) return ty_decay(t);
            if (ty_ptrlike(&u)) return ty_decay(u);
            return ty_from_ctype(usual(ty_ctype(&t), ty_ctype(&u)));
        case AST_BINOP:
            switch (n->u.binop.op) {
                case OP_EQ: case OP_NE: case OP_LT: case OP_GT: case OP_LE: case OP_GE:
                case OP_AND: case OP_OR:
                    return ty_make(TYPE_INT, MOD_NONE, 0, NULL);
                case OP_SHL: case OP_SHR:
                    t = type_of(n->u.binop.left);
                    return ty_from_ctype(ty_ctype(&t));
                default:
                    t = type_of(n->u.binop.left);
                    u = type_of(n->u.binop.right);
                    if (ty_ptrlike(&t) || ty_ptrlike(&u)) {
                        if (n->u.binop.op == OP_ADD) return ty_decay(ty_ptrlike(&t) ? t : u);
                        if (n->u.binop.op == OP_SUB) {
                            if (ty_ptrlike(&t) && ty_ptrlike(&u)) return ty_make(TYPE_INT, MOD_NONE, 0, NULL);
                            return ty_decay(t);
                        }
                    }
                    a = ty_ctype(&t);
                    b = ty_ctype(&u);
                    return ty_from_ctype(usual(a, b));
            }
        case AST_UNOP:
            if (n->u.unop.op == OP_NOT) return ty_make(TYPE_INT, MOD_NONE, 0, NULL);
            if (n->u.unop.op == OP_ADDR_OF) {
                t = type_of(n->u.unop.operand);
                t.ptr++;
                return t;
            }
            if (n->u.unop.op == OP_DEREF) {
                t = type_of(n->u.unop.operand);
                return ty_ptrlike(&t) ? ty_deref(t) : ty_make(TYPE_INT, MOD_NONE, 0, NULL);
            }
            t = type_of(n->u.unop.operand);
            return ty_from_ctype(ty_ctype(&t));
        case AST_ADDR_OF:
            t = type_of(n->u.unop.operand);
            t.ptr++;
            return t;
        case AST_DEREF:
            t = type_of(n->u.unop.operand);
            return ty_ptrlike(&t) ? ty_deref(t) : ty_make(TYPE_INT, MOD_NONE, 0, NULL);
        case AST_INDEX:
            t = type_of(n->u.index_expr.base);
            return ty_ptrlike(&t) ? ty_deref(t) : ty_make(TYPE_INT, MOD_NONE, 0, NULL);
        case AST_MEMBER_ACCESS:
        case AST_PTR_MEMBER_ACCESS:
            t = type_of(n->type == AST_MEMBER_ACCESS ? n->u.member_access.object : n->u.ptr_member_access.pointer);
            if (n->type == AST_PTR_MEMBER_ACCESS) {
                if (!ty_ptrlike(&t)) return ty_make(TYPE_INT, MOD_NONE, 0, NULL);
                t = ty_deref(t);
            }
            if (!ty_is_struct(&t) || !t.sname || !(sd = find_struct(t.sname))) return ty_make(TYPE_INT, MOD_NONE, 0, NULL);
            m = find_member(sd, n->type == AST_MEMBER_ACCESS ? n->u.member_access.member_name : n->u.ptr_member_access.member_name);
            return m ? member_ty(m) : ty_make(TYPE_INT, MOD_NONE, 0, NULL);
        case AST_CALL:
            f = find_func(n->u.call.name);
            return f ? f->ret_ty : ty_make(TYPE_INT, MOD_NONE, 0, NULL);
        case AST_ASSIGN:
            sym = find_symbol(n->u.assign.var_name);
            return sym ? sym_ty(sym) : ty_make(TYPE_INT, MOD_NONE, 0, NULL);
        case AST_ASSIGN_LV:
            return type_of(n->u.assign_lv.target);
        case AST_INCDEC_LV:
            return type_of(n->u.incdec_lv.operand);
        case AST_PRE_INC: case AST_POST_INC: case AST_PRE_DEC: case AST_POST_DEC:
            sym = find_symbol(n->u.inc_dec.var_name);
            return sym ? sym_ty(sym) : ty_make(TYPE_INT, MOD_NONE, 0, NULL);
        default:
            return ty_make(TYPE_INT, MOD_NONE, 0, NULL);
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

/* A holds a value being stored into an 8-bit type: keep the low byte, sign-extending it for signed char (plain char is
   unsigned here).  Every other 16-bit-or-wider value is already exact. */
static void emit_narrow(int is_signed) {
    emit("    AND #$00FF    ; narrow to 8 bits\n");
    if (is_signed) {
        int ok = new_label();
        emit("    CMP #$0080\n");
        emit("    BCC .L%d\n", ok);
        emit("    ORA #$FF00\n");
        emit(".L%d:\n", ok);
    }
}

/* A = the address of the frame byte at 'off' */
static void emit_frame_addr(int off) {
    emit("    TSC\n    CLC\n    ADC #$%04X\n", SPO(off) & 0xFFFF);
}

static void emit_add_const(int n) {
    if (n != 0) emit("    CLC\n    ADC #$%04X\n", n & 0xFFFF);
}

/* A *= es, an element size */
static void emit_scale(int es) {
    switch (es) {
        case 1: break;
        case 2: emit("    ASL A\n"); break;
        case 4: emit("    ASL A\n    ASL A\n"); break;
        default:
            emit("    STA %d,S\n    LDA #$%04X\n    MUL %d,S\n", SPO(SCRATCH0), es & 0xFFFF, SPO(SCRATCH0));
            break;
    }
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
    if (sym->type == TYPE_CHAR && sym->pointer_level == 0 && !sym->is_array)
        emit_narrow((sym->modifiers & MOD_SIGNED) != 0);
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
        compile_error("Error: Undefined variable '%s'\n", node->u.identifier);
        return;
    }
    if (sym->is_array) {                         /* an array used as a value is the address of its first element */
        emit_frame_addr(sym->offset);
        return;
    }
    if (sym->type == TYPE_STRUCT && sym->pointer_level == 0) {
        compile_error("Error: struct '%s' cannot be used as a value (use its members)\n", sym->name);
        return;
    }
    load_sym(sym);
}

static void codegen_string_literal(ASTNode* node) {
    (void)node;
    compile_error("Error: a string literal cannot be used as a pointer value (ROM data is not reachable through "
                  "pointers); initialise a char array with it: char s[] = \"...\";\n");
}

static void codegen_sizeof(ASTNode* node) {
    Ty t;
    int size;
    if (node->u.sizeof_expr.expr) {
        t = type_of(node->u.sizeof_expr.expr);        /* arrays do not decay under sizeof */
    } else {
        t = ty_make(node->u.sizeof_expr.size_type, MOD_NONE, node->u.sizeof_expr.pointer_level,
                    node->u.sizeof_expr.type_name);
    }
    size = ty_size(&t);
    emit("    LDA #$%04X\n", size & 0xFFFF);
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
            emit_narrow((node->mods & MOD_SIGNED) != 0);   /* truncate to 8 bits; signed char sign-extends bit 7 */
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
    if (op == OP_ADD || op == OP_SUB) {
        Ty plt = type_of(node->u.binop.left);
        Ty prt = type_of(node->u.binop.right);
        if (ty_ptrlike(&plt) || ty_ptrlike(&prt)) { codegen_ptr_arith(node); return; }
    }

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
        compile_error("Error: too many arguments in call to '%s'\n", node->u.call.name);
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
        compile_error("Error: Undefined variable '%s'\n", node->u.assign.var_name);
        return;
    }
    if (sym->is_array || (sym->type == TYPE_STRUCT && sym->pointer_level == 0)) {
        compile_error("Error: assignment to array, struct or union '%s' is not supported\n", sym->name);
        return;
    }
    codegen_expr_to(node->u.assign.value, sym_ctype(sym));
    store_sym(sym);
}

/* ====================================================================================================
 * Memory through pointers.  A pointer is a 16-bit offset into bank $80; an address lives in A (16 bits) and the
 * data is reached with LDA/STA $800000,X (X = the address).
 * ==================================================================================================== */

/* is there a side effect in this expression (so evaluating it twice would be wrong)? */
static int side_effect_visitor(ASTNode* n, void* unused) {
    (void)unused;
    switch (n->type) {
    case AST_CALL: case AST_ASSIGN: case AST_ASSIGN_LV: case AST_INCDEC_LV:
    case AST_PRE_INC: case AST_POST_INC: case AST_PRE_DEC: case AST_POST_DEC:
    case AST_INLINE_ASM:
        return 1;
    default:
        return 0;
    }
}

/* A = the address of the lvalue */
static void codegen_addr(ASTNode* node) {
    switch (node->type) {
    case AST_IDENTIFIER: {
        Symbol* sym = find_symbol(node->u.identifier);
        if (!sym) {
            compile_error("Error: Undefined variable '%s'\n", node->u.identifier);
            return;
        }
        emit_frame_addr(sym->offset);
        return;
    }
    case AST_INDEX: {
        ASTNode* base = node->u.index_expr.base;
        Ty bt = type_of(base);
        Ty et;
        int es;
        if (!ty_ptrlike(&bt)) {
            compile_error("Error: subscripted value is neither an array nor a pointer\n");
            return;
        }
        et = ty_deref(bt);
        if (et.base == TYPE_VOID && et.ptr == 0 && et.ndim == 0) {
            compile_error("Error: dereferencing a void pointer\n");
            return;
        }
        es = ty_size(&et);
        if (bt.ndim > 0) codegen_addr(base);          /* an array is its own address ... */
        else codegen_expression(base);                /* ... a pointer holds one */
        push_a();
        codegen_expr_to(node->u.index_expr.index, ctype(1, 0));
        emit_scale(es);
        emit("    CLC\n    ADC 1,S\n");
        emit_drop_bytes(2);
        ctx.depth -= 2;
        return;
    }
    case AST_MEMBER_ACCESS:
    case AST_PTR_MEMBER_ACCESS: {
        int arrow = (node->type == AST_PTR_MEMBER_ACCESS);
        ASTNode* obj = arrow ? node->u.ptr_member_access.pointer : node->u.member_access.object;
        const char* name = arrow ? node->u.ptr_member_access.member_name : node->u.member_access.member_name;
        Ty ot = type_of(obj);
        StructDef* sd;
        StructMember* m;
        if (arrow) {
            if (!ty_ptrlike(&ot)) {
                compile_error("Error: '->' applied to something that is not a pointer\n");
                return;
            }
            ot = ty_deref(ot);
        }
        if (!ty_is_struct(&ot) || !ot.sname || !(sd = find_struct(ot.sname))) {
            compile_error("Error: member '%s' requested in something that is not a struct or union\n", name);
            return;
        }
        m = find_member(sd, name);
        if (!m) {
            compile_error("Error: '%s' has no member named '%s'\n", ot.sname, name);
            return;
        }
        if (arrow) {
            Ty pt = type_of(obj);
            if (pt.ndim > 0) codegen_addr(obj);
            else codegen_expression(obj);
        } else {
            codegen_addr(obj);
        }
        emit_add_const(m->offset);
        return;
    }
    case AST_DEREF: {
        ASTNode* op = node->u.unop.operand;
        Ty ot = type_of(op);
        if (!ty_ptrlike(&ot)) {
            compile_error("Error: dereferencing something that is not a pointer\n");
            return;
        }
        codegen_expression(op);            /* an array decays to its address, a pointer is its value */
        return;
    }
    default:
        compile_error("Error: expression is not an lvalue\n");
        return;
    }
}

/* A holds an address: load the object of type t from it (A, and X for a long) */
static void emit_load_from_addr(const Ty* t) {
    int size = ty_size(t);
    emit("    TAX\n");
    if (size == 1) {
        emit("    SEP #$20\n    LDA $800000,X\n    REP #$20\n");
        emit_narrow((t->mods & MOD_SIGNED) != 0);
    } else if (size == 4) {
        emit("    LDA $800002,X\n    TAY\n    LDA $800000,X\n    TYX\n");
    } else {
        emit("    LDA $800000,X\n");
    }
}

/* the value of an lvalue expression (arrays do not load: their value is their address) */
static void codegen_lvalue_load(ASTNode* node) {
    Ty t = type_of(node);
    if (ty_is_struct(&t)) {
        compile_error("Error: a struct or union value cannot be used as an expression (use its members)\n");
        return;
    }
    if (t.base == TYPE_VOID && t.ptr == 0 && t.ndim == 0) {
        compile_error("Error: dereferencing a void pointer\n");
        return;
    }
    codegen_addr(node);
    if (t.ndim > 0) return;
    emit_load_from_addr(&t);
}

/* A (and X) hold a value already converted to the target's type: store it through the target's address.  Both
   registers still hold the value afterwards. */
static void codegen_store_lv(ASTNode* target, const Ty* tt) {
    int size;
    if (tt->ndim > 0 || ty_is_struct(tt)) {
        compile_error("Error: assignment to an array, struct or union is not supported\n");
        return;
    }
    if (target->type == AST_IDENTIFIER) {
        Symbol* sym = find_symbol(target->u.identifier);
        if (sym) store_sym(sym);
        return;
    }
    size = ty_size(tt);
    if (size == 4) push_ax(); else push_a();
    codegen_addr(target);
    emit("    TAX\n");
    if (size == 4) {
        emit("    LDA 3,S\n    STA $800002,X\n    LDA 1,S\n    STA $800000,X\n    LDA 3,S\n    TAX\n    LDA 1,S\n");
    } else if (size == 1) {
        emit("    LDA 1,S\n");
        emit_narrow((tt->mods & MOD_SIGNED) != 0);
        emit("    SEP #$20\n    STA $800000,X\n    REP #$20\n");
    } else {
        emit("    LDA 1,S\n    STA $800000,X\n");
    }
    emit_drop_bytes(size == 4 ? 4 : 2);
    ctx.depth -= (size == 4 ? 4 : 2);
}

static void codegen_assign_lv(ASTNode* node) {
    ASTNode* target = node->u.assign_lv.target;
    ASTNode* value = node->u.assign_lv.value;
    ASTNode* synth = NULL;
    Ty tt = type_of(target);
    if (node->u.assign_lv.op) {
        if (ast_walk(target, side_effect_visitor, NULL)) {
            compile_error("Error: compound assignment to an lvalue with side effects is not supported\n");
            return;
        }
        synth = ast_create_binop((BinOpType)(node->u.assign_lv.op - 1), target, value);   /* target op value */
        value = synth;
    }
    codegen_expr_to(value, ty_ctype(&tt));
    free(synth);                                 /* the shell only: its children belong to the tree */
    codegen_store_lv(target, &tt);
}

/* ++ / -- on an lvalue other than a plain variable */
static void codegen_incdec_lv(ASTNode* node) {
    ASTNode* target = node->u.incdec_lv.operand;
    int inc = node->u.incdec_lv.inc;
    int post = node->u.incdec_lv.post;
    Ty tt = type_of(target);
    CType ct;
    int step = 1;
    if (ast_walk(target, side_effect_visitor, NULL)) {
        compile_error("Error: ++/-- on an lvalue with side effects is not supported\n");
        return;
    }
    if (tt.ndim > 0 || ty_is_struct(&tt)) {
        compile_error("Error: cannot increment or decrement an array, struct or union\n");
        return;
    }
    if (tt.ptr > 0) {
        Ty e = ty_deref(tt);
        step = ty_size(&e);
    }
    ct = ty_ctype(&tt);
    codegen_lvalue_load(target);                              /* the old value */
    if (post) {
        if (ct.words == 2) push_ax(); else push_a();
    }
    if (ct.words == 1) {
        if (inc) emit("    CLC\n    ADC #$%04X\n", step & 0xFFFF);
        else     emit("    SEC\n    SBC #$%04X\n", step & 0xFFFF);
    } else if (inc) {
        emit("    CLC\n    ADC #$%04X\n    TAY\n    TXA\n    ADC #$0000\n    TAX\n    TYA\n", step & 0xFFFF);
    } else {
        emit("    SEC\n    SBC #$%04X\n    TAY\n    TXA\n    SBC #$0000\n    TAX\n    TYA\n", step & 0xFFFF);
    }
    codegen_store_lv(target, &tt);
    if (post) {                                               /* the value of x++ is the old value */
        if (ct.words == 2) {
            emit("    PLA\n    TAY\n    PLA\n    TAX\n    TYA\n");
            ctx.depth -= 4;
        } else {
            pop_a();
        }
    }
}

static void codegen_addr_of(ASTNode* node) {
    ASTNode* op = node->u.unop.operand;
    Ty t = type_of(op);
    if (t.ndim > 0) {
        compile_error("Error: the address of a whole array is not supported (an array already is an address)\n");
        return;
    }
    codegen_addr(op);
}

static void codegen_deref(ASTNode* node) {
    codegen_lvalue_load(node);
}

static ASTNode* mk_cast(DataType type, TypeModifier mods, ASTNode* e) {
    ASTNode* c = ast_create_cast(type, NULL, 0, e);
    c->mods = mods;
    return c;
}

/* pointer +/- integer, integer + pointer and pointer - pointer, as integer arithmetic on the unsigned address words
   with the integer scaled by the size of the pointed-to object.  Built from throwaway nodes so every operator
   keeps its one implementation; only the shells are freed, their children belong to the original tree. */
static void codegen_ptr_arith(ASTNode* node) {
    BinOpType op = node->u.binop.op;
    ASTNode* l = node->u.binop.left;
    ASTNode* r = node->u.binop.right;
    Ty lt = type_of(l);
    Ty rt = type_of(r);
    int lp = ty_ptrlike(&lt);
    int rp = ty_ptrlike(&rt);
    ASTNode* p;
    ASTNode* n;
    ASTNode* sc;
    ASTNode* pc;
    ASTNode* res;
    Ty pt, et;
    int es;

    if ((lp && lt.ndim > 1) || (rp && rt.ndim > 1)) {
        compile_error("Error: pointer arithmetic on a multi-dimensional array is not supported (index it instead)\n");
        return;
    }
    if (lp && rp) {
        Ty le, re;
        ASTNode* cl;
        ASTNode* cr;
        ASTNode* diff;
        ASTNode* num;
        if (op != OP_SUB) {
            compile_error("Error: two pointers cannot be added\n");
            return;
        }
        le = ty_deref(lt);
        re = ty_deref(rt);
        es = ty_size(&le);
        if (es != ty_size(&re)) {
            compile_error("Error: subtracting pointers to objects of different sizes\n");
            return;
        }
        if (le.base == TYPE_VOID && le.ptr == 0) es = 1;
        cl = mk_cast(TYPE_INT, MOD_NONE, l);
        cr = mk_cast(TYPE_INT, MOD_NONE, r);
        diff = ast_create_binop(OP_SUB, cl, cr);
        if (es > 1) {
            num = ast_create_number(es);
            res = ast_create_binop(OP_DIV, diff, num);
            codegen_expression(res);
            free(num);
        } else {
            res = diff;
            codegen_expression(res);
            diff = NULL;
        }
        free(res);
        free(diff);
        free(cl);
        free(cr);
        return;
    }
    if (lp) {
        p = l; n = r; pt = lt;
    } else {
        if (op != OP_ADD) {
            compile_error("Error: an integer minus a pointer is not valid\n");
            return;
        }
        p = r; n = l; pt = rt;
    }
    et = ty_deref(pt);
    es = ty_size(&et);
    if (et.base == TYPE_VOID && et.ptr == 0 && et.ndim == 0) es = 1;     /* void *: byte arithmetic (an extension) */
    {
        ASTNode* cn = mk_cast(TYPE_INT, MOD_NONE, n);
        ASTNode* num = NULL;
        if (es == 1) {
            sc = cn;
        } else {
            num = ast_create_number(es);
            sc = ast_create_binop(OP_MUL, cn, num);
        }
        pc = mk_cast(TYPE_INT, MOD_UNSIGNED, p);
        res = ast_create_binop(op, pc, sc);
        codegen_expression(res);
        free(res);
        free(pc);
        if (num) { free(sc); free(num); }
        free(cn);
    }
}

static void codegen_inc_dec(ASTNode* node) {
    Symbol* sym = find_symbol(node->u.inc_dec.var_name);
    CType t;
    Ty st;
    int step = 1;
    int inc = (node->type == AST_PRE_INC || node->type == AST_POST_INC);
    int post = (node->type == AST_POST_INC || node->type == AST_POST_DEC);
    if (!sym) {
        compile_error("Error: Undefined variable '%s'\n", node->u.inc_dec.var_name);
        return;
    }
    st = sym_ty(sym);
    if (st.ndim > 0 || ty_is_struct(&st)) {
        compile_error("Error: cannot increment or decrement array, struct or union '%s'\n", sym->name);
        return;
    }
    if (st.ptr > 0) {
        Ty e = ty_deref(st);
        step = ty_size(&e);
    }
    t = sym_ctype(sym);
    load_sym(sym);
    if (t.words == 1) {
        if (post) emit("    TAY\n");
        if (step == 1) emit(inc ? "    INC A\n" : "    DEC A\n");
        else if (inc) emit("    CLC\n    ADC #$%04X\n", step & 0xFFFF);
        else emit("    SEC\n    SBC #$%04X\n", step & 0xFFFF);
        store_sym(sym);                         /* narrows a char; A keeps the new value */
        if (post) emit("    TYA\n");
        return;
    }
    if (post) push_ax();
    if (inc) emit("    CLC\n    ADC #$%04X\n    TAY\n    TXA\n    ADC #$0000\n    TAX\n    TYA\n", step & 0xFFFF);
    else     emit("    SEC\n    SBC #$%04X\n    TAY\n    TXA\n    SBC #$0000\n    TAX\n    TYA\n", step & 0xFFFF);
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
        case AST_ASSIGN_LV:
            codegen_assign_lv(node);
            break;
        case AST_INCDEC_LV:
            codegen_incdec_lv(node);
            break;
        case AST_ADDR_OF:
            codegen_addr_of(node);
            break;
        case AST_INDEX:
        case AST_DEREF:
        case AST_MEMBER_ACCESS:
        case AST_PTR_MEMBER_ACCESS:
            codegen_lvalue_load(node);
            break;
        case AST_PRE_INC:
        case AST_POST_INC:
        case AST_PRE_DEC:
        case AST_POST_DEC:
            codegen_inc_dec(node);
            break;
        default:
            compile_error("Error: Unsupported expression type %d\n", node->type);
            break;
    }
}

/* Code generation for statements */
static void codegen_return(ASTNode* node) {
    if (node->u.ret.value) {
        codegen_expr_to(node->u.ret.value, ctype(ctx.ret_words, ctx.ret_uns));
        if (ctx.ret_char) emit_narrow(ctx.ret_signed);
    }

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
    int cond_label = new_label();
    int done_label = new_label();

    /* break leaves the loop; continue goes to the condition test, not back to the top of the body */
    push_loop(done_label, cond_label);

    emit(".L%d:\n", loop_label);

    /* Loop body (executes at least once) */
    codegen_statement(node->u.do_while_stmt.body);

    /* Evaluate condition */
    emit(".L%d:\n", cond_label);
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

/* The CPU has ONE hardware loop counter (loopCounter/loopStart), so LOOP/LPEND is only correct for an innermost loop that
   nothing else can disturb, and only when the loop does exactly what the counter does: N iterations of the body
   with the variable stepping 0..N-1.  Anything else must use an ordinary compare-and-branch loop. */
static int hw_loop_unsafe(ASTNode* n, void* ctx) {
    const char* var = (const char*)ctx;
    switch (n->type) {
    case AST_CALL:          /* the callee may use the loop counter itself */
    case AST_FOR:           /* nested loops share the single counter */
    case AST_WHILE:
    case AST_DO_WHILE:
    case AST_CONTINUE:      /* would skip the increment and LPEND */
    case AST_GOTO:          /* can leave or enter the loop body */
    case AST_LABEL:
    case AST_INLINE_ASM:    /* unknown instructions */
        return 1;
    case AST_ASSIGN:        /* the loop variable must only be stepped by the loop itself */
        return strcmp(n->u.assign.var_name, var) == 0;
    case AST_PRE_INC:
    case AST_POST_INC:
    case AST_PRE_DEC:
    case AST_POST_DEC:
        return strcmp(n->u.inc_dec.var_name, var) == 0;
    default:
        return 0;
    }
}

int codegen_no_hwloop = 0;

/* ast_walk visitor: remember the name of every variable that appears directly under & */
static int collect_addr_taken(ASTNode* n, void* unused) {
    ASTNode* op = NULL;
    (void)unused;
    if (n->type == AST_ADDR_OF) op = n->u.unop.operand;
    else if (n->type == AST_UNOP && n->u.unop.op == OP_ADDR_OF) op = n->u.unop.operand;
    if (op && op->type == AST_IDENTIFIER) {
        char** nl = (char**)realloc(ctx.addr_taken, sizeof(char*) * (ctx.addr_taken_count + 1));
        if (nl) {
            ctx.addr_taken = nl;
            ctx.addr_taken[ctx.addr_taken_count++] = zp_strdup(op->u.identifier);
        }
    }
    return 0;
}

static int is_addr_taken(const char* name) {
    int i;
    for (i = 0; i < ctx.addr_taken_count; i++)
        if (strcmp(ctx.addr_taken[i], name) == 0) return 1;
    return 0;
}

static int hw_loop_ok(ASTNode* node, char* loop_var, int loop_count) {
    if (codegen_no_hwloop) return 0;
    if (is_addr_taken(loop_var)) return 0;       /* a store through a pointer could change the counter variable */
    if (loop_count < 1 || loop_count > 32767) return 0;   /* LOOP #0 runs the body once; a negative count wraps */
    return !ast_walk(node->u.for_stmt.body, hw_loop_unsafe, loop_var);
}

static void codegen_for(ASTNode* node) {
    int done_label;
    int loop_label;
    int increment_label;
    char* loop_var = NULL;
    int loop_count = 0;

    /* Try to use hardware LOOP/LPEND for simple counted loops */
    if (is_simple_counted_loop(node->u.for_stmt.init, node->u.for_stmt.condition,
                                node->u.for_stmt.increment, &loop_var, &loop_count) &&
        hw_loop_ok(node, loop_var, loop_count)) {
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
    int mark = ctx.symbol_count;
    int saved_base = ctx.scope_base;
    ctx.scope_base = mark;
    for (i = 0; i < node->u.block.stmt_count; i++) {
        codegen_statement(node->u.block.statements[i]);
    }
    /* the block's variables go out of scope (their frame slots stay reserved: count_locals counted them) */
    pop_symbols_to(mark);
    ctx.scope_base = saved_base;
}

/* ====================================================================================================
 * Declarations and initializers
 * ==================================================================================================== */

/* the bytes of a string literal token ("..." with escapes); returns the length, without the NUL */
static int unescape_literal(const char* tok, unsigned char* out, int max) {
    int n = 0;
    const char* p = tok;
    if (*p == '"') p++;
    while (*p && !(*p == '"' && p[1] == '\0')) {
        int c = (unsigned char)*p++;
        if (c == '\\' && *p) {
            c = (unsigned char)*p++;
            switch (c) {
                case 'n': c = 10; break;
                case 't': c = 9; break;
                case 'r': c = 13; break;
                case 'a': c = 7; break;
                case 'b': c = 8; break;
                case 'f': c = 12; break;
                case 'v': c = 11; break;
                case 'x': {
                    int v = 0, any = 0;
                    while (isxdigit((unsigned char)*p)) {
                        v = v * 16 + (isdigit((unsigned char)*p) ? *p - '0' : (tolower((unsigned char)*p) - 'a' + 10));
                        p++;
                        any = 1;
                    }
                    c = any ? (v & 0xFF) : 'x';
                    break;
                }
                default:
                    if (c >= '0' && c <= '7') {
                        int v = c - '0', cnt = 1;
                        while (cnt < 3 && *p >= '0' && *p <= '7') { v = v * 8 + (*p - '0'); p++; cnt++; }
                        c = v & 0xFF;
                    }
                    break;       /* \\ \' \" \? keep the character */
            }
        }
        if (n < max) out[n] = (unsigned char)c;
        n++;
    }
    return n;
}

static int is_aggregate(const Ty* t) { return t->ndim > 0 || ty_is_struct(t); }

static void init_value(Ty t, int off, ASTNode** items, int n, int* pos, int dry);

/* store one scalar (or zero, when the initializer ran out) into the frame at 'off' */
static void init_scalar(const Ty* t, int off, ASTNode* item, int dry) {
    int size;
    if (dry) return;
    size = ty_size(t);
    if (item) {
        codegen_expr_to(item, ty_ctype(t));
    } else {
        emit("    LDA #$0000\n");
        if (size == 4) emit("    LDX #$0000\n");
    }
    if (size == 1) {
        emit("    SEP #$20\n    STA %d,S\n    REP #$20\n", SPO(off));
    } else if (size == 4) {
        emit("    STA %d,S\n    TXA\n    STA %d,S\n", SPO(off), SPO(off + 2));
    } else {
        emit("    STA %d,S\n", SPO(off));
    }
}

/* char array (t: array of char, dims[0] = number of elements) initialised from a string literal */
static void init_string(const Ty* t, int off, ASTNode* lit, int dry) {
    unsigned char buf[4];
    unsigned char* data;
    int len, count, i;
    if (dry) return;
    count = t->dims[0];
    len = unescape_literal(lit->u.string_literal, buf, 0);
    if (len > count) {
        compile_error("Error: initializer string is too long for the array\n");
        return;
    }
    data = (unsigned char*)malloc(len + 1);
    if (!data) { compile_error("Error: out of memory\n"); return; }
    unescape_literal(lit->u.string_literal, data, len);
    i = 0;
    while (i < count) {
        int b0 = (i < len) ? data[i] : 0;
        if (((off + i) & 1) == 0 && i + 1 < count) {          /* an aligned pair goes in as one word */
            int b1 = (i + 1 < len) ? data[i + 1] : 0;
            emit("    LDA #$%04X\n    STA %d,S\n", b0 | (b1 << 8), SPO(off + i));
            i += 2;
        } else {
            emit("    SEP #$20\n    LDA #$%02X\n    STA %d,S\n    REP #$20\n", b0, SPO(off + i));
            i++;
        }
    }
    free(data);
}

/* fill an object of type t at frame offset 'off' from the sequence items[*pos..n), consuming what it needs (braces
   may be elided, as in C) and zero-filling what the sequence does not cover */
static void init_fill(Ty t, int off, ASTNode** items, int n, int* pos, int dry) {
    if (t.ndim > 0) {
        Ty e = ty_deref(t);
        int es = ty_size(&e);
        int k;
        if (e.ndim == 0 && e.base == TYPE_CHAR && e.ptr == 0 && *pos < n && items[*pos]->type == AST_STRING_LITERAL) {
            init_string(&t, off, items[*pos], dry);
            (*pos)++;
            return;
        }
        for (k = 0; k < t.dims[0]; k++) init_value(e, off + k * es, items, n, pos, dry);
        return;
    }
    if (ty_is_struct(&t)) {
        StructDef* sd = t.sname ? find_struct(t.sname) : NULL;
        int k, nm;
        if (!sd) return;
        nm = sd->is_union ? (sd->member_count > 0 ? 1 : 0) : sd->member_count;     /* a union: its first member */
        for (k = 0; k < nm; k++) {
            Ty mt = member_ty(&sd->members[k]);
            init_value(mt, off + sd->members[k].offset, items, n, pos, dry);
        }
        return;
    }
    {
        ASTNode* item = NULL;
        if (*pos < n) {
            item = items[(*pos)++];
            while (item && item->type == AST_INIT_LIST)               /* int x = {1}: braces around a scalar */
                item = item->u.init_list.value_count > 0 ? item->u.init_list.values[0] : NULL;
        }
        init_scalar(&t, off, item, dry);
    }
}

static void init_value(Ty t, int off, ASTNode** items, int n, int* pos, int dry) {
    if (is_aggregate(&t) && *pos < n && items[*pos]->type == AST_INIT_LIST) {
        ASTNode* sub = items[(*pos)++];
        int p2 = 0;
        init_fill(t, off, sub->u.init_list.values, sub->u.init_list.value_count, &p2, dry);
        if (!dry && p2 < sub->u.init_list.value_count) compile_error("Error: excess elements in initializer\n");
        return;
    }
    init_fill(t, off, items, n, pos, dry);
}

/* the element count of an array declared with [] */
static int infer_first_dim(const Ty* t, ASTNode* init) {
    Ty e = ty_deref(*t);
    int count = 0, pos = 0;
    if (!init) return 0;
    if (init->type == AST_STRING_LITERAL) {
        unsigned char b[1];
        if (e.ndim == 0 && e.base == TYPE_CHAR && e.ptr == 0) return unescape_literal(init->u.string_literal, b, 0) + 1;
        return 0;
    }
    if (init->type != AST_INIT_LIST) return 0;
    while (pos < init->u.init_list.value_count) {
        int before = pos;
        init_value(e, 0, init->u.init_list.values, init->u.init_list.value_count, &pos, 1);
        count++;
        if (pos == before) break;
    }
    return count;
}

/* the declared type of a variable; an omitted first array dimension comes from the initializer */
static Ty decl_ty(ASTNode* d) {
    Ty t = ty_make(d->u.var_decl.var_type, d->u.var_decl.modifiers, d->u.var_decl.pointer_level, d->u.var_decl.struct_name);
    int nd = d->u.var_decl.array_dimensions;
    int i;
    if (nd > MAX_DIMS) nd = MAX_DIMS;
    t.ndim = nd;
    for (i = 0; i < nd; i++) t.dims[i] = d->u.var_decl.array_sizes[i];
    if (nd > 0 && t.dims[0] == 0) t.dims[0] = infer_first_dim(&t, d->u.var_decl.init_value);
    return t;
}

static void pop_symbols_to(int mark) {
    int i;
    for (i = mark; i < ctx.symbol_count; i++) {
        free(ctx.symbols[i].name);
        if (ctx.symbols[i].array_sizes) free(ctx.symbols[i].array_sizes);
        if (ctx.symbols[i].struct_name) free(ctx.symbols[i].struct_name);
    }
    ctx.symbol_count = mark;
}

static void codegen_var_decl(ASTNode* node) {
    Symbol* sym;
    Ty t = decl_ty(node);
    ASTNode* init = node->u.var_decl.init_value;
    const char* name = node->u.var_decl.var_name;
    int i;

    if (node->u.var_decl.array_dimensions > MAX_DIMS) {
        compile_error("Error: '%s' has more than %d array dimensions\n", name, MAX_DIMS);
        return;
    }
    if (t.base == TYPE_VOID && t.ptr == 0 && t.ndim == 0) {
        compile_error("Error: variable '%s' declared void\n", name);
        return;
    }
    if (t.base == TYPE_STRUCT && t.ptr == 0 && (!t.sname || !find_struct(t.sname))) {
        compile_error("Error: variable '%s' has an unknown or incomplete struct/union type\n", name);
        return;
    }
    if (t.ndim > 0 && t.dims[0] < 1) {
        compile_error("Error: array '%s' needs a size or an initializer to take it from\n", name);
        return;
    }
    for (i = 1; i < t.ndim; i++) {
        if (t.dims[i] < 1) {
            compile_error("Error: only the first dimension of array '%s' may be omitted\n", name);
            return;
        }
    }
    if (ty_size(&t) >= 32767) {
        compile_error("Error: '%s' is too large for the stack frame\n", name);
        return;
    }
    for (i = ctx.symbol_count - 1; i >= ctx.scope_base; i--) {
        if (strcmp(ctx.symbols[i].name, name) == 0) {
            compile_error("Error: redeclaration of '%s'\n", name);
            return;
        }
    }

    if (t.ndim > 0) {
        add_symbol_multidim(name, node->u.var_decl.var_type, 0, 0, t.dims, t.ndim,
                            node->u.var_decl.pointer_level, node->u.var_decl.struct_name);
    } else {
        add_symbol(name, node->u.var_decl.var_type, 0, 0, 0, 0,
                   node->u.var_decl.pointer_level, node->u.var_decl.struct_name);
    }
    sym = &ctx.symbols[ctx.symbol_count - 1];
    sym->modifiers = node->u.var_decl.modifiers;
    if (!init) return;

    if (init->type == AST_STRING_LITERAL) {
        if (!(t.ndim == 1 && t.base == TYPE_CHAR && t.ptr == 0)) {
            compile_error("Error: a string literal can only initialise a char array\n");
            return;
        }
        init_string(&t, sym->offset, init, 0);
        return;
    }
    if (init->type == AST_INIT_LIST) {
        int pos = 0;
        init_fill(t, sym->offset, init->u.init_list.values, init->u.init_list.value_count, &pos, 0);
        if (pos < init->u.init_list.value_count) compile_error("Error: excess elements in initializer\n");
        return;
    }
    if (t.ndim > 0 || ty_is_struct(&t)) {
        compile_error("Error: '%s' can only be initialised with a brace list\n", name);
        return;
    }
    codegen_expr_to(init, sym_ctype(sym));
    store_sym(sym);
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
        compile_error("Error: break statement outside of loop\n");
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
        compile_error("Error: continue statement outside of loop\n");
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
            compile_error("Error: Unsupported statement type\n");
            break;
    }
}


/* (count_locals_block and the original count_locals live below, rewritten) */



/* Code generation for functions */
/* Words a declaration occupies in the frame (must mirror add_symbol / add_symbol_multidim). */
static int decl_words(ASTNode* d) {
    Ty t = decl_ty(d);
    return (ty_size(&t) + 1) / 2;
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
    rt = type_ctype(node->u.func_decl.return_type, node->u.func_decl.return_modifiers, node->u.func_decl.return_pointer_level);
    ctx.ret_words = rt.words;
    ctx.ret_uns = rt.uns;
    ctx.ret_char = (node->u.func_decl.return_type == TYPE_CHAR && node->u.func_decl.return_pointer_level == 0);
    if (node->u.func_decl.return_type == TYPE_STRUCT && node->u.func_decl.return_pointer_level == 0)
        compile_error("Error: function '%s' returns a struct or union by value, which is not supported\n", node->u.func_decl.func_name);
    ctx.ret_signed = ((node->u.func_decl.return_modifiers & MOD_SIGNED) != 0);

    emit("\n; Function: %s\n", node->u.func_decl.func_name);
    emit("%s:\n", node->u.func_decl.func_name);

    /* Set to 16-bit mode */
    emit("    REP #$30        ; 16-bit A, X, Y\n");

    if (nparams > MAX_PARAMS) {
        compile_error("Error: function '%s' has more than %d parameters\n", node->u.func_decl.func_name, MAX_PARAMS);
        nparams = MAX_PARAMS;
    }

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
        if (param->u.param.param_type == TYPE_STRUCT && param->u.param.pointer_level == 0)
            compile_error("Error: parameter '%s' is a struct or union passed by value, which is not supported\n", param->u.param.param_name);
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

    /* an 8-bit parameter arrives as whatever word the caller pushed (or an assembly caller left): make it exact */
    for (i = 0; i < nparams; i++) {
        ASTNode* param = node->u.func_decl.params[i];
        if (param->u.param.param_type == TYPE_CHAR && param->u.param.pointer_level == 0) {
            Symbol* sym = &ctx.symbols[ctx.symbol_count - nparams + i];
            emit("    LDA %d,S\n", SPO(sym->offset));
            emit_narrow((param->u.param.modifiers & MOD_SIGNED) != 0);
            emit("    STA %d,S\n", SPO(sym->offset));
        }
    }

    /* variables whose address is taken anywhere in the function (a hardware loop must not count one of them) */
    ctx.addr_taken = NULL;
    ctx.addr_taken_count = 0;
    ast_walk(node->u.func_decl.body, collect_addr_taken, NULL);
    ctx.scope_base = ctx.symbol_count;

    /* Generate code for function body */
    codegen_statement(node->u.func_decl.body);

    /* Add default return if needed */
    emit("    ; Epilogue\n");
    emit_drop_bytes(ctx.frame_total);
    emit("    RTS\n");

    /* Clear symbols for next function */
    {
        int k;
        for (k = 0; k < ctx.addr_taken_count; k++) free(ctx.addr_taken[k]);
        free(ctx.addr_taken);
        ctx.addr_taken = NULL;
        ctx.addr_taken_count = 0;
    }
    clear_symbols();
    ctx.current_function = NULL;
}

static int member_align(const Ty* t) {
    return (t->ptr == 0 && (t->base == TYPE_CHAR || t->base == TYPE_BOOL)) ? 1 : 2;
}

/* lay out a struct (members one after another, 16-bit members on even offsets) or a union (all at offset 0); the
   total size is rounded up to a whole word so that arrays of them stay aligned */
static void layout_aggregate(const char* name, ASTNode** members, int count, int is_union) {
    StructDef sd;
    int i, j, offset = 0, maxsize = 0;
    sd.name = zp_strdup(name);
    sd.member_count = count;
    sd.is_union = is_union;
    sd.members = (StructMember*)malloc(sizeof(StructMember) * (count > 0 ? count : 1));
    for (i = 0; i < count; i++) {
        ASTNode* m = members[i];
        Ty mt;
        int size, align, nd;
        sd.members[i].name = zp_strdup(m->u.var_decl.var_name);
        sd.members[i].type = m->u.var_decl.var_type;
        sd.members[i].pointer_level = m->u.var_decl.pointer_level;
        sd.members[i].struct_name = m->u.var_decl.struct_name ? zp_strdup(m->u.var_decl.struct_name) : NULL;
        sd.members[i].modifiers = m->u.var_decl.modifiers;
        nd = m->u.var_decl.array_dimensions;
        if (nd > MAX_DIMS) {
            compile_error("Error: member '%s' has more than %d array dimensions\n", m->u.var_decl.var_name, MAX_DIMS);
            nd = MAX_DIMS;
        }
        sd.members[i].ndim = nd;
        for (j = 0; j < MAX_DIMS; j++) sd.members[i].dims[j] = (j < nd) ? m->u.var_decl.array_sizes[j] : 0;
        for (j = 0; j < nd; j++) {
            if (sd.members[i].dims[j] < 1) {
                compile_error("Error: member '%s' needs an array size\n", m->u.var_decl.var_name);
                sd.members[i].dims[j] = 1;
            }
        }
        mt = member_ty(&sd.members[i]);
        if (mt.base == TYPE_STRUCT && mt.ptr == 0 && (!mt.sname || !find_struct(mt.sname)))
            compile_error("Error: member '%s' has an incomplete struct/union type\n", m->u.var_decl.var_name);
        size = ty_size(&mt);
        align = member_align(&mt);
        if (is_union) {
            sd.members[i].offset = 0;
            if (size > maxsize) maxsize = size;
        } else {
            if (align == 2 && (offset & 1)) offset++;
            sd.members[i].offset = offset;
            offset += size;
        }
    }
    sd.total_size = is_union ? maxsize : offset;
    if (sd.total_size & 1) sd.total_size++;
    add_struct(sd);
    emit("; %s definition: %s (size = %d bytes)\n", is_union ? "Union" : "Struct", sd.name, sd.total_size);
}

static void codegen_struct_decl(ASTNode* node) {
    layout_aggregate(node->u.struct_decl.struct_name, node->u.struct_decl.members, node->u.struct_decl.member_count, 0);
}

static void codegen_union_decl(ASTNode* node) {
    layout_aggregate(node->u.union_decl.union_name, node->u.union_decl.members, node->u.union_decl.member_count, 1);
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

#include "ast.h"
#include <string.h>
#include "zpstrdup.h"
#include <stdarg.h>
#include <stdio.h>

int compile_error_count = 0;

void compile_error(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    compile_error_count++;
}

static ASTNode* ast_create_node(ASTNodeType type) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = type;
    node->data_type = TYPE_VOID;
    node->mods = MOD_NONE;
    return node;
}

ASTNode* ast_create_number(int value) {
    ASTNode* node = ast_create_node(AST_NUMBER);
    node->u.number = value;
    node->data_type = TYPE_INT;
    return node;
}

ASTNode* ast_create_identifier(const char* name) {
    ASTNode* node = ast_create_node(AST_IDENTIFIER);
    node->u.identifier = zp_strdup(name);
    return node;
}

ASTNode* ast_create_string_literal(const char* value) {
    ASTNode* node = ast_create_node(AST_STRING_LITERAL);
    node->u.string_literal = zp_strdup(value);
    node->data_type = TYPE_POINTER;  /* Strings are char* */
    return node;
}

ASTNode* ast_create_binop(BinOpType op, ASTNode* left, ASTNode* right) {
    ASTNode* node = ast_create_node(AST_BINOP);
    node->u.binop.op = op;
    node->u.binop.left = left;
    node->u.binop.right = right;
    return node;
}

ASTNode* ast_create_unop(UnOpType op, ASTNode* operand) {
    ASTNode* node = ast_create_node(AST_UNOP);
    node->u.unop.op = op;
    node->u.unop.operand = operand;
    return node;
}

ASTNode* ast_create_call(const char* name, ASTNode** args, int arg_count) {
    ASTNode* node = ast_create_node(AST_CALL);
    node->u.call.name = zp_strdup(name);
    node->u.call.args = args;
    node->u.call.arg_count = arg_count;
    return node;
}

ASTNode* ast_create_assign(const char* var_name, ASTNode* value) {
    ASTNode* node = ast_create_node(AST_ASSIGN);
    node->u.assign.var_name = zp_strdup(var_name);
    node->u.assign.value = value;
    return node;
}

ASTNode* ast_create_return(ASTNode* value) {
    ASTNode* node = ast_create_node(AST_RETURN);
    node->u.ret.value = value;
    return node;
}

ASTNode* ast_create_if(ASTNode* condition, ASTNode* then_stmt, ASTNode* else_stmt) {
    ASTNode* node = ast_create_node(AST_IF);
    node->u.if_stmt.condition = condition;
    node->u.if_stmt.then_stmt = then_stmt;
    node->u.if_stmt.else_stmt = else_stmt;
    return node;
}

ASTNode* ast_create_while(ASTNode* condition, ASTNode* body) {
    ASTNode* node = ast_create_node(AST_WHILE);
    node->u.while_stmt.condition = condition;
    node->u.while_stmt.body = body;
    return node;
}

ASTNode* ast_create_do_while(ASTNode* condition, ASTNode* body) {
    ASTNode* node = ast_create_node(AST_DO_WHILE);
    node->u.do_while_stmt.condition = condition;
    node->u.do_while_stmt.body = body;
    return node;
}

ASTNode* ast_create_for(ASTNode* init, ASTNode* condition, ASTNode* increment, ASTNode* body) {
    ASTNode* node = ast_create_node(AST_FOR);
    node->u.for_stmt.init = init;
    node->u.for_stmt.condition = condition;
    node->u.for_stmt.increment = increment;
    node->u.for_stmt.body = body;
    return node;
}

ASTNode* ast_create_block(ASTNode** statements, int stmt_count) {
    ASTNode* node = ast_create_node(AST_BLOCK);
    node->u.block.statements = statements;
    node->u.block.stmt_count = stmt_count;
    return node;
}

ASTNode* ast_create_array_subscript(const char* array_name, ASTNode* index) {
    ASTNode* node = ast_create_node(AST_ARRAY_SUBSCRIPT);
    node->u.array_subscript.array_name = zp_strdup(array_name);
    node->u.array_subscript.index = index;
    return node;
}

ASTNode* ast_create_array_assign(const char* array_name, ASTNode* index, ASTNode* value) {
    ASTNode* node = ast_create_node(AST_ARRAY_ASSIGN);
    node->u.array_assign.array_name = zp_strdup(array_name);
    node->u.array_assign.index = index;
    node->u.array_assign.value = value;
    return node;
}

ASTNode* ast_create_addr_of(ASTNode* operand) {
    ASTNode* node = ast_create_node(AST_ADDR_OF);
    node->u.unop.op = OP_ADDR_OF;
    node->u.unop.operand = operand;
    return node;
}

ASTNode* ast_create_deref(ASTNode* operand) {
    ASTNode* node = ast_create_node(AST_DEREF);
    node->u.unop.op = OP_DEREF;
    node->u.unop.operand = operand;
    return node;
}

ASTNode* ast_create_member_access(ASTNode* object, const char* member_name) {
    ASTNode* node = ast_create_node(AST_MEMBER_ACCESS);
    node->u.member_access.object = object;
    node->u.member_access.member_name = zp_strdup(member_name);
    return node;
}

ASTNode* ast_create_ptr_member_access(ASTNode* pointer, const char* member_name) {
    ASTNode* node = ast_create_node(AST_PTR_MEMBER_ACCESS);
    node->u.ptr_member_access.pointer = pointer;
    node->u.ptr_member_access.member_name = zp_strdup(member_name);
    return node;
}

ASTNode* ast_create_member_assign(ASTNode* object, const char* member_name, ASTNode* value) {
    ASTNode* node = ast_create_node(AST_MEMBER_ASSIGN);
    node->u.member_assign.object = object;
    node->u.member_assign.member_name = zp_strdup(member_name);
    node->u.member_assign.value = value;
    return node;
}

ASTNode* ast_create_ptr_member_assign(ASTNode* pointer, const char* member_name, ASTNode* value) {
    ASTNode* node = ast_create_node(AST_PTR_MEMBER_ASSIGN);
    node->u.ptr_member_assign.pointer = pointer;
    node->u.ptr_member_assign.member_name = zp_strdup(member_name);
    node->u.ptr_member_assign.value = value;
    return node;
}

ASTNode* ast_create_struct_decl(const char* name, ASTNode** members, int member_count) {
    ASTNode* node = ast_create_node(AST_STRUCT_DECL);
    node->u.struct_decl.struct_name = zp_strdup(name);
    node->u.struct_decl.members = members;
    node->u.struct_decl.member_count = member_count;
    return node;
}

ASTNode* ast_create_var_decl_list(ASTNode** declarations, int decl_count) {
    ASTNode* node = ast_create_node(AST_VAR_DECL_LIST);
    node->u.var_decl_list.declarations = declarations;
    node->u.var_decl_list.decl_count = decl_count;
    return node;
}

ASTNode* ast_create_enum_decl(const char* name, char** enumerator_names, int* enumerator_values, int enumerator_count) {
    ASTNode* node = ast_create_node(AST_ENUM_DECL);
    node->u.enum_decl.enum_name = zp_strdup(name);
    node->u.enum_decl.enumerator_names = enumerator_names;
    node->u.enum_decl.enumerator_values = enumerator_values;
    node->u.enum_decl.enumerator_count = enumerator_count;
    return node;
}

ASTNode* ast_create_typedef(DataType base_type, const char* type_name, int pointer_level, int* array_sizes, int array_dim_count) {
    ASTNode* node = ast_create_node(AST_TYPEDEF);
    node->u.typedef_decl.base_type = base_type;
    node->u.typedef_decl.type_name = zp_strdup(type_name);
    node->u.typedef_decl.pointer_level = pointer_level;
    node->u.typedef_decl.array_sizes = array_sizes;
    node->u.typedef_decl.array_dim_count = array_dim_count;
    return node;
}

ASTNode* ast_create_pre_inc(const char* var_name) {
    ASTNode* node = ast_create_node(AST_PRE_INC);
    node->u.inc_dec.var_name = zp_strdup(var_name);
    return node;
}

ASTNode* ast_create_post_inc(const char* var_name) {
    ASTNode* node = ast_create_node(AST_POST_INC);
    node->u.inc_dec.var_name = zp_strdup(var_name);
    return node;
}

ASTNode* ast_create_pre_dec(const char* var_name) {
    ASTNode* node = ast_create_node(AST_PRE_DEC);
    node->u.inc_dec.var_name = zp_strdup(var_name);
    return node;
}

ASTNode* ast_create_post_dec(const char* var_name) {
    ASTNode* node = ast_create_node(AST_POST_DEC);
    node->u.inc_dec.var_name = zp_strdup(var_name);
    return node;
}

ASTNode* ast_create_break() {
    ASTNode* node = ast_create_node(AST_BREAK);
    return node;
}

ASTNode* ast_create_continue() {
    ASTNode* node = ast_create_node(AST_CONTINUE);
    return node;
}

ASTNode* ast_create_goto(const char* label) {
    ASTNode* node = ast_create_node(AST_GOTO);
    node->u.goto_stmt.label = zp_strdup(label);
    return node;
}

ASTNode* ast_create_label(const char* label, ASTNode* statement) {
    ASTNode* node = ast_create_node(AST_LABEL);
    node->u.label_stmt.label = zp_strdup(label);
    node->u.label_stmt.statement = statement;
    return node;
}

ASTNode* ast_create_var_decl(DataType type, const char* name, ASTNode* init_value, int is_array, int array_size, int pointer_level, const char* struct_name) {
    ASTNode* node = ast_create_node(AST_VAR_DECL);
    node->u.var_decl.var_type = type;
    node->u.var_decl.var_name = zp_strdup(name);
    node->u.var_decl.init_value = init_value;
    node->u.var_decl.is_array = is_array;
    if (is_array) {
        node->u.var_decl.array_sizes = malloc(sizeof(int));
        node->u.var_decl.array_sizes[0] = array_size;
        node->u.var_decl.array_dimensions = 1;
    } else {
        node->u.var_decl.array_sizes = NULL;
        node->u.var_decl.array_dimensions = 0;
    }
    node->u.var_decl.pointer_level = pointer_level;
    node->u.var_decl.struct_name = struct_name ? zp_strdup(struct_name) : NULL;
    node->u.var_decl.modifiers = MOD_NONE;
    node->u.var_decl.qualifiers = QUAL_NONE;
    node->u.var_decl.storage = STORAGE_NONE;
    return node;
}

ASTNode* ast_create_var_decl_multidim(DataType type, const char* name, ASTNode* init_value, int* array_sizes, int dimensions, int pointer_level, const char* struct_name) {
    ASTNode* node = ast_create_node(AST_VAR_DECL);
    node->u.var_decl.var_type = type;
    node->u.var_decl.var_name = zp_strdup(name);
    node->u.var_decl.init_value = init_value;
    node->u.var_decl.is_array = (dimensions > 0);
    node->u.var_decl.array_sizes = malloc(sizeof(int) * dimensions);
    memcpy(node->u.var_decl.array_sizes, array_sizes, sizeof(int) * dimensions);
    node->u.var_decl.array_dimensions = dimensions;
    node->u.var_decl.pointer_level = pointer_level;
    node->u.var_decl.struct_name = struct_name ? zp_strdup(struct_name) : NULL;
    node->u.var_decl.modifiers = MOD_NONE;
    node->u.var_decl.qualifiers = QUAL_NONE;
    node->u.var_decl.storage = STORAGE_NONE;
    return node;
}

ASTNode* ast_create_func_decl(DataType return_type, const char* name, ASTNode** params, int param_count, ASTNode* body) {
    ASTNode* node = ast_create_node(AST_FUNC_DECL);
    node->u.func_decl.return_type = return_type;
    node->u.func_decl.func_name = zp_strdup(name);
    node->u.func_decl.params = params;
    node->u.func_decl.param_count = param_count;
    node->u.func_decl.body = body;
    node->u.func_decl.return_modifiers = MOD_NONE;
    node->u.func_decl.storage = STORAGE_NONE;
    return node;
}

ASTNode* ast_create_param(DataType type, const char* name, int pointer_level, const char* struct_name) {
    ASTNode* node = ast_create_node(AST_PARAM);
    node->u.param.param_type = type;
    node->u.param.param_name = zp_strdup(name);
    node->u.param.pointer_level = pointer_level;
    node->u.param.struct_name = struct_name ? zp_strdup(struct_name) : NULL;
    node->u.param.modifiers = MOD_NONE;
    node->u.param.qualifiers = QUAL_NONE;
    return node;
}

ASTNode* ast_create_program(ASTNode** decls, int decl_count) {
    ASTNode* node = ast_create_node(AST_PROGRAM);
    node->u.program.decls = decls;
    node->u.program.decl_count = decl_count;
    return node;
}

ASTNode* ast_create_expr_stmt(ASTNode* expr) {
    ASTNode* node = ast_create_node(AST_EXPR_STMT);
    node->u.expr_stmt.expr = expr;
    return node;
}

ASTNode* ast_create_inline_asm(const char* asm_code) {
    ASTNode* node = ast_create_node(AST_INLINE_ASM);
    node->u.inline_asm.asm_code = zp_strdup(asm_code);
    return node;
}

ASTNode* ast_create_sizeof_type(DataType type, const char* type_name, int pointer_level) {
    ASTNode* node = ast_create_node(AST_SIZEOF);
    node->u.sizeof_expr.size_type = type;
    node->u.sizeof_expr.type_name = type_name ? zp_strdup(type_name) : NULL;
    node->u.sizeof_expr.expr = NULL;
    node->u.sizeof_expr.pointer_level = pointer_level;
    node->u.sizeof_expr.is_array = 0;
    node->u.sizeof_expr.array_size = 0;
    node->data_type = TYPE_INT;  /* sizeof returns int */
    return node;
}

ASTNode* ast_create_sizeof_expr(ASTNode* expr) {
    ASTNode* node = ast_create_node(AST_SIZEOF);
    node->u.sizeof_expr.size_type = TYPE_VOID;
    node->u.sizeof_expr.type_name = NULL;
    node->u.sizeof_expr.expr = expr;
    node->u.sizeof_expr.pointer_level = 0;
    node->u.sizeof_expr.is_array = 0;
    node->u.sizeof_expr.array_size = 0;
    node->data_type = TYPE_INT;  /* sizeof returns int */
    return node;
}

ASTNode* ast_create_cast(DataType target_type, const char* type_name, int pointer_level, ASTNode* expr) {
    ASTNode* node = ast_create_node(AST_CAST);
    node->u.cast.target_type = target_type;
    node->u.cast.type_name = type_name ? zp_strdup(type_name) : NULL;
    node->u.cast.pointer_level = pointer_level;
    node->u.cast.expr = expr;
    /* Set the data type of the cast node to the target type */
    if (pointer_level > 0) {
        node->data_type = TYPE_POINTER;
    } else {
        node->data_type = target_type;
    }
    return node;
}

ASTNode* ast_create_comma(ASTNode** expressions, int expr_count) {
    ASTNode* node = ast_create_node(AST_COMMA);
    node->u.comma.expressions = expressions;
    node->u.comma.expr_count = expr_count;
    /* Type of comma expression is type of rightmost expression */
    if (expr_count > 0) {
        node->data_type = expressions[expr_count - 1]->data_type;
    }
    return node;
}

ASTNode* ast_create_ternary(ASTNode* condition, ASTNode* then_expr, ASTNode* else_expr) {
    ASTNode* node = ast_create_node(AST_TERNARY);
    node->u.ternary.condition = condition;
    node->u.ternary.then_expr = then_expr;
    node->u.ternary.else_expr = else_expr;
    return node;
}

ASTNode* ast_create_switch(ASTNode* expr, ASTNode** cases, int case_count) {
    ASTNode* node = ast_create_node(AST_SWITCH);
    node->u.switch_stmt.expr = expr;
    node->u.switch_stmt.cases = cases;
    node->u.switch_stmt.case_count = case_count;
    return node;
}

ASTNode* ast_create_case(int value, ASTNode** statements, int stmt_count) {
    ASTNode* node = ast_create_node(AST_CASE);
    node->u.case_stmt.case_value = value;
    node->u.case_stmt.statements = statements;
    node->u.case_stmt.stmt_count = stmt_count;
    return node;
}

ASTNode* ast_create_default(ASTNode** statements, int stmt_count) {
    ASTNode* node = ast_create_node(AST_DEFAULT);
    node->u.default_stmt.statements = statements;
    node->u.default_stmt.stmt_count = stmt_count;
    return node;
}

ASTNode* ast_create_init_list(ASTNode** values, int value_count) {
    ASTNode* node = ast_create_node(AST_INIT_LIST);
    node->u.init_list.values = values;
    node->u.init_list.value_count = value_count;
    return node;
}

/* New functions with type modifiers and qualifiers support */
ASTNode* ast_create_var_decl_with_quals(DataType type, const char* name, ASTNode* init_value, int is_array, int array_size, int pointer_level, const char* struct_name, TypeModifier mods, TypeQualifier quals, StorageClass storage) {
    ASTNode* node = ast_create_var_decl(type, name, init_value, is_array, array_size, pointer_level, struct_name);
    node->u.var_decl.modifiers = mods;
    node->u.var_decl.qualifiers = quals;
    node->u.var_decl.storage = storage;
    return node;
}

ASTNode* ast_create_var_decl_multidim_with_quals(DataType type, const char* name, ASTNode* init_value, int* array_sizes, int dimensions, int pointer_level, const char* struct_name, TypeModifier mods, TypeQualifier quals, StorageClass storage) {
    ASTNode* node = ast_create_var_decl_multidim(type, name, init_value, array_sizes, dimensions, pointer_level, struct_name);
    node->u.var_decl.modifiers = mods;
    node->u.var_decl.qualifiers = quals;
    node->u.var_decl.storage = storage;
    return node;
}

ASTNode* ast_create_func_decl_with_quals(DataType return_type, const char* name, ASTNode** params, int param_count, ASTNode* body, TypeModifier return_mods, StorageClass storage) {
    ASTNode* node = ast_create_func_decl(return_type, name, params, param_count, body);
    node->u.func_decl.return_modifiers = return_mods;
    node->u.func_decl.storage = storage;
    return node;
}

ASTNode* ast_create_param_with_quals(DataType type, const char* name, int pointer_level, const char* struct_name, TypeModifier mods, TypeQualifier quals) {
    ASTNode* node = ast_create_param(type, name, pointer_level, struct_name);
    node->u.param.modifiers = mods;
    node->u.param.qualifiers = quals;
    return node;
}

ASTNode* ast_create_union_decl(const char* name, ASTNode** members, int member_count) {
    ASTNode* node = ast_create_node(AST_UNION_DECL);
    node->u.union_decl.union_name = zp_strdup(name);
    node->u.union_decl.members = members;
    node->u.union_decl.member_count = member_count;
    return node;
}

/* Pre-order walk over a tree.  visit() returns nonzero to stop the walk; that value is returned. */
int ast_walk(ASTNode* node, int (*visit)(ASTNode*, void*), void* ctx) {
    int i, r;
    if (!node) return 0;
    r = visit(node, ctx);
    if (r) return r;

#define WALK(child) do { r = ast_walk((child), visit, ctx); if (r) return r; } while (0)
    switch (node->type) {
        case AST_BINOP:
            WALK(node->u.binop.left);
            WALK(node->u.binop.right);
            break;
        case AST_UNOP:
        case AST_ADDR_OF:
        case AST_DEREF:
            WALK(node->u.unop.operand);
            break;
        case AST_CALL:
            for (i = 0; i < node->u.call.arg_count; i++) WALK(node->u.call.args[i]);
            break;
        case AST_ASSIGN:
            WALK(node->u.assign.value);
            break;
        case AST_ARRAY_SUBSCRIPT:
            WALK(node->u.array_subscript.index);
            break;
        case AST_ARRAY_ASSIGN:
            WALK(node->u.array_assign.index);
            WALK(node->u.array_assign.value);
            break;
        case AST_RETURN:
            WALK(node->u.ret.value);
            break;
        case AST_IF:
            WALK(node->u.if_stmt.condition);
            WALK(node->u.if_stmt.then_stmt);
            WALK(node->u.if_stmt.else_stmt);
            break;
        case AST_WHILE:
            WALK(node->u.while_stmt.condition);
            WALK(node->u.while_stmt.body);
            break;
        case AST_DO_WHILE:
            WALK(node->u.do_while_stmt.condition);
            WALK(node->u.do_while_stmt.body);
            break;
        case AST_FOR:
            WALK(node->u.for_stmt.init);
            WALK(node->u.for_stmt.condition);
            WALK(node->u.for_stmt.increment);
            WALK(node->u.for_stmt.body);
            break;
        case AST_BLOCK:
            for (i = 0; i < node->u.block.stmt_count; i++) WALK(node->u.block.statements[i]);
            break;
        case AST_VAR_DECL:
            WALK(node->u.var_decl.init_value);
            break;
        case AST_VAR_DECL_LIST:
            for (i = 0; i < node->u.var_decl_list.decl_count; i++) WALK(node->u.var_decl_list.declarations[i]);
            break;
        case AST_MEMBER_ACCESS:
            WALK(node->u.member_access.object);
            break;
        case AST_PTR_MEMBER_ACCESS:
            WALK(node->u.ptr_member_access.pointer);
            break;
        case AST_MEMBER_ASSIGN:
            WALK(node->u.member_assign.object);
            WALK(node->u.member_assign.value);
            break;
        case AST_PTR_MEMBER_ASSIGN:
            WALK(node->u.ptr_member_assign.pointer);
            WALK(node->u.ptr_member_assign.value);
            break;
        case AST_EXPR_STMT:
            WALK(node->u.expr_stmt.expr);
            break;
        case AST_SIZEOF:
            WALK(node->u.sizeof_expr.expr);
            break;
        case AST_CAST:
            WALK(node->u.cast.expr);
            break;
        case AST_COMMA:
            for (i = 0; i < node->u.comma.expr_count; i++) WALK(node->u.comma.expressions[i]);
            break;
        case AST_TERNARY:
            WALK(node->u.ternary.condition);
            WALK(node->u.ternary.then_expr);
            WALK(node->u.ternary.else_expr);
            break;
        case AST_SWITCH:
            WALK(node->u.switch_stmt.expr);
            for (i = 0; i < node->u.switch_stmt.case_count; i++) WALK(node->u.switch_stmt.cases[i]);
            break;
        case AST_CASE:
            for (i = 0; i < node->u.case_stmt.stmt_count; i++) WALK(node->u.case_stmt.statements[i]);
            break;
        case AST_DEFAULT:
            for (i = 0; i < node->u.default_stmt.stmt_count; i++) WALK(node->u.default_stmt.statements[i]);
            break;
        case AST_INIT_LIST:
            for (i = 0; i < node->u.init_list.value_count; i++) WALK(node->u.init_list.values[i]);
            break;
        case AST_LABEL:
            WALK(node->u.label_stmt.statement);
            break;
        default:
            break;
    }
#undef WALK
    return 0;
}

void ast_free(ASTNode* node) {
    int i;
    if (!node) return;

    switch (node->type) {
        case AST_IDENTIFIER:
            free(node->u.identifier);
            break;
        case AST_BINOP:
            ast_free(node->u.binop.left);
            ast_free(node->u.binop.right);
            break;
        case AST_UNOP:
            ast_free(node->u.unop.operand);
            break;
        case AST_CALL:
            free(node->u.call.name);
            for (i = 0; i < node->u.call.arg_count; i++) {
                ast_free(node->u.call.args[i]);
            }
            free(node->u.call.args);
            break;
        case AST_ASSIGN:
            free(node->u.assign.var_name);
            ast_free(node->u.assign.value);
            break;
        case AST_ARRAY_SUBSCRIPT:
            free(node->u.array_subscript.array_name);
            ast_free(node->u.array_subscript.index);
            break;
        case AST_ARRAY_ASSIGN:
            free(node->u.array_assign.array_name);
            ast_free(node->u.array_assign.index);
            ast_free(node->u.array_assign.value);
            break;
        case AST_RETURN:
            ast_free(node->u.ret.value);
            break;
        case AST_IF:
            ast_free(node->u.if_stmt.condition);
            ast_free(node->u.if_stmt.then_stmt);
            ast_free(node->u.if_stmt.else_stmt);
            break;
        case AST_WHILE:
            ast_free(node->u.while_stmt.condition);
            ast_free(node->u.while_stmt.body);
            break;
        case AST_DO_WHILE:
            ast_free(node->u.do_while_stmt.condition);
            ast_free(node->u.do_while_stmt.body);
            break;
        case AST_FOR:
            ast_free(node->u.for_stmt.init);
            ast_free(node->u.for_stmt.condition);
            ast_free(node->u.for_stmt.increment);
            ast_free(node->u.for_stmt.body);
            break;
        case AST_BLOCK:
            for (i = 0; i < node->u.block.stmt_count; i++) {
                ast_free(node->u.block.statements[i]);
            }
            free(node->u.block.statements);
            break;
        case AST_VAR_DECL:
            free(node->u.var_decl.var_name);
            if (node->u.var_decl.struct_name) free(node->u.var_decl.struct_name);
            if (node->u.var_decl.array_sizes) free(node->u.var_decl.array_sizes);
            ast_free(node->u.var_decl.init_value);
            break;
        case AST_FUNC_DECL:
            free(node->u.func_decl.func_name);
            for (i = 0; i < node->u.func_decl.param_count; i++) {
                ast_free(node->u.func_decl.params[i]);
            }
            free(node->u.func_decl.params);
            ast_free(node->u.func_decl.body);
            break;
        case AST_PARAM:
            free(node->u.param.param_name);
            if (node->u.param.struct_name) free(node->u.param.struct_name);
            break;
        case AST_STRUCT_DECL:
            free(node->u.struct_decl.struct_name);
            for (i = 0; i < node->u.struct_decl.member_count; i++) {
                ast_free(node->u.struct_decl.members[i]);
            }
            free(node->u.struct_decl.members);
            break;
        case AST_UNION_DECL:
            free(node->u.union_decl.union_name);
            for (i = 0; i < node->u.union_decl.member_count; i++) {
                ast_free(node->u.union_decl.members[i]);
            }
            free(node->u.union_decl.members);
            break;
        case AST_VAR_DECL_LIST:
            for (i = 0; i < node->u.var_decl_list.decl_count; i++) {
                ast_free(node->u.var_decl_list.declarations[i]);
            }
            free(node->u.var_decl_list.declarations);
            break;
        case AST_ENUM_DECL:
            free(node->u.enum_decl.enum_name);
            for (i = 0; i < node->u.enum_decl.enumerator_count; i++) {
                free(node->u.enum_decl.enumerator_names[i]);
            }
            free(node->u.enum_decl.enumerator_names);
            free(node->u.enum_decl.enumerator_values);
            break;
        case AST_TYPEDEF:
            free(node->u.typedef_decl.type_name);
            if (node->u.typedef_decl.array_sizes) {
                free(node->u.typedef_decl.array_sizes);
            }
            break;
        case AST_ADDR_OF:
        case AST_DEREF:
            ast_free(node->u.unop.operand);
            break;
        case AST_MEMBER_ACCESS:
            ast_free(node->u.member_access.object);
            free(node->u.member_access.member_name);
            break;
        case AST_PTR_MEMBER_ACCESS:
            ast_free(node->u.ptr_member_access.pointer);
            free(node->u.ptr_member_access.member_name);
            break;
        case AST_PRE_INC:
        case AST_POST_INC:
        case AST_PRE_DEC:
        case AST_POST_DEC:
            free(node->u.inc_dec.var_name);
            break;
        case AST_PROGRAM:
            for (i = 0; i < node->u.program.decl_count; i++) {
                ast_free(node->u.program.decls[i]);
            }
            free(node->u.program.decls);
            break;
        case AST_EXPR_STMT:
            ast_free(node->u.expr_stmt.expr);
            break;
        case AST_STRING_LITERAL:
            free(node->u.string_literal);
            break;
        case AST_SIZEOF:
            if (node->u.sizeof_expr.type_name) free(node->u.sizeof_expr.type_name);
            if (node->u.sizeof_expr.expr) ast_free(node->u.sizeof_expr.expr);
            break;
        case AST_CAST:
            if (node->u.cast.type_name) free(node->u.cast.type_name);
            ast_free(node->u.cast.expr);
            break;
        case AST_COMMA:
            for (i = 0; i < node->u.comma.expr_count; i++) {
                ast_free(node->u.comma.expressions[i]);
            }
            free(node->u.comma.expressions);
            break;
        case AST_TERNARY:
            ast_free(node->u.ternary.condition);
            ast_free(node->u.ternary.then_expr);
            ast_free(node->u.ternary.else_expr);
            break;
        case AST_SWITCH:
            ast_free(node->u.switch_stmt.expr);
            for (i = 0; i < node->u.switch_stmt.case_count; i++) {
                ast_free(node->u.switch_stmt.cases[i]);
            }
            free(node->u.switch_stmt.cases);
            break;
        case AST_CASE:
            for (i = 0; i < node->u.case_stmt.stmt_count; i++) {
                ast_free(node->u.case_stmt.statements[i]);
            }
            free(node->u.case_stmt.statements);
            break;
        case AST_DEFAULT:
            for (i = 0; i < node->u.default_stmt.stmt_count; i++) {
                ast_free(node->u.default_stmt.statements[i]);
            }
            free(node->u.default_stmt.statements);
            break;
        case AST_INIT_LIST:
            for (i = 0; i < node->u.init_list.value_count; i++) {
                ast_free(node->u.init_list.values[i]);
            }
            free(node->u.init_list.values);
            break;
        case AST_GOTO:
            free(node->u.goto_stmt.label);
            break;
        case AST_LABEL:
            free(node->u.label_stmt.label);
            ast_free(node->u.label_stmt.statement);
            break;
        case AST_INLINE_ASM:
            free(node->u.inline_asm.asm_code);
            break;
        default:
            break;
    }

    free(node);
}

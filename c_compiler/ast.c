#include "ast.h"
#include <string.h>
#include <stdio.h>

static ASTNode* ast_create_node(ASTNodeType type) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = type;
    node->data_type = TYPE_VOID;
    return node;
}

ASTNode* ast_create_number(int value) {
    ASTNode* node = ast_create_node(AST_NUMBER);
    node->number = value;
    node->data_type = TYPE_INT;
    return node;
}

ASTNode* ast_create_identifier(const char* name) {
    ASTNode* node = ast_create_node(AST_IDENTIFIER);
    node->identifier = strdup(name);
    return node;
}

ASTNode* ast_create_binop(BinOpType op, ASTNode* left, ASTNode* right) {
    ASTNode* node = ast_create_node(AST_BINOP);
    node->binop.op = op;
    node->binop.left = left;
    node->binop.right = right;
    return node;
}

ASTNode* ast_create_unop(UnOpType op, ASTNode* operand) {
    ASTNode* node = ast_create_node(AST_UNOP);
    node->unop.op = op;
    node->unop.operand = operand;
    return node;
}

ASTNode* ast_create_call(const char* name, ASTNode** args, int arg_count) {
    ASTNode* node = ast_create_node(AST_CALL);
    node->call.name = strdup(name);
    node->call.args = args;
    node->call.arg_count = arg_count;
    return node;
}

ASTNode* ast_create_assign(const char* var_name, ASTNode* value) {
    ASTNode* node = ast_create_node(AST_ASSIGN);
    node->assign.var_name = strdup(var_name);
    node->assign.value = value;
    return node;
}

ASTNode* ast_create_return(ASTNode* value) {
    ASTNode* node = ast_create_node(AST_RETURN);
    node->ret.value = value;
    return node;
}

ASTNode* ast_create_if(ASTNode* condition, ASTNode* then_stmt, ASTNode* else_stmt) {
    ASTNode* node = ast_create_node(AST_IF);
    node->if_stmt.condition = condition;
    node->if_stmt.then_stmt = then_stmt;
    node->if_stmt.else_stmt = else_stmt;
    return node;
}

ASTNode* ast_create_while(ASTNode* condition, ASTNode* body) {
    ASTNode* node = ast_create_node(AST_WHILE);
    node->while_stmt.condition = condition;
    node->while_stmt.body = body;
    return node;
}

ASTNode* ast_create_for(ASTNode* init, ASTNode* condition, ASTNode* increment, ASTNode* body) {
    ASTNode* node = ast_create_node(AST_FOR);
    node->for_stmt.init = init;
    node->for_stmt.condition = condition;
    node->for_stmt.increment = increment;
    node->for_stmt.body = body;
    return node;
}

ASTNode* ast_create_block(ASTNode** statements, int stmt_count) {
    ASTNode* node = ast_create_node(AST_BLOCK);
    node->block.statements = statements;
    node->block.stmt_count = stmt_count;
    return node;
}

ASTNode* ast_create_array_subscript(const char* array_name, ASTNode* index) {
    ASTNode* node = ast_create_node(AST_ARRAY_SUBSCRIPT);
    node->array_subscript.array_name = strdup(array_name);
    node->array_subscript.index = index;
    return node;
}

ASTNode* ast_create_array_assign(const char* array_name, ASTNode* index, ASTNode* value) {
    ASTNode* node = ast_create_node(AST_ARRAY_ASSIGN);
    node->array_assign.array_name = strdup(array_name);
    node->array_assign.index = index;
    node->array_assign.value = value;
    return node;
}

ASTNode* ast_create_addr_of(ASTNode* operand) {
    ASTNode* node = ast_create_node(AST_ADDR_OF);
    node->unop.op = OP_ADDR_OF;
    node->unop.operand = operand;
    return node;
}

ASTNode* ast_create_deref(ASTNode* operand) {
    ASTNode* node = ast_create_node(AST_DEREF);
    node->unop.op = OP_DEREF;
    node->unop.operand = operand;
    return node;
}

ASTNode* ast_create_member_access(ASTNode* object, const char* member_name) {
    ASTNode* node = ast_create_node(AST_MEMBER_ACCESS);
    node->member_access.object = object;
    node->member_access.member_name = strdup(member_name);
    return node;
}

ASTNode* ast_create_ptr_member_access(ASTNode* pointer, const char* member_name) {
    ASTNode* node = ast_create_node(AST_PTR_MEMBER_ACCESS);
    node->ptr_member_access.pointer = pointer;
    node->ptr_member_access.member_name = strdup(member_name);
    return node;
}

ASTNode* ast_create_member_assign(ASTNode* object, const char* member_name, ASTNode* value) {
    ASTNode* node = ast_create_node(AST_MEMBER_ASSIGN);
    node->member_assign.object = object;
    node->member_assign.member_name = strdup(member_name);
    node->member_assign.value = value;
    return node;
}

ASTNode* ast_create_ptr_member_assign(ASTNode* pointer, const char* member_name, ASTNode* value) {
    ASTNode* node = ast_create_node(AST_PTR_MEMBER_ASSIGN);
    node->ptr_member_assign.pointer = pointer;
    node->ptr_member_assign.member_name = strdup(member_name);
    node->ptr_member_assign.value = value;
    return node;
}

ASTNode* ast_create_struct_decl(const char* name, ASTNode** members, int member_count) {
    ASTNode* node = ast_create_node(AST_STRUCT_DECL);
    node->struct_decl.struct_name = strdup(name);
    node->struct_decl.members = members;
    node->struct_decl.member_count = member_count;
    return node;
}

ASTNode* ast_create_pre_inc(const char* var_name) {
    ASTNode* node = ast_create_node(AST_PRE_INC);
    node->inc_dec.var_name = strdup(var_name);
    return node;
}

ASTNode* ast_create_post_inc(const char* var_name) {
    ASTNode* node = ast_create_node(AST_POST_INC);
    node->inc_dec.var_name = strdup(var_name);
    return node;
}

ASTNode* ast_create_pre_dec(const char* var_name) {
    ASTNode* node = ast_create_node(AST_PRE_DEC);
    node->inc_dec.var_name = strdup(var_name);
    return node;
}

ASTNode* ast_create_post_dec(const char* var_name) {
    ASTNode* node = ast_create_node(AST_POST_DEC);
    node->inc_dec.var_name = strdup(var_name);
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

ASTNode* ast_create_var_decl(DataType type, const char* name, ASTNode* init_value, int is_array, int array_size, int pointer_level, const char* struct_name) {
    ASTNode* node = ast_create_node(AST_VAR_DECL);
    node->var_decl.var_type = type;
    node->var_decl.var_name = strdup(name);
    node->var_decl.init_value = init_value;
    node->var_decl.is_array = is_array;
    node->var_decl.array_size = array_size;
    node->var_decl.pointer_level = pointer_level;
    node->var_decl.struct_name = struct_name ? strdup(struct_name) : NULL;
    return node;
}

ASTNode* ast_create_func_decl(DataType return_type, const char* name, ASTNode** params, int param_count, ASTNode* body) {
    ASTNode* node = ast_create_node(AST_FUNC_DECL);
    node->func_decl.return_type = return_type;
    node->func_decl.func_name = strdup(name);
    node->func_decl.params = params;
    node->func_decl.param_count = param_count;
    node->func_decl.body = body;
    return node;
}

ASTNode* ast_create_param(DataType type, const char* name, int pointer_level, const char* struct_name) {
    ASTNode* node = ast_create_node(AST_PARAM);
    node->param.param_type = type;
    node->param.param_name = strdup(name);
    node->param.pointer_level = pointer_level;
    node->param.struct_name = struct_name ? strdup(struct_name) : NULL;
    return node;
}

ASTNode* ast_create_program(ASTNode** decls, int decl_count) {
    ASTNode* node = ast_create_node(AST_PROGRAM);
    node->program.decls = decls;
    node->program.decl_count = decl_count;
    return node;
}

ASTNode* ast_create_expr_stmt(ASTNode* expr) {
    ASTNode* node = ast_create_node(AST_EXPR_STMT);
    node->expr_stmt.expr = expr;
    return node;
}

void ast_free(ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case AST_IDENTIFIER:
            free(node->identifier);
            break;
        case AST_BINOP:
            ast_free(node->binop.left);
            ast_free(node->binop.right);
            break;
        case AST_UNOP:
            ast_free(node->unop.operand);
            break;
        case AST_CALL:
            free(node->call.name);
            for (int i = 0; i < node->call.arg_count; i++) {
                ast_free(node->call.args[i]);
            }
            free(node->call.args);
            break;
        case AST_ASSIGN:
            free(node->assign.var_name);
            ast_free(node->assign.value);
            break;
        case AST_ARRAY_SUBSCRIPT:
            free(node->array_subscript.array_name);
            ast_free(node->array_subscript.index);
            break;
        case AST_ARRAY_ASSIGN:
            free(node->array_assign.array_name);
            ast_free(node->array_assign.index);
            ast_free(node->array_assign.value);
            break;
        case AST_RETURN:
            ast_free(node->ret.value);
            break;
        case AST_IF:
            ast_free(node->if_stmt.condition);
            ast_free(node->if_stmt.then_stmt);
            ast_free(node->if_stmt.else_stmt);
            break;
        case AST_WHILE:
            ast_free(node->while_stmt.condition);
            ast_free(node->while_stmt.body);
            break;
        case AST_FOR:
            ast_free(node->for_stmt.init);
            ast_free(node->for_stmt.condition);
            ast_free(node->for_stmt.increment);
            ast_free(node->for_stmt.body);
            break;
        case AST_BLOCK:
            for (int i = 0; i < node->block.stmt_count; i++) {
                ast_free(node->block.statements[i]);
            }
            free(node->block.statements);
            break;
        case AST_VAR_DECL:
            free(node->var_decl.var_name);
            if (node->var_decl.struct_name) free(node->var_decl.struct_name);
            ast_free(node->var_decl.init_value);
            break;
        case AST_FUNC_DECL:
            free(node->func_decl.func_name);
            for (int i = 0; i < node->func_decl.param_count; i++) {
                ast_free(node->func_decl.params[i]);
            }
            free(node->func_decl.params);
            ast_free(node->func_decl.body);
            break;
        case AST_PARAM:
            free(node->param.param_name);
            if (node->param.struct_name) free(node->param.struct_name);
            break;
        case AST_STRUCT_DECL:
            free(node->struct_decl.struct_name);
            for (int i = 0; i < node->struct_decl.member_count; i++) {
                ast_free(node->struct_decl.members[i]);
            }
            free(node->struct_decl.members);
            break;
        case AST_ADDR_OF:
        case AST_DEREF:
            ast_free(node->unop.operand);
            break;
        case AST_MEMBER_ACCESS:
            ast_free(node->member_access.object);
            free(node->member_access.member_name);
            break;
        case AST_PTR_MEMBER_ACCESS:
            ast_free(node->ptr_member_access.pointer);
            free(node->ptr_member_access.member_name);
            break;
        case AST_PRE_INC:
        case AST_POST_INC:
        case AST_PRE_DEC:
        case AST_POST_DEC:
            free(node->inc_dec.var_name);
            break;
        case AST_PROGRAM:
            for (int i = 0; i < node->program.decl_count; i++) {
                ast_free(node->program.decls[i]);
            }
            free(node->program.decls);
            break;
        case AST_EXPR_STMT:
            ast_free(node->expr_stmt.expr);
            break;
        default:
            break;
    }

    free(node);
}

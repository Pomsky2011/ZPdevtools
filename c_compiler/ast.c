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

ASTNode* ast_create_var_decl(DataType type, const char* name, ASTNode* init_value, int is_array, int array_size) {
    ASTNode* node = ast_create_node(AST_VAR_DECL);
    node->var_decl.var_type = type;
    node->var_decl.var_name = strdup(name);
    node->var_decl.init_value = init_value;
    node->var_decl.is_array = is_array;
    node->var_decl.array_size = array_size;
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

ASTNode* ast_create_param(DataType type, const char* name) {
    ASTNode* node = ast_create_node(AST_PARAM);
    node->param.param_type = type;
    node->param.param_name = strdup(name);
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

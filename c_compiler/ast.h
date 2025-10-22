#ifndef AST_H
#define AST_H

#include <stdlib.h>

// AST node types
typedef enum {
    // Expressions
    AST_NUMBER,
    AST_IDENTIFIER,
    AST_BINOP,
    AST_UNOP,
    AST_CALL,
    AST_ASSIGN,
    AST_ARRAY_SUBSCRIPT,
    AST_ARRAY_ASSIGN,

    // Statements
    AST_EXPR_STMT,
    AST_RETURN,
    AST_IF,
    AST_WHILE,
    AST_FOR,
    AST_BLOCK,

    // Declarations
    AST_VAR_DECL,
    AST_FUNC_DECL,
    AST_PARAM,

    // Program
    AST_PROGRAM
} ASTNodeType;

// Data types
typedef enum {
    TYPE_VOID,
    TYPE_INT,
    TYPE_CHAR,
    TYPE_POINTER
} DataType;

// Binary operators
typedef enum {
    OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD,
    OP_EQ, OP_NE, OP_LT, OP_GT, OP_LE, OP_GE,
    OP_AND, OP_OR,
    OP_BIT_AND, OP_BIT_OR, OP_BIT_XOR,
    OP_SHL, OP_SHR
} BinOpType;

// Unary operators
typedef enum {
    OP_NEG, OP_NOT, OP_BIT_NOT
} UnOpType;

// Forward declaration
struct ASTNode;

// AST node structure
typedef struct ASTNode {
    ASTNodeType type;
    DataType data_type;

    union {
        // Literals
        int number;
        char* identifier;

        // Binary operation
        struct {
            BinOpType op;
            struct ASTNode* left;
            struct ASTNode* right;
        } binop;

        // Unary operation
        struct {
            UnOpType op;
            struct ASTNode* operand;
        } unop;

        // Function call
        struct {
            char* name;
            struct ASTNode** args;
            int arg_count;
        } call;

        // Assignment
        struct {
            char* var_name;
            struct ASTNode* value;
        } assign;

        // Array subscript
        struct {
            char* array_name;
            struct ASTNode* index;
        } array_subscript;

        // Array assignment
        struct {
            char* array_name;
            struct ASTNode* index;
            struct ASTNode* value;
        } array_assign;

        // Return statement
        struct {
            struct ASTNode* value;
        } ret;

        // If statement
        struct {
            struct ASTNode* condition;
            struct ASTNode* then_stmt;
            struct ASTNode* else_stmt;
        } if_stmt;

        // While statement
        struct {
            struct ASTNode* condition;
            struct ASTNode* body;
        } while_stmt;

        // For statement
        struct {
            struct ASTNode* init;
            struct ASTNode* condition;
            struct ASTNode* increment;
            struct ASTNode* body;
        } for_stmt;

        // Block statement
        struct {
            struct ASTNode** statements;
            int stmt_count;
        } block;

        // Variable declaration
        struct {
            DataType var_type;
            char* var_name;
            struct ASTNode* init_value;
            int is_array;
            int array_size;
        } var_decl;

        // Function declaration
        struct {
            DataType return_type;
            char* func_name;
            struct ASTNode** params;
            int param_count;
            struct ASTNode* body;
        } func_decl;

        // Parameter
        struct {
            DataType param_type;
            char* param_name;
        } param;

        // Program (list of declarations)
        struct {
            struct ASTNode** decls;
            int decl_count;
        } program;

        // Expression statement
        struct {
            struct ASTNode* expr;
        } expr_stmt;
    };
} ASTNode;

// AST creation functions
ASTNode* ast_create_number(int value);
ASTNode* ast_create_identifier(const char* name);
ASTNode* ast_create_binop(BinOpType op, ASTNode* left, ASTNode* right);
ASTNode* ast_create_unop(UnOpType op, ASTNode* operand);
ASTNode* ast_create_call(const char* name, ASTNode** args, int arg_count);
ASTNode* ast_create_assign(const char* var_name, ASTNode* value);
ASTNode* ast_create_array_subscript(const char* array_name, ASTNode* index);
ASTNode* ast_create_array_assign(const char* array_name, ASTNode* index, ASTNode* value);
ASTNode* ast_create_return(ASTNode* value);
ASTNode* ast_create_if(ASTNode* condition, ASTNode* then_stmt, ASTNode* else_stmt);
ASTNode* ast_create_while(ASTNode* condition, ASTNode* body);
ASTNode* ast_create_for(ASTNode* init, ASTNode* condition, ASTNode* increment, ASTNode* body);
ASTNode* ast_create_block(ASTNode** statements, int stmt_count);
ASTNode* ast_create_var_decl(DataType type, const char* name, ASTNode* init_value, int is_array, int array_size);
ASTNode* ast_create_func_decl(DataType return_type, const char* name, ASTNode** params, int param_count, ASTNode* body);
ASTNode* ast_create_param(DataType type, const char* name);
ASTNode* ast_create_program(ASTNode** decls, int decl_count);
ASTNode* ast_create_expr_stmt(ASTNode* expr);

// Free AST
void ast_free(ASTNode* node);

#endif

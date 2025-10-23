#ifndef AST_H
#define AST_H

#include <stdlib.h>

// AST node types
typedef enum {
    // Expressions
    AST_NUMBER,
    AST_IDENTIFIER,
    AST_STRING_LITERAL,   // String literal ("hello")
    AST_BINOP,
    AST_UNOP,
    AST_CALL,
    AST_ASSIGN,
    AST_ARRAY_SUBSCRIPT,
    AST_ARRAY_ASSIGN,
    AST_ADDR_OF,          // Address-of operator (&)
    AST_DEREF,            // Dereference operator (*)
    AST_SIZEOF,           // sizeof operator
    AST_CAST,             // Cast operator ((type)expr)
    AST_COMMA,            // Comma operator (expr1, expr2)
    AST_TERNARY,          // Ternary operator (cond ? then : else)
    AST_MEMBER_ACCESS,    // Struct member access (.)
    AST_PTR_MEMBER_ACCESS, // Pointer member access (->)
    AST_MEMBER_ASSIGN,    // Struct member assignment (struct.member = value)
    AST_PTR_MEMBER_ASSIGN, // Pointer member assignment (ptr->member = value)
    AST_PRE_INC,          // Pre-increment (++x)
    AST_POST_INC,         // Post-increment (x++)
    AST_PRE_DEC,          // Pre-decrement (--x)
    AST_POST_DEC,         // Post-decrement (x--)

    // Statements
    AST_EXPR_STMT,
    AST_RETURN,
    AST_IF,
    AST_WHILE,
    AST_DO_WHILE,
    AST_FOR,
    AST_BLOCK,
    AST_BREAK,
    AST_CONTINUE,
    AST_GOTO,             // goto statement
    AST_LABEL,            // Label declaration
    AST_SWITCH,           // Switch statement
    AST_CASE,             // Case label
    AST_DEFAULT,          // Default label
    AST_INIT_LIST,        // Initialization list {1, 2, 3}

    // Declarations
    AST_VAR_DECL,
    AST_VAR_DECL_LIST,    // Multiple variable declarations (int a, b, c;)
    AST_FUNC_DECL,
    AST_PARAM,
    AST_STRUCT_DECL,      // Struct definition
    AST_ENUM_DECL,        // Enum definition
    AST_TYPEDEF,          // Typedef declaration

    // Program
    AST_PROGRAM
} ASTNodeType;

// Forward declaration for struct definition
struct StructDef;

// Data types
typedef enum {
    TYPE_VOID,
    TYPE_INT,
    TYPE_CHAR,
    TYPE_POINTER,
    TYPE_STRUCT
} DataType;

// Type information (includes base type, pointer levels, and struct name)
typedef struct TypeInfo {
    DataType base_type;
    int pointer_level;        // 0 = not a pointer, 1 = *, 2 = **, etc.
    char* struct_name;        // NULL for non-struct types
} TypeInfo;

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
    OP_NEG, OP_NOT, OP_BIT_NOT, OP_ADDR_OF, OP_DEREF
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
        char* string_literal;

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

        // Do-while statement
        struct {
            struct ASTNode* condition;
            struct ASTNode* body;
        } do_while_stmt;

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
            int* array_sizes;        // Array of sizes for each dimension
            int array_dimensions;    // Number of dimensions (0 = not array, 1 = arr[N], 2 = arr[N][M])
            int pointer_level;       // 0 = not a pointer, 1 = *, 2 = **, etc.
            char* struct_name;       // For struct types
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
            int pointer_level;
            char* struct_name;
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

        // Struct declaration
        struct {
            char* struct_name;
            struct ASTNode** members;  // Array of AST_VAR_DECL nodes
            int member_count;
        } struct_decl;

        // Variable declaration list (int a, b, c;)
        struct {
            struct ASTNode** declarations;  // Array of AST_VAR_DECL nodes
            int decl_count;
        } var_decl_list;

        // Enum declaration
        struct {
            char* enum_name;
            char** enumerator_names;   // Array of enumerator names
            int* enumerator_values;    // Array of enumerator values
            int enumerator_count;
        } enum_decl;

        // Typedef declaration
        struct {
            DataType base_type;
            char* type_name;          // The new type alias
            int pointer_level;        // Number of pointer indirections
            int* array_sizes;         // Array dimensions (NULL if not array)
            int array_dim_count;      // Number of array dimensions
        } typedef_decl;

        // Member access (struct.member)
        struct {
            struct ASTNode* object;    // Can be identifier or another member access
            char* member_name;
        } member_access;

        // Pointer member access (ptr->member)
        struct {
            struct ASTNode* pointer;
            char* member_name;
        } ptr_member_access;

        // Member assignment (struct.member = value)
        struct {
            struct ASTNode* object;
            char* member_name;
            struct ASTNode* value;
        } member_assign;

        // Pointer member assignment (ptr->member = value)
        struct {
            struct ASTNode* pointer;
            char* member_name;
            struct ASTNode* value;
        } ptr_member_assign;

        // Increment/decrement (++x, x++, --x, x--)
        struct {
            char* var_name;
        } inc_dec;

        // Sizeof expression
        struct {
            DataType size_type;
            char* type_name;     // For sizeof(type)
            struct ASTNode* expr; // For sizeof(expr)
            int pointer_level;
            int is_array;
            int array_size;
        } sizeof_expr;

        // Cast expression ((type)expr)
        struct {
            DataType target_type;
            char* type_name;     // For struct casts
            int pointer_level;
            struct ASTNode* expr;
        } cast;

        // Comma expression (expr1, expr2, ...)
        struct {
            struct ASTNode** expressions;
            int expr_count;
        } comma;

        // Ternary expression (cond ? then : else)
        struct {
            struct ASTNode* condition;
            struct ASTNode* then_expr;
            struct ASTNode* else_expr;
        } ternary;

        // Switch statement
        struct {
            struct ASTNode* expr;
            struct ASTNode** cases;  // Array of AST_CASE and AST_DEFAULT nodes
            int case_count;
        } switch_stmt;

        // Case statement
        struct {
            int case_value;      // Value for case label
            struct ASTNode** statements;  // Statements in this case
            int stmt_count;
        } case_stmt;

        // Default statement
        struct {
            struct ASTNode** statements;  // Statements in default case
            int stmt_count;
        } default_stmt;

        // Initialization list {1, 2, 3}
        struct {
            struct ASTNode** values;  // Array of expression nodes
            int value_count;
        } init_list;

        // Goto statement
        struct {
            char* label;
        } goto_stmt;

        // Label statement
        struct {
            char* label;
            struct ASTNode* statement;  // Statement after label
        } label_stmt;
    };
} ASTNode;

// AST creation functions
ASTNode* ast_create_number(int value);
ASTNode* ast_create_identifier(const char* name);
ASTNode* ast_create_string_literal(const char* value);
ASTNode* ast_create_binop(BinOpType op, ASTNode* left, ASTNode* right);
ASTNode* ast_create_unop(UnOpType op, ASTNode* operand);
ASTNode* ast_create_call(const char* name, ASTNode** args, int arg_count);
ASTNode* ast_create_assign(const char* var_name, ASTNode* value);
ASTNode* ast_create_array_subscript(const char* array_name, ASTNode* index);
ASTNode* ast_create_array_assign(const char* array_name, ASTNode* index, ASTNode* value);
ASTNode* ast_create_return(ASTNode* value);
ASTNode* ast_create_if(ASTNode* condition, ASTNode* then_stmt, ASTNode* else_stmt);
ASTNode* ast_create_while(ASTNode* condition, ASTNode* body);
ASTNode* ast_create_do_while(ASTNode* condition, ASTNode* body);
ASTNode* ast_create_for(ASTNode* init, ASTNode* condition, ASTNode* increment, ASTNode* body);
ASTNode* ast_create_block(ASTNode** statements, int stmt_count);
ASTNode* ast_create_var_decl(DataType type, const char* name, ASTNode* init_value, int is_array, int array_size, int pointer_level, const char* struct_name);
ASTNode* ast_create_var_decl_multidim(DataType type, const char* name, ASTNode* init_value, int* array_sizes, int dimensions, int pointer_level, const char* struct_name);
ASTNode* ast_create_func_decl(DataType return_type, const char* name, ASTNode** params, int param_count, ASTNode* body);
ASTNode* ast_create_param(DataType type, const char* name, int pointer_level, const char* struct_name);
ASTNode* ast_create_struct_decl(const char* name, ASTNode** members, int member_count);
ASTNode* ast_create_var_decl_list(ASTNode** declarations, int decl_count);
ASTNode* ast_create_enum_decl(const char* name, char** enumerator_names, int* enumerator_values, int enumerator_count);
ASTNode* ast_create_typedef(DataType base_type, const char* type_name, int pointer_level, int* array_sizes, int array_dim_count);
ASTNode* ast_create_member_access(ASTNode* object, const char* member_name);
ASTNode* ast_create_ptr_member_access(ASTNode* pointer, const char* member_name);
ASTNode* ast_create_member_assign(ASTNode* object, const char* member_name, ASTNode* value);
ASTNode* ast_create_ptr_member_assign(ASTNode* pointer, const char* member_name, ASTNode* value);
ASTNode* ast_create_addr_of(ASTNode* operand);
ASTNode* ast_create_deref(ASTNode* operand);
ASTNode* ast_create_sizeof_type(DataType type, const char* type_name, int pointer_level);
ASTNode* ast_create_sizeof_expr(ASTNode* expr);
ASTNode* ast_create_cast(DataType target_type, const char* type_name, int pointer_level, ASTNode* expr);
ASTNode* ast_create_comma(ASTNode** expressions, int expr_count);
ASTNode* ast_create_ternary(ASTNode* condition, ASTNode* then_expr, ASTNode* else_expr);
ASTNode* ast_create_switch(ASTNode* expr, ASTNode** cases, int case_count);
ASTNode* ast_create_case(int value, ASTNode** statements, int stmt_count);
ASTNode* ast_create_default(ASTNode** statements, int stmt_count);
ASTNode* ast_create_init_list(ASTNode** values, int value_count);
ASTNode* ast_create_pre_inc(const char* var_name);
ASTNode* ast_create_post_inc(const char* var_name);
ASTNode* ast_create_pre_dec(const char* var_name);
ASTNode* ast_create_post_dec(const char* var_name);
ASTNode* ast_create_break();
ASTNode* ast_create_continue();
ASTNode* ast_create_goto(const char* label);
ASTNode* ast_create_label(const char* label, ASTNode* statement);
ASTNode* ast_create_program(ASTNode** decls, int decl_count);
ASTNode* ast_create_expr_stmt(ASTNode* expr);

// Free AST
void ast_free(ASTNode* node);

#endif

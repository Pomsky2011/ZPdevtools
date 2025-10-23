%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

extern int yylex();
extern int yyparse();
extern FILE* yyin;
extern int line_num;
extern int col_num;

void yyerror(const char* s);

ASTNode* ast_root = NULL;

// Global variables for passing type info to declarators
static DataType current_decl_type;
static char* current_decl_struct_name;

// Global variables for enum enumerators
static char** enum_names_temp;
static int* enum_values_temp;
static int enum_count_temp;
%}

%union {
    int number;
    char* string;
    ASTNode* node;
    DataType dtype;
    BinOpType binop;
    UnOpType unop;
    ASTNode** node_list;
    struct {
        ASTNode** items;
        int count;
    } list;
}

%token <number> NUMBER CHAR_LITERAL
%token <string> IDENTIFIER STRING_LITERAL
%token INT CHAR VOID STRUCT ENUM TYPEDEF
%token IF ELSE WHILE DO FOR RETURN BREAK CONTINUE
%token SWITCH CASE DEFAULT GOTO SIZEOF
%token EQ NE LE GE AND OR SHL SHR ARROW INC DEC
%token ADD_ASSIGN SUB_ASSIGN MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN
%token AND_ASSIGN OR_ASSIGN XOR_ASSIGN

%type <node> program declaration function_declaration statement struct_declaration enum_declaration typedef_declaration
%type <node> expression primary_expression postfix_expression
%type <node> unary_expression multiplicative_expression additive_expression
%type <node> shift_expression relational_expression equality_expression
%type <node> and_expression xor_expression or_expression
%type <node> logical_and_expression logical_or_expression
%type <node> conditional_expression assignment_expression
%type <node> compound_statement statement_list
%type <node> expression_statement selection_statement iteration_statement
%type <node> jump_statement variable_declaration switch_statement
%type <node> case_statement default_statement
%type <dtype> type_specifier
%type <list> declaration_list parameter_list argument_list struct_member_list
%type <list> case_list statement_list_in_case declarator_list enumerator_list initializer_list
%type <string> struct_or_union_specifier
%type <number> pointer enum_constant
%type <list> array_sizes
%type <node> declarator

%left ','
%right '=' ADD_ASSIGN SUB_ASSIGN MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN AND_ASSIGN OR_ASSIGN XOR_ASSIGN
%right '?' ':'
%left OR
%left AND
%left '|'
%left '^'
%left '&'
%left EQ NE
%left '<' '>' LE GE
%left SHL SHR
%left '+' '-'
%left '*' '/' '%'
%right '!' '~' UMINUS UDEREF SIZEOF
%left '.' ARROW '[' ']'

%%

program:
    declaration_list {
        ast_root = ast_create_program($1.items, $1.count);
    }
    ;

declaration_list:
    declaration {
        $$.items = malloc(sizeof(ASTNode*));
        $$.items[0] = $1;
        $$.count = 1;
    }
    | declaration_list declaration {
        $$.items = realloc($1.items, sizeof(ASTNode*) * ($1.count + 1));
        $$.items[$1.count] = $2;
        $$.count = $1.count + 1;
    }
    ;

declaration:
    function_declaration { $$ = $1; }
    | variable_declaration { $$ = $1; }
    | struct_declaration { $$ = $1; }
    | enum_declaration { $$ = $1; }
    | typedef_declaration { $$ = $1; }
    ;

type_specifier:
    INT { $$ = TYPE_INT; current_decl_struct_name = NULL; }
    | CHAR { $$ = TYPE_CHAR; current_decl_struct_name = NULL; }
    | VOID { $$ = TYPE_VOID; current_decl_struct_name = NULL; }
    | struct_or_union_specifier { $$ = TYPE_STRUCT; current_decl_struct_name = $1; }
    ;

struct_or_union_specifier:
    STRUCT IDENTIFIER { $$ = $2; }
    ;

pointer:
    /* empty */ { $$ = 0; }
    | '*' pointer { $$ = $2 + 1; }
    ;

struct_declaration:
    STRUCT IDENTIFIER '{' struct_member_list '}' ';' {
        $$ = ast_create_struct_decl($2, $4.items, $4.count);
        free($2);
    }
    ;

enum_declaration:
    ENUM IDENTIFIER '{' {
        // Initialize enum globals
        enum_names_temp = NULL;
        enum_values_temp = NULL;
        enum_count_temp = 0;
      } enumerator_list '}' ';' {
        // enumerator_list has populated enum_names_temp and enum_values_temp
        $$ = ast_create_enum_decl($2, enum_names_temp, enum_values_temp, enum_count_temp);
        free($2);
    }
    ;

typedef_declaration:
    TYPEDEF type_specifier pointer IDENTIFIER ';' {
        // Simple typedef: typedef int myint; or typedef int* intptr;
        $$ = ast_create_typedef($2, $4, $3, NULL, 0);
        free($4);
    }
    | TYPEDEF type_specifier pointer IDENTIFIER array_sizes ';' {
        // Array typedef: typedef int arr[10];
        int* sizes = malloc(sizeof(int) * $5.count);
        for (int i = 0; i < $5.count; i++) {
            sizes[i] = ((int*)$5.items)[i];
        }
        free($5.items);
        $$ = ast_create_typedef($2, $4, $3, sizes, $5.count);
        free($4);
    }
    ;

struct_member_list:
    type_specifier pointer IDENTIFIER ';' {
        char* struct_name = ($1 == TYPE_STRUCT) ? yylval.string : NULL;
        $$.items = malloc(sizeof(ASTNode*));
        $$.items[0] = ast_create_var_decl($1, $3, NULL, 0, 0, $2, struct_name);
        $$.count = 1;
        free($3);
    }
    | struct_member_list type_specifier pointer IDENTIFIER ';' {
        char* struct_name = ($2 == TYPE_STRUCT) ? yylval.string : NULL;
        $$.items = realloc($1.items, sizeof(ASTNode*) * ($1.count + 1));
        $$.items[$1.count] = ast_create_var_decl($2, $4, NULL, 0, 0, $3, struct_name);
        $$.count = $1.count + 1;
        free($4);
    }
    ;

enum_constant:
    NUMBER { $$ = $1; }
    | '-' NUMBER { $$ = -$2; }
    ;

enumerator_list:
    IDENTIFIER {
        // First enumerator, value = 0
        enum_count_temp = 1;
        enum_names_temp = malloc(sizeof(char*));
        enum_values_temp = malloc(sizeof(int));
        enum_names_temp[0] = strdup($1);
        enum_values_temp[0] = 0;
        free($1);
        $$.count = 1;
    }
    | IDENTIFIER '=' enum_constant {
        // First enumerator with explicit value
        enum_count_temp = 1;
        enum_names_temp = malloc(sizeof(char*));
        enum_values_temp = malloc(sizeof(int));
        enum_names_temp[0] = strdup($1);
        enum_values_temp[0] = $3;
        free($1);
        $$.count = 1;
    }
    | enumerator_list ',' IDENTIFIER {
        // Add enumerator, value = previous + 1
        enum_count_temp++;
        enum_names_temp = realloc(enum_names_temp, sizeof(char*) * enum_count_temp);
        enum_values_temp = realloc(enum_values_temp, sizeof(int) * enum_count_temp);
        enum_names_temp[enum_count_temp - 1] = strdup($3);
        enum_values_temp[enum_count_temp - 1] = enum_values_temp[enum_count_temp - 2] + 1;
        free($3);
        $$.count = enum_count_temp;
    }
    | enumerator_list ',' IDENTIFIER '=' enum_constant {
        // Add enumerator with explicit value
        enum_count_temp++;
        enum_names_temp = realloc(enum_names_temp, sizeof(char*) * enum_count_temp);
        enum_values_temp = realloc(enum_values_temp, sizeof(int) * enum_count_temp);
        enum_names_temp[enum_count_temp - 1] = strdup($3);
        enum_values_temp[enum_count_temp - 1] = $5;
        free($3);
        $$.count = enum_count_temp;
    }
    ;

array_sizes:
    '[' NUMBER ']' {
        $$.items = malloc(sizeof(int));
        ((int*)$$.items)[0] = $2;
        $$.count = 1;
    }
    | array_sizes '[' NUMBER ']' {
        $$.items = realloc($1.items, sizeof(int) * ($1.count + 1));
        ((int*)$$.items)[$1.count] = $3;
        $$.count = $1.count + 1;
    }
    ;

variable_declaration:
    type_specifier {
        current_decl_type = $1;
        // current_decl_struct_name is already set by type_specifier
      } declarator_list ';' {
        // $3 is declarator_list (shifted due to mid-rule action)
        ASTNode** decls = malloc(sizeof(ASTNode*) * $3.count);
        for (int i = 0; i < $3.count; i++) {
            decls[i] = $3.items[i];
        }
        if ($3.count == 1) {
            // Single declarator - return it directly
            $$ = decls[0];
            free(decls);
        } else {
            // Multiple declarators - return var_decl_list
            $$ = ast_create_var_decl_list(decls, $3.count);
        }
    }
    ;

declarator_list:
    declarator {
        $$.items = malloc(sizeof(ASTNode*));
        $$.items[0] = $1;
        $$.count = 1;
    }
    | declarator_list ',' declarator {
        $$.items = realloc($1.items, sizeof(ASTNode*) * ($1.count + 1));
        $$.items[$1.count] = $3;
        $$.count = $1.count + 1;
    }
    ;

initializer_list:
    assignment_expression {
        $$.items = malloc(sizeof(ASTNode*));
        $$.items[0] = $1;
        $$.count = 1;
    }
    | initializer_list ',' assignment_expression {
        $$.items = realloc($1.items, sizeof(ASTNode*) * ($1.count + 1));
        $$.items[$1.count] = $3;
        $$.count = $1.count + 1;
    }
    ;

declarator:
    pointer IDENTIFIER {
        $$ = ast_create_var_decl(current_decl_type, $2, NULL, 0, 0, $1, current_decl_struct_name);
        free($2);
    }
    | pointer IDENTIFIER '=' expression {
        $$ = ast_create_var_decl(current_decl_type, $2, $4, 0, 0, $1, current_decl_struct_name);
        free($2);
    }
    | pointer IDENTIFIER array_sizes {
        $$ = ast_create_var_decl_multidim(current_decl_type, $2, NULL, (int*)$3.items, $3.count, $1, current_decl_struct_name);
        free($2);
    }
    | pointer IDENTIFIER array_sizes '=' '{' initializer_list '}' {
        // Array initialization: int arr[3] = {1, 2, 3};
        ASTNode* init_list = ast_create_init_list($6.items, $6.count);
        $$ = ast_create_var_decl_multidim(current_decl_type, $2, init_list, (int*)$3.items, $3.count, $1, current_decl_struct_name);
        free($2);
    }
    ;

function_declaration:
    type_specifier IDENTIFIER '(' ')' compound_statement {
        $$ = ast_create_func_decl($1, $2, NULL, 0, $5);
        free($2);
    }
    | type_specifier IDENTIFIER '(' parameter_list ')' compound_statement {
        $$ = ast_create_func_decl($1, $2, $4.items, $4.count, $6);
        free($2);
    }
    ;

parameter_list:
    type_specifier pointer IDENTIFIER {
        char* struct_name = ($1 == TYPE_STRUCT) ? yylval.string : NULL;
        $$.items = malloc(sizeof(ASTNode*));
        $$.items[0] = ast_create_param($1, $3, $2, struct_name);
        $$.count = 1;
        free($3);
    }
    | parameter_list ',' type_specifier pointer IDENTIFIER {
        char* struct_name = ($3 == TYPE_STRUCT) ? yylval.string : NULL;
        $$.items = realloc($1.items, sizeof(ASTNode*) * ($1.count + 1));
        $$.items[$1.count] = ast_create_param($3, $5, $4, struct_name);
        $$.count = $1.count + 1;
        free($5);
    }
    ;

statement:
    compound_statement { $$ = $1; }
    | expression_statement { $$ = $1; }
    | selection_statement { $$ = $1; }
    | iteration_statement { $$ = $1; }
    | jump_statement { $$ = $1; }
    | variable_declaration { $$ = $1; }
    | switch_statement { $$ = $1; }
    | IDENTIFIER ':' statement {
        $$ = ast_create_label($1, $3);
        free($1);
    }
    ;

compound_statement:
    '{' '}' {
        $$ = ast_create_block(NULL, 0);
    }
    | '{' statement_list '}' {
        $$ = $2;
    }
    ;

statement_list:
    statement {
        ASTNode** stmts = malloc(sizeof(ASTNode*));
        stmts[0] = $1;
        $$ = ast_create_block(stmts, 1);
    }
    | statement_list statement {
        ASTNode** stmts = realloc($1->block.statements,
                                   sizeof(ASTNode*) * ($1->block.stmt_count + 1));
        stmts[$1->block.stmt_count] = $2;
        $$ = ast_create_block(stmts, $1->block.stmt_count + 1);
        free($1);
    }
    ;

expression_statement:
    ';' {
        $$ = ast_create_expr_stmt(NULL);
    }
    | expression ';' {
        $$ = ast_create_expr_stmt($1);
    }
    ;

selection_statement:
    IF '(' expression ')' statement {
        $$ = ast_create_if($3, $5, NULL);
    }
    | IF '(' expression ')' statement ELSE statement {
        $$ = ast_create_if($3, $5, $7);
    }
    ;

switch_statement:
    SWITCH '(' expression ')' '{' case_list '}' {
        $$ = ast_create_switch($3, $6.items, $6.count);
    }
    ;

case_list:
    case_statement {
        $$.items = malloc(sizeof(ASTNode*));
        $$.items[0] = $1;
        $$.count = 1;
    }
    | default_statement {
        $$.items = malloc(sizeof(ASTNode*));
        $$.items[0] = $1;
        $$.count = 1;
    }
    | case_list case_statement {
        $$.items = realloc($1.items, sizeof(ASTNode*) * ($1.count + 1));
        $$.items[$1.count] = $2;
        $$.count = $1.count + 1;
    }
    | case_list default_statement {
        $$.items = realloc($1.items, sizeof(ASTNode*) * ($1.count + 1));
        $$.items[$1.count] = $2;
        $$.count = $1.count + 1;
    }
    ;

case_statement:
    CASE NUMBER ':' statement_list_in_case {
        $$ = ast_create_case($2, $4.items, $4.count);
    }
    ;

default_statement:
    DEFAULT ':' statement_list_in_case {
        $$ = ast_create_default($3.items, $3.count);
    }
    ;

statement_list_in_case:
    statement {
        $$.items = malloc(sizeof(ASTNode*));
        $$.items[0] = $1;
        $$.count = 1;
    }
    | statement_list_in_case statement {
        $$.items = realloc($1.items, sizeof(ASTNode*) * ($1.count + 1));
        $$.items[$1.count] = $2;
        $$.count = $1.count + 1;
    }
    ;

iteration_statement:
    WHILE '(' expression ')' statement {
        $$ = ast_create_while($3, $5);
    }
    | DO statement WHILE '(' expression ')' ';' {
        $$ = ast_create_do_while($5, $2);
    }
    | FOR '(' expression_statement expression_statement ')' statement {
        $$ = ast_create_for($3, $4, NULL, $6);
    }
    | FOR '(' expression_statement expression_statement expression ')' statement {
        $$ = ast_create_for($3, $4, ast_create_expr_stmt($5), $7);
    }
    ;

jump_statement:
    RETURN ';' {
        $$ = ast_create_return(NULL);
    }
    | RETURN expression ';' {
        $$ = ast_create_return($2);
    }
    | BREAK ';' {
        $$ = ast_create_break();
    }
    | CONTINUE ';' {
        $$ = ast_create_continue();
    }
    | GOTO IDENTIFIER ';' {
        $$ = ast_create_goto($2);
        free($2);
    }
    ;

expression:
    assignment_expression { $$ = $1; }
    | expression ',' assignment_expression {
        // Build comma expression
        if ($1->type == AST_COMMA) {
            // Extend existing comma expression
            ASTNode** exprs = realloc($1->comma.expressions, sizeof(ASTNode*) * ($1->comma.expr_count + 1));
            exprs[$1->comma.expr_count] = $3;
            $$ = ast_create_comma(exprs, $1->comma.expr_count + 1);
            free($1);
        } else {
            // Create new comma expression
            ASTNode** exprs = malloc(sizeof(ASTNode*) * 2);
            exprs[0] = $1;
            exprs[1] = $3;
            $$ = ast_create_comma(exprs, 2);
        }
    }
    ;

assignment_expression:
    conditional_expression { $$ = $1; }
    | IDENTIFIER '=' assignment_expression {
        $$ = ast_create_assign($1, $3);
        free($1);
    }
    | IDENTIFIER '[' expression ']' '=' assignment_expression {
        $$ = ast_create_array_assign($1, $3, $6);
        free($1);
    }
    | IDENTIFIER ADD_ASSIGN assignment_expression {
        ASTNode* var = ast_create_identifier($1);
        ASTNode* add = ast_create_binop(OP_ADD, var, $3);
        $$ = ast_create_assign($1, add);
        free($1);
    }
    | IDENTIFIER SUB_ASSIGN assignment_expression {
        ASTNode* var = ast_create_identifier($1);
        ASTNode* sub = ast_create_binop(OP_SUB, var, $3);
        $$ = ast_create_assign($1, sub);
        free($1);
    }
    | IDENTIFIER MUL_ASSIGN assignment_expression {
        ASTNode* var = ast_create_identifier($1);
        ASTNode* mul = ast_create_binop(OP_MUL, var, $3);
        $$ = ast_create_assign($1, mul);
        free($1);
    }
    | IDENTIFIER DIV_ASSIGN assignment_expression {
        ASTNode* var = ast_create_identifier($1);
        ASTNode* div = ast_create_binop(OP_DIV, var, $3);
        $$ = ast_create_assign($1, div);
        free($1);
    }
    | IDENTIFIER MOD_ASSIGN assignment_expression {
        ASTNode* var = ast_create_identifier($1);
        ASTNode* mod = ast_create_binop(OP_MOD, var, $3);
        $$ = ast_create_assign($1, mod);
        free($1);
    }
    | IDENTIFIER AND_ASSIGN assignment_expression {
        ASTNode* var = ast_create_identifier($1);
        ASTNode* and = ast_create_binop(OP_BIT_AND, var, $3);
        $$ = ast_create_assign($1, and);
        free($1);
    }
    | IDENTIFIER OR_ASSIGN assignment_expression {
        ASTNode* var = ast_create_identifier($1);
        ASTNode* or = ast_create_binop(OP_BIT_OR, var, $3);
        $$ = ast_create_assign($1, or);
        free($1);
    }
    | IDENTIFIER XOR_ASSIGN assignment_expression {
        ASTNode* var = ast_create_identifier($1);
        ASTNode* xor = ast_create_binop(OP_BIT_XOR, var, $3);
        $$ = ast_create_assign($1, xor);
        free($1);
    }
    | IDENTIFIER '.' IDENTIFIER '=' assignment_expression {
        ASTNode* object = ast_create_identifier($1);
        $$ = ast_create_member_assign(object, $3, $5);
        free($1);
        free($3);
    }
    | IDENTIFIER ARROW IDENTIFIER '=' assignment_expression {
        ASTNode* pointer = ast_create_identifier($1);
        $$ = ast_create_ptr_member_assign(pointer, $3, $5);
        free($1);
        free($3);
    }
    ;

conditional_expression:
    logical_or_expression { $$ = $1; }
    | logical_or_expression '?' expression ':' conditional_expression {
        $$ = ast_create_ternary($1, $3, $5);
    }
    ;

logical_or_expression:
    logical_and_expression { $$ = $1; }
    | logical_or_expression OR logical_and_expression {
        $$ = ast_create_binop(OP_OR, $1, $3);
    }
    ;

logical_and_expression:
    or_expression { $$ = $1; }
    | logical_and_expression AND or_expression {
        $$ = ast_create_binop(OP_AND, $1, $3);
    }
    ;

or_expression:
    xor_expression { $$ = $1; }
    | or_expression '|' xor_expression {
        $$ = ast_create_binop(OP_BIT_OR, $1, $3);
    }
    ;

xor_expression:
    and_expression { $$ = $1; }
    | xor_expression '^' and_expression {
        $$ = ast_create_binop(OP_BIT_XOR, $1, $3);
    }
    ;

and_expression:
    equality_expression { $$ = $1; }
    | and_expression '&' equality_expression {
        $$ = ast_create_binop(OP_BIT_AND, $1, $3);
    }
    ;

equality_expression:
    relational_expression { $$ = $1; }
    | equality_expression EQ relational_expression {
        $$ = ast_create_binop(OP_EQ, $1, $3);
    }
    | equality_expression NE relational_expression {
        $$ = ast_create_binop(OP_NE, $1, $3);
    }
    ;

relational_expression:
    shift_expression { $$ = $1; }
    | relational_expression '<' shift_expression {
        $$ = ast_create_binop(OP_LT, $1, $3);
    }
    | relational_expression '>' shift_expression {
        $$ = ast_create_binop(OP_GT, $1, $3);
    }
    | relational_expression LE shift_expression {
        $$ = ast_create_binop(OP_LE, $1, $3);
    }
    | relational_expression GE shift_expression {
        $$ = ast_create_binop(OP_GE, $1, $3);
    }
    ;

shift_expression:
    additive_expression { $$ = $1; }
    | shift_expression SHL additive_expression {
        $$ = ast_create_binop(OP_SHL, $1, $3);
    }
    | shift_expression SHR additive_expression {
        $$ = ast_create_binop(OP_SHR, $1, $3);
    }
    ;

additive_expression:
    multiplicative_expression { $$ = $1; }
    | additive_expression '+' multiplicative_expression {
        $$ = ast_create_binop(OP_ADD, $1, $3);
    }
    | additive_expression '-' multiplicative_expression {
        $$ = ast_create_binop(OP_SUB, $1, $3);
    }
    ;

multiplicative_expression:
    unary_expression { $$ = $1; }
    | multiplicative_expression '*' unary_expression {
        $$ = ast_create_binop(OP_MUL, $1, $3);
    }
    | multiplicative_expression '/' unary_expression {
        $$ = ast_create_binop(OP_DIV, $1, $3);
    }
    | multiplicative_expression '%' unary_expression {
        $$ = ast_create_binop(OP_MOD, $1, $3);
    }
    ;

unary_expression:
    postfix_expression { $$ = $1; }
    | '-' unary_expression %prec UMINUS {
        $$ = ast_create_unop(OP_NEG, $2);
    }
    | '!' unary_expression {
        $$ = ast_create_unop(OP_NOT, $2);
    }
    | '~' unary_expression {
        $$ = ast_create_unop(OP_BIT_NOT, $2);
    }
    | '&' unary_expression {
        $$ = ast_create_addr_of($2);
    }
    | '*' unary_expression %prec UDEREF {
        $$ = ast_create_deref($2);
    }
    | INC IDENTIFIER {
        $$ = ast_create_pre_inc($2);
        free($2);
    }
    | DEC IDENTIFIER {
        $$ = ast_create_pre_dec($2);
        free($2);
    }
    | '(' type_specifier pointer ')' unary_expression {
        char* type_name = ($2 == TYPE_STRUCT) ? yylval.string : NULL;
        $$ = ast_create_cast($2, type_name, $3, $5);
    }
    | SIZEOF '(' type_specifier pointer ')' {
        char* type_name = ($3 == TYPE_STRUCT) ? yylval.string : NULL;
        $$ = ast_create_sizeof_type($3, type_name, $4);
    }
    | SIZEOF '(' unary_expression ')' {
        $$ = ast_create_sizeof_expr($3);
    }
    ;

postfix_expression:
    primary_expression { $$ = $1; }
    | IDENTIFIER '(' ')' {
        $$ = ast_create_call($1, NULL, 0);
        free($1);
    }
    | IDENTIFIER '(' argument_list ')' {
        $$ = ast_create_call($1, $3.items, $3.count);
        free($1);
    }
    | IDENTIFIER '[' expression ']' {
        $$ = ast_create_array_subscript($1, $3);
        free($1);
    }
    | postfix_expression '.' IDENTIFIER {
        $$ = ast_create_member_access($1, $3);
        free($3);
    }
    | postfix_expression ARROW IDENTIFIER {
        $$ = ast_create_ptr_member_access($1, $3);
        free($3);
    }
    | IDENTIFIER INC {
        $$ = ast_create_post_inc($1);
        free($1);
    }
    | IDENTIFIER DEC {
        $$ = ast_create_post_dec($1);
        free($1);
    }
    ;

argument_list:
    assignment_expression {
        $$.items = malloc(sizeof(ASTNode*));
        $$.items[0] = $1;
        $$.count = 1;
    }
    | argument_list ',' assignment_expression {
        $$.items = realloc($1.items, sizeof(ASTNode*) * ($1.count + 1));
        $$.items[$1.count] = $3;
        $$.count = $1.count + 1;
    }
    ;

primary_expression:
    IDENTIFIER {
        $$ = ast_create_identifier($1);
        free($1);
    }
    | NUMBER {
        $$ = ast_create_number($1);
    }
    | CHAR_LITERAL {
        $$ = ast_create_number($1);
    }
    | STRING_LITERAL {
        $$ = ast_create_string_literal($1);
        free($1);
    }
    | '(' expression ')' {
        $$ = $2;
    }
    ;

%%

void yyerror(const char* s) {
    fprintf(stderr, "Parse error at line %d, column %d: %s\n", line_num, col_num, s);
}

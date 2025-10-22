%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

extern int yylex();
extern int yyparse();
extern FILE* yyin;
extern int line_num;

void yyerror(const char* s);

ASTNode* ast_root = NULL;
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

%token <number> NUMBER
%token <string> IDENTIFIER
%token INT CHAR VOID
%token IF ELSE WHILE FOR RETURN
%token EQ NE LE GE AND OR SHL SHR

%type <node> program declaration function_declaration statement
%type <node> expression primary_expression postfix_expression
%type <node> unary_expression multiplicative_expression additive_expression
%type <node> shift_expression relational_expression equality_expression
%type <node> and_expression xor_expression or_expression
%type <node> logical_and_expression logical_or_expression
%type <node> assignment_expression
%type <node> compound_statement statement_list
%type <node> expression_statement selection_statement iteration_statement
%type <node> jump_statement variable_declaration
%type <dtype> type_specifier
%type <list> declaration_list parameter_list argument_list

%left ','
%right '='
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
%right '!' '~' UMINUS

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
    ;

type_specifier:
    INT { $$ = TYPE_INT; }
    | CHAR { $$ = TYPE_CHAR; }
    | VOID { $$ = TYPE_VOID; }
    ;

variable_declaration:
    type_specifier IDENTIFIER ';' {
        $$ = ast_create_var_decl($1, $2, NULL, 0, 0);
        free($2);
    }
    | type_specifier IDENTIFIER '=' expression ';' {
        $$ = ast_create_var_decl($1, $2, $4, 0, 0);
        free($2);
    }
    | type_specifier IDENTIFIER '[' NUMBER ']' ';' {
        $$ = ast_create_var_decl($1, $2, NULL, 1, $4);
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
    type_specifier IDENTIFIER {
        $$.items = malloc(sizeof(ASTNode*));
        $$.items[0] = ast_create_param($1, $2);
        $$.count = 1;
        free($2);
    }
    | parameter_list ',' type_specifier IDENTIFIER {
        $$.items = realloc($1.items, sizeof(ASTNode*) * ($1.count + 1));
        $$.items[$1.count] = ast_create_param($3, $4);
        $$.count = $1.count + 1;
        free($4);
    }
    ;

statement:
    compound_statement { $$ = $1; }
    | expression_statement { $$ = $1; }
    | selection_statement { $$ = $1; }
    | iteration_statement { $$ = $1; }
    | jump_statement { $$ = $1; }
    | variable_declaration { $$ = $1; }
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

iteration_statement:
    WHILE '(' expression ')' statement {
        $$ = ast_create_while($3, $5);
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
    ;

expression:
    assignment_expression { $$ = $1; }
    ;

assignment_expression:
    logical_or_expression { $$ = $1; }
    | IDENTIFIER '=' assignment_expression {
        $$ = ast_create_assign($1, $3);
        free($1);
    }
    | IDENTIFIER '[' expression ']' '=' assignment_expression {
        $$ = ast_create_array_assign($1, $3, $6);
        free($1);
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
    | '(' expression ')' {
        $$ = $2;
    }
    ;

%%

void yyerror(const char* s) {
    fprintf(stderr, "Parse error at line %d: %s\n", line_num, s);
}

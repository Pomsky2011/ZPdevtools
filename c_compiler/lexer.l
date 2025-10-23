%{
#include <stdio.h>
#include <string.h>
#include "ast.h"
#include "parser.tab.h"

int line_num = 1;
int col_num = 1;
%}

%option noyywrap

%%

[ \t]+              { col_num += yyleng; /* ignore whitespace */ }
\n                  { line_num++; col_num = 1; }
"//".*              { /* ignore C++ style comments */ }
"/*"([^*]|\*+[^*/])*"*/" {
    for (int i = 0; i < yyleng; i++) {
        if (yytext[i] == '\n') {
            line_num++;
            col_num = 1;
        } else {
            col_num++;
        }
    }
}

"int"               { col_num += yyleng; return INT; }
"char"              { col_num += yyleng; return CHAR; }
"void"              { col_num += yyleng; return VOID; }
"struct"            { col_num += yyleng; return STRUCT; }
"if"                { col_num += yyleng; return IF; }
"else"              { col_num += yyleng; return ELSE; }
"while"             { col_num += yyleng; return WHILE; }
"for"               { col_num += yyleng; return FOR; }
"return"            { col_num += yyleng; return RETURN; }
"break"             { col_num += yyleng; return BREAK; }
"continue"          { col_num += yyleng; return CONTINUE; }
"switch"            { col_num += yyleng; return SWITCH; }
"case"              { col_num += yyleng; return CASE; }
"default"           { col_num += yyleng; return DEFAULT; }
"sizeof"            { col_num += yyleng; return SIZEOF; }

[a-zA-Z_][a-zA-Z0-9_]* { col_num += yyleng; yylval.string = strdup(yytext); return IDENTIFIER; }
[0-9]+              { col_num += yyleng; yylval.number = atoi(yytext); return NUMBER; }
"0x"[0-9a-fA-F]+    { col_num += yyleng; yylval.number = strtol(yytext, NULL, 16); return NUMBER; }
\"([^"\\]|\\.)*\"   { col_num += yyleng; yylval.string = strdup(yytext); return STRING_LITERAL; }
'.'                 { col_num += yyleng; yylval.number = yytext[1]; return CHAR_LITERAL; }
'\\n'               { col_num += yyleng; yylval.number = '\n'; return CHAR_LITERAL; }
'\\t'               { col_num += yyleng; yylval.number = '\t'; return CHAR_LITERAL; }
'\\0'               { col_num += yyleng; yylval.number = '\0'; return CHAR_LITERAL; }

"=="                { col_num += 2; return EQ; }
"!="                { col_num += 2; return NE; }
"<="                { col_num += 2; return LE; }
">="                { col_num += 2; return GE; }
"&&"                { col_num += 2; return AND; }
"||"                { col_num += 2; return OR; }
"<<"                { col_num += 2; return SHL; }
">>"                { col_num += 2; return SHR; }
"->"                { col_num += 2; return ARROW; }
"++"                { col_num += 2; return INC; }
"--"                { col_num += 2; return DEC; }
"+="                { col_num += 2; return ADD_ASSIGN; }
"-="                { col_num += 2; return SUB_ASSIGN; }
"*="                { col_num += 2; return MUL_ASSIGN; }
"/="                { col_num += 2; return DIV_ASSIGN; }
"%="                { col_num += 2; return MOD_ASSIGN; }
"&="                { col_num += 2; return AND_ASSIGN; }
"|="                { col_num += 2; return OR_ASSIGN; }
"^="                { col_num += 2; return XOR_ASSIGN; }

"("                 { col_num++; return '('; }
")"                 { col_num++; return ')'; }
"{"                 { col_num++; return '{'; }
"}"                 { col_num++; return '}'; }
"["                 { col_num++; return '['; }
"]"                 { col_num++; return ']'; }
";"                 { col_num++; return ';'; }
","                 { col_num++; return ','; }
"."                 { col_num++; return '.'; }
"="                 { col_num++; return '='; }
"+"                 { col_num++; return '+'; }
"-"                 { col_num++; return '-'; }
"*"                 { col_num++; return '*'; }
"/"                 { col_num++; return '/'; }
"%"                 { col_num++; return '%'; }
"<"                 { col_num++; return '<'; }
">"                 { col_num++; return '>'; }
"&"                 { col_num++; return '&'; }
"|"                 { col_num++; return '|'; }
"^"                 { col_num++; return '^'; }
"~"                 { col_num++; return '~'; }
"!"                 { col_num++; return '!'; }
"?"                 { col_num++; return '?'; }
":"                 { col_num++; return ':'; }

.                   { fprintf(stderr, "Error at line %d, col %d: Unexpected character '%s'\n", line_num, col_num, yytext); col_num++; }

%%

%{
#include <stdio.h>
#include <string.h>
#include "ast.h"
#include "parser.tab.h"

int line_num = 1;
%}

%option noyywrap

%%

[ \t]+              { /* ignore whitespace */ }
\n                  { line_num++; }
"//".*              { /* ignore C++ style comments */ }
"/*"([^*]|\*+[^*/])*"*/" { /* ignore C style comments */ }

"int"               { return INT; }
"char"              { return CHAR; }
"void"              { return VOID; }
"struct"            { return STRUCT; }
"if"                { return IF; }
"else"              { return ELSE; }
"while"             { return WHILE; }
"for"               { return FOR; }
"return"            { return RETURN; }
"break"             { return BREAK; }
"continue"          { return CONTINUE; }

[a-zA-Z_][a-zA-Z0-9_]* { yylval.string = strdup(yytext); return IDENTIFIER; }
[0-9]+              { yylval.number = atoi(yytext); return NUMBER; }
"0x"[0-9a-fA-F]+    { yylval.number = strtol(yytext, NULL, 16); return NUMBER; }

"=="                { return EQ; }
"!="                { return NE; }
"<="                { return LE; }
">="                { return GE; }
"&&"                { return AND; }
"||"                { return OR; }
"<<"                { return SHL; }
">>"                { return SHR; }
"->"                { return ARROW; }
"++"                { return INC; }
"--"                { return DEC; }
"+="                { return ADD_ASSIGN; }
"-="                { return SUB_ASSIGN; }
"*="                { return MUL_ASSIGN; }
"/="                { return DIV_ASSIGN; }
"%="                { return MOD_ASSIGN; }
"&="                { return AND_ASSIGN; }
"|="                { return OR_ASSIGN; }
"^="                { return XOR_ASSIGN; }

"("                 { return '('; }
")"                 { return ')'; }
"{"                 { return '{'; }
"}"                 { return '}'; }
"["                 { return '['; }
"]"                 { return ']'; }
";"                 { return ';'; }
","                 { return ','; }
"."                 { return '.'; }
"="                 { return '='; }
"+"                 { return '+'; }
"-"                 { return '-'; }
"*"                 { return '*'; }
"/"                 { return '/'; }
"%"                 { return '%'; }
"<"                 { return '<'; }
">"                 { return '>'; }
"&"                 { return '&'; }
"|"                 { return '|'; }
"^"                 { return '^'; }
"~"                 { return '~'; }
"!"                 { return '!'; }

.                   { fprintf(stderr, "Unexpected character: %s\n", yytext); }

%%

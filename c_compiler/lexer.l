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
"if"                { return IF; }
"else"              { return ELSE; }
"while"             { return WHILE; }
"for"               { return FOR; }
"return"            { return RETURN; }

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

"("                 { return '('; }
")"                 { return ')'; }
"{"                 { return '{'; }
"}"                 { return '}'; }
"["                 { return '['; }
"]"                 { return ']'; }
";"                 { return ';'; }
","                 { return ','; }
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

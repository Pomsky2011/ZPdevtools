/* lexer_manual.c - Hand-written C89-compliant lexer for DEF88186 C Compiler */
/* Replaces Flex-generated lexer.c to enable DOS/Turbo C compilation */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "ast.h"
#include "parser.h"

/* Lexer state */
FILE* yyin = NULL; /* Exported for main.c */
static int current_char = 0;
int line_num = 1; /* Exported for parser */
int col_num = 1;  /* Exported for parser */

/* Token buffer */
static char token_buf[1024];
static int token_len = 0;

/* External parser interface */
extern YYSTYPE yylval;

/* Keyword table */
typedef struct {
    const char* word;
    int token;
} Keyword;

static const Keyword keywords[] = {
    {"int", INT},
    {"char", CHAR},
    {"void", VOID},
    {"short", SHORT},
    {"long", LONG},
    {"signed", SIGNED},
    {"unsigned", UNSIGNED},
    {"const", CONST},
    {"volatile", VOLATILE},
    {"static", STATIC},
    {"extern", EXTERN},
    {"struct", STRUCT},
    {"union", UNION},
    {"enum", ENUM},
    {"typedef", TYPEDEF},
    {"if", IF},
    {"else", ELSE},
    {"while", WHILE},
    {"do", DO},
    {"for", FOR},
    {"return", RETURN},
    {"break", BREAK},
    {"continue", CONTINUE},
    {"switch", SWITCH},
    {"case", CASE},
    {"default", DEFAULT},
    {"goto", GOTO},
    {"sizeof", SIZEOF},
    {"__asm__", ASM},
    {"asm", ASM},
    {NULL, 0}
};

/* Forward declarations */
static int next_char(void);
static void unget_char(int c);
static void skip_whitespace(void);
static void skip_comment(void);
static int read_identifier(void);
static int read_number(void);
static int read_string(void);
static int read_char_literal(void);

/* Initialize lexer with input file */
void yylex_init(FILE* input) {
    yyin = input;
    current_char = 0;
    line_num = 1;
    col_num = 1;
}

/* Read next character from input */
static int next_char(void) {
    int c;

    if (yyin == NULL) {
        return EOF;
    }

    c = fgetc(yyin);

    if (c == '\n') {
        line_num++;
        col_num = 1;
    } else if (c != EOF) {
        col_num++;
    }

    return c;
}

/* Push character back to input */
static void unget_char(int c) {
    if (c == EOF || yyin == NULL) {
        return;
    }

    if (c == '\n') {
        line_num--;
        col_num = 1; /* Approximate */
    } else {
        col_num--;
    }

    ungetc(c, yyin);
}

/* Skip whitespace */
static void skip_whitespace(void) {
    int c;

    while ((c = next_char()) != EOF) {
        if (!isspace(c)) {
            unget_char(c);
            break;
        }
    }
}

/* Skip C-style comment */
static void skip_comment(void) {
    int c;
    int prev;

    prev = 0;

    while ((c = next_char()) != EOF) {
        if (prev == '*' && c == '/') {
            return;
        }
        prev = c;
    }
}

/* Skip C++-style comment */
static void skip_line_comment(void) {
    int c;

    while ((c = next_char()) != EOF) {
        if (c == '\n') {
            unget_char(c);
            break;
        }
    }
}

/* Check if identifier is a keyword */
static int lookup_keyword(const char* id) {
    const Keyword* kw;

    for (kw = keywords; kw->word != NULL; kw++) {
        if (strcmp(id, kw->word) == 0) {
            return kw->token;
        }
    }

    return IDENTIFIER;
}

/* Read identifier or keyword */
static int read_identifier(void) {
    int c;

    token_len = 0;

    c = next_char();

    /* First character: letter or underscore */
    if (isalpha(c) || c == '_') {
        token_buf[token_len++] = c;
    } else {
        unget_char(c);
        return 0;
    }

    /* Remaining characters: letter, digit, or underscore */
    while ((c = next_char()) != EOF) {
        if (isalnum(c) || c == '_') {
            if (token_len < sizeof(token_buf) - 1) {
                token_buf[token_len++] = c;
            }
        } else {
            unget_char(c);
            break;
        }
    }

    token_buf[token_len] = '\0';

    /* Check if it's a keyword */
    c = lookup_keyword(token_buf);

    if (c == IDENTIFIER) {
        yylval.string = strdup(token_buf);
    }

    return c;
}

/* Read decimal or hexadecimal number */
static int read_number(void) {
    int c;
    int base;

    token_len = 0;
    base = 10;

    c = next_char();

    if (!isdigit(c)) {
        unget_char(c);
        return 0;
    }

    token_buf[token_len++] = c;

    /* Check for hex prefix */
    if (c == '0') {
        c = next_char();
        if (c == 'x' || c == 'X') {
            base = 16;
            token_buf[token_len++] = c;
        } else {
            unget_char(c);
        }
    }

    /* Read remaining digits */
    while ((c = next_char()) != EOF) {
        if (base == 16 && isxdigit(c)) {
            if (token_len < sizeof(token_buf) - 1) {
                token_buf[token_len++] = c;
            }
        } else if (base == 10 && isdigit(c)) {
            if (token_len < sizeof(token_buf) - 1) {
                token_buf[token_len++] = c;
            }
        } else {
            unget_char(c);
            break;
        }
    }

    token_buf[token_len] = '\0';

    yylval.number = (int)strtol(token_buf, NULL, base);

    return NUMBER;
}

/* Read string literal */
static int read_string(void) {
    int c;

    token_len = 0;

    c = next_char();

    if (c != '"') {
        unget_char(c);
        return 0;
    }

    token_buf[token_len++] = c;

    /* Read until closing quote */
    while ((c = next_char()) != EOF) {
        if (token_len < sizeof(token_buf) - 1) {
            token_buf[token_len++] = c;
        }

        if (c == '\\') {
            /* Escaped character */
            c = next_char();
            if (c != EOF && token_len < sizeof(token_buf) - 1) {
                token_buf[token_len++] = c;
            }
        } else if (c == '"') {
            break;
        }
    }

    token_buf[token_len] = '\0';

    yylval.string = strdup(token_buf);

    return STRING_LITERAL;
}

/* Read character literal */
static int read_char_literal(void) {
    int c;
    int value;

    c = next_char();

    if (c != '\'') {
        unget_char(c);
        return 0;
    }

    c = next_char();

    if (c == '\\') {
        /* Escape sequence */
        c = next_char();
        switch (c) {
            case 'n': value = '\n'; break;
            case 't': value = '\t'; break;
            case 'r': value = '\r'; break;
            case '0': value = '\0'; break;
            case '\\': value = '\\'; break;
            case '\'': value = '\''; break;
            case '"': value = '"'; break;
            default: value = c; break;
        }
    } else {
        value = c;
    }

    c = next_char();
    if (c != '\'') {
        fprintf(stderr, "Error at line %d: Unterminated character literal\n", line_num);
    }

    yylval.number = value;

    return CHAR_LITERAL;
}

/* Main lexer function - called by parser */
int yylex(void) {
    int c;
    int c2;
    int token;

    if (yyin == NULL) {
        return 0;
    }

    skip_whitespace();

    c = next_char();

    if (c == EOF) {
        return 0;
    }

    /* Check for comments */
    if (c == '/') {
        c2 = next_char();
        if (c2 == '*') {
            skip_comment();
            return yylex(); /* Recursively get next token */
        } else if (c2 == '/') {
            skip_line_comment();
            return yylex(); /* Recursively get next token */
        } else if (c2 == '=') {
            return DIV_ASSIGN;
        } else {
            unget_char(c2);
            return '/';
        }
    }

    /* Check for identifiers and keywords */
    if (isalpha(c) || c == '_') {
        unget_char(c);
        return read_identifier();
    }

    /* Check for numbers */
    if (isdigit(c)) {
        unget_char(c);
        return read_number();
    }

    /* Check for string literals */
    if (c == '"') {
        unget_char(c);
        return read_string();
    }

    /* Check for character literals */
    if (c == '\'') {
        unget_char(c);
        return read_char_literal();
    }

    /* Check for two-character operators */
    c2 = next_char();

    switch (c) {
        case '=':
            if (c2 == '=') return EQ;
            unget_char(c2);
            return '=';

        case '!':
            if (c2 == '=') return NE;
            unget_char(c2);
            return '!';

        case '<':
            if (c2 == '=') return LE;
            if (c2 == '<') return SHL;
            unget_char(c2);
            return '<';

        case '>':
            if (c2 == '=') return GE;
            if (c2 == '>') return SHR;
            unget_char(c2);
            return '>';

        case '&':
            if (c2 == '&') return AND;
            if (c2 == '=') return AND_ASSIGN;
            unget_char(c2);
            return '&';

        case '|':
            if (c2 == '|') return OR;
            if (c2 == '=') return OR_ASSIGN;
            unget_char(c2);
            return '|';

        case '+':
            if (c2 == '+') return INC;
            if (c2 == '=') return ADD_ASSIGN;
            unget_char(c2);
            return '+';

        case '-':
            if (c2 == '-') return DEC;
            if (c2 == '=') return SUB_ASSIGN;
            if (c2 == '>') return ARROW;
            unget_char(c2);
            return '-';

        case '*':
            if (c2 == '=') return MUL_ASSIGN;
            unget_char(c2);
            return '*';

        case '%':
            if (c2 == '=') return MOD_ASSIGN;
            unget_char(c2);
            return '%';

        case '^':
            if (c2 == '=') return XOR_ASSIGN;
            unget_char(c2);
            return '^';

        default:
            unget_char(c2);
            break;
    }

    /* Single-character tokens */
    token = c;
    return token;
}

/* Get current line number */
int yyget_lineno(void) {
    return line_num;
}

/* Dummy function for compatibility */
int yywrap(void) {
    return 1;
}

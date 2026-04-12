#include "preprocessor.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 4096

// Helper function to trim whitespace
static char* trim(char* str) {
    char* end;
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

// Helper function to check if a character is valid in an identifier
static int is_ident_char(char c) {
    return isalnum(c) || c == '_';
}

// Create preprocessor context
PreprocessorContext* preprocessor_create(FILE* input, FILE* output) {
    PreprocessorContext* ctx = malloc(sizeof(PreprocessorContext));
    ctx->input = input;
    ctx->output = output;
    ctx->macros = NULL;
    ctx->include_paths = malloc(sizeof(char*) * 16);
    ctx->include_path_count = 0;
    ctx->line_num = 1;
    ctx->if_depth = 0;
    ctx->skip_mode = 0;

    // Add default include paths
    preprocessor_add_include_path(ctx, ".");
    preprocessor_add_include_path(ctx, "/usr/include");
    preprocessor_add_include_path(ctx, "/usr/local/include");

    return ctx;
}

// Add include path
void preprocessor_add_include_path(PreprocessorContext* ctx, const char* path) {
    ctx->include_paths[ctx->include_path_count++] = strdup(path);
}

// Find a macro by name
Macro* preprocessor_find_macro(PreprocessorContext* ctx, const char* name) {
    Macro* m = ctx->macros;
    while (m) {
        if (strcmp(m->name, name) == 0) return m;
        m = m->next;
    }
    return NULL;
}

// Define a simple macro
void preprocessor_define(PreprocessorContext* ctx, const char* name, const char* replacement) {
    // Check if already defined
    Macro* existing = preprocessor_find_macro(ctx, name);
    if (existing) {
        free(existing->replacement);
        existing->replacement = strdup(replacement);
        return;
    }

    Macro* m = malloc(sizeof(Macro));
    m->name = strdup(name);
    m->replacement = strdup(replacement);
    m->type = MACRO_SIMPLE;
    m->params = NULL;
    m->param_count = 0;
    m->next = ctx->macros;
    ctx->macros = m;
}

// Define a function-like macro
void preprocessor_define_function(PreprocessorContext* ctx, const char* name,
                                   char** params, int param_count, const char* replacement) {
    Macro* m = malloc(sizeof(Macro));
    m->name = strdup(name);
    m->replacement = strdup(replacement);
    m->type = MACRO_FUNCTION;
    m->params = malloc(sizeof(char*) * param_count);
    for (int i = 0; i < param_count; i++) {
        m->params[i] = strdup(params[i]);
    }
    m->param_count = param_count;
    m->next = ctx->macros;
    ctx->macros = m;
}

// Undefine a macro
void preprocessor_undef(PreprocessorContext* ctx, const char* name) {
    Macro** m = &ctx->macros;
    while (*m) {
        if (strcmp((*m)->name, name) == 0) {
            Macro* to_free = *m;
            *m = (*m)->next;
            free(to_free->name);
            free(to_free->replacement);
            if (to_free->type == MACRO_FUNCTION) {
                for (int i = 0; i < to_free->param_count; i++) {
                    free(to_free->params[i]);
                }
                free(to_free->params);
            }
            free(to_free);
            return;
        }
        m = &(*m)->next;
    }
}

// Expand macros in a line
char* preprocessor_expand_macros(PreprocessorContext* ctx, const char* line) {
    char* result = malloc(MAX_LINE_LENGTH);
    char* out = result;
    const char* p = line;

    while (*p) {
        // Skip whitespace
        if (isspace(*p)) {
            *out++ = *p++;
            continue;
        }

        // Check for identifier (potential macro)
        if (isalpha(*p) || *p == '_') {
            char ident[256];
            int i = 0;
            while (is_ident_char(*p) && i < 255) {
                ident[i++] = *p++;
            }
            ident[i] = '\0';

            Macro* m = preprocessor_find_macro(ctx, ident);
            if (m && m->type == MACRO_SIMPLE) {
                // Replace with macro value
                const char* repl = m->replacement;
                while (*repl) {
                    *out++ = *repl++;
                }
            } else if (m && m->type == MACRO_FUNCTION && *p == '(') {
                // Function-like macro - parse arguments
                p++; // skip '('
                char** args = malloc(sizeof(char*) * m->param_count);
                for (int i = 0; i < m->param_count; i++) {
                    args[i] = malloc(256);
                    int arg_len = 0;
                    int paren_depth = 0;

                    while (*p && (paren_depth > 0 || (*p != ',' && *p != ')'))) {
                        if (*p == '(') paren_depth++;
                        if (*p == ')') paren_depth--;
                        if (paren_depth >= 0) {
                            args[i][arg_len++] = *p;
                        }
                        p++;
                    }
                    args[i][arg_len] = '\0';
                    args[i] = trim(args[i]);

                    if (*p == ',') p++;
                }
                if (*p == ')') p++;

                // Replace parameters in replacement text
                const char* repl = m->replacement;
                while (*repl) {
                    if (isalpha(*repl) || *repl == '_') {
                        char param_name[256];
                        int i = 0;
                        while (is_ident_char(*repl) && i < 255) {
                            param_name[i++] = *repl++;
                        }
                        param_name[i] = '\0';

                        // Check if this is a parameter
                        int found = 0;
                        for (int j = 0; j < m->param_count; j++) {
                            if (strcmp(param_name, m->params[j]) == 0) {
                                const char* arg = args[j];
                                while (*arg) {
                                    *out++ = *arg++;
                                }
                                found = 1;
                                break;
                            }
                        }
                        if (!found) {
                            // Not a parameter, copy as is
                            const char* pn = param_name;
                            while (*pn) {
                                *out++ = *pn++;
                            }
                        }
                    } else {
                        *out++ = *repl++;
                    }
                }

                // Free arguments
                for (int i = 0; i < m->param_count; i++) {
                    free(args[i]);
                }
                free(args);
            } else {
                // Not a macro, copy identifier
                const char* id = ident;
                while (*id) {
                    *out++ = *id++;
                }
            }
        } else {
            // Not an identifier, copy as is
            *out++ = *p++;
        }
    }

    *out = '\0';
    return result;
}

// Process #include directive
static int process_include(PreprocessorContext* ctx, const char* line) {
    // Parse #include "file" or #include <file>
    const char* p = line;
    while (*p && !(*p == '"' || *p == '<')) p++;

    if (*p == '\0') {
        fprintf(stderr, "Preprocessor error at line %d: Invalid #include directive\n", ctx->line_num);
        return 0;
    }

    char delim = (*p == '"') ? '"' : '>';
    p++; // skip opening quote/bracket

    char filename[256];
    int i = 0;
    while (*p && *p != delim && i < 255) {
        filename[i++] = *p++;
    }
    filename[i] = '\0';

    // Try to open the file
    FILE* inc_file = NULL;
    char full_path[512];

    if (delim == '"') {
        // Try current directory first
        inc_file = fopen(filename, "r");
    }

    if (!inc_file) {
        // Try include paths
        for (int i = 0; i < ctx->include_path_count; i++) {
            snprintf(full_path, sizeof(full_path), "%s/%s", ctx->include_paths[i], filename);
            inc_file = fopen(full_path, "r");
            if (inc_file) break;
        }
    }

    if (!inc_file) {
        fprintf(stderr, "Preprocessor error at line %d: Cannot open include file '%s'\n",
                ctx->line_num, filename);
        return 0;
    }

    // Process the included file recursively
    PreprocessorContext* inc_ctx = preprocessor_create(inc_file, ctx->output);
    inc_ctx->macros = ctx->macros; // Share macro definitions
    inc_ctx->include_paths = ctx->include_paths;
    inc_ctx->include_path_count = ctx->include_path_count;

    int result = preprocessor_run(inc_ctx);

    // Don't free macros (shared with parent)
    inc_ctx->macros = NULL;
    inc_ctx->include_paths = NULL;
    fclose(inc_file);
    free(inc_ctx);

    return result;
}

// Process #define directive
static void process_define(PreprocessorContext* ctx, const char* line) {
    const char* p = line;

    // Skip "define"
    while (*p && !isspace(*p)) p++;
    while (*p && isspace(*p)) p++;

    // Get macro name
    char name[256];
    int i = 0;
    while (*p && is_ident_char(*p) && i < 255) {
        name[i++] = *p++;
    }
    name[i] = '\0';

    // Check if function-like macro
    if (*p == '(') {
        p++; // skip '('

        // Parse parameters
        char** params = malloc(sizeof(char*) * 32);
        int param_count = 0;

        while (*p && *p != ')') {
            while (*p && isspace(*p)) p++;

            char param[256];
            int j = 0;
            while (*p && is_ident_char(*p) && j < 255) {
                param[j++] = *p++;
            }
            param[j] = '\0';

            if (j > 0) {
                params[param_count++] = strdup(param);
            }

            while (*p && isspace(*p)) p++;
            if (*p == ',') p++;
        }

        if (*p == ')') p++;
        while (*p && isspace(*p)) p++;

        // Rest is replacement text
        char* replacement = strdup(p);
        preprocessor_define_function(ctx, name, params, param_count, replacement);

        free(replacement);
        for (int i = 0; i < param_count; i++) {
            free(params[i]);
        }
        free(params);
    } else {
        // Simple macro
        while (*p && isspace(*p)) p++;
        char* replacement = strdup(p);
        preprocessor_define(ctx, name, replacement);
        free(replacement);
    }
}

// Evaluate simple #if expression (only supports defined() and integer constants)
static int evaluate_if_expression(PreprocessorContext* ctx, const char* expr) {
    const char* p = expr;

    // Skip whitespace
    while (*p && isspace(*p)) p++;

    // Check for "defined(MACRO)" or "defined MACRO"
    if (strncmp(p, "defined", 7) == 0) {
        p += 7;
        while (*p && isspace(*p)) p++;

        int has_parens = (*p == '(');
        if (has_parens) p++;

        while (*p && isspace(*p)) p++;

        char name[256];
        int i = 0;
        while (*p && is_ident_char(*p) && i < 255) {
            name[i++] = *p++;
        }
        name[i] = '\0';

        if (has_parens) {
            while (*p && isspace(*p)) p++;
            if (*p == ')') p++;
        }

        return (preprocessor_find_macro(ctx, name) != NULL) ? 1 : 0;
    }

    // Try to parse as integer
    return atoi(expr);
}

// Run the preprocessor
int preprocessor_run(PreprocessorContext* ctx) {
    char line[MAX_LINE_LENGTH];
    char continued_line[MAX_LINE_LENGTH * 4]; // For line continuations
    continued_line[0] = '\0';

    while (fgets(line, sizeof(line), ctx->input)) {
        ctx->line_num++;

        // Handle line continuations
        int len = strlen(line);
        if (len > 0 && line[len-1] == '\n') line[len-1] = '\0';
        if (len > 1 && line[len-2] == '\\') {
            line[len-2] = '\0';
            strcat(continued_line, line);
            continue;
        }

        // Append to continued line
        if (continued_line[0] != '\0') {
            strcat(continued_line, line);
            strcpy(line, continued_line);
            continued_line[0] = '\0';
        }

        char* trimmed = trim(line);

        // Process preprocessor directives
        if (trimmed[0] == '#') {
            char* directive = trimmed + 1;
            while (*directive && isspace(*directive)) directive++;

            if (strncmp(directive, "include", 7) == 0) {
                if (!ctx->skip_mode) {
                    if (!process_include(ctx, directive + 7)) return 0;
                }
            } else if (strncmp(directive, "define", 6) == 0) {
                if (!ctx->skip_mode) {
                    process_define(ctx, directive + 6);
                }
            } else if (strncmp(directive, "undef", 5) == 0) {
                if (!ctx->skip_mode) {
                    const char* p = directive + 5;
                    while (*p && isspace(*p)) p++;
                    char name[256];
                    int i = 0;
                    while (*p && is_ident_char(*p) && i < 255) {
                        name[i++] = *p++;
                    }
                    name[i] = '\0';
                    preprocessor_undef(ctx, name);
                }
            } else if (strncmp(directive, "ifdef", 5) == 0) {
                const char* p = directive + 5;
                while (*p && isspace(*p)) p++;
                char name[256];
                int i = 0;
                while (*p && is_ident_char(*p) && i < 255) {
                    name[i++] = *p++;
                }
                name[i] = '\0';

                int defined = (preprocessor_find_macro(ctx, name) != NULL);
                ctx->if_stack[ctx->if_depth++] = defined;
                if (!defined) ctx->skip_mode++;
            } else if (strncmp(directive, "ifndef", 6) == 0) {
                const char* p = directive + 6;
                while (*p && isspace(*p)) p++;
                char name[256];
                int i = 0;
                while (*p && is_ident_char(*p) && i < 255) {
                    name[i++] = *p++;
                }
                name[i] = '\0';

                int defined = (preprocessor_find_macro(ctx, name) != NULL);
                ctx->if_stack[ctx->if_depth++] = !defined;
                if (defined) ctx->skip_mode++;
            } else if (strncmp(directive, "if", 2) == 0) {
                const char* expr = directive + 2;
                int result = evaluate_if_expression(ctx, expr);
                ctx->if_stack[ctx->if_depth++] = result;
                if (!result) ctx->skip_mode++;
            } else if (strncmp(directive, "elif", 4) == 0) {
                // Simple implementation: treat as #else if previous was false
                if (ctx->if_depth > 0) {
                    if (!ctx->if_stack[ctx->if_depth-1]) {
                        ctx->skip_mode--;
                        const char* expr = directive + 4;
                        int result = evaluate_if_expression(ctx, expr);
                        ctx->if_stack[ctx->if_depth-1] = result;
                        if (!result) ctx->skip_mode++;
                    }
                }
            } else if (strncmp(directive, "else", 4) == 0) {
                if (ctx->if_depth > 0) {
                    if (ctx->if_stack[ctx->if_depth-1]) {
                        ctx->skip_mode++;
                    } else {
                        ctx->skip_mode--;
                    }
                    ctx->if_stack[ctx->if_depth-1] = !ctx->if_stack[ctx->if_depth-1];
                }
            } else if (strncmp(directive, "endif", 5) == 0) {
                if (ctx->if_depth > 0) {
                    if (!ctx->if_stack[ctx->if_depth-1]) {
                        ctx->skip_mode--;
                    }
                    ctx->if_depth--;
                }
            } else if (strncmp(directive, "pragma", 6) == 0) {
                // #pragma directives are passed through for later processing
                if (!ctx->skip_mode) {
                    fprintf(ctx->output, "%s\n", trimmed);
                }
            }
        } else {
            // Regular line - expand macros and output
            if (!ctx->skip_mode) {
                char* expanded = preprocessor_expand_macros(ctx, line);
                fprintf(ctx->output, "%s\n", expanded);
                free(expanded);
            }
        }
    }

    return 1;
}

// Destroy preprocessor context
void preprocessor_destroy(PreprocessorContext* ctx) {
    // Free macros
    Macro* m = ctx->macros;
    while (m) {
        Macro* next = m->next;
        free(m->name);
        free(m->replacement);
        if (m->type == MACRO_FUNCTION) {
            for (int i = 0; i < m->param_count; i++) {
                free(m->params[i]);
            }
            free(m->params);
        }
        free(m);
        m = next;
    }

    // Free include paths
    for (int i = 0; i < ctx->include_path_count; i++) {
        free(ctx->include_paths[i]);
    }
    free(ctx->include_paths);

    free(ctx);
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ast.h"
#include "codegen.h"
#include "preprocessor.h"

extern int yyparse(void);
extern FILE* yyin;
extern ASTNode* ast_root;

typedef struct {
    char** input_files;
    int input_count;
    char* output_file;
    char** include_paths;
    int include_count;
    char** defines;
    int define_count;
    int compile_only;      /* -c flag: compile to .asm only */
    int preprocess_only;   /* -E flag: preprocess only */
    int link;              /* Link and assemble to .bin */
} CompilerOptions;

void print_usage(const char* prog_name) {
    fprintf(stderr, "DEF88186 C Compiler - ZeroPoint Fantasy Console\n\n");
    fprintf(stderr, "Usage: %s [options] <input.c> [<input2.c> ...]\n\n", prog_name);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -o <file>      Output file name\n");
    fprintf(stderr, "  -c             Compile to assembly (.asm) only (default)\n");
    fprintf(stderr, "  -S             Same as -c\n");
    fprintf(stderr, "  -E             Preprocess only, output to stdout\n");
    fprintf(stderr, "  -I <path>      Add include search path\n");
    fprintf(stderr, "  -D <macro>     Define preprocessor macro\n");
    fprintf(stderr, "  -D <macro>=<v> Define preprocessor macro with value\n");
    fprintf(stderr, "  -bin           Compile, link, and assemble to binary (.bin)\n");
    fprintf(stderr, "  -h, --help     Show this help message\n\n");
    fprintf(stderr, "Examples:\n");
    fprintf(stderr, "  %s main.c                    # Compile to main.asm\n", prog_name);
    fprintf(stderr, "  %s main.c -o prog.asm        # Compile to prog.asm\n", prog_name);
    fprintf(stderr, "  %s file1.c file2.c -bin      # Compile and link to a.out.bin\n", prog_name);
    fprintf(stderr, "  %s -DDEBUG -I./include main.c # With defines and includes\n", prog_name);
}

void parse_args(int argc, char** argv, CompilerOptions* opts) {
    int i;
    opts->input_files = malloc(sizeof(char*) * argc);
    opts->input_count = 0;
    opts->output_file = NULL;
    opts->include_paths = malloc(sizeof(char*) * 32);
    opts->include_count = 0;
    opts->defines = malloc(sizeof(char*) * 32);
    opts->define_count = 0;
    opts->compile_only = 1;  /* Default: compile to .asm */
    opts->preprocess_only = 0;
    opts->link = 0;

    for (i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
                opts->output_file = argv[++i];
            } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "-S") == 0) {
                opts->compile_only = 1;
                opts->link = 0;
            } else if (strcmp(argv[i], "-E") == 0) {
                opts->preprocess_only = 1;
            } else if (strcmp(argv[i], "-I") == 0 && i + 1 < argc) {
                opts->include_paths[opts->include_count++] = argv[++i];
            } else if (strncmp(argv[i], "-I", 2) == 0) {
                opts->include_paths[opts->include_count++] = argv[i] + 2;
            } else if (strcmp(argv[i], "-D") == 0 && i + 1 < argc) {
                opts->defines[opts->define_count++] = argv[++i];
            } else if (strncmp(argv[i], "-D", 2) == 0) {
                opts->defines[opts->define_count++] = argv[i] + 2;
            } else if (strcmp(argv[i], "-bin") == 0) {
                opts->link = 1;
                opts->compile_only = 0;
            } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
                print_usage(argv[0]);
                exit(0);
            } else {
                fprintf(stderr, "Unknown option: %s\n", argv[i]);
                print_usage(argv[0]);
                exit(1);
            }
        } else {
            opts->input_files[opts->input_count++] = argv[i];
        }
    }

    if (opts->input_count == 0) {
        fprintf(stderr, "Error: No input files specified\n\n");
        print_usage(argv[0]);
        exit(1);
    }
}

/* Preprocess a file and return temp file path */
char* preprocess_file(const char* input_file, CompilerOptions* opts) {
    FILE* input;
    char* temp_file;
    int i;

    input = fopen(input_file, "r");
    if (!input) {
        fprintf(stderr, "Error: Cannot open input file '%s'\n", input_file);
        return NULL;
    }

    temp_file = malloc(256);
    if (opts->preprocess_only) {
        /* Output to stdout */
        PreprocessorContext* pp_ctx = preprocessor_create(input, stdout);

        /* Add include paths */
        for (i = 0; i < opts->include_count; i++) {
            preprocessor_add_include_path(pp_ctx, opts->include_paths[i]);
        }

        /* Add macro definitions */
        for (i = 0; i < opts->define_count; i++) {
            char* define = strdup(opts->defines[i]);
            char* eq = strchr(define, '=');
            if (eq) {
                *eq = '\0';
                preprocessor_define(pp_ctx, define, eq + 1);
            } else {
                preprocessor_define(pp_ctx, define, "1");
            }
            free(define);
        }

        if (!preprocessor_run(pp_ctx)) {
            fclose(input);
            preprocessor_destroy(pp_ctx);
            return NULL;
        }

        fclose(input);
        preprocessor_destroy(pp_ctx);
        exit(0);  /* Done with preprocessing */
    } else {
        /* Create temp file - extract base name from input path */
        const char* base_name = strrchr(input_file, '/');
        if (base_name) {
            base_name++;  /* Skip the '/' */
        } else {
            base_name = input_file;
        }
        snprintf(temp_file, 256, "/tmp/cc_%d_%s.i", getpid(), base_name);

        FILE* temp = fopen(temp_file, "w");
        if (!temp) {
            fprintf(stderr, "Error: Cannot create temp file '%s'\n", temp_file);
            fclose(input);
            free(temp_file);
            return NULL;
        }

        PreprocessorContext* pp_ctx = preprocessor_create(input, temp);

        /* Add include paths */
        for (i = 0; i < opts->include_count; i++) {
            preprocessor_add_include_path(pp_ctx, opts->include_paths[i]);
        }

        /* Add macro definitions */
        for (i = 0; i < opts->define_count; i++) {
            char* define = strdup(opts->defines[i]);
            char* eq = strchr(define, '=');
            if (eq) {
                *eq = '\0';
                preprocessor_define(pp_ctx, define, eq + 1);
            } else {
                preprocessor_define(pp_ctx, define, "1");
            }
            free(define);
        }

        if (!preprocessor_run(pp_ctx)) {
            fclose(input);
            fclose(temp);
            preprocessor_destroy(pp_ctx);
            unlink(temp_file);
            free(temp_file);
            return NULL;
        }

        fclose(input);
        fclose(temp);
        preprocessor_destroy(pp_ctx);

        return temp_file;
    }
}

/* Compile preprocessed file to assembly */
int compile_to_asm(const char* input_file, const char* output_file) {
    yyin = fopen(input_file, "r");
    if (!yyin) {
        fprintf(stderr, "Error: Cannot open preprocessed file '%s'\n", input_file);
        return 0;
    }

    printf("Parsing %s...\n", input_file);
    if (yyparse() != 0) {
        fprintf(stderr, "Parse failed\n");
        fclose(yyin);
        return 0;
    }
    fclose(yyin);

    if (!ast_root) {
        fprintf(stderr, "Error: No AST generated\n");
        return 0;
    }

    FILE* output = fopen(output_file, "w");
    if (!output) {
        fprintf(stderr, "Error: Cannot open output file '%s'\n", output_file);
        ast_free(ast_root);
        return 0;
    }

    printf("Generating assembly to %s...\n", output_file);
    codegen_program(ast_root, output);
    fclose(output);

    ast_free(ast_root);
    ast_root = NULL;

    return 1;
}

int main(int argc, char** argv) {
    CompilerOptions opts;
    char** asm_files;
    int asm_count;
    int i;

    parse_args(argc, argv, &opts);

    asm_files = malloc(sizeof(char*) * opts.input_count);
    asm_count = 0;

    /* Process each input file */
    for (i = 0; i < opts.input_count; i++) {
        const char* input_file = opts.input_files[i];

        /* Preprocess */
        printf("Preprocessing %s...\n", input_file);
        char* preprocessed = preprocess_file(input_file, &opts);
        if (!preprocessed) {
            fprintf(stderr, "Preprocessing failed for %s\n", input_file);
            return 1;
        }

        /* Determine output filename */
        char* output_asm = malloc(256);
        if (opts.input_count == 1 && opts.output_file) {
            /* Single file with specified output */
            strcpy(output_asm, opts.output_file);
        } else {
            /* Multiple files or no output specified */
            const char* base = input_file;
            const char* ext = strrchr(base, '.');
            size_t base_len = ext ? (size_t)(ext - base) : strlen(base);
            strncpy(output_asm, base, base_len);
            output_asm[base_len] = '\0';
            strcat(output_asm, ".asm");
        }

        /* Compile to assembly */
        if (!compile_to_asm(preprocessed, output_asm)) {
            fprintf(stderr, "Compilation failed for %s\n", input_file);
            unlink(preprocessed);
            free(preprocessed);
            free(output_asm);
            return 1;
        }

        /* Clean up temp file */
        unlink(preprocessed);
        free(preprocessed);

        asm_files[asm_count++] = output_asm;
    }

    printf("Compilation successful!\n");

    /* Link and assemble if requested */
    if (opts.link) {
        char* final_asm = malloc(256);
        char* final_bin = malloc(256);

        if (opts.output_file) {
            strcpy(final_bin, opts.output_file);
        } else {
            strcpy(final_bin, "a.out.bin");
        }

        /* Remove .bin extension to get base name */
        char* ext = strrchr(final_bin, '.');
        size_t base_len = ext ? (size_t)(ext - final_bin) : strlen(final_bin);
        strncpy(final_asm, final_bin, base_len);
        final_asm[base_len] = '\0';
        strcat(final_asm, ".asm");

        /* Concatenate all assembly files */
        FILE* combined = fopen(final_asm, "w");
        if (!combined) {
            fprintf(stderr, "Error: Cannot create combined assembly file '%s'\n", final_asm);
            return 1;
        }

        fprintf(combined, "; Combined assembly from %d source file(s)\n\n", asm_count);

        for (i = 0; i < asm_count; i++) {
            FILE* asm_file = fopen(asm_files[i], "r");
            if (asm_file) {
                fprintf(combined, "; ========== %s ==========\n", asm_files[i]);
                char line[4096];
                while (fgets(line, sizeof(line), asm_file)) {
                    fputs(line, combined);
                }
                fclose(asm_file);
                fprintf(combined, "\n");
            }
        }

        fclose(combined);

        printf("Linking to %s...\n", final_asm);
        printf("Assembling with cpuasm...\n");

        /* Call cpuasm */
        char cmd[512];
        snprintf(cmd, sizeof(cmd), "../cpuasm %s %s", final_asm, final_bin);
        int ret = system(cmd);

        if (ret != 0) {
            fprintf(stderr, "Error: cpuasm failed\n");
            free(final_asm);
            free(final_bin);
            return 1;
        }

        printf("Binary created: %s\n", final_bin);

        free(final_asm);
        free(final_bin);
    } else {
        printf("Assembly files created:\n");
        for (i = 0; i < asm_count; i++) {
            printf("  %s\n", asm_files[i]);
        }
        printf("\nTo assemble, run:\n");
        if (asm_count == 1) {
            printf("  ../cpuasm %s output.bin\n", asm_files[0]);
        } else {
            printf("  cat %s", asm_files[0]);
            for (i = 1; i < asm_count; i++) {
                printf(" %s", asm_files[i]);
            }
            printf(" > combined.asm\n");
            printf("  ../cpuasm combined.asm output.bin\n");
        }
    }

    /* Clean up */
    for (i = 0; i < asm_count; i++) {
        free(asm_files[i]);
    }
    free(asm_files);
    free(opts.input_files);
    free(opts.include_paths);
    free(opts.defines);

    return 0;
}

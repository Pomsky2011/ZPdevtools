#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "codegen.h"

extern int yyparse();
extern FILE* yyin;
extern ASTNode* ast_root;

void print_usage(const char* prog_name) {
    fprintf(stderr, "Usage: %s <input.c> [-o <output.asm>]\n", prog_name);
    fprintf(stderr, "  If no output file is specified, uses input filename with .asm extension\n");
}

int main(int argc, char** argv) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    const char* input_file = argv[1];
    char* output_file = NULL;

    // Parse command line arguments
    if (argc >= 4 && strcmp(argv[2], "-o") == 0) {
        output_file = argv[3];
    } else {
        // Generate output filename from input filename
        const char* ext = strrchr(input_file, '.');
        size_t base_len = ext ? (size_t)(ext - input_file) : strlen(input_file);

        output_file = malloc(base_len + 5);  // +5 for ".asm" + null
        strncpy(output_file, input_file, base_len);
        strcpy(output_file + base_len, ".asm");
    }

    // Open input file
    yyin = fopen(input_file, "r");
    if (!yyin) {
        fprintf(stderr, "Error: Cannot open input file '%s'\n", input_file);
        return 1;
    }

    // Parse input
    printf("Parsing %s...\n", input_file);
    if (yyparse() != 0) {
        fprintf(stderr, "Parse failed\n");
        fclose(yyin);
        return 1;
    }
    fclose(yyin);

    if (!ast_root) {
        fprintf(stderr, "Error: No AST generated\n");
        return 1;
    }

    // Open output file
    FILE* output = fopen(output_file, "w");
    if (!output) {
        fprintf(stderr, "Error: Cannot open output file '%s'\n", output_file);
        ast_free(ast_root);
        return 1;
    }

    // Generate code
    printf("Generating assembly to %s...\n", output_file);
    codegen_program(ast_root, output);
    fclose(output);

    // Clean up
    ast_free(ast_root);

    printf("Compilation successful!\n");
    printf("Next step: Assemble with: ./cpuasm %s\n", output_file);

    if (argc < 4) {
        free(output_file);
    }

    return 0;
}

# DEF88186 C Compiler

![C](https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white)
![Flex](https://img.shields.io/badge/Flex-Lexer-blue?style=for-the-badge)
![Bison](https://img.shields.io/badge/Bison-Parser-orange?style=for-the-badge)
![Assembly](https://img.shields.io/badge/Assembly-DEF88186-red?style=for-the-badge)
![Status](https://img.shields.io/badge/Status-Production-success?style=for-the-badge)

A complete C-to-assembly compiler targeting the DEF88186 hybrid CPU architecture for the ZeroPoint Fantasy Console. This compiler bridges high-level C programming with the low-level 65C816/8086-inspired instruction set, enabling efficient game development and system programming.

## Overview

The DEF88186 C Compiler is a full-featured compiler toolchain that translates a practical subset of C into native DEF88186 assembly language. Built with industry-standard tools (Flex & Bison), it provides a robust compilation pipeline with lexical analysis, syntax parsing, Abstract Syntax Tree (AST) construction, and optimized code generation targeting the 16-bit DEF88186 CPU.

### Key Highlights

- **Full Compiler Pipeline**: Lexer → Parser → AST → Code Generator → Assembly Output
- **Hardware-Aware**: Leverages DEF88186's hardware multiply/divide instructions for performance
- **ABI Compliant**: Follows standard DEF88186 calling conventions for interoperability
- **Stack Management**: Automatic stack frame allocation and cleanup
- **Register Optimization**: Efficient use of A, X, Y registers and direct page addressing

## System Architecture

```mermaid
flowchart TB
    subgraph Input["Input Layer"]
        C[C Source Code<br/>*.c files]
        style C fill:#e1f5ff
    end

    subgraph Compiler["DEF88186 C Compiler Pipeline"]
        LEX[Lexical Analysis<br/>Flex Lexer<br/>lexer.l]
        PARSE[Syntax Analysis<br/>Bison Parser<br/>parser.y]
        AST[Abstract Syntax Tree<br/>AST Builder<br/>ast.c/h]
        SYMTAB[Symbol Table<br/>Variable Tracking<br/>Scope Management]
        CODEGEN[Code Generator<br/>DEF88186 Backend<br/>codegen.c]

        LEX -->|Tokens| PARSE
        PARSE -->|Parse Tree| AST
        AST -->|Type Info| SYMTAB
        AST --> CODEGEN
        SYMTAB --> CODEGEN

        style LEX fill:#ffe1e1
        style PARSE fill:#fff4e1
        style AST fill:#e1ffe1
        style SYMTAB fill:#e1e1ff
        style CODEGEN fill:#ffe1ff
    end

    subgraph Assembler["Assembly Layer"]
        ASM[DEF88186 Assembly<br/>*.asm files]
        CPUASM[CPU Assembler<br/>cpuasm]
        BIN[Binary Output<br/>*.bin files]

        ASM --> CPUASM
        CPUASM --> BIN

        style ASM fill:#fff9e1
        style CPUASM fill:#e1f9ff
        style BIN fill:#f9e1ff
    end

    subgraph Runtime["ZeroPoint Runtime Environment"]
        CPU[DEF88186 CPU<br/>16-bit Hybrid Processor<br/>65C816 + 8086 Extensions]
        PPU[PPU<br/>Graphics Processor<br/>64 MHz Microcode]
        APU[APU<br/>Audio Processor<br/>4.2 MHz RISC]
        MEM[Memory System<br/>16 MB Addressable<br/>24-bit Addressing]

        CPU -->|Commands| PPU
        CPU -->|Commands| APU
        CPU <-->|R/W| MEM

        style CPU fill:#ffcccc
        style PPU fill:#ccffcc
        style APU fill:#ccccff
        style MEM fill:#ffffcc
    end

    C --> LEX
    CODEGEN -->|Generated ASM| ASM
    BIN -->|Execute| CPU

    subgraph Features["Compiler Features"]
        F1[Hardware Multiply/Divide<br/>MUL, DIV instructions]
        F2[Stack Frame Management<br/>Auto allocation/cleanup]
        F3[Register Allocation<br/>A, X, Y optimized usage]
        F4[Control Flow<br/>if/else, while, for loops]
        F5[Function Calls<br/>ABI-compliant calling]
        F6[Expression Evaluation<br/>Arithmetic, logical, bitwise]

        style F1 fill:#e8f4f8
        style F2 fill:#e8f4f8
        style F3 fill:#e8f4f8
        style F4 fill:#e8f4f8
        style F5 fill:#e8f4f8
        style F6 fill:#e8f4f8
    end

    CODEGEN -.->|Implements| F1
    CODEGEN -.->|Implements| F2
    CODEGEN -.->|Implements| F3
    CODEGEN -.->|Implements| F4
    CODEGEN -.->|Implements| F5
    CODEGEN -.->|Implements| F6

    classDef default font-size:11px
```

## Compilation Flow

```mermaid
sequenceDiagram
    participant User
    participant Compiler as def88186cc
    participant Lexer as Flex Lexer
    participant Parser as Bison Parser
    participant AST as AST Builder
    participant CodeGen as Code Generator
    participant Output as Assembly File
    participant CPUasm as cpuasm
    participant Binary as Binary Output

    User->>Compiler: ./def88186cc program.c
    Compiler->>Lexer: Read source code
    Lexer->>Lexer: Tokenize (keywords, identifiers, operators)
    Lexer->>Parser: Stream of tokens
    Parser->>Parser: Build parse tree (grammar rules)
    Parser->>AST: Create AST nodes
    AST->>AST: Build symbol table
    AST->>CodeGen: Pass AST root + symbols
    CodeGen->>CodeGen: Allocate registers
    CodeGen->>CodeGen: Generate DEF88186 instructions
    CodeGen->>CodeGen: Manage stack frames
    CodeGen->>Output: Write .asm file
    Output-->>Compiler: program.asm created
    Compiler-->>User: Compilation successful!

    User->>CPUasm: ./cpuasm program.asm
    CPUasm->>CPUasm: Assemble instructions
    CPUasm->>Binary: program.bin
    Binary-->>User: Ready to execute on DEF88186
```

## Features

- Compiles a subset of C to DEF88186 assembly
- Supports basic data types: `int` (16-bit), `char` (8-bit), `void`
- Functions with parameters and return values
- Local variables and function arguments
- Control flow: `if/else`, `while`, `for`
- Arithmetic and logical operations
- Following DEF88186 calling conventions

## Supported C Subset

### Data Types
- `int` - 16-bit signed integer
- `char` - 8-bit signed character
- `void` - no return value
- Pointers (basic support)

### Operators
- Arithmetic: `+`, `-`, `*`, `/`, `%`
- Comparison: `==`, `!=`, `<`, `>`, `<=`, `>=`
- Logical: `&&`, `||`, `!`
- Bitwise: `&`, `|`, `^`, `~`, `<<`, `>>`
- Assignment: `=`

### Control Flow
- `if (expr) stmt`
- `if (expr) stmt else stmt`
- `while (expr) stmt`
- `for (init; cond; incr) stmt`
- `return expr;`

### Functions
```c
int add(int a, int b) {
    return a + b;
}
```

### Variables
- Local variables
- Function parameters
- Global variables (basic support)

## Building

```bash
cd c_compiler
make
```

**Requirements:**
- GCC or Clang C compiler
- Flex (lexical analyzer generator)
- Bison (parser generator)
- Make build system

## Usage

```bash
# Compile C source to assembly
./def88186cc input.c -o output.asm

# Auto-generate output filename (input.asm)
./def88186cc input.c

# Assemble to binary
./cpuasm output.asm output.bin
```

## Examples

See `examples/` directory for sample programs:

**test1.c** - Function calls and recursion:
```c
int factorial(int n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}
```

**test2.c** - Control flow and loops:
```c
int sum_to_n(int n) {
    int sum = 0;
    int i = 1;
    while (i <= n) {
        sum = sum + i;
        i = i + 1;
    }
    return sum;
}
```

## Architecture Notes

The compiler follows DEF88186 calling conventions:
- First 3 parameters: A, X, Y registers
- Additional parameters: stack (right-to-left)
- Return value: A register
- 16-bit mode by default (REP #$30)
- Caller-saved: A, X, Y
- Callee-saved: D, DB

### Code Generation Strategy

1. **Stack Frame Setup**: Automatically allocates space for local variables
2. **Register Allocation**:
   - A: Primary accumulator, return values, expression evaluation
   - X: Second parameter, temporary storage, array indexing
   - Y: Third parameter, secondary indexing
3. **Hardware Instructions**: Uses DEF88186 `MUL` and `DIV` for efficient arithmetic
4. **Label Management**: Generates unique labels for control flow structures

## Compiler Internals

| Component | File | Description |
|-----------|------|-------------|
| Lexer | `lexer.l` | Tokenizes C source code (Flex) |
| Parser | `parser.y` | Builds parse tree (Bison) |
| AST | `ast.h/c` | Abstract Syntax Tree implementation |
| Symbol Table | `codegen.c` | Variable and scope tracking |
| Code Generator | `codegen.c` | DEF88186 assembly emission |
| Driver | `main.c` | Compiler entry point |

## Limitations

- No structs/unions
- No floating point
- Limited pointer arithmetic
- No type qualifiers (const, volatile)
- No preprocessor (use cpp separately)
- No arrays (coming soon)
- No inline assembly

## Performance Considerations

- **Hardware Multiply/Divide**: 8-13 cycles vs 100+ cycles for software implementation
- **Register Parameters**: First 3 parameters avoid stack overhead
- **Direct Page Access**: Local variables use fast DP addressing when possible
- **Tail Call Optimization**: Not yet implemented

## Future Enhancements

- [ ] Array support with pointer arithmetic
- [ ] Struct and union support
- [ ] Preprocessor integration
- [ ] Optimization passes (constant folding, dead code elimination)
- [ ] Inline assembly support
- [ ] Better error messages with line/column numbers
- [ ] Optimization flags (-O1, -O2, -O3)

## Contributing

Part of the ZeroPoint Fantasy Console project. Built with Claude Code.

## License

See main project LICENSE file.

---

**Built with [Claude Code](https://claude.com/claude-code)**

Co-Authored-By: Claude <noreply@anthropic.com>

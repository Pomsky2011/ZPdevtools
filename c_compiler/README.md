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
%%{init: {'theme':'base', 'themeVariables': { 'primaryColor':'#4A90E2','primaryTextColor':'#fff','primaryBorderColor':'#2E5C8A','lineColor':'#F39C12','secondaryColor':'#E74C3C','tertiaryColor':'#27AE60','noteBkgColor':'#FFF9C4','noteTextColor':'#000','actorBkg':'#3498DB','actorBorder':'#2C3E50','actorTextColor':'#fff','actorLineColor':'#34495E','signalColor':'#2C3E50','signalTextColor':'#000','labelBoxBkgColor':'#ECF0F1','labelBoxBorderColor':'#95A5A6','labelTextColor':'#2C3E50','loopTextColor':'#fff','activationBorderColor':'#666','activationBkgColor':'#f4f4f4','sequenceNumberColor':'#fff'}}}%%
sequenceDiagram
    autonumber

    box rgb(52, 152, 219) User Input
    participant User
    end

    box rgb(231, 76, 60) Compiler Frontend
    participant Compiler as def88186cc<br/>Driver
    participant Lexer as Flex Lexer<br/>Tokenizer
    participant Parser as Bison Parser<br/>Syntax Analysis
    end

    box rgb(46, 204, 113) Compiler Backend
    participant AST as AST Builder<br/>Tree Construction
    participant CodeGen as Code Generator<br/>DEF88186 Backend
    end

    box rgb(241, 196, 15) Output & Assembly
    participant Output as Assembly<br/>*.asm File
    participant CPUasm as cpuasm<br/>Assembler
    participant Binary as Binary<br/>*.bin Executable
    end

    User->>+Compiler: ./def88186cc program.c
    Note over User,Compiler: Step 1: Start Compilation

    Compiler->>+Lexer: Read C source code
    Note over Lexer: Tokenization Phase
    Lexer->>Lexer: Scan characters<br/>Identify tokens<br/>(keywords, identifiers, operators)
    Lexer->>-Parser: Token stream

    Note over Parser: Parsing Phase
    Parser->>+Parser: Apply grammar rules<br/>Check syntax<br/>Build parse tree
    Parser->>-AST: Syntax tree nodes

    Note over AST: Semantic Analysis
    AST->>AST: Build symbol table<br/>Type checking<br/>Scope resolution
    AST->>+CodeGen: AST + Symbol Table

    Note over CodeGen: Code Generation Phase
    CodeGen->>CodeGen: Register allocation<br/>(A, X, Y registers)
    CodeGen->>CodeGen: Generate DEF88186<br/>instructions
    CodeGen->>CodeGen: Stack frame<br/>management
    CodeGen->>-Output: Write assembly code

    Output-->>Compiler: program.asm created ✓
    Compiler-->>-User: ✓ Compilation successful!

    rect rgb(200, 230, 200)
    Note over User,Binary: Assembly Phase (Separate Tool)
    User->>+CPUasm: ./cpuasm program.asm
    CPUasm->>CPUasm: Parse assembly<br/>Encode instructions<br/>Resolve labels
    CPUasm->>-Binary: Binary machine code
    Binary-->>User: ✓ program.bin ready!
    end
```

## Features

- **Complete C Compiler**: Compiles practical C subset to DEF88186 assembly
- **Data Types**: `int` (16-bit), `char` (8-bit), `void`, `struct`, arrays, pointers
- **Functions**: Parameters, return values, recursion, calling conventions
- **Variables**: Local, global, function parameters with proper scoping
- **Control Flow**: `if/else`, `while`, `for`, `break`, `continue`, `return`
- **Operators**: Arithmetic, logical, bitwise, comparison, compound assignments, increment/decrement
- **Pointers**: Address-of (`&`), dereference (`*`), multi-level pointers
- **Structs**: Member access (`.`), pointer access (`->`), member assignment
- **Arrays**: Fixed-size arrays with subscript access and assignment
- **Hardware Optimization**: Automatic `LOOP`/`LPEND` for counted loops, hardware `MUL`/`DIV`
- **ABI Compliant**: Follows DEF88186 calling conventions for interoperability

## Supported C Subset

### Data Types
- `int` - 16-bit signed integer
- `char` - 8-bit signed character
- `void` - no return value
- `struct` - Structured data types with member access
- Arrays - Fixed-size arrays (e.g., `int arr[10]`)
- Pointers - Single and multi-level pointers with dereference

### Operators
- Arithmetic: `+`, `-`, `*`, `/`, `%`
- Comparison: `==`, `!=`, `<`, `>`, `<=`, `>=`
- Logical: `&&`, `||`, `!`
- Bitwise: `&`, `|`, `^`, `~`, `<<`, `>>`
- Assignment: `=`
- Compound Assignment: `+=`, `-=`, `*=`, `/=`, `%=`, `&=`, `|=`, `^=`
- Increment/Decrement: `++`, `--` (prefix and postfix)
- Pointer: `&` (address-of), `*` (dereference)
- Member Access: `.` (struct), `->` (pointer to struct)

### Control Flow
- `if (expr) stmt`
- `if (expr) stmt else stmt`
- `while (expr) stmt`
- `for (init; cond; incr) stmt`
- `break` - Exit loop early
- `continue` - Skip to next iteration
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
- **Arrays**: Fixed-size local arrays with subscript access

### Arrays
```c
int main() {
    int arr[10];      // Declare array
    arr[0] = 42;      // Write to array
    arr[5] = arr[0];  // Read from array
    return arr[5];
}
```

**Array Features:**
- Declaration: `type identifier[size];`
- Subscript access: `arr[index]`
- Assignment: `arr[index] = value;`
- Stack-allocated (local arrays only)
- Supports variable indexing

### Pointers
```c
int main() {
    int x = 42;
    int *ptr = &x;    // Get address of x
    int y = *ptr;     // Dereference pointer
    *ptr = 100;       // Modify through pointer
    return y;
}
```

**Pointer Features:**
- Address-of operator: `&variable`
- Dereference operator: `*pointer`
- Multi-level pointers: `int **ptr`
- Pointer arithmetic: Basic support
- Pointer parameters and return values

### Structs
```c
struct Point {
    int x;
    int y;
};

int main() {
    struct Point p;
    p.x = 10;         // Member assignment
    p.y = 20;

    struct Point *ptr = &p;
    ptr->x = 30;      // Pointer member access

    return p.x + p.y;
}
```

**Struct Features:**
- Member access: `struct.member`
- Pointer member access: `ptr->member`
- Member assignment: `p.x = value`
- Nested member access supported
- Stack-allocated structs

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
3. **Hardware Instructions**:
   - Uses DEF88186 `MUL` and `DIV` for efficient arithmetic
   - **LOOP/LPEND optimization**: Automatically detects simple counted loops and uses hardware loop instructions
4. **Label Management**: Generates unique labels for control flow structures

### Hardware Loop Optimization

The compiler intelligently detects simple counted loops and uses the DEF88186's hardware `LOOP`/`LPEND` instructions:

**Pattern Detected:**
```c
for (i = 0; i < N; i = i + 1)  // N must be constant
```

**Generated Code:**
```asm
LOOP #N          ; Hardware loop counter
    ; loop body
    i = i + 1    ; increment
LPEND            ; Auto-decrement and branch
```

**Benefits:**
- Faster execution (hardware-managed counter)
- Smaller code size (no manual labels/branches)
- More efficient than manual `CMP`/`BRA` loops

**Falls back to manual loops when:**
- Loop bound is not a constant
- Complex condition (e.g., `i < j` where both are variables)
- Non-unit increment (e.g., `i = i + 2`)

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

- No floating point arithmetic
- No type qualifiers (const, volatile, static)
- No preprocessor (use cpp separately)
- No inline assembly
- No multi-dimensional arrays
- No array initialization lists
- No unions
- No function pointers
- No variadic functions (printf-style)
- Pointer arithmetic is basic (no complex expressions)

## Performance Considerations

- **Hardware Multiply/Divide**: 8-13 cycles vs 100+ cycles for software implementation
- **Hardware Loops**: `LOOP`/`LPEND` instructions optimize simple counted loops
- **Register Parameters**: First 3 parameters avoid stack overhead
- **Direct Page Access**: Local variables use fast DP addressing when possible
- **Compound Assignments**: Efficiently compiled using read-modify-write patterns
- **Tail Call Optimization**: Not yet implemented

## Future Enhancements

- [x] Array support ✓ **DONE!**
- [x] Pointer support with pointer arithmetic ✓ **DONE!**
- [x] Struct support with member access ✓ **DONE!**
- [x] Increment/decrement operators (++/--) ✓ **DONE!**
- [x] Compound assignment operators (+=, -=, etc.) ✓ **DONE!**
- [x] Break and continue statements ✓ **DONE!**
- [ ] Multi-dimensional arrays
- [ ] Array initialization lists
- [ ] Union support
- [ ] Function pointers
- [ ] Switch/case statements
- [ ] Ternary operator (?:)
- [ ] Comma operator
- [ ] sizeof operator
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

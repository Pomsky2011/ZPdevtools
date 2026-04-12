#!/usr/bin/env python3
"""
Generate ZeroPoint PPU assembly for a visible gradient grid.
Draws pixels in a grid pattern to make gradient visible.
"""

def build_value(reg, value, using_reg14=True):
    """Generate assembly to build a value in a register efficiently."""
    code = []
    code.append(f"    CLR {reg}")

    if value == 0:
        return code

    # Build using additions of R14 (16) and multiplication
    if value % 16 == 0 and value <= 256:
        # Multiple of 16
        count = value // 16
        for i in range(count):
            code.append(f"    ADD {reg}, R14")
    elif value == 32:
        code.append(f"    ADD {reg}, R14")
        code.append(f"    ADD {reg}, R14")
    elif value == 64:
        code.append(f"    ADD {reg}, R14")
        code.append(f"    MUL {reg}, R14")
        code.append(f"    DEC {reg}")
        code.append(f"    DEC {reg}")
        code.append(f"    DEC {reg}")
        code.append(f"    DEC {reg}")
        code.append(f"    DEC {reg}")
        code.append(f"    DEC {reg}")
        code.append(f"    DEC {reg}")
        code.append(f"    DEC {reg}")
    elif value == 96:
        code.append(f"    ADD {reg}, R14")
        code.append(f"    ADD {reg}, R14")
        code.append(f"    ADD {reg}, R14")
        code.append(f"    ADD {reg}, R14")
        code.append(f"    ADD {reg}, R14")
        code.append(f"    ADD {reg}, R14")
    elif value == 128:
        code.append(f"    ADD {reg}, R14")
        code.append(f"    MUL {reg}, R14")
        for i in range(8):
            code.append(f"    DEC {reg}")
    elif value == 160:
        code.append(f"    ADD {reg}, R14")
        code.append(f"    ADD {reg}, R14")
        code.append(f"    ADD {reg}, R14")
        code.append(f"    ADD {reg}, R14")
        code.append(f"    ADD {reg}, R14")
        code.append(f"    ADD {reg}, R14")
        code.append(f"    ADD {reg}, R14")
        code.append(f"    ADD {reg}, R14")
        code.append(f"    ADD {reg}, R14")
        code.append(f"    ADD {reg}, R14")
    elif value == 192:
        code.append(f"    ADD {reg}, R14")
        code.append(f"    ADD {reg}, R14")
        code.append(f"    ADD {reg}, R14")
        code.append(f"    ADD {reg}, R14")
        code.append(f"    ADD {reg}, R14")
        code.append(f"    ADD {reg}, R14")
        code.append(f"    ADD {reg}, R14")
        code.append(f"    ADD {reg}, R14")
        code.append(f"    ADD {reg}, R14")
        code.append(f"    ADD {reg}, R14")
        code.append(f"    ADD {reg}, R14")
        code.append(f"    ADD {reg}, R14")
    elif value == 224:
        code.append(f"    ADD {reg}, R14")
        code.append(f"    MUL {reg}, R14")
        for i in range(32):
            code.append(f"    DEC {reg}")
    else:
        # Just use INC for small values
        for i in range(min(value, 20)):
            code.append(f"    INC {reg}")

    return code

print("; Auto-generated gradient grid demo")
print("; Draws 8x8 grid of pixels (every 32 units) with gradient colors")
print()
print("init:")
print("    SETRENDMOD 1")
print()
print("    ; Build R14 = 16, R7 = 256")
print("    CLR R14")
print("    INC R14")
print("    INC R14")
print("    MUL R14, R14")
print("    MUL R14, R14")
print("    CLR R7")
print("    ADD R7, R14")
print("    MUL R7, R14")
print()
print("    ; Build R6 = 255")
print("    CLR R1")
print("    INC R1")
print("    CLR R6")
print("    ADD R6, R14")
print("    MUL R6, R14")
print("    SUB R6, R1")
print()
print("    ; Green = 128")
print("    CLR R2")
print("    ADD R2, R14")
for i in range(8):
    print("    DEC R2")
print("    MUL R2, R14")
print()

# Draw 8x8 grid with 32-pixel spacing
for gy in range(8):
    for gx in range(8):
        x = gx * 32
        y = gy * 32

        print(f"    ; Pixel at ({x}, {y}) - Red={y}, Blue={x}")

        # Build X in R0
        for line in build_value("R0", x):
            print(line)

        # Build Y in R3
        for line in build_value("R3", y):
            print(line)

        print(f"    SETDP R7")
        print(f"    MOVDP R0")
        print(f"    INC DP")
        print(f"    INC DP")
        print(f"    MOVDP R3")
        print(f"    INC DP")
        print(f"    INC DP")
        print(f"    MOVDP R3  ; Red = Y")
        print(f"    INC DP")
        print(f"    INC DP")
        print(f"    MOVDP R2  ; Green = 128")
        print(f"    INC DP")
        print(f"    INC DP")
        print(f"    MOVDP R0  ; Blue = X")
        print(f"    INC DP")
        print(f"    INC DP")
        print(f"    MOVDP R6  ; Alpha = 255")
        print()

print("    HALT")

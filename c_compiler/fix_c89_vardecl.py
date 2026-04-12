#!/usr/bin/env python3
"""
Fix mixed declarations by moving variable declarations to the start of functions.
This handles cases where variables are used but not declared in the current function scope.
"""

import re
import sys

def find_function_starts(lines):
    """Find all function definitions and their start lines."""
    functions = []
    i = 0
    while i < len(lines):
        line = lines[i]
        # Match function definitions (simplified - looks for { after function name)
        if re.match(r'^(static\s+)?\w+(\s+\**)?\s+\w+\([^)]*\)\s*\{', line):
            functions.append(i)
        elif re.match(r'^(static\s+)?\w+(\s+\**)?(\s+\w+)\([^)]*\)$', line):
            # Function definition spans multiple lines
            if i + 1 < len(lines) and lines[i + 1].strip() == '{':
                functions.append(i + 1)
        i += 1
    return functions

def find_function_end(lines, start):
    """Find the closing brace of a function."""
    brace_count = 0
    for i in range(start, len(lines)):
        brace_count += lines[i].count('{')
        brace_count -= lines[i].count('}')
        if brace_count == 0 and i > start:
            return i
    return len(lines) - 1

def get_used_variables(lines, start, end):
    """Find all variables used in a function."""
    vars_used = set()
    # Common loop variables
    common_vars = ['i', 'j', 'k', 'n', 'm']

    for i in range(start, end + 1):
        line = lines[i]
        # Look for 'for (var =' or references to common loop variables
        for var in common_vars:
            if re.search(r'\b' + var + r'\s*[=<>!]', line) or \
               re.search(r'\b' + var + r'\s*\+\+', line) or \
               re.search(r'\+\+\s*' + var + r'\b', line) or \
               re.search(r'\b' + var + r'\s*--', line) or \
               re.search(r'--\s*' + var + r'\b', line) or \
               re.search(r'for\s*\(\s*' + var + r'\s*=', line):
                vars_used.add(var)

    return vars_used

def has_declaration(lines, start, end, var):
    """Check if a variable is already declared in the function."""
    for i in range(start, end + 1):
        line = lines[i]
        if re.search(r'\b(int|long|short|char|unsigned)\s+' + var + r'\s*[;,=]', line) or \
           re.search(r'\b(int|long|short|char|unsigned)\s+' + var + r'\s*$', line):
            return True
    return False

def fix_declarations(content):
    """Fix variable declarations in functions."""
    lines = content.split('\n')
    function_starts = find_function_starts(lines)

    for func_start in function_starts:
        func_end = find_function_end(lines, func_start)
        vars_used = get_used_variables(lines, func_start, func_end)

        # Find where to insert declarations (after opening { and any existing declarations)
        insert_line = func_start + 1

        # Skip existing variable declarations
        while insert_line < func_end:
            line = lines[insert_line].strip()
            if line and not re.match(r'(int|long|short|char|unsigned|const|static|struct|enum|typedef)\s+', line):
                break
            if line:
                insert_line += 1
            else:
                break

        # Check which variables need to be declared
        to_declare = []
        for var in sorted(vars_used):
            if not has_declaration(lines, func_start, func_end, var):
                to_declare.append(var)

        # Insert declarations
        if to_declare:
            indent = '    '  # Standard 4-space indent
            decl_line = indent + 'int ' + ', '.join(to_declare) + ';'
            lines.insert(insert_line, decl_line)

    return '\n'.join(lines)

def main():
    if len(sys.argv) < 2:
        print("Usage: python fix_c89_vardecl.py <file>")
        sys.exit(1)

    filename = sys.argv[1]

    with open(filename, 'r') as f:
        content = f.read()

    # Fix declarations
    content = fix_declarations(content)

    with open(filename, 'w') as f:
        f.write(content)

    print(f"Fixed variable declarations in {filename}")

if __name__ == '__main__':
    main()

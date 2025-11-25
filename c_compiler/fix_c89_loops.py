#!/usr/bin/env python3
"""
Fix C99 for loop declarations to C89 style.
Moves 'for (int i = ...' declarations out of the for loop.
"""

import re
import sys

def fix_for_loops(content):
    """Fix for loop variable declarations."""
    lines = content.split('\n')
    result = []
    i = 0
    declared_vars = set()

    while i < len(lines):
        line = lines[i]

        # Match for loop with variable declaration
        match = re.match(r'^(\s*)for\s*\(\s*(int|long|short|char|unsigned)\s+(\w+)\s*=', line)
        if match:
            indent = match.group(1)
            var_type = match.group(2)
            var_name = match.group(3)

            # Check if we've already declared this variable
            if var_name not in declared_vars:
                # Add declaration before the for loop
                result.append(f"{indent}{var_type} {var_name};")
                declared_vars.add(var_name)

            # Remove type from for loop
            fixed_line = re.sub(r'for\s*\(\s*(int|long|short|char|unsigned)\s+(\w+)\s*=',
                               r'for (\2 =', line)
            result.append(fixed_line)
        else:
            result.append(line)

        i += 1

    return '\n'.join(result)

def main():
    if len(sys.argv) < 2:
        print("Usage: python fix_c89_loops.py <file>")
        sys.exit(1)

    filename = sys.argv[1]

    with open(filename, 'r') as f:
        content = f.read()

    # Fix for loops
    content = fix_for_loops(content)

    with open(filename, 'w') as f:
        f.write(content)

    print(f"Fixed for loops in {filename}")

if __name__ == '__main__':
    main()

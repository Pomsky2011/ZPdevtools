#!/usr/bin/env python3
"""
Convert C99 code to C89 compliance for ZeroPoint def88186cc compiler.
Converts // comments to /* */ and fixes other common issues.
"""

import re
import sys

def convert_comments(content):
    """Convert // comments to /* */ comments."""
    lines = content.split('\n')
    result = []

    for line in lines:
        # Find // comments (but not in strings)
        match = re.search(r'([^"]*?)//(.*)$', line)
        if match:
            before = match.group(1)
            comment = match.group(2).rstrip()
            line = before + '/*' + comment + ' */'
        result.append(line)

    return '\n'.join(result)

def fix_function_prototypes(content):
    """Fix function prototypes without parameters to have (void)."""
    # Fix function declarations that end with ()
    content = re.sub(r'(\w+\s+\w+)\(\);', r'\1(void);', content)
    return content

def main():
    if len(sys.argv) < 2:
        print("Usage: python convert_c89.py <file>")
        sys.exit(1)

    filename = sys.argv[1]

    with open(filename, 'r') as f:
        content = f.read()

    # Convert comments
    content = convert_comments(content)

    # Fix function prototypes
    content = fix_function_prototypes(content)

    with open(filename, 'w') as f:
        f.write(content)

    print(f"Converted {filename} to C89 compliance")

if __name__ == '__main__':
    main()

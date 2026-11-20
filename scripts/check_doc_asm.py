#!/usr/bin/env python3
"""Assemble every ```cpuasm / ```apuasm / ```ppuasm block in the Markdown docs, so an example can't rot.

  python3 scripts/check_doc_asm.py [file-or-dir ...]     (default: docs/)

A block runs through the matching assembler in executables/ and fails if it reports an error or writes nothing.
Mark a block that is deliberately not a complete program with ```cpuasm skip.
Exit status is 1 if any block fails.
"""
import os
import re
import subprocess
import sys
import tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
TOOLS = {lang: os.path.join(ROOT, "executables", lang) for lang in ("cpuasm", "apuasm", "ppuasm")}
BLOCK = re.compile(r"^```(cpuasm|apuasm|ppuasm)([^\n]*)\n(.*?)^```", re.S | re.M)


def md_files(paths):
    for p in paths:
        if os.path.isdir(p):
            for d, _, names in sorted(os.walk(p)):
                for n in sorted(names):
                    if n.endswith(".md"):
                        yield os.path.join(d, n)
        else:
            yield p


def main():
    paths = sys.argv[1:] or [os.path.join(ROOT, "docs")]
    checked = failed = 0
    for path in md_files(paths):
        text = open(path).read()
        for m in BLOCK.finditer(text):
            lang, flags, code = m.group(1), m.group(2).strip(), m.group(3)
            if "skip" in flags.split():
                continue
            line = text[: m.start()].count("\n") + 2
            checked += 1
            with tempfile.TemporaryDirectory() as d:
                src, out = os.path.join(d, "t.asm"), os.path.join(d, "t.bin")
                open(src, "w").write(code)
                r = subprocess.run([TOOLS[lang], src, out], capture_output=True, text=True, cwd=d)
                ok = r.returncode == 0 and os.path.exists(out) and os.path.getsize(out) > 0
                msg = [l for l in (r.stdout + r.stderr).splitlines() if "rror" in l]
                if ok and msg:
                    ok = False
            if not ok:
                failed += 1
                print("FAIL %s:%d (%s): %s" % (os.path.relpath(path, ROOT), line, lang, "; ".join(msg[:2]) or "no output"))
    print("%d blocks checked, %d failed" % (checked, failed))
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())

#!/usr/bin/env python3
"""Keep the cycle column of cpuasm's INST() table equal to what the emulator measures.

    sync_cpuasm_cycles.py --check    exit 1 and list every disagreement (used by `make check`)
    sync_cpuasm_cycles.py --write    rewrite the wrong numbers in src/cpuasm.c

The reference is spec/DEF88186-measured.tsv (ZeroPoint tools/dump_cpu_table). A cost that depends on the operand (a read
across a page, a taken branch) is listed there as "3/4": the table keeps the cheapest, the cost without the extra cycle.
Block moves cost a cycle per byte and are listed as "var"; they and the opcodes the TSV has no row for are skipped.
"""
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC = os.path.join(ROOT, "src", "cpuasm.c")
TSV = os.path.join(ROOT, "spec", "DEF88186-measured.tsv")
INST = re.compile(r'(INST\("([\w.]+)",\s*(\w+),\s*0x([0-9A-Fa-f]{2}),\s*(\d+),\s*)(\d+)(\);)')


def measured():
    rows = {}
    with open(TSV) as f:
        header = f.readline().split()
        for line in f:
            cols = dict(zip(header, line.rstrip("\n").split("\t")))
            costs = [cols[k] for k in ("c_m1x1", "c_m1x0", "c_m0x1", "c_m0x0")]
            if any(c == "var" for c in costs):
                continue
            rows[int(cols["op"], 16)] = (int(cols["len"]), min(int(x) for c in costs for x in c.split("/")))
    return rows


def main():
    mode = sys.argv[1] if len(sys.argv) > 1 else "--check"
    if mode not in ("--check", "--write"):
        sys.exit(__doc__)
    ref = measured()
    text = open(SRC, newline="").read()
    bad = []

    def fix(m):
        op = int(m.group(4), 16)
        if op not in ref:
            return m.group(0)
        length, cost = ref[op]
        if int(m.group(5)) != length:
            bad.append("%s %s ($%02X): length %s, emulator %d" % (m.group(2), m.group(3), op, m.group(5), length))
        if int(m.group(6)) != cost:
            bad.append("%s %s ($%02X): %s cycles, emulator %d" % (m.group(2), m.group(3), op, m.group(6), cost))
            return m.group(1) + str(cost) + m.group(7)
        return m.group(0)

    new = INST.sub(fix, text)
    for b in bad:
        print(b)
    if mode == "--write":
        if new != text:
            open(SRC, "w", newline="").write(new)
        print("%d entries rewritten" % len(bad))
        return 0
    if bad:
        print("%d disagreements with spec/DEF88186-measured.tsv; run scripts/sync_cpuasm_cycles.py --write" % len(bad))
        return 1
    print("cpuasm's cycle table matches the emulator")
    return 0


sys.exit(main())

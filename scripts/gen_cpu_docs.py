#!/usr/bin/env python3
"""Regenerate docs/cpu/opcode-table.md.

Joins three things, from ZPdevtools/:
  spec/DEF88186.csv            opcode -> mnemonic, alias, name, addressing mode (names only; its Bytes/Flags/Cycles
                               columns are not used: they have blanks and errors)
  spec/DEF88186-measured.tsv   what the emulator really does: length, cycles, flags, stack (ZeroPoint's
                               tools/dump_cpu_table, emulator revision 10)
  executables/cpuasm           each opcode's syntax is assembled and the first byte and length checked

Run from anywhere:  python3 scripts/gen_cpu_docs.py
"""
import csv
import os
import re
import subprocess
import sys
import tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CPUASM = os.path.join(ROOT, "executables", "cpuasm")
OUT = os.path.join(ROOT, "docs", "cpu", "opcode-table.md")

# The CSV has typos and a few spellings cpuasm does not accept; these are the forms that assemble (checked below).
SYNTAX_FIX = {
    "STA _dp_X": "STA dp,X", "JSR (addr,X))": "JSR (addr,X)", "ADC ( dp),Y": "ADC (dp),Y",
    "BRA long": "BRA addr", "BVS long": "BVS addr",        # 16-bit targets in the current bank, not 24-bit
    "LOOP word": "LOOP #word", "SDB byte": "SDB #byte", "PUSH word": "PUSH #word", "BRK": "BRK #sig",
    "INC long": "INC.L addr", "DEC long": "DEC.L addr",     # still a 16-bit DB-relative address, see instructions.md
}

GROUPS = [
    ("Loads and stores", "LDA LDX LDY STA STX STY STZ"),
    ("Register transfers", "TAX TXA TAY TYA TXY TYX TCD TDC TSC TCS"),
    ("Block move", "MVN MVP"),
    ("Stack", "PHA PHX PHY PHP PHB PHD PHK PLA POPF PUSH PEA PEI PER"),
    ("Arithmetic", "ADC SBC MUL DIV INC DEC INX INY DEX DEY"),
    ("Logic", "AND ORA XOR EOR BIT"),
    ("Shifts and rotates", "ASL LSR ROL ROR SHL SHR RCL"),
    ("Compare", "CMP CPX CPY"),
    ("Branches, jumps and calls", "BMI BRA BRL BVS BCS BEQ JMP JSR CALL RTS RTL RTI RET LOOP LPEND"),
    ("Flags", "SEC CLC SED CLD SEI CLI CLV SEP REP"),
    ("System", "NOP BRK COP WAI HLT SDB"),
]

# Sample operands used to assemble each syntax form.
SAMPLES = [
    ("srcbk,destbk", "$01,$02"), ("sr,S", "$05,S"), ("#const", "#$12"), ("long", "$123456"), ("addr", "$1234"),
    ("word", "$1234"), ("byte", "$12"), ("sig", "$00"), ("dp", "$10"), ("label", "target"),
]


def sample_operand(syntax):
    s = syntax
    for name, value in SAMPLES:
        s = re.sub(r"(?<![A-Za-z])" + re.escape(name) + r"(?![A-Za-z])", value, s)
    return s


def assemble(line):
    """Assemble one instruction at $8000. Returns bytes or None."""
    with tempfile.TemporaryDirectory() as d:
        src, out = os.path.join(d, "t.asm"), os.path.join(d, "t.bin")
        with open(src, "w") as f:
            f.write(".org $8000\ntarget:\n%s\n" % line)
        r = subprocess.run([CPUASM, src, out], capture_output=True, text=True, cwd=d)
        if r.returncode != 0 or not os.path.exists(out):
            return None
        return open(out, "rb").read()


def load_measured():
    rows = {}
    with open(os.path.join(ROOT, "spec", "DEF88186-measured.tsv")) as f:
        lines = f.read().splitlines()
    head = lines[0].split("\t")
    for l in lines[1:]:
        v = dict(zip(head, l.split("\t")))
        rows[int(v["op"], 16)] = v
    return rows


def cycles_of(m):
    seen = set()
    for k in ("c_m1x1", "c_m1x0", "c_m0x1", "c_m0x0"):
        if m[k] == "var":
            return "varies"
        seen.update(int(x) for x in m[k].split("/"))
    return "/".join(str(c) for c in sorted(seen))


MODE_COLUMNS = [
    ("#", "Immediate"), ("dp", "Direct Page"), ("dp,X", "DP Indexed,X"), ("dp,Y", "DP Indexed,Y"),
    ("addr", "Absolute"), ("addr,X", "Absolute Indexed,X"), ("addr,Y", "Absolute Indexed,Y"),
    ("long", "Absolute Long"), ("long,X", "Absolute Long Indexed,X"), ("long,Y", "Absolute Long Indexed,Y"),
    ("(dp)", "DP Indirect"), ("(dp,X)", "DP Indexed Indirect,X"), ("(dp),Y", "DP Indirect Indexed, Y"),
    ("[dp]", "DP Indirect Long"), ("[dp],Y", "DP Indirect Long Indexed, Y"),
    ("n,S", "Stack Relative"), ("(n,S),Y", "SR Indirect Indexed,Y"),
]
MATRIX_OPS = "LDA STA STZ LDX LDY STX STY ADC SBC AND ORA XOR CMP CPX CPY BIT MUL INC DEC ASL LSR ROL ROR".split()


def mode_matrix(spec):
    """Which operand forms each data instruction has: the opcode map is irregular enough to need a table."""
    have = {}
    for s in spec.values():
        base = s["asm"].split()[0].upper().split(".")[0]
        base = "XOR" if base == "EOR" else base
        # INC.L / DEC.L are absolute, not long; the CSV names them "Absolute Long"
        mode = "Absolute" if s["asm"].split()[0].upper().endswith(".L") else s["mode"]
        have.setdefault(base, set()).add(mode)
    lines = ["## Operand forms by instruction\n",
             "A dot means the opcode does not exist. For example `LDA` has no `(dp)`, `(dp,X)` or `(dp),Y` form, only "
             "the long-pointer ones, and `CPX`/`CPY` exist only as `addr`. `INC`/`DEC` on memory have no long form "
             "(`INC.L` is the same 16-bit absolute address as `INC`, just slower). What each form means is in "
             "[addressing-modes.md](addressing-modes.md).\n",
             "| | " + " | ".join(c for c, _ in MODE_COLUMNS) + " |",
             "|---|" + "---|" * len(MODE_COLUMNS)]
    for op in MATRIX_OPS:
        row = ["x" if m in have.get(op, set()) else "." for _, m in MODE_COLUMNS]
        lines.append("| `%s` | %s |" % (op, " | ".join(row)))
    return "\n".join(lines) + "\n"


def main():
    measured = load_measured()
    spec = {}
    with open(os.path.join(ROOT, "spec", "DEF88186.csv")) as f:
        for r in list(csv.reader(f))[1:]:
            if len(r) < 9 or not r[4].strip():
                continue
            asm = SYNTAX_FIX.get(r[0].strip(), r[0].strip())
            spec[int(r[4], 16)] = dict(asm=asm, alias=r[1].strip(), name=r[2].strip(), mode=r[5].strip())

    base_of = {op: s["asm"].split()[0].upper().split(".")[0] for op, s in spec.items()}
    asm_ok, mismatches = {}, []
    for op, s in spec.items():
        got = assemble(sample_operand(s["asm"]))
        if got is None or got[:1] != bytes([op]):
            asm_ok[op] = False
        else:
            asm_ok[op] = True
            want = int(measured[op]["len"])
            if len(got) != want:
                mismatches.append((op, s["asm"], len(got), want))

    # Keep only aliases cpuasm really accepts.
    dead_aliases = []
    for op, s_ in spec.items():
        good = []
        for alias in [a for a in s_["alias"].replace("/", " ").split() if a]:
            line = sample_operand(s_["asm"]).replace(s_["asm"].split()[0], alias, 1)
            got = assemble(line)
            if got is not None and got[:1] == bytes([op]):
                good.append(alias)
            else:
                dead_aliases.append((op, alias))
        s_["alias"] = "/".join(good)

    out = []
    w = out.append
    w("# DEF88186 opcode table\n")
    w("<!-- GENERATED by scripts/gen_cpu_docs.py from spec/ and a live cpuasm run: do not edit by hand. -->\n")
    w("All 256 opcodes, measured on the emulator (revision 10) rather than copied from a datasheet. The "
      "semantics, quirks and examples are in [instructions.md](instructions.md); what the operand forms mean is in "
      "[addressing-modes.md](addressing-modes.md).\n")
    w("An instruction is its one-byte opcode followed by its operand bytes, low byte first. There are no prefix bytes, "
      "and all 256 opcode values are used (`FF` is `HLT`, `00` is `NOP`).\n")
    w("How to read the columns:\n")
    w("- **Bytes** is the whole instruction. `#const` operands are always 16 bits wide in the encoding, whatever M "
      "and X are.")
    w("- **Cycles** is every cost the instruction was seen to take, in CPU cycles (17.002839 MHz). `3/4` means 4 "
      "when an index addition crosses a page; for branches, `2/3/4` is not taken / taken / taken across a page. "
      "The fixed cost never depends on M or X (X only changes how often an index crosses a page). `varies` means it "
      "depends on a register (block moves: one cycle per byte).")
    w("- **Flags** are the P bits the instruction can change (`NVMXDIZC`, `.` = never touched).")
    w("- **Writes** lists registers other than P and PC that can change (`S` is SP, `B` is DB), and the net stack "
      "movement (`-2` pushes two bytes).")
    w("- **Asm** is whether `cpuasm` assembles this form (✓), and `—` if it does not.\n")

    used = set()
    for title, names in GROUPS:
        members = set(names.split())
        ops = sorted(op for op in spec if base_of[op] in members)
        if not ops:
            continue
        used.update(ops)
        w("## %s\n" % title)
        w("| Op | Syntax | Mode | Bytes | Cycles | Flags | Writes | Asm |")
        w("|----|--------|------|-------|--------|-------|--------|-----|")
        for op in ops:
            s, m = spec[op], measured[op]
            syntax = "`%s`" % s["asm"]
            if s["alias"]:
                syntax += " (alias `%s`)" % s["alias"].replace("/", "`, `")
            writes = [] if m["regs"] == "-" else [m["regs"]]
            if m["dsp"] not in ("0",):
                writes.append("stack %s" % m["dsp"].replace("/", " or "))
            if m["end"] == "H":
                writes.append("halts")
            if m["end"] == "W":
                writes.append("waits")
            w("| `%02X` | %s | %s | %s | %s | `%s` | %s | %s |" % (
                op, syntax, s["mode"], m["len"], cycles_of(m), m["flags"], ", ".join(writes) or "—",
                "✓" if asm_ok[op] else "—"))
        w("")
    leftover = sorted(set(spec) - used)
    if leftover:
        sys.exit("opcodes not in any group: " + " ".join("%02X(%s)" % (o, base_of[o]) for o in leftover))

    out.append(mode_matrix(spec))
    with open(OUT, "w") as f:
        f.write("\n".join(out))
    print("wrote", OUT)
    missing = [op for op in sorted(spec) if not asm_ok[op]]
    print("cpuasm cannot assemble %d opcodes: %s" % (len(missing), " ".join("%02X" % o for o in missing)))
    for op, alias in dead_aliases:
        print("alias %s of %02X is not accepted by cpuasm (left out of the table)" % (alias, op))
    for op, asm, got, want in mismatches:
        print("LENGTH MISMATCH %02X %-22s cpuasm=%d emulator=%d" % (op, asm, got, want))


if __name__ == "__main__":
    main()

# spec/

Machine-read specifications. The prose is in `docs/`.

| File | What | Used by |
|------|------|---------|
| `DEF88186.csv` | CPU opcode map: opcode, mnemonic, alias, name, addressing mode | ZeroPoint's `tools/gen_cpu_instructions.py` (generates the emulator's opcode handlers), `scripts/gen_cpu_docs.py` |
| `DEF88186-measured.tsv` | What the emulator really does with each opcode: length, cycles per M/X mode, flags changed, registers written, stack movement | `scripts/gen_cpu_docs.py` |
| `DEF88186.numbers` | The spreadsheet the CSV was exported from. Untracked (`*.numbers` is ignored). | nobody |

## Which columns of the CSV to trust

Trust the **opcode**, **mnemonic** and **addressing mode** columns: the emulator's handlers are generated from them. The
**Cycles** column also feeds the generator, but only for instructions whose handler does not count its own cycles.

Do **not** trust the **Bytes** and **Flags Set** columns. They have blanks, and the Bytes column is wrong for 17 opcodes
(every `#const`, `BRA`, `BVS`, `INC long`, `DEC long`, `DEC A`, `RCL A`). Use `DEF88186-measured.tsv`, or the generated
`docs/cpu/opcode-table.md`, which is built from it.

## Regenerating the measured file

From the ZeroPoint emulator repo, after building (`dump_cpu_table` is a CMake target):

```sh
build/bin/dump_cpu_table > ../ZPdevtools/spec/DEF88186-measured.tsv
python3 ../ZPdevtools/scripts/gen_cpu_docs.py
```

The emulator is frozen at revision 10, so the file only changes if the CPU does.

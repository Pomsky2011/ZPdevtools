# Compiler regression tests

`run.sh` compiles every `t*.c` three ways and compares what each records:

* with the host **gcc** (`-DHOST`) - the reference, so the expected values are real C semantics;
* with **zpcc** and with **def88186cc** (`-DZP_CC_DOS`), run in the emulator through ZeroPoint's `zp_run_c`.

```sh
make -C llvm_compiler && make -C c_compiler          # build the compilers
(cd ../ZeroPoint/build && make zp_run_c)             # the emulator half
tests/cc/run.sh                  # all tests, both compilers
tests/cc/run.sh dos -- tests/cc/t04_long.c           # one compiler, one test
BOOT=../ZPbootROM/build/bootrom.bin tests/cc/run.sh  # also the tests that need the real boot ROM (COP services)
```

Tests call `rec(v)` / `rec32(v)` (`rec.asm`) to record results. Keep them inside 16-bit `int` /
32-bit `long` range with no overflow so the host's wider `int` agrees. SDK tests that can't run on
the host carry `/* EXPECT: ... */`; tests that need the boot ROM carry `/* NEEDS: boot */`.

| Test | Covers |
|------|--------|
| `t01_calls` | calls, nesting, recursion, up to 8 arguments, locals, modified parameters |
| `t02_ops` | operators, loops, conditionals, short-circuit logic |
| `t03_runtime_ops` | every 16-bit operator behind a call (so nothing folds), signed/unsigned compares, signed divide |
| `t04_long` | 32-bit arithmetic, compares, shifts, widening/narrowing, register-pair arguments and returns |
| `t05_sdk` | the C SDK driving real peek/poke and DMA |
| `t06_cop` | every COP signature through `zp_cop.h` |
| `t07_loops` | counted/nested/zero-trip loops, `continue`, `break`, calls in loops |
| `t08_narrow` | `char`/`short` stores, casts, parameters, returns |
| `t09_arrays` | local arrays: indexing, initialisers, 2-D, char/long elements, array parameters |
| `t10_pointers` | `&`, `*`, `p[i]`, pointer arithmetic/comparison, pointers to pointers, pointer parameters and results |
| `t11_structs` | structs and unions: members, nesting, member arrays, arrays of structs, `->`, initialisers |
| `t12_scope_promo` | block scoping and the integer promotions |
| `t13_u16_semantics` | 16-bit unsigned semantics (expected values listed in the file, a 32-bit host would disagree) |

`ZP_TRACE=1` / `ZP_STOP=<pc>` on `zp_run_c` print the last instructions of a hung or wild test.

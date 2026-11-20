# C calling convention (ABI)

Both C compilers, `zpcc` and `def88186cc`, use the same ABI, and so does hand-written assembly that calls compiled code. It
is a software convention, not hardware. Everything below was read from the code the two compilers emit, not from a
description.

## Registers and data

- Compiled code runs in **16-bit mode**: `REP #$30` at the start of every function, so `A`, `X` and `Y` are 16 bits.
- `int` is 16 bits and `long` is 32. A pointer is a 16-bit offset into **bank `$80`**.
- **No register survives a call.** `A`, `X` and `Y` are all caller-saved.
- The stack is in bank `$80`; `S` points at the next free byte, so the top of the stack is `1,S`. A frame can be at most 255
  bytes, because a stack-relative operand is one byte; both compilers refuse a larger one.

## Arguments

The first three **words** go in registers, in order: the first word in `A`, the second in `X`, the third in `Y`. A 32-bit
argument takes two words, low word first. The first argument that does not fit in what is left, and every argument after
it, is pushed on the stack, **right to left**, before the call. The caller removes the pushed words after the call returns.

For `int f(int a, int b, int c, int d)`, the call `f(1, 2, 3, 4)` is:

```cpuasm skip
        LDA #$0004
        PHA                     ; d: pushed
        LDA #$0003
        TAY                     ; c in Y
        LDA #$0002
        TAX                     ; b in X
        LDA #$0001              ; a in A
        JSR f
        TAY                     ; keep the result
        TSC
        CLC
        ADC #$0002              ; drop the pushed argument
        TCS
        TYA                     ; result back in A
```

For `long g(long a, int b, int c)`, the call `g(70000L, 5, 6)` puts the low word of `a` (`$1170`) in `A`, its high word
(`$0001`) in `X`, `b` in `Y`, and pushes `c`.

## Result

- A 16-bit result is in `A`.
- A 32-bit result is in `A` (low word) and `X` (high word).

## The callee

A function saves its incoming `Y`, `X` and `A` into its frame with `PHY`, `PHX`, `PHA` (so the arguments become ordinary
stack slots), allocates its locals with `TSC / SEC / SBC / TCS`, addresses everything as `n,S`, and releases its whole frame
itself before `RTS`. Calls are `JSR` and `RTS`, within a bank.

A pushed (fourth or later) argument is above the saved registers and the return address, so its offset depends on the frame
size.

## Layout

- Struct layout follows clang's 16-bit rules: a struct of `char`s only is byte-aligned, anything else is word-aligned.
- Locals, arrays and structs live in the stack frame. Taking an address gives an offset into bank `$80`.
- `zpcc` global variables are ROM: they can be read by name, but taking one's address as a pointer, or writing it, is an
  error. `def88186cc` refuses globals. Neither accepts string literals as pointers or struct values; the exact subset
  `def88186cc` accepts is in `c_compiler/README.md`.

## The 32-bit runtime

`long` arithmetic beyond load, store and convert calls routines in `lib/zp_long.asm`, which use their **own** convention, not
the one above: the operands are in fixed scratch words, the result in `A` (low) and `X` (high), and compares return -1, 0 or
1 in `A`. They clobber `A`, `X`, `Y` and the scratch area `$80:2C00-$80:2C2F`, and are **not re-entrant**. Link
`lib/zp_long.asm` into any program that does `long` arithmetic.

## Calling compiled code from assembly

Load `A`, `X`, `Y` with the first three words, push the rest right to left, `JSR`, then remove what you pushed; the result
is in `A` or `A:X`. The assembly code you write is also free to use the C ABI for its own functions: the three-register
form keeps small helpers cheap.

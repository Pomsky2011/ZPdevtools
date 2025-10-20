# ZPdevtools TODO

## Critical Bugs

### 🔴 Loop Counters Get Stuck (2025-10-19)
**Status**: CRITICAL - Suspected assembler bug
**Description**: Programs assembled with zpasm have broken loops - counters increment to 1 and get stuck

**Test Case**:
```asm
R10 = COUNTER
start:
    CLR COUNTER
loop:
    INC COUNTER
    TARREG 2, LSB, R15
    SETBYTE 2, 5
    CMP COUNTER, R15
    JNG loop
    HLT
```

**Expected Behavior**: Counter should increment 0→1→2→3→4→5→halt
**Actual Behavior**: Counter increments to 1 and enters infinite loop

**Disassembly** (loop_test.bin):
```
00000000  30 0a 80 0a e0 8f e4 85  42 8f e0 3e e0 7e e4 02  |0.......B..>.~..|
00000010  e4 40 78 00 e0 3e e0 7e  e4 14 e4 40 f7 00        |.@x..>.~...@..|
```

**Possible Causes**:
1. Label `loop:` might be resolved to wrong address
2. `JNG loop` expansion might be encoding wrong PC value
3. Jump shorthand macro might have off-by-one error
4. Label might be placed at `start:` instead of at `INC COUNTER`

**Files Affected**:
- All demos using loops (color_bars, gradients, etc.)
- Any program using INC + CMP + JNG pattern

**Impact**: BLOCKING - No loop-based programs can execute correctly

**Next Steps**:
1. Add verbose mode to zpasm to show label addresses
2. Disassemble working vs non-working programs
3. Test if manual PC loading works (without JNG shorthand)
4. Check label resolution logic in zpasm.c

## Recent Changes

### R59/R60 Interrupt Documentation (2025-10-19)
- Updated README.md with R59/R60 interrupt register documentation
- Noted automatic return address push behavior
- Added warning about target register 0 and 1 corruption by shorthands

### Shorthand System (Previous)
- Added label-based jump shorthands (JMR, JMZ, JNZ, JNG, JMG)
- Added stack operation shorthands (PUSH, POP, JSR, RET)
- Added HLT shorthand (6-instruction infinite loop)
- Added variable aliases (e.g., `R10 = COUNTER`)

## Known Issues

1. **HLT is not HALT**: HLT expands to 6-instruction loop, not a real CPU halt
2. **Target Register Corruption**: Shorthands overwrite target registers 0 and 1
3. **No Immediate Values**: Cannot do `MOV R0, 42` - must build values step-by-step
4. **No Macro System**: No way to define reusable code blocks
5. **No Include Files**: Cannot split code across multiple files

## Testing Needed

### Urgent (Loop Bug Investigation)
- [ ] Add --verbose flag to show label resolution
- [ ] Test manual PC loading vs JNG shorthand
- [ ] Verify label placement in assembled binaries
- [ ] Create minimal failing test case

### General
- [ ] Test all shorthands with edge cases
- [ ] Test variable aliases with edge cases
- [ ] Verify interrupt handler examples work
- [ ] Test PUSH/POP/JSR/RET stack operations

## Future Enhancements

### High Priority
- [ ] **Fix loop bug** (CRITICAL)
- [ ] Add verbose/debug mode
- [ ] Add disassembler tool
- [ ] Better error messages with line numbers

### Medium Priority
- [ ] Macro system
- [ ] Include file support
- [ ] Immediate value syntax (`MOV R0, #42`)
- [ ] Constant definitions (`CONST SCREEN_WIDTH 256`)
- [ ] Expression evaluation (`SETBYTE 2, SCREEN_WIDTH/2`)

### Low Priority
- [ ] Optimization passes
- [ ] Dead code elimination
- [ ] Peephole optimization
- [ ] Symbol table export for debugging

## Documentation Status

### Complete
- ✅ README.md with basic usage
- ✅ Instruction reference
- ✅ Shorthand documentation (docs/ppu/preset-e-and-shorthands.txt)
- ✅ Interrupt system (R59/R60)
- ✅ Target register warnings

### Needed
- [ ] Troubleshooting guide
- [ ] Loop debugging guide
- [ ] Best practices document
- [ ] Performance optimization guide
- [ ] Example programs (working ones, once bug is fixed)

## Examples Status

### Working (No Loops)
- ✅ Simple programs without loops
- ✅ HLT detection

### Broken (Loops)
- ❌ color_bars.asm
- ❌ color_bars_clean.asm
- ❌ gradient_demo_simple.asm
- ❌ gradient_test.asm
- ❌ All loop-based demos

### Not Tested
- ⚠️ Interrupt handler examples
- ⚠️ JSR/RET subroutines
- ⚠️ Complex nested loops

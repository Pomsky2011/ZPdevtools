// zpcc - LLVM/clang-based C compiler for the DEF88186 CPU (ZeroPoint Fantasy Console)
//
// This is the "modern" companion to def88186cc.  Where def88186cc is a small,
// self-contained, DOS-portable C compiler, zpcc leans on a full clang/LLVM
// front end for fast, standards-accurate parsing and then lowers the resulting
// LLVM IR to DEF88186 assembly that the cpuasm assembler accepts.
//
// Pipeline:
//     input.c --clang(--target=msp430 -emit-llvm -O0)--> input.ll
//     input.ll --zpcc backend--> input.asm --cpuasm--> input.bin
//
// The MSP430 target is used purely as a 16-bit data-layout model: it gives us
// 16-bit int and 16-bit pointers, an exact match for the DEF88186's 16-bit
// programming model.  We never use LLVM's MSP430 code generator; zpcc walks the
// IR itself and emits DEF88186 mnemonics.
//
// ABI (identical to def88186cc so the two toolchains interoperate):
//     - 16-bit mode (REP #$30)
//     - first three arguments in A, X, Y; remaining args pushed right-to-left
//     - return value in A
//     - each function allocates a fixed-size stack frame in its prologue and
//       leaves S untouched for the body, so locals stay addressable as off,S.
//
// Supported IR subset (clang -O0): alloca, load, store, getelementptr,
// add/sub/mul/[us]div/[us]rem, and/or/xor, shl/lshr/ashr, icmp, br, ret, call,
// zext/sext/trunc/bitcast/ptrtoint/inttoptr, select, and integer/pointer
// globals.  Unsupported constructs are reported with a clear diagnostic.

#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/GetElementPtrTypeIterator.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <map>
#include <string>
#include <vector>
#include <unistd.h>

using namespace llvm;

namespace {

// ---------------------------------------------------------------------------
// Output helpers
// ---------------------------------------------------------------------------
static FILE *out = nullptr;
static bool hadError = false;

static void emit(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(out, fmt, ap);
  va_end(ap);
}

static void backendError(const std::string &msg) {
  fprintf(stderr, "zpcc: error: %s\n", msg.c_str());
  hadError = true;
}

// ---------------------------------------------------------------------------
// Per-function lowering state
// ---------------------------------------------------------------------------
struct FnLower {
  Function &F;
  const DataLayout &DL;
  std::map<const Value *, int> slot;   // frame byte offset for a value's storage
  std::map<const BasicBlock *, int> blockId;
  std::map<std::pair<const BasicBlock *, const BasicBlock *>, int> edgeLabel;
  std::vector<std::pair<const BasicBlock *, const BasicBlock *>> edgeList;
  int frameSize = 0;                   // total frame bytes (word aligned)
  int labelCounter = 0;
  int spDelta = 0;                     // extra bytes currently pushed (call setup)
  int nRegArgs = 0;                    // register words pushed by the prologue (PHY/PHX/PHA); a long is two
  std::vector<int> argReg;             // first register word of each argument, or -1 when it is on the stack
  int extraSize = 0;                   // frame bytes the prologue allocates below those pushes
  int scratch0 = 0, scratch1 = 0, scratch2 = 0;   // frame slots standing in for memory temporaries
  int saveD = 0;                       // frame slot holding D across a load/store through a pointer
  std::string fnName;

  FnLower(Function &Fn, const DataLayout &Layout) : F(Fn), DL(Layout) {
    fnName = F.getName().str();
  }

  int newLabel() { return labelCounter++; }

  static int roundWord(int n) { return (n + 1) & ~1; }

  // Words a value of this type occupies: everything up to 16 bits (and pointers) is one, i32 is two (A = low, X =
  // high in registers, low word first in memory). Wider integers are not supported - say so instead of truncating.
  int wordsOf(Type *T) {
    if (auto *IT = dyn_cast<IntegerType>(T)) {
      unsigned bw = IT->getBitWidth();
      if (bw <= 16) return 1;
      if (bw <= 32) return 2;
      backendError("integer wider than 32 bits (i" + std::to_string(bw) + ") in " + fnName);
    }
    return 1;
  }
  int wordsOf(const Value *V) { return wordsOf(V->getType()); }

  // The frame is [S+1 .. S+frameSize] once the prologue has run (S points at the next free byte, so the top of the
  // stack is S+1, not S+0), lowest address first:  extra area (allocas, temporaries, copies of stack arguments),
  // then the register arguments the prologue pushed (arg0 lowest).  A slot offset is a byte offset from S+1.
  //
  // Operand for "off,S": the slot plus the 1 that S points below the stack top, plus whatever call setup has pushed.
  int sp(int slotOff) const { return slotOff + 1 + spDelta; }

  // Assign a frame slot to every alloca (its storage) and value-producing instruction, then the arguments.
  void assignSlots() {
    int off = 0;
    scratch0 = off; off += 2;
    scratch1 = off; off += 2;
    scratch2 = off; off += 2;
    saveD = off;    off += 2;
    // Register assignment: a 32-bit argument takes two of A, X, Y; once one does not fit, it and every later
    // argument is on the stack. Stack arguments are copied into the frame by the prologue.
    nRegArgs = 0;
    bool onStack = false;
    argReg.assign(F.arg_size(), -1);
    for (Argument &A : F.args()) {
      int w = wordsOf(&A);
      if (!onStack && nRegArgs + w <= 3) {
        argReg[A.getArgNo()] = nRegArgs;
        nRegArgs += w;
      } else {
        onStack = true;
        slot[&A] = off;
        off += 2 * w;
      }
    }
    for (BasicBlock &BB : F) {
      for (Instruction &I : BB) {
        if (auto *AI = dyn_cast<AllocaInst>(&I)) {
          uint64_t bytes =
              DL.getTypeAllocSize(AI->getAllocatedType()).getFixedValue();
          if (auto *cnt = dyn_cast<ConstantInt>(AI->getArraySize()))
            bytes *= cnt->getZExtValue();
          if (bytes < 2)
            bytes = 2;
          slot[AI] = off;
          off += roundWord((int)bytes);
          continue;
        }
        if (!I.getType()->isVoidTy()) {
          slot[&I] = off;
          off += 2 * wordsOf(&I);
        }
      }
    }
    extraSize = roundWord(off);
    for (Argument &A : F.args())
      if (argReg[A.getArgNo()] >= 0)
        slot[&A] = extraSize + 2 * argReg[A.getArgNo()];
    frameSize = extraSize + 2 * nRegArgs;
  }

  bool hasSlot(const Value *V) const { return slot.count(V) != 0; }
  int slotOf(const Value *V) const { return slot.at(V); }

  // -------------------------------------------------------------------------
  // Loading operands into A
  // -------------------------------------------------------------------------

  // Emit code that leaves the *value* of V in the A register.
  void loadToA(Value *V) {
    V = V->stripPointerCastsSameRepresentation();
    if (auto *CI = dyn_cast<ConstantInt>(V)) {
      emit("    LDA #$%04X\n", (unsigned)(CI->getZExtValue() & 0xFFFF));
      if (wordsOf(V) == 2)
        emit("    LDX #$%04X\n", (unsigned)((CI->getZExtValue() >> 16) & 0xFFFF));
      return;
    }
    if (isa<ConstantPointerNull>(V)) {
      emit("    LDA #$0000\n");
      return;
    }
    if (auto *GV = dyn_cast<GlobalValue>(V)) {
      // Address of a global is its label.
      emit("    LDA #%s\n", GV->getName().str().c_str());
      return;
    }
    if (auto *AI = dyn_cast<AllocaInst>(V)) {
      // The value of an alloca is the address of its storage: S + off.
      int off = sp(slotOf(AI));
      emit("    TSC\n");
      emit("    CLC\n");
      emit("    ADC #$%04X\n", off & 0xFFFF);
      return;
    }
    if (auto *CE = dyn_cast<ConstantExpr>(V)) {
      // Fold a constant getelementptr / cast on a global into label+offset.
      APInt acc(16, 0);
      Value *base = V;
      if (CE->getOpcode() == Instruction::GetElementPtr) {
        auto *GEP = cast<GEPOperator>(CE);
        if (GEP->accumulateConstantOffset(DL, acc)) {
          base = GEP->getPointerOperand()->stripPointerCasts();
          if (auto *GV = dyn_cast<GlobalValue>(base)) {
            long o = acc.getSExtValue();
            if (o)
              emit("    LDA #%s+%ld\n", GV->getName().str().c_str(), o);
            else
              emit("    LDA #%s\n", GV->getName().str().c_str());
            return;
          }
        }
      }
      backendError("unsupported constant expression in " + fnName);
      emit("    LDA #$0000\n");
      return;
    }
    if (hasSlot(V)) {
      if (wordsOf(V) == 2) {
        emit("    LDA %d,S\n", sp(slotOf(V)) + 2);
        emit("    TAX\n");
      }
      emit("    LDA %d,S\n", sp(slotOf(V)));
      return;
    }
    backendError("cannot load operand in " + fnName);
    emit("    LDA #$0000\n");
  }

  // Store A into the slot of a value-producing instruction.
  void storeAToResult(Instruction *I) {
    if (I->getType()->isVoidTy())
      return;
    emit("    STA %d,S\n", sp(slotOf(I)));
    if (wordsOf(I) == 2) {
      emit("    TXA\n");
      emit("    STA %d,S\n", sp(slotOf(I)) + 2);
    }
  }

  // Compute the effective address of a pointer operand into A.
  void addressToA(Value *Ptr) {
    if (auto *AI = dyn_cast<AllocaInst>(Ptr->stripPointerCasts())) {
      int off = sp(slotOf(AI));
      emit("    TSC\n");
      emit("    CLC\n");
      emit("    ADC #$%04X\n", off & 0xFFFF);
      return;
    }
    loadToA(Ptr); // already an address-valued operand
  }

  // -------------------------------------------------------------------------
  // Instruction lowering
  // -------------------------------------------------------------------------
  void lowerAlloca(AllocaInst *) {
    // Storage reserved in the frame; nothing to emit.
  }

  void lowerLoad(LoadInst *LI) {
    Value *Ptr = LI->getPointerOperand();
    const bool wide = wordsOf(LI) == 2;
    if (auto *AI = dyn_cast<AllocaInst>(Ptr->stripPointerCasts())) {
      if (wide) {
        emit("    LDA %d,S\n", sp(slotOf(AI)) + 2);
        emit("    TAX\n");
      }
      emit("    LDA %d,S\n", sp(slotOf(AI)));
    } else {
      // Direct page is the only register-indirect mode, so D is borrowed as the pointer and put back after.
      emit("    TDC\n");
      emit("    STA %d,S\n", sp(saveD));
      addressToA(Ptr);
      emit("    TCD          ; DP = pointer\n");
      if (wide) {
        emit("    LDA $02\n");
        emit("    STA %d,S\n", sp(scratch1));
      }
      emit("    LDA $00      ; load through pointer\n");
      emit("    TAY\n");
      emit("    LDA %d,S\n", sp(saveD));
      emit("    TCD          ; restore D\n");
      if (wide) {
        emit("    LDA %d,S\n", sp(scratch1));
        emit("    TAX\n");
      }
      emit("    TYA\n");
    }
    storeAToResult(LI);
  }

  void lowerStore(StoreInst *SI) {
    Value *Val = SI->getValueOperand();
    Value *Ptr = SI->getPointerOperand();
    const bool wide = wordsOf(Val) == 2;
    if (auto *AI = dyn_cast<AllocaInst>(Ptr->stripPointerCasts())) {
      loadToA(Val);
      emit("    STA %d,S\n", sp(slotOf(AI)));
      if (wide) {
        emit("    TXA\n");
        emit("    STA %d,S\n", sp(slotOf(AI)) + 2);
      }
    } else {
      loadToA(Val);
      emit("    STA %d,S\n", sp(scratch0));
      if (wide) {
        emit("    TXA\n");
        emit("    STA %d,S\n", sp(scratch1));
      }
      emit("    TDC\n");
      emit("    STA %d,S\n", sp(saveD));
      addressToA(Ptr);
      emit("    TCD          ; DP = pointer\n");
      if (wide) {
        emit("    LDA %d,S\n", sp(scratch1));
        emit("    STA $02\n");
      }
      emit("    LDA %d,S\n", sp(scratch0));
      emit("    STA $00      ; store through pointer\n");
      emit("    LDA %d,S\n", sp(saveD));
      emit("    TCD          ; restore D\n");
    }
  }

  // The right-hand operand of an ALU instruction: "#imm", "n,S" for a value that lives in a slot, or (anything else,
  // e.g. an address) the value parked in scratch0 first.  loadToA of the left operand comes after this.
  std::string aluOperand(Value *V) {
    V = V->stripPointerCastsSameRepresentation();
    char buf[48];
    if (auto *CI = dyn_cast<ConstantInt>(V)) {
      snprintf(buf, sizeof buf, "#$%04X", (unsigned)(CI->getZExtValue() & 0xFFFF));
      return buf;
    }
    if (hasSlot(V) && !isa<AllocaInst>(V)) {
      snprintf(buf, sizeof buf, "%d,S", sp(slotOf(V)));
      return buf;
    }
    loadToA(V);
    emit("    STA %d,S\n", sp(scratch0));
    snprintf(buf, sizeof buf, "%d,S", sp(scratch0));
    return buf;
  }

  // 32-bit operators are runtime calls (lib/zp_long.asm): left operand in $80:2C00, right in $80:2C04, result in
  // A:X. A shift count is the low word of the right operand.
  void lowerBinary32(BinaryOperator *BO) {
    const char *helper = nullptr;
    switch (BO->getOpcode()) {
    case Instruction::Add:  helper = "__zp_ladd"; break;
    case Instruction::Sub:  helper = "__zp_lsub"; break;
    case Instruction::Mul:  helper = "__zp_lmul"; break;
    case Instruction::And:  helper = "__zp_land"; break;
    case Instruction::Or:   helper = "__zp_lor"; break;
    case Instruction::Xor:  helper = "__zp_lxor"; break;
    case Instruction::UDiv: helper = "__zp_ludiv"; break;
    case Instruction::SDiv: helper = "__zp_lsdiv"; break;
    case Instruction::URem: helper = "__zp_lumod"; break;
    case Instruction::SRem: helper = "__zp_lsmod"; break;
    case Instruction::Shl:  helper = "__zp_lshl"; break;
    case Instruction::LShr: helper = "__zp_llsr"; break;
    case Instruction::AShr: helper = "__zp_lasr"; break;
    default:
      backendError("unsupported 32-bit binary op in " + fnName);
      return;
    }
    loadToA(BO->getOperand(1));
    emit("    STA $802C04\n");
    emit("    TXA\n");
    emit("    STA $802C06\n");
    loadToA(BO->getOperand(0));
    emit("    STA $802C00\n");
    emit("    TXA\n");
    emit("    STA $802C02\n");
    emit("    JSR %s\n", helper);
    storeAToResult(BO);
  }

  void lowerBinary(BinaryOperator *BO) {
    if (wordsOf(BO) == 2) { lowerBinary32(BO); return; }
    unsigned op = BO->getOpcode();
    Value *lhs = BO->getOperand(0), *rhs = BO->getOperand(1);
    switch (op) {
    case Instruction::Add:
    case Instruction::Sub:
    case Instruction::And:
    case Instruction::Or:
    case Instruction::Xor: {
      std::string r = aluOperand(rhs);
      loadToA(lhs);
      if (op == Instruction::Add) { emit("    CLC\n"); emit("    ADC %s\n", r.c_str()); }
      else if (op == Instruction::Sub) { emit("    SEC\n"); emit("    SBC %s\n", r.c_str()); }
      else if (op == Instruction::And) emit("    AND %s\n", r.c_str());
      else if (op == Instruction::Or)  emit("    ORA %s\n", r.c_str());
      else if (r[0] == '#') emit("    EOR %s\n", r.c_str());
      else emit("    XOR %s\n", r.c_str());   // the stack-relative form is spelled XOR
      break;
    }
    case Instruction::Mul: {
      // MUL takes a memory operand (there is no register form): A = low word of A * operand.
      std::string r = aluOperand(rhs);
      loadToA(lhs);
      emit("    MUL %s\n", r.c_str());
      break;
    }
    case Instruction::UDiv:
    case Instruction::URem:
      // DIV X,Y: X / Y -> quotient in A, remainder in X.
      loadToA(rhs);
      emit("    TAY\n");
      loadToA(lhs);
      emit("    TAX\n");
      emit("    DIV X,Y\n");
      if (op == Instruction::URem) emit("    TXA\n");
      break;
    case Instruction::SDiv:
    case Instruction::SRem:
      lowerSignedDivRem(op == Instruction::SRem, lhs, rhs);
      break;
    case Instruction::Shl:
    case Instruction::LShr:
    case Instruction::AShr: {
      // INX/DEX down-counter: the ISA has no BNE and no working CPX immediate,
      // and DEX sets Z when the count reaches zero.
      loadToA(rhs);
      emit("    TAX\n");
      loadToA(lhs);
      int loop = newLabel(), done = newLabel();
      emit("    INX\n");
      emit(".L%s_%d:\n", fnName.c_str(), loop);
      emit("    DEX\n");
      emit("    BEQ .L%s_%d\n", fnName.c_str(), done);
      if (op == Instruction::AShr) {
        emit("    CMP #$8000   ; C = sign bit, rotated back in below\n");
        emit("    ROR A\n");
      } else {
        emit("    %s\n", op == Instruction::Shl ? "ASL A" : "LSR A");
      }
      emit("    BRA .L%s_%d\n", fnName.c_str(), loop);
      emit(".L%s_%d:\n", fnName.c_str(), done);
      break;
    }
    default:
      backendError("unsupported binary op in " + fnName);
      emit("    LDA #$0000\n");
      break;
    }
    storeAToResult(BO);
  }

  // A = |A| (16-bit two's complement).
  void emitAbs() {
    int neg = newLabel(), done = newLabel();
    emit("    BMI .L%s_%d\n", fnName.c_str(), neg);
    emit("    BRA .L%s_%d\n", fnName.c_str(), done);
    emit(".L%s_%d:\n", fnName.c_str(), neg);
    emit("    EOR #$FFFF\n");
    emit("    INC A\n");
    emit(".L%s_%d:\n", fnName.c_str(), done);
  }

  // Signed 16-bit divide / remainder on top of the unsigned DIV: divide the magnitudes, then negate the result when
  // its sign calls for it - the quotient is negative when the operand signs differ, the remainder takes the
  // dividend's sign (C99).
  void lowerSignedDivRem(bool rem, Value *lhs, Value *rhs) {
    const int s0 = sp(scratch0), s1 = sp(scratch1), s2 = sp(scratch2);
    loadToA(lhs);
    emit("    STA %d,S      ; dividend\n", s0);
    loadToA(rhs);
    emit("    STA %d,S      ; divisor\n", s1);
    emit("    LDA %d,S\n", s0);
    if (!rem) emit("    XOR %d,S      ; bit 15 = signs differ\n", s1);
    emit("    STA %d,S      ; sign of the result in bit 15\n", s2);
    emit("    LDA %d,S\n", s1);
    emitAbs();
    emit("    TAY            ; Y = |divisor|\n");
    emit("    LDA %d,S\n", s0);
    emitAbs();
    emit("    TAX            ; X = |dividend|\n");
    emit("    DIV X,Y\n");
    if (rem) emit("    TXA\n");
    emit("    STA %d,S\n", s0);
    emit("    LDA %d,S\n", s2);
    int neg = newLabel(), pos = newLabel(), done = newLabel();
    emit("    BMI .L%s_%d\n", fnName.c_str(), neg);
    emit("    BRA .L%s_%d\n", fnName.c_str(), pos);
    emit(".L%s_%d:\n", fnName.c_str(), neg);
    emit("    LDA %d,S\n", s0);
    emit("    EOR #$FFFF\n");
    emit("    INC A\n");
    emit("    BRA .L%s_%d\n", fnName.c_str(), done);
    emit(".L%s_%d:\n", fnName.c_str(), pos);
    emit("    LDA %d,S\n", s0);
    emit(".L%s_%d:\n", fnName.c_str(), done);
  }

  void bra(const char *op, int label) {
    emit("    %s .L%s_%d\n", op, fnName.c_str(), label);
  }
  void deflabel(int label) { emit(".L%s_%d:\n", fnName.c_str(), label); }

  // Branches the CPU really has: BEQ (Z), BMI (N), BVS (V), BCS (C) and BRA; cpuasm synthesizes BNE/BPL/BVC/BCC from
  // them. BGE is only an alias for BCS (unsigned >=), so it cannot be used for signed compares.
  //
  // Unsigned relations come straight from CMP's carry (C = lhs >= rhs). Signed ones need the overflow flag, which
  // CMP does not set but SBC does: after SEC/SBC, lhs < rhs exactly when N != V.
  // 32-bit compare: the helper leaves -1 / 0 / +1 in A, then the relation is read off that.
  void lowerICmp32(ICmpInst *CI) {
    loadToA(CI->getOperand(1));
    emit("    STA $802C04\n");
    emit("    TXA\n");
    emit("    STA $802C06\n");
    loadToA(CI->getOperand(0));
    emit("    STA $802C00\n");
    emit("    TXA\n");
    emit("    STA $802C02\n");
    emit("    JSR %s\n", CI->isSigned() ? "__zp_lcmps" : "__zp_lcmpu");
    emit("    CMP #$0000\n");
    int trueL = newLabel(), falseL = newLabel(), doneL = newLabel();
    switch (CI->getPredicate()) {
    case CmpInst::ICMP_EQ:  bra("BEQ", trueL); bra("BRA", falseL); break;
    case CmpInst::ICMP_NE:  bra("BEQ", falseL); bra("BRA", trueL); break;
    case CmpInst::ICMP_ULT:
    case CmpInst::ICMP_SLT: bra("BMI", trueL); bra("BRA", falseL); break;
    case CmpInst::ICMP_ULE:
    case CmpInst::ICMP_SLE: bra("BMI", trueL); bra("BEQ", trueL); bra("BRA", falseL); break;
    case CmpInst::ICMP_UGT:
    case CmpInst::ICMP_SGT: bra("BMI", falseL); bra("BEQ", falseL); bra("BRA", trueL); break;
    case CmpInst::ICMP_UGE:
    case CmpInst::ICMP_SGE: bra("BMI", falseL); bra("BRA", trueL); break;
    default:
      backendError("unsupported icmp predicate in " + fnName);
      return;
    }
    deflabel(trueL);
    emit("    LDA #$0001\n");
    bra("BRA", doneL);
    deflabel(falseL);
    emit("    LDA #$0000\n");
    deflabel(doneL);
    storeAToResult(CI);
  }

  void lowerICmp(ICmpInst *CI) {
    if (wordsOf(CI->getOperand(0)) == 2) { lowerICmp32(CI); return; }
    Value *lhs = CI->getOperand(0), *rhs = CI->getOperand(1);
    const CmpInst::Predicate pr = CI->getPredicate();
    const bool sgn = CI->isSigned();
    std::string r = aluOperand(rhs);
    loadToA(lhs);
    if (sgn && pr != CmpInst::ICMP_EQ && pr != CmpInst::ICMP_NE) {
      emit("    SEC\n");
      emit("    SBC %s        ; lhs - rhs: N != V means lhs < rhs\n", r.c_str());
    } else {
      emit("    CMP %s        ; flags for lhs - rhs\n", r.c_str());
    }

    int trueL = newLabel(), falseL = newLabel(), doneL = newLabel();
    auto jump = [&](const char *op, int label) { bra(op, label); };
    // "N != V" / "N == V" as branches to the true or false label.
    auto signedLess = [&](bool lessIsTrue) {
      int ovf = newLabel();
      jump("BVS", ovf);
      jump("BMI", lessIsTrue ? trueL : falseL);    // no overflow: N set means less
      jump("BRA", lessIsTrue ? falseL : trueL);
      deflabel(ovf);
      jump("BMI", lessIsTrue ? falseL : trueL);    // overflow: N set means N == V, so not less
      jump("BRA", lessIsTrue ? trueL : falseL);
    };

    switch (pr) {
    case CmpInst::ICMP_EQ:  jump("BEQ", trueL); jump("BRA", falseL); break;
    case CmpInst::ICMP_NE:  jump("BEQ", falseL); jump("BRA", trueL); break;
    case CmpInst::ICMP_UGE: jump("BCS", trueL); jump("BRA", falseL); break;
    case CmpInst::ICMP_ULT: jump("BCS", falseL); jump("BRA", trueL); break;
    case CmpInst::ICMP_UGT: jump("BEQ", falseL); jump("BCS", trueL); jump("BRA", falseL); break;
    case CmpInst::ICMP_ULE: jump("BEQ", trueL); jump("BCS", falseL); jump("BRA", trueL); break;
    case CmpInst::ICMP_SLT: signedLess(true); break;
    case CmpInst::ICMP_SGE: signedLess(false); break;
    case CmpInst::ICMP_SGT: jump("BEQ", falseL); signedLess(false); break;
    case CmpInst::ICMP_SLE: jump("BEQ", trueL); signedLess(true); break;
    default:
      backendError("unsupported icmp predicate in " + fnName);
      return;
    }

    deflabel(trueL);
    emit("    LDA #$0001\n");
    bra("BRA", doneL);
    deflabel(falseL);
    emit("    LDA #$0000\n");
    deflabel(doneL);
    storeAToResult(CI);
  }

  void lowerGEP(GetElementPtrInst *GEP) {
    // result = base + sum(index_i * stride_i)
    addressToA(GEP->getPointerOperand());
    emit("    STA %d,S     ; gep base\n", sp(scratch1));
    int constOff = 0;
    std::vector<std::pair<Value *, int>> varTerms;
    gep_type_iterator GTI = gep_type_begin(GEP);
    for (auto it = GEP->idx_begin(), end = GEP->idx_end(); it != end;
         ++it, ++GTI) {
      Value *idx = it->get();
      if (StructType *ST = GTI.getStructTypeOrNull()) {
        unsigned field = cast<ConstantInt>(idx)->getZExtValue();
        constOff += (int)DL.getStructLayout(ST)->getElementOffset(field);
      } else {
        int stride = (int)DL.getTypeAllocSize(GTI.getIndexedType())
                         .getFixedValue();
        if (auto *c = dyn_cast<ConstantInt>(idx))
          constOff += (int)c->getSExtValue() * stride;
        else
          varTerms.push_back({idx, stride});
      }
    }
    for (auto &term : varTerms) {
      loadToA(term.first);
      if (term.second != 1)
        emit("    MUL #$%04X       ; idx * stride\n", term.second & 0xFFFF);
      emit("    CLC\n");
      emit("    ADC %d,S\n", sp(scratch1));
      emit("    STA %d,S\n", sp(scratch1));
    }
    emit("    LDA %d,S\n", sp(scratch1));
    if (constOff) {
      emit("    CLC\n");
      emit("    ADC #$%04X       ; + const field offset\n", constOff & 0xFFFF);
    }
    storeAToResult(GEP);
  }

  void lowerCast(Instruction *I) {
    Value *src = I->getOperand(0);
    loadToA(src);                 // A (and X for a 32-bit source)
    const int sw = wordsOf(src), dw = wordsOf(I);
    if (auto *TR = dyn_cast<TruncInst>(I)) {
      unsigned bits = TR->getDestTy()->getIntegerBitWidth();
      if (bits <= 8)
        emit("    AND #$00FF   ; trunc to i8\n");
    } else if (auto *SX = dyn_cast<SExtInst>(I)) {
      if (SX->getSrcTy()->getIntegerBitWidth() <= 8) {
        int pos = newLabel();
        emit("    AND #$00FF\n");
        emit("    CMP #$0080\n");
        emit("    BMI .L%s_%d\n", fnName.c_str(), pos);
        emit("    ORA #$FF00   ; sign-extend i8\n");
        emit(".L%s_%d:\n", fnName.c_str(), pos);
      }
      if (dw == 2 && sw == 1) {   // X = the sign of A
        int pos = newLabel(), done = newLabel();
        emit("    CMP #$0000\n");
        emit("    BPL .L%s_%d\n", fnName.c_str(), pos);
        emit("    LDX #$FFFF\n");
        emit("    BRA .L%s_%d\n", fnName.c_str(), done);
        emit(".L%s_%d:\n", fnName.c_str(), pos);
        emit("    LDX #$0000\n");
        emit(".L%s_%d:\n", fnName.c_str(), done);
      }
    } else if (auto *ZX = dyn_cast<ZExtInst>(I)) {
      if (ZX->getSrcTy()->getIntegerBitWidth() <= 8)
        emit("    AND #$00FF   ; zero-extend from i8\n");
      if (dw == 2 && sw == 1)
        emit("    LDX #$0000\n");
    } else if (dw == 2 && sw == 1) {
      emit("    LDX #$0000\n");   // ptrtoint to a 32-bit integer: pointers are one word
    }
    // bitcast / inttoptr: nothing else to do (a 32-bit source's low word is already in A).
    storeAToResult(I);
  }

  void lowerSelect(SelectInst *SI) {
    int elseL = newLabel(), doneL = newLabel();
    loadToA(SI->getCondition());
    emit("    CMP #$0000\n");
    emit("    BEQ .L%s_%d\n", fnName.c_str(), elseL);
    loadToA(SI->getTrueValue());
    emit("    BRA .L%s_%d\n", fnName.c_str(), doneL);
    emit(".L%s_%d:\n", fnName.c_str(), elseL);
    loadToA(SI->getFalseValue());
    emit(".L%s_%d:\n", fnName.c_str(), doneL);
    storeAToResult(SI);
  }

  static bool hasPhi(const BasicBlock *BB) {
    return !BB->phis().empty();
  }

  // Emit copies that move each PHI's incoming value (for this edge) into the
  // PHI's frame slot. Done sequentially, which is correct for the -O0 patterns
  // clang emits for && / || / ?: .
  void emitPhiCopies(const BasicBlock *pred, const BasicBlock *succ) {
    for (const PHINode &phi : succ->phis()) {
      Value *in = phi.getIncomingValueForBlock(pred);
      loadToA(in);
      emit("    STA %d,S     ; phi\n", sp(slotOf(&phi)));
      if (wordsOf(&phi) == 2) {
        emit("    TXA\n");
        emit("    STA %d,S\n", sp(slotOf(&phi)) + 2);
      }
    }
  }

  // Branch target label for the edge pred->succ: either the block itself, or a
  // dedicated critical-edge block that first performs the PHI copies.
  std::string edgeTarget(const BasicBlock *pred, const BasicBlock *succ) {
    auto key = std::make_pair(pred, succ);
    if (edgeLabel.count(key)) {
      char buf[64];
      snprintf(buf, sizeof(buf), ".Ledge_%s_%d", fnName.c_str(),
               edgeLabel[key]);
      return buf;
    }
    char buf[64];
    snprintf(buf, sizeof(buf), ".Lblk_%s_%d", fnName.c_str(), blockId[succ]);
    return buf;
  }

  void lowerBranch(BranchInst *BI) {
    const BasicBlock *cur = BI->getParent();
    if (BI->isUnconditional()) {
      const BasicBlock *succ = BI->getSuccessor(0);
      // Sole successor: safe to place PHI copies inline before the jump.
      if (hasPhi(succ))
        emitPhiCopies(cur, succ);
      emit("    BRA .Lblk_%s_%d\n", fnName.c_str(), blockId[succ]);
      return;
    }
    loadToA(BI->getCondition());
    emit("    CMP #$0000\n");
    emit("    BEQ %s\n", edgeTarget(cur, BI->getSuccessor(1)).c_str());
    emit("    BRA %s\n", edgeTarget(cur, BI->getSuccessor(0)).c_str());
  }

  // S += n without touching X or A: A is parked in Y (free: caller-saved, and no result lives in it).
  void emitPopBytes(int n) {
    emit("    TAY\n");
    emit("    TSC\n");
    emit("    CLC\n");
    emit("    ADC #$%04X\n", n & 0xFFFF);
    emit("    TCS\n");
    emit("    TYA\n");
  }

  void emitEpilogue() {
    if (frameSize)
      emitPopBytes(frameSize);
    emit("    RTS\n");
  }

  void lowerReturn(ReturnInst *RI) {
    if (RI->getReturnValue())
      loadToA(RI->getReturnValue());
    emitEpilogue();
  }

  void lowerCall(CallInst *CI) {
    Function *callee = CI->getCalledFunction();
    unsigned n = CI->arg_size();

    // Where each argument goes: register words (A, X, Y; a 32-bit one takes two) until one does not fit, then
    // everything from that argument on is pushed, right to left.
    std::vector<int> reg(n, -1);
    int regsUsed = 0;
    unsigned stackFrom = n;
    for (unsigned i = 0; i < n; ++i) {
      int w = wordsOf(CI->getArgOperand(i));
      if (stackFrom == n && regsUsed + w <= 3) {
        reg[i] = regsUsed;
        regsUsed += w;
      } else {
        if (stackFrom == n) stackFrom = i;
      }
    }

    // Indirect calls: the only hardware indirect-call opcode is
    // JSR (addr,X), which reads its 2-byte target from addr+X. X is also
    // this ABI's arg1 register, so an indirect call can only safely carry
    // arg0 (A) - a nonzero arg1 in X at the JSR would corrupt the target
    // address lookup (it isn't restored afterward; the callee ends up with
    // X=0 as arg1 too, and the jump itself goes to the wrong place).
    if (!callee && (n > 1 || regsUsed > 1)) {
      backendError("indirect call in " + fnName + " takes " +
                   std::to_string(n) +
                   " arguments; only 0 or 1 (one word) is supported through a function "
                   "pointer (X is both the arg1 register and the "
                   "JSR (addr,X) index register)");
      return;
    }

    if (!callee) {
      // Materialize the callee address into the shared icall_target slot
      // before touching A for arg0 below.
      loadToA(CI->getCalledOperand());
      emit("    STA icall_target\n");
    }

    // Push stack args right-to-left (a 32-bit one low word on top).  spDelta tracks the
    // shifting stack pointer so off,S operands stay correct.
    spDelta = 0;
    for (unsigned i = n; i > stackFrom; --i) {
      Value *a = CI->getArgOperand(i - 1);
      loadToA(a);
      if (wordsOf(a) == 2) {
        emit("    PHX\n");
        spDelta += 2;
      }
      emit("    PHA\n");
      spDelta += 2;
    }
    // Register args, highest register first so A (loaded last) is never disturbed; operands are values
    // already in slots, so loading one cannot clobber another.
    for (int i = (int)stackFrom - 1; i >= 0; --i) {
      Value *a = CI->getArgOperand(i);
      loadToA(a);
      if (wordsOf(a) == 1) {
        if (reg[i] == 1) emit("    TAX\n");
        else if (reg[i] == 2) emit("    TAY\n");
      } else if (reg[i] == 1) {
        emit("    TXY\n");      // X:Y = low:high
        emit("    TAX\n");
      }
    }

    if (callee) {
      emit("    JSR %s\n", callee->getName().str().c_str());
    } else {
      emit("    LDX #$0000\n");
      emit("    JSR (icall_target,X)\n");
    }

    const int pushed = spDelta;
    spDelta = 0;
    if (pushed)
      emitPopBytes(pushed);   // keeps the result in A (and X)
    if (!CI->getType()->isVoidTy())
      storeAToResult(CI);
  }

  void lowerInstruction(Instruction *I) {
    if (auto *AI = dyn_cast<AllocaInst>(I)) return lowerAlloca(AI);
    if (auto *LI = dyn_cast<LoadInst>(I)) return lowerLoad(LI);
    if (auto *SI = dyn_cast<StoreInst>(I)) return lowerStore(SI);
    if (auto *BO = dyn_cast<BinaryOperator>(I)) return lowerBinary(BO);
    if (auto *CI = dyn_cast<ICmpInst>(I)) return lowerICmp(CI);
    if (auto *GEP = dyn_cast<GetElementPtrInst>(I)) return lowerGEP(GEP);
    if (auto *SI = dyn_cast<SelectInst>(I)) return lowerSelect(SI);
    if (auto *BI = dyn_cast<BranchInst>(I)) return lowerBranch(BI);
    if (auto *RI = dyn_cast<ReturnInst>(I)) return lowerReturn(RI);
    if (auto *CI = dyn_cast<CallInst>(I)) return lowerCall(CI);
    if (isa<TruncInst>(I) || isa<SExtInst>(I) || isa<ZExtInst>(I) ||
        isa<BitCastInst>(I) || isa<PtrToIntInst>(I) || isa<IntToPtrInst>(I))
      return lowerCast(I);
    if (isa<UnreachableInst>(I)) { emit("    ; unreachable\n"); return; }
    if (isa<PHINode>(I)) {
      // PHI is realized by copies emitted on predecessor edges (see
      // emitPhiCopies); nothing to do at the PHI's own site.
      return;
    }
    backendError(std::string("unsupported instruction '") +
                 I->getOpcodeName() + "' in " + fnName);
  }

  void run() {
    if (F.isDeclaration())
      return;
    assignSlots();
    int id = 0;
    for (BasicBlock &BB : F)
      blockId[&BB] = id++;

    // A critical edge is one from a block with multiple successors into a block
    // that has PHI nodes. Such edges get a dedicated landing block so the PHI
    // copies happen only on that edge.
    for (BasicBlock &BB : F) {
      auto *BI = dyn_cast<BranchInst>(BB.getTerminator());
      if (!BI || BI->isUnconditional())
        continue;
      for (BasicBlock *succ : successors(&BB)) {
        if (hasPhi(succ)) {
          int lbl = newLabel();
          edgeLabel[{&BB, succ}] = lbl;
          edgeList.push_back({&BB, succ});
        }
      }
    }

    emit("\n; Function: %s\n", fnName.c_str());
    emit("%s:\n", fnName.c_str());
    emit("    REP #$30        ; 16-bit A, X, Y\n");
    // Pushing the register arguments both spills them and starts the frame; only then is A free for TSC/SBC/TCS
    // (allocating first used to destroy arg0).
    if (nRegArgs >= 3) emit("    PHY          ; arg2\n");
    if (nRegArgs >= 2) emit("    PHX          ; arg1\n");
    if (nRegArgs >= 1) emit("    PHA          ; arg0\n");
    if (extraSize) {
      emit("    TSC\n");
      emit("    SEC\n");
      emit("    SBC #$%04X\n", extraSize & 0xFFFF);
      emit("    TCS\n");
    }
    // Copy stack arguments into frame slots: [S+frameSize+1..2] is the return address, so the first stack
    // argument is at S+frameSize+3, the next after it, and a 32-bit one takes two words.
    int stackOff = 0;
    for (Argument &A : F.args()) {
      if (argReg[A.getArgNo()] >= 0) continue;
      for (int w = 0; w < wordsOf(&A); ++w) {
        emit("    LDA %d,S     ; arg%u (stack)\n", frameSize + 3 + stackOff, A.getArgNo());
        emit("    STA %d,S\n", sp(slotOf(&A)) + 2 * w);
        stackOff += 2;
      }
    }

    for (BasicBlock &BB : F) {
      emit(".Lblk_%s_%d:\n", fnName.c_str(), blockId[&BB]);
      for (Instruction &I : BB)
        lowerInstruction(&I);
    }

    // Emit critical-edge landing blocks: PHI copies, then jump to the target.
    for (auto &edge : edgeList) {
      emit(".Ledge_%s_%d:\n", fnName.c_str(), edgeLabel[edge]);
      emitPhiCopies(edge.first, edge.second);
      emit("    BRA .Lblk_%s_%d\n", fnName.c_str(), blockId[edge.second]);
    }
  }
};

// ---------------------------------------------------------------------------
// Global variables
// ---------------------------------------------------------------------------
static void emitGlobal(GlobalVariable &GV, const DataLayout &DL) {
  if (GV.isDeclaration())
    return;
  std::string name = GV.getName().str();
  Type *ty = GV.getValueType();
  Constant *init = GV.getInitializer();

  emit("%s:\n", name.c_str());

  // String / byte array initializer.
  if (auto *arr = dyn_cast<ConstantDataArray>(init)) {
    if (arr->getElementByteSize() == 1) {
      emit("    .byte ");
      for (unsigned i = 0; i < arr->getNumElements(); ++i)
        emit("%s$%02X", i ? "," : "",
             (unsigned)(arr->getElementAsInteger(i) & 0xFF));
      emit("\n");
      return;
    }
  }
  if (auto *ci = dyn_cast<ConstantInt>(init)) {
    const uint64_t sz = DL.getTypeAllocSize(ty).getFixedValue();
    if (sz == 1) {
      emit("    .byte $%02X\n", (unsigned)(ci->getZExtValue() & 0xFF));
    } else {
      emit("    .word $%04X\n", (unsigned)(ci->getZExtValue() & 0xFFFF));
      if (sz == 4)
        emit("    .word $%04X\n", (unsigned)((ci->getZExtValue() >> 16) & 0xFFFF));
    }
    return;
  }
  if (isa<ConstantAggregateZero>(init) || init->isNullValue()) {
    uint64_t bytes = DL.getTypeAllocSize(ty).getFixedValue();
    for (uint64_t i = 0; i < (bytes + 1) / 2; ++i)
      emit("    .word $0000\n");
    return;
  }
  emit("    .word $0000  ; unsupported global initializer\n");
}

// ---------------------------------------------------------------------------
// Driver
// ---------------------------------------------------------------------------
static bool runClang(const std::string &cSource, const std::string &llOut,
                     int optLevel, const std::string &userFlags) {
  // clang front end -> textual LLVM IR using the 16-bit MSP430 data layout.
  char cmd[4096];
  // -include stdbool.h makes bare `bool`/`true`/`false` work without an
  // explicit include, matching def88186cc's built-in bool support (the
  // "modern 1992 C" dialect both compilers accept).  userFlags carries the
  // -I/-D options passed through on the zpcc command line.
  snprintf(cmd, sizeof(cmd),
           "clang --target=msp430 -S -emit-llvm -O%d -ffreestanding "
           "-include stdbool.h -Wno-implicit-function-declaration %s "
           "-o '%s' '%s'",
           optLevel, userFlags.c_str(), llOut.c_str(), cSource.c_str());
  int rc = system(cmd);
  return rc == 0;
}

static void emitModule(Module &M) {
  const DataLayout &DL = M.getDataLayout();
  emit("; Generated by zpcc (clang/LLVM DEF88186 backend)\n");
  emit("; ZeroPoint Fantasy Console\n\n");
  emit(".data\n");
  emit("icall_target: .word 0\n");
  for (GlobalVariable &GV : M.globals())
    emitGlobal(GV, DL);
  emit("\n.code\n");
  for (Function &F : M) {
    FnLower FL(F, DL);
    FL.run();
  }
}

} // namespace

static void usage(const char *prog) {
  fprintf(stderr,
          "zpcc - clang/LLVM C compiler for the DEF88186 CPU\n\n"
          "Usage: %s [options] <input.c|input.ll>\n\n"
          "Options:\n"
          "  -o <file>     Output assembly file (default: <input>.asm)\n"
          "  -I <dir>      Add a header search directory (forwarded to clang)\n"
          "  -D <macro>    Define a preprocessor macro (forwarded to clang)\n"
          "  -O<n>         clang optimization level for the front end (default 0)\n"
          "  --emit-llvm   Emit the intermediate .ll and stop\n"
          "  --keep-ll     Keep the intermediate .ll file\n"
          "  -h, --help    Show this help\n",
          prog);
}

int main(int argc, char **argv) {
  std::string input, output, userFlags;
  int optLevel = 0;
  bool emitLLVMOnly = false, keepLL = false;

  // Shell-quote a preprocessor flag argument for the clang command line.
  auto appendFlag = [&](const std::string &f) {
    if (!userFlags.empty()) userFlags += ' ';
    userFlags += '\'' + f + '\'';
  };

  for (int i = 1; i < argc; ++i) {
    std::string a = argv[i];
    if (a == "-h" || a == "--help") { usage(argv[0]); return 0; }
    else if (a == "-o" && i + 1 < argc) output = argv[++i];
    else if (a == "--emit-llvm") emitLLVMOnly = true;
    else if (a == "--keep-ll") keepLL = true;
    else if (a.rfind("-O", 0) == 0 && a.size() >= 2) optLevel = atoi(a.c_str() + 2);
    // Preprocessor flags forwarded to the clang front end.
    else if (a == "-I" && i + 1 < argc) { appendFlag("-I" + std::string(argv[++i])); }
    else if (a == "-D" && i + 1 < argc) { appendFlag("-D" + std::string(argv[++i])); }
    else if (a.rfind("-I", 0) == 0 && a.size() > 2) { appendFlag(a); }
    else if (a.rfind("-D", 0) == 0 && a.size() > 2) { appendFlag(a); }
    else if (!a.empty() && a[0] == '-') { fprintf(stderr, "zpcc: unknown option %s\n", a.c_str()); return 1; }
    else input = a;
  }

  if (input.empty()) { usage(argv[0]); return 1; }

  std::string base = input;
  size_t dot = base.find_last_of('.');
  if (dot != std::string::npos) base = base.substr(0, dot);
  if (output.empty()) output = base + ".asm";

  bool isIR = input.size() > 3 && input.substr(input.size() - 3) == ".ll";
  std::string llFile = isIR ? input : base + ".ll";

  if (!isIR) {
    if (!runClang(input, llFile, optLevel, userFlags)) {
      fprintf(stderr, "zpcc: clang front end failed\n");
      return 1;
    }
  }

  if (emitLLVMOnly) {
    printf("Emitted LLVM IR: %s\n", llFile.c_str());
    return 0;
  }

  LLVMContext ctx;
  SMDiagnostic err;
  std::unique_ptr<Module> M = parseIRFile(llFile, err, ctx);
  if (!M) {
    err.print("zpcc", errs());
    return 1;
  }

  out = fopen(output.c_str(), "w");
  if (!out) { fprintf(stderr, "zpcc: cannot open %s\n", output.c_str()); return 1; }
  emitModule(*M);
  fclose(out);

  if (!isIR && !keepLL)
    unlink(llFile.c_str());

  if (hadError) {
    fprintf(stderr, "zpcc: compilation completed with errors; %s may be incomplete\n",
            output.c_str());
    return 1;
  }
  printf("Wrote %s\n", output.c_str());
  return 0;
}

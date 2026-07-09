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
  std::string fnName;

  FnLower(Function &Fn, const DataLayout &Layout) : F(Fn), DL(Layout) {
    fnName = F.getName().str();
  }

  int newLabel() { return labelCounter++; }

  static int roundWord(int n) { return (n + 1) & ~1; }

  // Byte size of a value when materialized in a 16-bit slot.
  int valueWords() const { return 2; }

  // Assign a frame slot to every argument, alloca (its storage), and
  // value-producing instruction.  Returns total frame size.
  void assignSlots() {
    int off = 0;
    for (Argument &A : F.args()) {
      slot[&A] = off;
      off += 2;
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
          off += 2;
        }
      }
    }
    frameSize = roundWord(off);
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
      int off = slotOf(AI) + spDelta;
      emit("    TSC\n");
      if (off != 0) {
        emit("    CLC\n");
        emit("    ADC #$%04X\n", off & 0xFFFF);
      }
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
      emit("    LDA %d,S\n", slotOf(V) + spDelta);
      return;
    }
    backendError("cannot load operand in " + fnName);
    emit("    LDA #$0000\n");
  }

  // Store A into the slot of a value-producing instruction.
  void storeAToResult(Instruction *I) {
    if (I->getType()->isVoidTy())
      return;
    emit("    STA %d,S\n", slotOf(I) + spDelta);
  }

  // Compute the effective address of a pointer operand into A.
  void addressToA(Value *Ptr) {
    if (auto *AI = dyn_cast<AllocaInst>(Ptr->stripPointerCasts())) {
      int off = slotOf(AI) + spDelta;
      emit("    TSC\n");
      if (off != 0) {
        emit("    CLC\n");
        emit("    ADC #$%04X\n", off & 0xFFFF);
      }
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
    if (auto *AI = dyn_cast<AllocaInst>(Ptr->stripPointerCasts())) {
      emit("    LDA %d,S\n", slotOf(AI) + spDelta);
    } else {
      addressToA(Ptr);
      emit("    TCD          ; DP = pointer\n");
      emit("    LDA $00      ; load through pointer\n");
    }
    storeAToResult(LI);
  }

  void lowerStore(StoreInst *SI) {
    Value *Val = SI->getValueOperand();
    Value *Ptr = SI->getPointerOperand();
    if (auto *AI = dyn_cast<AllocaInst>(Ptr->stripPointerCasts())) {
      loadToA(Val);
      emit("    STA %d,S\n", slotOf(AI) + spDelta);
    } else {
      loadToA(Val);
      emit("    STA t0\n");
      addressToA(Ptr);
      emit("    TCD          ; DP = pointer\n");
      emit("    LDA t0\n");
      emit("    STA $00      ; store through pointer\n");
    }
  }

  void lowerBinary(BinaryOperator *BO) {
    unsigned op = BO->getOpcode();
    Value *lhs = BO->getOperand(0), *rhs = BO->getOperand(1);
    switch (op) {
    case Instruction::Add:
    case Instruction::Sub:
    case Instruction::And:
    case Instruction::Or:
    case Instruction::Xor:
      loadToA(rhs);
      emit("    STA t0\n");
      loadToA(lhs);
      if (op == Instruction::Add) { emit("    CLC\n"); emit("    ADC t0\n"); }
      else if (op == Instruction::Sub) { emit("    SEC\n"); emit("    SBC t0\n"); }
      else if (op == Instruction::And) emit("    AND t0\n");
      else if (op == Instruction::Or)  emit("    ORA t0\n");
      else emit("    EOR t0\n");
      break;
    case Instruction::Mul:
      loadToA(rhs);
      emit("    TAX\n");
      loadToA(lhs);
      emit("    MUL X\n");
      break;
    case Instruction::SDiv:
    case Instruction::UDiv:
      loadToA(rhs);
      emit("    TAX\n");
      loadToA(lhs);
      emit("    DIV X\n");
      break;
    case Instruction::SRem:
    case Instruction::URem:
      loadToA(rhs);
      emit("    TAX\n");
      loadToA(lhs);
      emit("    STA t1\n");
      emit("    DIV X\n");
      emit("    MUL X\n");
      emit("    STA t0\n");
      emit("    LDA t1\n");
      emit("    SEC\n");
      emit("    SBC t0\n");
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
      emit("    %s\n", op == Instruction::Shl ? "ASL A" : "LSR A");
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

  void bra(const char *op, int label) {
    emit("    %s .L%s_%d\n", op, fnName.c_str(), label);
  }
  void deflabel(int label) { emit(".L%s_%d:\n", fnName.c_str(), label); }

  // The DEF88186 assembler only provides BEQ, BMI, BGE, BCC, BCS, BVS, BRA
  // (no BNE / BPL). Comparisons are synthesized from that set. Signed relations
  // use BGE (N==V, overflow-correct); unsigned use the carry (BCS = "unsigned
  // >=", BCC = "unsigned <"). Each predicate is expressed as a small list of
  // conditional branches (to the true or false result) plus a default result.
  void lowerICmp(ICmpInst *CI) {
    Value *lhs = CI->getOperand(0), *rhs = CI->getOperand(1);
    loadToA(rhs);
    emit("    STA t0\n");
    loadToA(lhs);
    emit("    CMP t0        ; flags for lhs - rhs\n");

    const char *ge = CI->isSigned() ? "BGE" : "BCS"; // "lhs >= rhs"
    struct Br { const char *op; bool toTrue; };
    std::vector<Br> seq;
    bool dfltTrue;
    switch (CI->getPredicate()) {
    case CmpInst::ICMP_EQ:  seq = {{"BEQ", true}};  dfltTrue = false; break;
    case CmpInst::ICMP_NE:  seq = {{"BEQ", false}}; dfltTrue = true;  break;
    case CmpInst::ICMP_SGE:
    case CmpInst::ICMP_UGE: seq = {{ge, true}};     dfltTrue = false; break;
    case CmpInst::ICMP_SLT:
    case CmpInst::ICMP_ULT: seq = {{ge, false}};    dfltTrue = true;  break;
    case CmpInst::ICMP_SGT:
    case CmpInst::ICMP_UGT: seq = {{"BEQ", false}, {ge, true}};  dfltTrue = false; break;
    case CmpInst::ICMP_SLE:
    case CmpInst::ICMP_ULE: seq = {{"BEQ", true}, {ge, false}};  dfltTrue = true;  break;
    default:
      backendError("unsupported icmp predicate in " + fnName);
      return;
    }

    int trueL = newLabel(), falseL = newLabel(), doneL = newLabel();
    for (auto &b : seq)
      bra(b.op, b.toTrue ? trueL : falseL);
    bra("BRA", dfltTrue ? trueL : falseL);
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
    emit("    STA t1       ; gep base\n");
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
      if (term.second != 1) {
        emit("    STA t0\n");           // idx
        emit("    LDA #$%04X\n", term.second & 0xFFFF);
        emit("    TAX\n");
        emit("    LDA t0\n");
        emit("    MUL X            ; idx * stride\n");
      }
      emit("    CLC\n");
      emit("    ADC t1\n");
      emit("    STA t1\n");
    }
    emit("    LDA t1\n");
    if (constOff) {
      emit("    CLC\n");
      emit("    ADC #$%04X       ; + const field offset\n", constOff & 0xFFFF);
    }
    storeAToResult(GEP);
  }

  void lowerCast(Instruction *I) {
    Value *src = I->getOperand(0);
    loadToA(src);
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
    } else if (auto *ZX = dyn_cast<ZExtInst>(I)) {
      if (ZX->getSrcTy()->getIntegerBitWidth() <= 8)
        emit("    AND #$00FF   ; zero-extend from i8\n");
      else if (ZX->getSrcTy()->getIntegerBitWidth() == 1)
        emit("    AND #$0001   ; zero-extend from i1\n");
    }
    // bitcast / ptrtoint / inttoptr: value already correct.
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
      emit("    STA %d,S     ; phi\n", slotOf(&phi));
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

  void emitEpilogue() {
    if (frameSize) {
      emit("    TSC\n");
      emit("    CLC\n");
      emit("    ADC #$%04X\n", frameSize & 0xFFFF);
      emit("    TCS\n");
    }
    emit("    RTS\n");
  }

  void lowerReturn(ReturnInst *RI) {
    if (RI->getReturnValue())
      loadToA(RI->getReturnValue());
    emitEpilogue();
  }

  void lowerCall(CallInst *CI) {
    Function *callee = CI->getCalledFunction();
    if (!callee) {
      backendError("indirect calls are not supported in " + fnName);
      return;
    }
    unsigned n = CI->arg_size();
    unsigned nStack = n > 3 ? n - 3 : 0;

    // Push stack args (indices >= 3) right-to-left.  spDelta tracks the
    // shifting stack pointer so off,S operands stay correct.
    spDelta = 0;
    for (unsigned i = n; i > 3; --i) {
      loadToA(CI->getArgOperand(i - 1));
      emit("    PHA\n");
      spDelta += 2;
    }
    // Register args, loaded last so A holds arg0 at the JSR.
    if (n >= 3) { loadToA(CI->getArgOperand(2)); emit("    TAY\n"); }
    if (n >= 2) { loadToA(CI->getArgOperand(1)); emit("    TAX\n"); }
    if (n >= 1) { loadToA(CI->getArgOperand(0)); }

    emit("    JSR %s\n", callee->getName().str().c_str());

    if (nStack) {
      emit("    TSC\n");
      emit("    CLC\n");
      emit("    ADC #$%04X   ; pop %u stack arg(s)\n", (nStack * 2) & 0xFFFF,
           nStack);
      emit("    TCS\n");
    }
    spDelta = 0;
    if (!CI->getType()->isVoidTy())
      emit("    STA %d,S\n", slotOf(CI));
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
    if (frameSize) {
      emit("    TSC\n");
      emit("    SEC\n");
      emit("    SBC #$%04X\n", frameSize & 0xFFFF);
      emit("    TCS\n");
    }
    // Spill incoming register/stack arguments into their frame slots.
    unsigned ai = 0;
    for (Argument &A : F.args()) {
      if (ai == 0) emit("    STA %d,S     ; arg0\n", slotOf(&A));
      else if (ai == 1) { emit("    TXA\n"); emit("    STA %d,S     ; arg1\n", slotOf(&A)); }
      else if (ai == 2) { emit("    TYA\n"); emit("    STA %d,S     ; arg2\n", slotOf(&A)); }
      else {
        int srcOff = frameSize + 2 + (int)(ai - 3) * 2; // above return addr
        emit("    LDA %d,S     ; arg%u (stack)\n", srcOff, ai);
        emit("    STA %d,S\n", slotOf(&A));
      }
      ++ai;
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
    if (DL.getTypeAllocSize(ty).getFixedValue() == 1)
      emit("    .byte $%02X\n", (unsigned)(ci->getZExtValue() & 0xFF));
    else
      emit("    .word $%04X\n", (unsigned)(ci->getZExtValue() & 0xFFFF));
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
  emit("t0: .word 0\n");
  emit("t1: .word 0\n");
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

// This file is part of SymCC.
//
// SymCC is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// SymCC is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// SymCC. If not, see <https://www.gnu.org/licenses/>.

#include "SymCC/Pass.h"

#include <llvm/ADT/SmallVector.h>
#include <llvm/CodeGen/IntrinsicLowering.h>
#include <llvm/CodeGen/TargetLowering.h>
#include <llvm/CodeGen/TargetSubtargetInfo.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Transforms/Utils/ModuleUtils.h>

#if LLVM_VERSION_MAJOR < 14
#include <llvm/Support/TargetRegistry.h>
#else
#include <llvm/MC/TargetRegistry.h>
#endif

#include "SymCC/Runtime.h"
#include "SymCC/Symbolizer.h"

using namespace llvm;

#ifndef NDEBUG
#define DEBUG(X)                                                               \
  do {                                                                         \
    X;                                                                         \
  } while (false)
#else
#define DEBUG(X) ((void)0)
#endif

char SymbolizeLegacyPass::ID = 0;

// Compiler flag to enable/disable SymCC symbolic execution instrumentation.
// Enabled via: -mllvm -enable-symcc  (per-TU)
//          or: -Wl,-plugin-opt=-enable-symcc  (LTO link)
static cl::opt<bool> ClEnableSymCC(
    "enable-symcc",
    cl::desc("Enable SymCC symbolic execution instrumentation"),
    cl::init(false));

namespace {

static constexpr char kSymCtorName[] = "__sym_ctor";

// Prefix used by RSan SafeStack to mark functions that should not be instrumented
#define NOINSTRUMENT_PREFIX "__noinstrument_"

static bool isNoInstrumentFunction(const Function &F) {
  auto name = F.getName();
  if (name.startswith(NOINSTRUMENT_PREFIX))
    return true;
  // Support for mangled C++ names
  if (name.startswith("_Z") && name.find(NOINSTRUMENT_PREFIX, 2) != StringRef::npos)
    return true;
  return false;
}

static bool shouldInstrument(Function &F) {
  if (F.isDeclaration())
    return false;

  if (isNoInstrumentFunction(F))
    return false;

  // Skip SymCC's own constructor (including duplicates like __sym_ctor.1)
  if (F.getName().startswith(kSymCtorName))
    return false;

  // Skip SafeStack runtime initialization
  if (F.getName() == "__safestack_init")
    return false;

  // Skip compiler-rt interceptor functions
  if (F.getName().startswith("__interceptor_"))
    return false;

  // Respect LLVM's disable sanitizer instrumentation attribute
  if (F.hasFnAttribute(llvm::Attribute::DisableSanitizerInstrumentation))
    return false;

  return true;
}

bool instrumentModule(Module &M) {
  DEBUG(errs() << "Symbolizer module instrumentation (LTO CodeGen phase)\n");

  LLVMContext &C = M.getContext();
  IntegerType *Int8Ty  = IntegerType::getInt8Ty(C);
  IntegerType *Int32Ty = IntegerType::getInt32Ty(C);

  /* Get globals for the SHM region and the previous location. Note that
     __afl_prev_loc is thread-local. */

  GlobalVariable *AFLMapPtr = new GlobalVariable(
      M, PointerType::get(Int8Ty, 0), false,
      GlobalValue::ExternalLinkage, 0, "__afl_area_ptr");

  GlobalVariable *AFLPrevLoc = new GlobalVariable(
      M, Int32Ty, false, GlobalValue::ExternalLinkage, 0, "__afl_prev_loc",
      0, GlobalVariable::GeneralDynamicTLSModel, 0, false);

  llvm::appendToUsed(M, {AFLMapPtr, AFLPrevLoc});

  // Note: We intentionally do NOT globally rename intercepted functions
  // (e.g., malloc -> malloc_symbolized) here. Instead, per-call-site
  // redirection is done in Symbolizer::handleFunctionCall. This ensures
  // that functions skipped by shouldInstrument() (like __noinstrument_*
  // SafeStack runtime functions) continue calling the original functions.

  // Insert a constructor that initializes the runtime and any globals.
  Function *ctor;

  std::tie(ctor, std::ignore) = createSanitizerCtorAndInitFunctions(
      M, kSymCtorName, "_sym_initialize", {}, {});
  appendToGlobalCtors(M, ctor, 0);

  std::tie(ctor, std::ignore) = createSanitizerCtorAndInitFunctions(
      M, kSymCtorName, "__afl_auto_init", {}, {});
  appendToGlobalCtors(M, ctor, 0);

  return true;
}

bool canLower(const CallInst *CI) {
  const Function *Callee = CI->getCalledFunction();
  if (!Callee)
    return false;

  switch (Callee->getIntrinsicID()) {
  case Intrinsic::expect:
  case Intrinsic::ctpop:
  case Intrinsic::ctlz:
  case Intrinsic::cttz:
  case Intrinsic::prefetch:
  case Intrinsic::pcmarker:
  case Intrinsic::dbg_declare:
  case Intrinsic::dbg_label:
  case Intrinsic::annotation:
  case Intrinsic::ptr_annotation:
  case Intrinsic::assume:
#if LLVM_VERSION_MAJOR > 11
  case Intrinsic::experimental_noalias_scope_decl:
#endif
  case Intrinsic::var_annotation:
  case Intrinsic::sqrt:
  case Intrinsic::log:
  case Intrinsic::log2:
  case Intrinsic::log10:
  case Intrinsic::exp:
  case Intrinsic::exp2:
  case Intrinsic::pow:
  case Intrinsic::sin:
  case Intrinsic::cos:
  case Intrinsic::floor:
  case Intrinsic::ceil:
  case Intrinsic::trunc:
  case Intrinsic::round:
#if LLVM_VERSION_MAJOR > 10
  case Intrinsic::roundeven:
#endif
  case Intrinsic::copysign:
#if LLVM_VERSION_MAJOR < 16
  case Intrinsic::flt_rounds:
#else
  case Intrinsic::get_rounding:
#endif
  case Intrinsic::invariant_start:
  case Intrinsic::lifetime_start:
  case Intrinsic::invariant_end:
  case Intrinsic::lifetime_end:
    return true;
  default:
    return false;
  }

  llvm_unreachable("Control cannot reach here");
}

void liftInlineAssembly(CallInst *CI) {
  Function *F = CI->getFunction();
  Module *M = F->getParent();
  auto triple = M->getTargetTriple();

  std::string error;
  auto target = TargetRegistry::lookupTarget(triple, error);
  if (!target) {
    errs() << "Warning: can't get target info to lift inline assembly\n";
    return;
  }

  auto cpu = F->getFnAttribute("target-cpu").getValueAsString();
  auto features = F->getFnAttribute("target-features").getValueAsString();

  std::unique_ptr<TargetMachine> TM(
      target->createTargetMachine(triple, cpu, features, TargetOptions(), {}));
  auto subTarget = TM->getSubtargetImpl(*F);
  if (subTarget == nullptr)
    return;

  auto targetLowering = subTarget->getTargetLowering();
  if (targetLowering == nullptr)
    return;

  targetLowering->ExpandInlineAsm(CI);
}

bool instrumentFunction(Function &F) {
  if (!shouldInstrument(F))
    return false;

  DEBUG(errs() << "Symbolizing function ");
  DEBUG(errs().write_escaped(F.getName()) << '\n');

  SmallVector<Instruction *, 0> allInstructions;
  allInstructions.reserve(F.getInstructionCount());
  for (auto &I : instructions(F))
    allInstructions.push_back(&I);

  IntrinsicLowering IL(F.getParent()->getDataLayout());
  for (auto *I : allInstructions) {
    if (auto *CI = dyn_cast<CallInst>(I)) {
      if (canLower(CI)) {
        IL.LowerIntrinsicCall(CI);
      } else if (isa<InlineAsm>(CI->getCalledOperand())) {
        liftInlineAssembly(CI);
      }
    }
  }

  allInstructions.clear();
  for (auto &I : instructions(F))
    allInstructions.push_back(&I);

  Symbolizer symbolizer(*F.getParent());
  symbolizer.symbolizeFunctionArguments(F);

  for (auto &basicBlock : F)
    symbolizer.insertBasicBlockNotification(basicBlock);

  for (auto *instPtr : allInstructions)
    symbolizer.visit(instPtr);

  symbolizer.finalizePHINodes();
  symbolizer.shortCircuitExpressionUses();

  assert(!verifyFunction(F, &errs()) &&
         "SymbolizePass produced invalid bitcode");

  // -----------------------------------------------------------------------------
  // AFL coverage instrument
  Module &M = (*F.getParent());
  LLVMContext &C = M.getContext();
  IntegerType *Int8Ty  = IntegerType::getInt8Ty(C);
  IntegerType *Int32Ty = IntegerType::getInt32Ty(C);

  GlobalVariable *AFLMapPtr = M.getGlobalVariable("__afl_area_ptr");
  GlobalVariable *AFLPrevLoc = M.getGlobalVariable("__afl_prev_loc");

  if (!AFLMapPtr || !AFLPrevLoc) {
    errs() << "Error: Missing global variables __afl_area_ptr or __afl_prev_loc\n";
    return false;
  }

  for (auto &BB : F) {
    BasicBlock::iterator IP = BB.getFirstInsertionPt();
    IRBuilder<> IRB(&(*IP));

    unsigned int cur_loc = AFL_R(MAP_SIZE);
    ConstantInt *CurLoc = ConstantInt::get(Int32Ty, cur_loc);

    LoadInst *PrevLoc = IRB.CreateLoad(Int32Ty, AFLPrevLoc);
    PrevLoc->setMetadata(M.getMDKindID("nosanitize"), MDNode::get(C, std::nullopt));
    Value *PrevLocCasted = IRB.CreateZExt(PrevLoc, IRB.getInt32Ty());

    LoadInst *MapPtr = IRB.CreateLoad(PointerType::get(Int8Ty, 0), AFLMapPtr);
    MapPtr->setMetadata(M.getMDKindID("nosanitize"), MDNode::get(C, std::nullopt));
    Value *MapPtrIdx =
        IRB.CreateGEP(Int8Ty, MapPtr, IRB.CreateXor(PrevLocCasted, CurLoc));

    LoadInst *Counter = IRB.CreateLoad(Int8Ty, MapPtrIdx);
    Counter->setMetadata(M.getMDKindID("nosanitize"), MDNode::get(C, std::nullopt));
    Value *Incr = IRB.CreateAdd(Counter, ConstantInt::get(Int8Ty, 1));
    IRB.CreateStore(Incr, MapPtrIdx)
        ->setMetadata(M.getMDKindID("nosanitize"), MDNode::get(C, std::nullopt));

    StoreInst *Store =
        IRB.CreateStore(ConstantInt::get(Int32Ty, cur_loc >> 1), AFLPrevLoc);
    Store->setMetadata(M.getMDKindID("nosanitize"), MDNode::get(C, std::nullopt));
  }

  return true;
}

} // namespace

bool SymbolizeLegacyPass::doInitialization(Module &M) {
  if (!ClEnableSymCC)
    return false;
  return instrumentModule(M);
}

bool SymbolizeLegacyPass::runOnFunction(Function &F) {
  if (!ClEnableSymCC)
    return false;
  return instrumentFunction(F);
}

#if LLVM_VERSION_MAJOR >= 13

PreservedAnalyses SymbolizePass::run(Function &F, FunctionAnalysisManager &) {
  if (!ClEnableSymCC)
    return PreservedAnalyses::all();
  return instrumentFunction(F) ? PreservedAnalyses::none()
                               : PreservedAnalyses::all();
}

PreservedAnalyses SymbolizePass::run(Module &M, ModuleAnalysisManager &) {
  if (!ClEnableSymCC)
    return PreservedAnalyses::all();
  return instrumentModule(M) ? PreservedAnalyses::none()
                             : PreservedAnalyses::all();
}

#endif

// Factory function for external use (called from TargetPassConfig)
llvm::FunctionPass *createSymCCSymbolizePass() {
  return new SymbolizeLegacyPass();
}

//===- SafeStack.h - SafeStack Pass (new PM) --------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// M2-A (ADR 0008): new-PM module pass for the vusec RSan extension of
// SafeStack. Registered in PassBuilderPipelines just before the OptimizerLast
// extension point so SymSan's TaintPass (also an OptimizerLast plugin)
// symbolizes the generated memory-safety checks. The legacy codegen-level
// registration (TargetPassConfig) is skipped via the module flag
// "llvm.safestack.done" to prevent double instrumentation.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CODEGEN_SAFESTACK_H
#define LLVM_CODEGEN_SAFESTACK_H

#include "llvm/IR/PassManager.h"

namespace llvm {

class SafeStackPass : public PassInfoMixin<SafeStackPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);
};

} // end namespace llvm

#endif // LLVM_CODEGEN_SAFESTACK_H

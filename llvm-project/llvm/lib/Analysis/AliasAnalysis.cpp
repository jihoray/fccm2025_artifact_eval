//==- AliasAnalysis.cpp - Generic Alias Analysis Interface Implementation --==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
// And has the following additional copyright:
//
// (C) Copyright 2016-2021 Xilinx, Inc.
// Copyright (C) 2023, Advanced Micro Devices, Inc.
// All Rights Reserved.
//
//===----------------------------------------------------------------------===//
//
// This file implements the generic AliasAnalysis interface which is used as the
// common interface used by all clients and implementations of alias analysis.
//
// This file also implements the default version of the AliasAnalysis interface
// that is to be used when no other implementation is specified.  This does some
// simple tests that detect obvious cases: two different global pointers cannot
// alias, a global cannot alias a malloc, two different mallocs cannot alias,
// etc.
//
// This alias analysis implementation really isn't very good for anything, but
// it is very fast, and makes a nice clean default implementation.  Because it
// handles lots of little corner cases, other, more complex, alias analysis
// implementations may choose to rely on this pass to resolve these simple and
// easy cases.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/CFLAndersAliasAnalysis.h"
#include "llvm/Analysis/CFLSteensAliasAnalysis.h"
#include "llvm/Analysis/CaptureTracking.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/MemoryLocation.h"
#include "llvm/Analysis/ObjCARCAliasAnalysis.h"
#include "llvm/Analysis/ScalarEvolutionAliasAnalysis.h"
#include "llvm/Analysis/ScopedNoAliasAA.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TypeBasedAliasAnalysis.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/Pass.h"
#include "llvm/Support/AtomicOrdering.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"
#include <algorithm>
#include <cassert>
#include <functional>
#include <iterator>

using namespace llvm;

/// Allow disabling BasicAA from the AA results. This is particularly useful
/// when testing to isolate a single AA implementation.
static cl::opt<bool> DisableBasicAA("disable-basicaa", cl::Hidden,
                                    cl::init(false));

AAResults::AAResults(AAResults &&Arg)
    : TLI(Arg.TLI), AAs(std::move(Arg.AAs)), AADeps(std::move(Arg.AADeps)) {
  for (auto &AA : AAs)
    AA->setAAResults(this);
}

AAResults::~AAResults() {
// FIXME; It would be nice to at least clear out the pointers back to this
// aggregation here, but we end up with non-nesting lifetimes in the legacy
// pass manager that prevent this from working. In the legacy pass manager
// we'll end up with dangling references here in some cases.
#if 0
  for (auto &AA : AAs)
    AA->setAAResults(nullptr);
#endif
}

bool AAResults::invalidate(Function &F, const PreservedAnalyses &PA,
                           FunctionAnalysisManager::Invalidator &Inv) {
  // Check if the AA manager itself has been invalidated.
  auto PAC = PA.getChecker<AAManager>();
  if (!PAC.preserved() && !PAC.preservedSet<AllAnalysesOn<Function>>())
    return true; // The manager needs to be blown away, clear everything.

  // Check all of the dependencies registered.
  for (AnalysisKey *ID : AADeps)
    if (Inv.invalidate(ID, F, PA))
      return true;

  // Everything we depend on is still fine, so are we. Nothing to invalidate.
  return false;
}

//===----------------------------------------------------------------------===//
// Default chaining methods
//===----------------------------------------------------------------------===//

AliasResult AAResults::alias(const MemoryLocation &LocA,
                             const MemoryLocation &LocB) {
  for (const auto &AA : AAs) {
    auto Result = AA->alias(LocA, LocB);
    if (Result != MayAlias)
      return Result;
  }
  return MayAlias;
}

bool AAResults::pointsToConstantMemory(const MemoryLocation &Loc,
                                       bool OrLocal) {
  for (const auto &AA : AAs)
    if (AA->pointsToConstantMemory(Loc, OrLocal))
      return true;

  return false;
}

ModRefInfo AAResults::getArgModRefInfo(ImmutableCallSite CS, unsigned ArgIdx) {
  ModRefInfo Result = ModRefInfo::ModRef;

  for (const auto &AA : AAs) {
    Result = intersectModRef(Result, AA->getArgModRefInfo(CS, ArgIdx));

    // Early-exit the moment we reach the bottom of the lattice.
    if (isNoModRef(Result))
      return ModRefInfo::NoModRef;
  }

  return Result;
}

ModRefInfo AAResults::getModRefInfo(Instruction *I1, Instruction *I2) {
  if (auto CS2 = ImmutableCallSite(I2)) {
    return getModRefInfo(I1, CS2);
  } else if (I2->isFenceLike()) {
    // If this is a fence, just return ModRef.
    return ModRefInfo::ModRef;
  } else {
    const MemoryLocation Loc2 = MemoryLocation::get(I2);
    ModRefInfo MR = getModRefInfo(I1, Loc2);
    if (isModOrRefSet(MR))
      return setModAndRef(MR);
  }
  return ModRefInfo::NoModRef;
}

ModRefInfo AAResults::getModRefInfo(Instruction *I, ImmutableCallSite Call) {
  // We may have two calls.
  if (auto CS = ImmutableCallSite(I)) {
    // Check if the two calls modify the same memory.
    return getModRefInfo(CS, Call);
  } else if (I->isFenceLike()) {
    // If this is a fence, just return ModRef.
    return ModRefInfo::ModRef;
  } else {
    // Otherwise, check if the call modifies or references the
    // location this memory access defines.  The best we can say
    // is that if the call references what this instruction
    // defines, it must be clobbered by this location.
    const MemoryLocation DefLoc = MemoryLocation::get(I);
    assert(DefLoc.Ptr && "Expect non null Memory Location!");
    ModRefInfo MR = getModRefInfo(Call, DefLoc);
    if (isModOrRefSet(MR))
      return setModAndRef(MR);
  }
  return ModRefInfo::NoModRef;
}

ModRefInfo AAResults::getModRefInfo(ImmutableCallSite CS,
                                    const MemoryLocation &Loc) {
  ModRefInfo Result = ModRefInfo::ModRef;

  for (const auto &AA : AAs) {
    Result = intersectModRef(Result, AA->getModRefInfo(CS, Loc));

    // Early-exit the moment we reach the bottom of the lattice.
    if (isNoModRef(Result))
      return ModRefInfo::NoModRef;
  }

  // Try to refine the mod-ref info further using other API entry points to the
  // aggregate set of AA results.
  auto MRB = getModRefBehavior(CS);
  if (MRB == FMRB_DoesNotAccessMemory ||
      MRB == FMRB_OnlyAccessesInaccessibleMem)
    return ModRefInfo::NoModRef;

  if (onlyReadsMemory(MRB))
    Result = clearMod(Result);
  else if (doesNotReadMemory(MRB))
    Result = clearRef(Result);

  if (onlyAccessesArgPointees(MRB) || onlyAccessesInaccessibleOrArgMem(MRB)) {
    bool DoesAlias = false;
    bool IsMustAlias = true;
    ModRefInfo AllArgsMask = ModRefInfo::NoModRef;
    if (doesAccessArgPointees(MRB)) {
      for (auto AI = CS.arg_begin(), AE = CS.arg_end(); AI != AE; ++AI) {
        const Value *Arg = *AI;
        if (!Arg->getType()->isPointerTy())
          continue;
        unsigned ArgIdx = std::distance(CS.arg_begin(), AI);
        MemoryLocation ArgLoc = MemoryLocation::getForArgument(CS, ArgIdx, TLI);
        AliasResult ArgAlias = alias(ArgLoc, Loc);
        if (ArgAlias != NoAlias) {
          ModRefInfo ArgMask = getArgModRefInfo(CS, ArgIdx);
          DoesAlias = true;
          AllArgsMask = unionModRef(AllArgsMask, ArgMask);
        }
        // Conservatively clear IsMustAlias unless only MustAlias is found.
        IsMustAlias &= (ArgAlias == MustAlias);
      }
    }
    // Return NoModRef if no alias found with any argument.
    if (!DoesAlias)
      return ModRefInfo::NoModRef;
    // Logical & between other AA analyses and argument analysis.
    Result = intersectModRef(Result, AllArgsMask);
    // If only MustAlias found above, set Must bit.
    Result = IsMustAlias ? setMust(Result) : clearMust(Result);
  }

  // If Loc is a constant memory location, the call definitely could not
  // modify the memory location.
  if (isModSet(Result) && pointsToConstantMemory(Loc, /*OrLocal*/ false))
    Result = clearMod(Result);

  return Result;
}

ModRefInfo AAResults::getModRefInfo(ImmutableCallSite CS1,
                                    ImmutableCallSite CS2) {
  ModRefInfo Result = ModRefInfo::ModRef;

  for (const auto &AA : AAs) {
    Result = intersectModRef(Result, AA->getModRefInfo(CS1, CS2));

    // Early-exit the moment we reach the bottom of the lattice.
    if (isNoModRef(Result))
      return ModRefInfo::NoModRef;
  }

  // Try to refine the mod-ref info further using other API entry points to the
  // aggregate set of AA results.

  // If CS1 or CS2 are readnone, they don't interact.
  auto CS1B = getModRefBehavior(CS1);
  if (CS1B == FMRB_DoesNotAccessMemory)
    return ModRefInfo::NoModRef;

  auto CS2B = getModRefBehavior(CS2);
  if (CS2B == FMRB_DoesNotAccessMemory)
    return ModRefInfo::NoModRef;

  // If they both only read from memory, there is no dependence.
  if (onlyReadsMemory(CS1B) && onlyReadsMemory(CS2B))
    return ModRefInfo::NoModRef;

  // If CS1 only reads memory, the only dependence on CS2 can be
  // from CS1 reading memory written by CS2.
  if (onlyReadsMemory(CS1B))
    Result = clearMod(Result);
  else if (doesNotReadMemory(CS1B))
    Result = clearRef(Result);

  // If CS2 only access memory through arguments, accumulate the mod/ref
  // information from CS1's references to the memory referenced by
  // CS2's arguments.
  if (onlyAccessesArgPointees(CS2B)) {
    if (!doesAccessArgPointees(CS2B))
      return ModRefInfo::NoModRef;
    ModRefInfo R = ModRefInfo::NoModRef;
    bool IsMustAlias = true;
    for (auto I = CS2.arg_begin(), E = CS2.arg_end(); I != E; ++I) {
      const Value *Arg = *I;
      if (!Arg->getType()->isPointerTy())
        continue;
      unsigned CS2ArgIdx = std::distance(CS2.arg_begin(), I);
      auto CS2ArgLoc = MemoryLocation::getForArgument(CS2, CS2ArgIdx, TLI);

      // ArgModRefCS2 indicates what CS2 might do to CS2ArgLoc, and the
      // dependence of CS1 on that location is the inverse:
      // - If CS2 modifies location, dependence exists if CS1 reads or writes.
      // - If CS2 only reads location, dependence exists if CS1 writes.
      ModRefInfo ArgModRefCS2 = getArgModRefInfo(CS2, CS2ArgIdx);
      ModRefInfo ArgMask = ModRefInfo::NoModRef;
      if (isModSet(ArgModRefCS2))
        ArgMask = ModRefInfo::ModRef;
      else if (isRefSet(ArgModRefCS2))
        ArgMask = ModRefInfo::Mod;

      // ModRefCS1 indicates what CS1 might do to CS2ArgLoc, and we use
      // above ArgMask to update dependence info.
      ModRefInfo ModRefCS1 = getModRefInfo(CS1, CS2ArgLoc);
      ArgMask = intersectModRef(ArgMask, ModRefCS1);

      // Conservatively clear IsMustAlias unless only MustAlias is found.
      IsMustAlias &= isMustSet(ModRefCS1);

      R = intersectModRef(unionModRef(R, ArgMask), Result);
      if (R == Result) {
        // On early exit, not all args were checked, cannot set Must.
        if (I + 1 != E)
          IsMustAlias = false;
        break;
      }
    }

    if (isNoModRef(R))
      return ModRefInfo::NoModRef;

    // If MustAlias found above, set Must bit.
    return IsMustAlias ? setMust(R) : clearMust(R);
  }

  // If CS1 only accesses memory through arguments, check if CS2 references
  // any of the memory referenced by CS1's arguments. If not, return NoModRef.
  if (onlyAccessesArgPointees(CS1B)) {
    if (!doesAccessArgPointees(CS1B))
      return ModRefInfo::NoModRef;
    ModRefInfo R = ModRefInfo::NoModRef;
    bool IsMustAlias = true;
    for (auto I = CS1.arg_begin(), E = CS1.arg_end(); I != E; ++I) {
      const Value *Arg = *I;
      if (!Arg->getType()->isPointerTy())
        continue;
      unsigned CS1ArgIdx = std::distance(CS1.arg_begin(), I);
      auto CS1ArgLoc = MemoryLocation::getForArgument(CS1, CS1ArgIdx, TLI);

      // ArgModRefCS1 indicates what CS1 might do to CS1ArgLoc; if CS1 might
      // Mod CS1ArgLoc, then we care about either a Mod or a Ref by CS2. If
      // CS1 might Ref, then we care only about a Mod by CS2.
      ModRefInfo ArgModRefCS1 = getArgModRefInfo(CS1, CS1ArgIdx);
      ModRefInfo ModRefCS2 = getModRefInfo(CS2, CS1ArgLoc);
      if ((isModSet(ArgModRefCS1) && isModOrRefSet(ModRefCS2)) ||
          (isRefSet(ArgModRefCS1) && isModSet(ModRefCS2)))
        R = intersectModRef(unionModRef(R, ArgModRefCS1), Result);

      // Conservatively clear IsMustAlias unless only MustAlias is found.
      IsMustAlias &= isMustSet(ModRefCS2);

      if (R == Result) {
        // On early exit, not all args were checked, cannot set Must.
        if (I + 1 != E)
          IsMustAlias = false;
        break;
      }
    }

    if (isNoModRef(R))
      return ModRefInfo::NoModRef;

    // If MustAlias found above, set Must bit.
    return IsMustAlias ? setMust(R) : clearMust(R);
  }

  return Result;
}

FunctionModRefBehavior AAResults::getModRefBehavior(ImmutableCallSite CS) {
  FunctionModRefBehavior Result = FMRB_UnknownModRefBehavior;

  for (const auto &AA : AAs) {
    Result = FunctionModRefBehavior(Result & AA->getModRefBehavior(CS));

    // Early-exit the moment we reach the bottom of the lattice.
    if (Result == FMRB_DoesNotAccessMemory)
      return Result;
  }

  return Result;
}

FunctionModRefBehavior AAResults::getModRefBehavior(const Function *F) {
  FunctionModRefBehavior Result = FMRB_UnknownModRefBehavior;

  for (const auto &AA : AAs) {
    Result = FunctionModRefBehavior(Result & AA->getModRefBehavior(F));

    // Early-exit the moment we reach the bottom of the lattice.
    if (Result == FMRB_DoesNotAccessMemory)
      return Result;
  }

  return Result;
}

//===----------------------------------------------------------------------===//
// Helper method implementation
//===----------------------------------------------------------------------===//

ModRefInfo AAResults::getModRefInfo(const LoadInst *L,
                                    const MemoryLocation &Loc) {
  // Be conservative in the face of atomic.
  if (isStrongerThan(L->getOrdering(), AtomicOrdering::Unordered))
    return ModRefInfo::ModRef;

  // If the load address doesn't alias the given address, it doesn't read
  // or write the specified memory.
  if (Loc.Ptr) {
    AliasResult AR = alias(MemoryLocation::get(L), Loc);
    if (AR == NoAlias)
      return ModRefInfo::NoModRef;
    if (AR == MustAlias)
      return ModRefInfo::MustRef;
  }
  // Otherwise, a load just reads.
  return ModRefInfo::Ref;
}

ModRefInfo AAResults::getModRefInfo(const StoreInst *S,
                                    const MemoryLocation &Loc) {
  // Be conservative in the face of atomic.
  if (isStrongerThan(S->getOrdering(), AtomicOrdering::Unordered))
    return ModRefInfo::ModRef;

  if (Loc.Ptr) {
    AliasResult AR = alias(MemoryLocation::get(S), Loc);
    // If the store address cannot alias the pointer in question, then the
    // specified memory cannot be modified by the store.
    if (AR == NoAlias)
      return ModRefInfo::NoModRef;

    // If the pointer is a pointer to constant memory, then it could not have
    // been modified by this store.
    if (pointsToConstantMemory(Loc))
      return ModRefInfo::NoModRef;

    // If the store address aliases the pointer as must alias, set Must.
    if (AR == MustAlias)
      return ModRefInfo::MustMod;
  }

  // Otherwise, a store just writes.
  return ModRefInfo::Mod;
}

ModRefInfo AAResults::getModRefInfo(const FenceInst *S, const MemoryLocation &Loc) {
  // If we know that the location is a constant memory location, the fence
  // cannot modify this location.
  if (Loc.Ptr && pointsToConstantMemory(Loc))
    return ModRefInfo::Ref;
  return ModRefInfo::ModRef;
}

ModRefInfo AAResults::getModRefInfo(const VAArgInst *V,
                                    const MemoryLocation &Loc) {
  if (Loc.Ptr) {
    AliasResult AR = alias(MemoryLocation::get(V), Loc);
    // If the va_arg address cannot alias the pointer in question, then the
    // specified memory cannot be accessed by the va_arg.
    if (AR == NoAlias)
      return ModRefInfo::NoModRef;

    // If the pointer is a pointer to constant memory, then it could not have
    // been modified by this va_arg.
    if (pointsToConstantMemory(Loc))
      return ModRefInfo::NoModRef;

    // If the va_arg aliases the pointer as must alias, set Must.
    if (AR == MustAlias)
      return ModRefInfo::MustModRef;
  }

  // Otherwise, a va_arg reads and writes.
  return ModRefInfo::ModRef;
}

ModRefInfo AAResults::getModRefInfo(const CatchPadInst *CatchPad,
                                    const MemoryLocation &Loc) {
  if (Loc.Ptr) {
    // If the pointer is a pointer to constant memory,
    // then it could not have been modified by this catchpad.
    if (pointsToConstantMemory(Loc))
      return ModRefInfo::NoModRef;
  }

  // Otherwise, a catchpad reads and writes.
  return ModRefInfo::ModRef;
}

ModRefInfo AAResults::getModRefInfo(const CatchReturnInst *CatchRet,
                                    const MemoryLocation &Loc) {
  if (Loc.Ptr) {
    // If the pointer is a pointer to constant memory,
    // then it could not have been modified by this catchpad.
    if (pointsToConstantMemory(Loc))
      return ModRefInfo::NoModRef;
  }

  // Otherwise, a catchret reads and writes.
  return ModRefInfo::ModRef;
}

ModRefInfo AAResults::getModRefInfo(const AtomicCmpXchgInst *CX,
                                    const MemoryLocation &Loc) {
  // Acquire/Release cmpxchg has properties that matter for arbitrary addresses.
  if (isStrongerThanMonotonic(CX->getSuccessOrdering()))
    return ModRefInfo::ModRef;

  if (Loc.Ptr) {
    AliasResult AR = alias(MemoryLocation::get(CX), Loc);
    // If the cmpxchg address does not alias the location, it does not access
    // it.
    if (AR == NoAlias)
      return ModRefInfo::NoModRef;

    // If the cmpxchg address aliases the pointer as must alias, set Must.
    if (AR == MustAlias)
      return ModRefInfo::MustModRef;
  }

  return ModRefInfo::ModRef;
}

ModRefInfo AAResults::getModRefInfo(const AtomicRMWInst *RMW,
                                    const MemoryLocation &Loc) {
  // Acquire/Release atomicrmw has properties that matter for arbitrary addresses.
  if (isStrongerThanMonotonic(RMW->getOrdering()))
    return ModRefInfo::ModRef;

  if (Loc.Ptr) {
    AliasResult AR = alias(MemoryLocation::get(RMW), Loc);
    // If the atomicrmw address does not alias the location, it does not access
    // it.
    if (AR == NoAlias)
      return ModRefInfo::NoModRef;

    // If the atomicrmw address aliases the pointer as must alias, set Must.
    if (AR == MustAlias)
      return ModRefInfo::MustModRef;
  }

  return ModRefInfo::ModRef;
}

/// \brief Return information about whether a particular call site modifies
/// or reads the specified memory location \p MemLoc before instruction \p I
/// in a BasicBlock.
/// FIXME: this is really just shoring-up a deficiency in alias analysis.
/// BasicAA isn't willing to spend linear time determining whether an alloca
/// was captured before or after this particular call, while we are. However,
/// with a smarter AA in place, this test is just wasting compile time.
ModRefInfo AAResults::callCapturesBefore(const Instruction *I,
                                         const MemoryLocation &MemLoc,
                                         DominatorTree *DT) {
  if (!DT)
    return ModRefInfo::ModRef;

  const Value *Object =
      GetUnderlyingObject(MemLoc.Ptr, I->getModule()->getDataLayout());
  if (!isIdentifiedObject(Object) || isa<GlobalValue>(Object) ||
      isa<Constant>(Object))
    return ModRefInfo::ModRef;

  ImmutableCallSite CS(I);
  if (!CS.getInstruction() || CS.getInstruction() == Object)
    return ModRefInfo::ModRef;

  if (PointerMayBeCapturedBefore(Object, /* ReturnCaptures */ true,
                                 /* StoreCaptures */ true, I, DT,
                                 /* include Object */ true))
    return ModRefInfo::ModRef;

  unsigned ArgNo = 0;
  ModRefInfo R = ModRefInfo::NoModRef;
  bool MustAlias = true;
  // Set flag only if no May found and all operands processed.
  for (auto CI = CS.data_operands_begin(), CE = CS.data_operands_end();
       CI != CE; ++CI, ++ArgNo) {
    // Only look at the no-capture or byval pointer arguments.  If this
    // pointer were passed to arguments that were neither of these, then it
    // couldn't be no-capture.
    if (!(*CI)->getType()->isPointerTy() ||
        (!CS.doesNotCapture(ArgNo) &&
         ArgNo < CS.getNumArgOperands() && !CS.isByValArgument(ArgNo)))
      continue;

    AliasResult AR = alias(MemoryLocation(*CI), MemoryLocation(Object));
    // If this is a no-capture pointer argument, see if we can tell that it
    // is impossible to alias the pointer we're checking.  If not, we have to
    // assume that the call could touch the pointer, even though it doesn't
    // escape.
    if (AR != MustAlias)
      MustAlias = false;
    if (AR == NoAlias)
      continue;
    if (CS.doesNotAccessMemory(ArgNo))
      continue;
    if (CS.onlyReadsMemory(ArgNo)) {
      R = ModRefInfo::Ref;
      continue;
    }
    // Not returning MustModRef since we have not seen all the arguments.
    return ModRefInfo::ModRef;
  }
  return MustAlias ? setMust(R) : clearMust(R);
}

/// canBasicBlockModify - Return true if it is possible for execution of the
/// specified basic block to modify the location Loc.
///
bool AAResults::canBasicBlockModify(const BasicBlock &BB,
                                    const MemoryLocation &Loc) {
  return canInstructionRangeModRef(BB.front(), BB.back(), Loc, ModRefInfo::Mod);
}

/// canInstructionRangeModRef - Return true if it is possible for the
/// execution of the specified instructions to mod\ref (according to the
/// mode) the location Loc. The instructions to consider are all
/// of the instructions in the range of [I1,I2] INCLUSIVE.
/// I1 and I2 must be in the same basic block.
bool AAResults::canInstructionRangeModRef(const Instruction &I1,
                                          const Instruction &I2,
                                          const MemoryLocation &Loc,
                                          const ModRefInfo Mode) {
  assert(I1.getParent() == I2.getParent() &&
         "Instructions not in same basic block!");
  BasicBlock::const_iterator I = I1.getIterator();
  BasicBlock::const_iterator E = I2.getIterator();
  ++E;  // Convert from inclusive to exclusive range.

  for (; I != E; ++I) // Check every instruction in range
    if (isModOrRefSet(intersectModRef(getModRefInfo(&*I, Loc), Mode)))
      return true;
  return false;
}

// Provide a definition for the root virtual destructor.
AAResults::Concept::~Concept() = default;

// Provide a definition for the static object used to identify passes.
AnalysisKey AAManager::Key;

namespace {

/// A wrapper pass for external alias analyses. This just squirrels away the
/// callback used to run any analyses and register their results.
struct ExternalAAWrapperPass : ImmutablePass {
  using CallbackT = std::function<void(Pass &, Function &, AAResults &)>;

  CallbackT CB;

  static char ID;

  ExternalAAWrapperPass() : ImmutablePass(ID) {
    initializeExternalAAWrapperPassPass(*PassRegistry::getPassRegistry());
  }

  explicit ExternalAAWrapperPass(CallbackT CB)
      : ImmutablePass(ID), CB(std::move(CB)) {
    initializeExternalAAWrapperPassPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }
};

} // end anonymous namespace

char ExternalAAWrapperPass::ID = 0;

INITIALIZE_PASS(ExternalAAWrapperPass, "external-aa", "External Alias Analysis",
                false, true)

ImmutablePass *
llvm::createExternalAAWrapperPass(ExternalAAWrapperPass::CallbackT Callback) {
  return new ExternalAAWrapperPass(std::move(Callback));
}

AAResultsWrapperPass::AAResultsWrapperPass() : FunctionPass(ID) {
  initializeAAResultsWrapperPassPass(*PassRegistry::getPassRegistry());
}

char AAResultsWrapperPass::ID = 0;

INITIALIZE_PASS_BEGIN(AAResultsWrapperPass, "aa",
                      "Function Alias Analysis Results", false, true)
INITIALIZE_PASS_DEPENDENCY(BasicAAWrapperPass)
INITIALIZE_PASS_DEPENDENCY(CFLAndersAAWrapperPass)
INITIALIZE_PASS_DEPENDENCY(CFLSteensAAWrapperPass)
INITIALIZE_PASS_DEPENDENCY(ExternalAAWrapperPass)
INITIALIZE_PASS_DEPENDENCY(GlobalsAAWrapperPass)
INITIALIZE_PASS_DEPENDENCY(ObjCARCAAWrapperPass)
INITIALIZE_PASS_DEPENDENCY(SCEVAAWrapperPass)
INITIALIZE_PASS_DEPENDENCY(ScopedNoAliasAAWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TypeBasedAAWrapperPass)
INITIALIZE_PASS_END(AAResultsWrapperPass, "aa",
                    "Function Alias Analysis Results", false, true)

FunctionPass *llvm::createAAResultsWrapperPass() {
  return new AAResultsWrapperPass();
}

/// Run the wrapper pass to rebuild an aggregation over known AA passes.
///
/// This is the legacy pass manager's interface to the new-style AA results
/// aggregation object. Because this is somewhat shoe-horned into the legacy
/// pass manager, we hard code all the specific alias analyses available into
/// it. While the particular set enabled is configured via commandline flags,
/// adding a new alias analysis to LLVM will require adding support for it to
/// this list.
bool AAResultsWrapperPass::runOnFunction(Function &F) {
  // NB! This *must* be reset before adding new AA results to the new
  // AAResults object because in the legacy pass manager, each instance
  // of these will refer to the *same* immutable analyses, registering and
  // unregistering themselves with them. We need to carefully tear down the
  // previous object first, in this case replacing it with an empty one, before
  // registering new results.
  AAR.reset(
      new AAResults(getAnalysis<TargetLibraryInfoWrapperPass>().getTLI()));

  // BasicAA is always available for function analyses. Also, we add it first
  // so that it can trump TBAA results when it proves MustAlias.
  // FIXME: TBAA should have an explicit mode to support this and then we
  // should reconsider the ordering here.
  if (!DisableBasicAA)
    AAR->addAAResult(getAnalysis<BasicAAWrapperPass>().getResult());

  // Populate the results with the currently available AAs.
  if (auto *WrapperPass = getAnalysisIfAvailable<ScopedNoAliasAAWrapperPass>())
    AAR->addAAResult(WrapperPass->getResult());
  if (auto *WrapperPass = getAnalysisIfAvailable<TypeBasedAAWrapperPass>())
    AAR->addAAResult(WrapperPass->getResult());
  if (auto *WrapperPass =
          getAnalysisIfAvailable<objcarc::ObjCARCAAWrapperPass>())
    AAR->addAAResult(WrapperPass->getResult());
  if (auto *WrapperPass = getAnalysisIfAvailable<GlobalsAAWrapperPass>())
    AAR->addAAResult(WrapperPass->getResult());
  if (auto *WrapperPass = getAnalysisIfAvailable<SCEVAAWrapperPass>())
    AAR->addAAResult(WrapperPass->getResult());
  if (auto *WrapperPass = getAnalysisIfAvailable<CFLAndersAAWrapperPass>())
    AAR->addAAResult(WrapperPass->getResult());
  if (auto *WrapperPass = getAnalysisIfAvailable<CFLSteensAAWrapperPass>())
    AAR->addAAResult(WrapperPass->getResult());

  // If available, run an external AA providing callback over the results as
  // well.
  if (auto *WrapperPass = getAnalysisIfAvailable<ExternalAAWrapperPass>())
    if (WrapperPass->CB)
      WrapperPass->CB(*this, F, *AAR);

  // Analyses don't mutate the IR, so return false.
  return false;
}

void AAResultsWrapperPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<BasicAAWrapperPass>();
  AU.addRequired<TargetLibraryInfoWrapperPass>();

  // We also need to mark all the alias analysis passes we will potentially
  // probe in runOnFunction as used here to ensure the legacy pass manager
  // preserves them. This hard coding of lists of alias analyses is specific to
  // the legacy pass manager.
  AU.addUsedIfAvailable<ScopedNoAliasAAWrapperPass>();
  AU.addUsedIfAvailable<TypeBasedAAWrapperPass>();
  AU.addUsedIfAvailable<objcarc::ObjCARCAAWrapperPass>();
  AU.addUsedIfAvailable<GlobalsAAWrapperPass>();
  AU.addUsedIfAvailable<SCEVAAWrapperPass>();
  AU.addUsedIfAvailable<CFLAndersAAWrapperPass>();
  AU.addUsedIfAvailable<CFLSteensAAWrapperPass>();
}

AAResults llvm::createLegacyPMAAResults(Pass &P, Function &F,
                                        BasicAAResult &BAR) {
  AAResults AAR(P.getAnalysis<TargetLibraryInfoWrapperPass>().getTLI());

  // Add in our explicitly constructed BasicAA results.
  if (!DisableBasicAA)
    AAR.addAAResult(BAR);

  // Populate the results with the other currently available AAs.
  if (auto *WrapperPass =
          P.getAnalysisIfAvailable<ScopedNoAliasAAWrapperPass>())
    AAR.addAAResult(WrapperPass->getResult());
  if (auto *WrapperPass = P.getAnalysisIfAvailable<TypeBasedAAWrapperPass>())
    AAR.addAAResult(WrapperPass->getResult());
  if (auto *WrapperPass =
          P.getAnalysisIfAvailable<objcarc::ObjCARCAAWrapperPass>())
    AAR.addAAResult(WrapperPass->getResult());
  if (auto *WrapperPass = P.getAnalysisIfAvailable<GlobalsAAWrapperPass>())
    AAR.addAAResult(WrapperPass->getResult());
  if (auto *WrapperPass = P.getAnalysisIfAvailable<CFLAndersAAWrapperPass>())
    AAR.addAAResult(WrapperPass->getResult());
  if (auto *WrapperPass = P.getAnalysisIfAvailable<CFLSteensAAWrapperPass>())
    AAR.addAAResult(WrapperPass->getResult());

  return AAR;
}

bool llvm::isNoAliasCall(const Value *V) {
  if (auto CS = ImmutableCallSite(V))
    return CS.hasRetAttr(Attribute::NoAlias);
  return false;
}

bool llvm::isNoAliasArgument(const Value *V) {
  if (const Argument *A = dyn_cast<Argument>(V))
    return A->hasNoAliasAttr();
  return false;
}

bool llvm::isIdentifiedObject(const Value *V) {
  if (isa<AllocaInst>(V))
    return true;
  if (isa<GlobalValue>(V) && !isa<GlobalAlias>(V))
    return true;
  if (isNoAliasCall(V))
    return true;
  if (const Argument *A = dyn_cast<Argument>(V))
    return A->hasNoAliasAttr() || A->hasByValAttr();
  return false;
}

bool llvm::isIdentifiedFunctionLocal(const Value *V) {
  return isa<AllocaInst>(V) || isNoAliasCall(V) || isNoAliasArgument(V);
}

void llvm::getAAResultsAnalysisUsage(AnalysisUsage &AU) {
  // This function needs to be in sync with llvm::createLegacyPMAAResults -- if
  // more alias analyses are added to llvm::createLegacyPMAAResults, they need
  // to be added here also.
  AU.addRequired<TargetLibraryInfoWrapperPass>();
  AU.addUsedIfAvailable<ScopedNoAliasAAWrapperPass>();
  AU.addUsedIfAvailable<TypeBasedAAWrapperPass>();
  AU.addUsedIfAvailable<objcarc::ObjCARCAAWrapperPass>();
  AU.addUsedIfAvailable<GlobalsAAWrapperPass>();
  AU.addUsedIfAvailable<CFLAndersAAWrapperPass>();
  AU.addUsedIfAvailable<CFLSteensAAWrapperPass>();
}

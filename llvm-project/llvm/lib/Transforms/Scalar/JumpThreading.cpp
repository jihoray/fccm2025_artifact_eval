//===- JumpThreading.cpp - Thread control through conditional blocks ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
// And has the following additional copyright:
//
// (C) Copyright 2016-2022 Xilinx, Inc.
// Copyright (C) 2023, Advanced Micro Devices, Inc.
// All Rights Reserved.
//
//===----------------------------------------------------------------------===//
//
// This file implements the Jump Threading pass.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Scalar/JumpThreading.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Analysis/BranchProbabilityInfo.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/Analysis/ConstantFolding.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/InstructionSimplify.h"
#include "llvm/Analysis/LazyValueInfo.h"
#include "llvm/Analysis/Loads.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/Analysis/XILINXFunctionInfoUtils.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/ConstantRange.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/MDBuilder.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Use.h"
#include "llvm/IR/User.h"
#include "llvm/IR/Value.h"
#include "llvm/Pass.h"
#include "llvm/Support/BlockFrequency.h"
#include "llvm/Support/BranchProbability.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/SSAUpdater.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <memory>
#include <utility>

using namespace llvm;
using namespace jumpthreading;

extern cl::opt<bool> HLS;
#define DEBUG_TYPE "jump-threading"

STATISTIC(NumThreads, "Number of jumps threaded");
STATISTIC(NumFolds,   "Number of terminators folded");
STATISTIC(NumDupes,   "Number of branch blocks duplicated to eliminate phi");

static cl::opt<unsigned>
BBDuplicateThreshold("jump-threading-threshold",
          cl::desc("Max block size to duplicate for jump threading"),
          cl::init(6), cl::Hidden);

static cl::opt<unsigned>
ImplicationSearchThreshold(
  "jump-threading-implication-search-threshold",
  cl::desc("The number of predecessors to search for a stronger "
           "condition to use to thread over a weaker condition"),
  cl::init(3), cl::Hidden);

static cl::opt<bool> PrintLVIAfterJumpThreading(
    "print-lvi-after-jump-threading",
    cl::desc("Print the LazyValueInfo cache after JumpThreading"), cl::init(false),
    cl::Hidden);

namespace {

  /// This pass performs 'jump threading', which looks at blocks that have
  /// multiple predecessors and multiple successors.  If one or more of the
  /// predecessors of the block can be proven to always jump to one of the
  /// successors, we forward the edge from the predecessor to the successor by
  /// duplicating the contents of this block.
  ///
  /// An example of when this can occur is code like this:
  ///
  ///   if () { ...
  ///     X = 4;
  ///   }
  ///   if (X < 3) {
  ///
  /// In this case, the unconditional branch at the end of the first if can be
  /// revectored to the false side of the second if.
  class JumpThreading : public FunctionPass {
    JumpThreadingPass Impl;

  public:
    static char ID; // Pass identification

    JumpThreading(int T = -1, bool enableSinkHoist = false) : FunctionPass(ID), Impl(T) {
      initializeJumpThreadingPass(*PassRegistry::getPassRegistry());
    }

    bool runOnFunction(Function &F) override;

    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.addRequired<DominatorTreeWrapperPass>();
      AU.addPreserved<DominatorTreeWrapperPass>();
      AU.addRequired<AAResultsWrapperPass>();
      AU.addRequired<LazyValueInfoWrapperPass>();
      AU.addPreserved<LazyValueInfoWrapperPass>();
      AU.addPreserved<GlobalsAAWrapperPass>();
      AU.addRequired<TargetLibraryInfoWrapperPass>();
      AU.addRequiredTransitive<MemoryDependenceWrapperPass>();
    }

    void releaseMemory() override { Impl.releaseMemory(); }
  };

} // end anonymous namespace

char JumpThreading::ID = 0;

INITIALIZE_PASS_BEGIN(JumpThreading, "jump-threading",
                "Jump Threading", false, false)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LazyValueInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(AAResultsWrapperPass)
INITIALIZE_PASS_DEPENDENCY(MemoryDependenceWrapperPass)
INITIALIZE_PASS_END(JumpThreading, "jump-threading",
                "Jump Threading", false, false)

// Public interface to the Jump Threading pass
FunctionPass *llvm::createJumpThreadingPass(int Threshold, bool enableSinkHoist) {
  return new JumpThreading(Threshold);
}

JumpThreadingPass::JumpThreadingPass(int T, bool enableSinkHoist)
    : EnableSinkHoist(enableSinkHoist) {
  BBDupThreshold = (T == -1) ? BBDuplicateThreshold : unsigned(T);
}

// Update branch probability information according to conditional
// branch probablity. This is usually made possible for cloned branches
// in inline instances by the context specific profile in the caller.
// For instance,
//
//  [Block PredBB]
//  [Branch PredBr]
//  if (t) {
//     Block A;
//  } else {
//     Block B;
//  }
//
//  [Block BB]
//  cond = PN([true, %A], [..., %B]); // PHI node
//  [Branch CondBr]
//  if (cond) {
//    ...  // P(cond == true) = 1%
//  }
//
//  Here we know that when block A is taken, cond must be true, which means
//      P(cond == true | A) = 1
//
//  Given that P(cond == true) = P(cond == true | A) * P(A) +
//                               P(cond == true | B) * P(B)
//  we get:
//     P(cond == true ) = P(A) + P(cond == true | B) * P(B)
//
//  which gives us:
//     P(A) is less than P(cond == true), i.e.
//     P(t == true) <= P(cond == true)
//
//  In other words, if we know P(cond == true) is unlikely, we know
//  that P(t == true) is also unlikely.
//
static void updatePredecessorProfileMetadata(PHINode *PN, BasicBlock *BB) {
  BranchInst *CondBr = dyn_cast<BranchInst>(BB->getTerminator());
  if (!CondBr)
    return;

  BranchProbability BP;
  uint64_t TrueWeight, FalseWeight;
  if (!CondBr->extractProfMetadata(TrueWeight, FalseWeight))
    return;

  // Returns the outgoing edge of the dominating predecessor block
  // that leads to the PhiNode's incoming block:
  auto GetPredOutEdge =
      [](BasicBlock *IncomingBB,
         BasicBlock *PhiBB) -> std::pair<BasicBlock *, BasicBlock *> {
    auto *PredBB = IncomingBB;
    auto *SuccBB = PhiBB;
    while (true) {
      BranchInst *PredBr = dyn_cast<BranchInst>(PredBB->getTerminator());
      if (PredBr && PredBr->isConditional())
        return {PredBB, SuccBB};
      auto *SinglePredBB = PredBB->getSinglePredecessor();
      if (!SinglePredBB)
        return {nullptr, nullptr};
      SuccBB = PredBB;
      PredBB = SinglePredBB;
    }
  };

  for (unsigned i = 0, e = PN->getNumIncomingValues(); i != e; ++i) {
    Value *PhiOpnd = PN->getIncomingValue(i);
    ConstantInt *CI = dyn_cast<ConstantInt>(PhiOpnd);

    if (!CI || !CI->getType()->isIntegerTy(1))
      continue;

    BP = (CI->isOne() ? BranchProbability::getBranchProbability(
                            TrueWeight, TrueWeight + FalseWeight)
                      : BranchProbability::getBranchProbability(
                            FalseWeight, TrueWeight + FalseWeight));

    auto PredOutEdge = GetPredOutEdge(PN->getIncomingBlock(i), BB);
    if (!PredOutEdge.first)
      return;

    BasicBlock *PredBB = PredOutEdge.first;
    BranchInst *PredBr = cast<BranchInst>(PredBB->getTerminator());

    uint64_t PredTrueWeight, PredFalseWeight;
    // FIXME: We currently only set the profile data when it is missing.
    // With PGO, this can be used to refine even existing profile data with
    // context information. This needs to be done after more performance
    // testing.
    if (PredBr->extractProfMetadata(PredTrueWeight, PredFalseWeight))
      continue;

    // We can not infer anything useful when BP >= 50%, because BP is the
    // upper bound probability value.
    if (BP >= BranchProbability(50, 100))
      continue;

    SmallVector<uint32_t, 2> Weights;
    if (PredBr->getSuccessor(0) == PredOutEdge.second) {
      Weights.push_back(BP.getNumerator());
      Weights.push_back(BP.getCompl().getNumerator());
    } else {
      Weights.push_back(BP.getCompl().getNumerator());
      Weights.push_back(BP.getNumerator());
    }
    PredBr->setMetadata(LLVMContext::MD_prof,
                        MDBuilder(PredBr->getParent()->getContext())
                            .createBranchWeights(Weights));
  }
}

/// runOnFunction - Toplevel algorithm.
bool JumpThreading::runOnFunction(Function &F) {
  if (skipFunction(F))
    return false;
  auto TLI = &getAnalysis<TargetLibraryInfoWrapperPass>().getTLI();
  auto DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  auto LVI = &getAnalysis<LazyValueInfoWrapperPass>().getLVI();
  auto AA = &getAnalysis<AAResultsWrapperPass>().getAAResults();
  auto MDR = &getAnalysis<MemoryDependenceWrapperPass>().getMemDep();

  DeferredDominance DDT(*DT);
  std::unique_ptr<BlockFrequencyInfo> BFI;
  std::unique_ptr<BranchProbabilityInfo> BPI;
  bool HasProfileData = F.hasProfileData();
  if (HasProfileData) {
    LoopInfo LI{DominatorTree(F)};
    BPI.reset(new BranchProbabilityInfo(F, LI, TLI));
    BFI.reset(new BlockFrequencyInfo(F, *BPI, LI));
  }

  bool Changed = Impl.runImpl(F, TLI, LVI, AA, &DDT, HasProfileData,
                              std::move(BFI), std::move(BPI), MDR);

  if (PrintLVIAfterJumpThreading) {
    dbgs() << "LVI for function '" << F.getName() << "':\n";
    LVI->printLVI(F, *DT, dbgs());
  }
  return Changed;
}

PreservedAnalyses JumpThreadingPass::run(Function &F,
                                         FunctionAnalysisManager &AM) {
  auto &TLI = AM.getResult<TargetLibraryAnalysis>(F);
  auto &DT = AM.getResult<DominatorTreeAnalysis>(F);
  auto &LVI = AM.getResult<LazyValueAnalysis>(F);
  auto &AA = AM.getResult<AAManager>(F);
  auto &PDT = AM.getResult<PostDominatorTreeAnalysis>(F);
  auto &MDR = AM.getResult<MemoryDependenceAnalysis>(F);
  DeferredDominance DDT(DT);

  std::unique_ptr<BlockFrequencyInfo> BFI;
  std::unique_ptr<BranchProbabilityInfo> BPI;
  if (F.hasProfileData()) {
    LoopInfo LI{DominatorTree(F)};
    BPI.reset(new BranchProbabilityInfo(F, LI, &TLI));
    BFI.reset(new BlockFrequencyInfo(F, *BPI, LI));
  }

  bool Changed = runImpl(F, &TLI, &LVI, &AA, &DDT, HasProfileData,
                         std::move(BFI), std::move(BPI), &MDR);

  if (!Changed)
    return PreservedAnalyses::all();
  PreservedAnalyses PA;
  PA.preserve<GlobalsAA>();
  PA.preserve<DominatorTreeAnalysis>();
  PA.preserve<LazyValueAnalysis>();
  return PA;
}

bool JumpThreadingPass::runImpl(Function &F, TargetLibraryInfo *TLI_,
                                LazyValueInfo *LVI_, AliasAnalysis *AA_,
                                DeferredDominance *DDT_, bool HasProfileData_,
                                std::unique_ptr<BlockFrequencyInfo> BFI_,
                                std::unique_ptr<BranchProbabilityInfo> BPI_,
                                MemoryDependenceResults *MDR_) {
  DEBUG(dbgs() << "Jump threading on function '" << F.getName() << "'\n");
  TLI = TLI_;
  LVI = LVI_;
  AA = AA_;
  DDT = DDT_;
  BFI.reset();
  BPI.reset();
  JTSinkHoist =
      llvm::make_unique<JumpThreadingSinkHoist>(MDR_, AA, TLI, BBDupThreshold);

  // When profile data is available, we need to update edge weights after
  // successful jump threading, which requires both BPI and BFI being available.
  HasProfileData = HasProfileData_;
  auto *GuardDecl = F.getParent()->getFunction(
      Intrinsic::getName(Intrinsic::experimental_guard));
  HasGuards = GuardDecl && !GuardDecl->use_empty();
  if (HasProfileData) {
    BPI = std::move(BPI_);
    BFI = std::move(BFI_);
  }

  // Remove unreachable blocks from function as they may result in infinite
  // loop. We do threading if we found something profitable. Jump threading a
  // branch can create other opportunities. If these opportunities form a cycle
  // i.e. if any jump threading is undoing previous threading in the path, then
  // we will loop forever. We take care of this issue by not jump threading for
  // back edges. This works for normal cases but not for unreachable blocks as
  // they may have cycle with no back edge.
  bool EverChanged = false;
  EverChanged |= removeUnreachableBlocks(F, LVI, DDT);

  FindLoopHeaders(F);

  bool Changed;
  do {
    Changed = false;
    for (Function::iterator I = F.begin(), E = F.end(); I != E;) {
      BasicBlock *BB = &*I;
      // Thread all of the branches we can over this block.
      while (ProcessBlock(BB))
        Changed = true;

      ++I;

      // Don't thread branches over a block that's slated for deletion.
      if (DDT->pendingDeletedBB(BB))
        continue;

      // If the block is trivially dead, zap it.  This eliminates the successor
      // edges which simplifies the CFG.
      if (pred_empty(BB) &&
          BB != &BB->getParent()->getEntryBlock()) {
        DEBUG(dbgs() << "  JT: Deleting dead block '" << BB->getName()
              << "' with terminator: " << *BB->getTerminator() << '\n');
        LoopHeaders.erase(BB);
        LVI->eraseBlock(BB);
        DeleteDeadBlock(BB, DDT);
        Changed = true;
        continue;
      }

      BranchInst *BI = dyn_cast<BranchInst>(BB->getTerminator());

      // Can't thread an unconditional jump, but if the block is "almost
      // empty", we can replace uses of it with uses of the successor and make
      // this dead.
      // We should not eliminate the loop header or latch either, because
      // eliminating a loop header or latch might later prevent LoopSimplify
      // from transforming nested loops into simplified form. We will rely on
      // later passes in backend to clean up empty blocks.
      if (BI && BI->isUnconditional() &&
          BB != &BB->getParent()->getEntryBlock() &&
          // If the terminator is the only non-phi instruction, try to nuke it.
          BB->getFirstNonPHIOrDbg()->isTerminator() && !LoopHeaders.count(BB) &&
          !LoopHeaders.count(BI->getSuccessor(0))) {
        // FIXME: It is always conservatively correct to drop the info
        // for a block even if it doesn't get erased.  This isn't totally
        // awesome, but it allows us to use AssertingVH to prevent nasty
        // dangling pointer issues within LazyValueInfo.
        LVI->eraseBlock(BB);
        if (TryToSimplifyUncondBranchFromEmptyBlock(BB, DDT))
          Changed = true;
      }
    }
    EverChanged |= Changed;
  } while (Changed);

  LoopHeaders.clear();
  DDT->flush();
  return EverChanged;
}

// Replace uses of Cond with ToVal when safe to do so. If all uses are
// replaced, we can remove Cond. We cannot blindly replace all uses of Cond
// because we may incorrectly replace uses when guards/assumes are uses of
// of `Cond` and we used the guards/assume to reason about the `Cond` value
// at the end of block. RAUW unconditionally replaces all uses
// including the guards/assumes themselves and the uses before the
// guard/assume.
static void ReplaceFoldableUses(Instruction *Cond, Value *ToVal) {
  assert(Cond->getType() == ToVal->getType());
  auto *BB = Cond->getParent();
  // We can unconditionally replace all uses in non-local blocks (i.e. uses
  // strictly dominated by BB), since LVI information is true from the
  // terminator of BB.
  replaceNonLocalUsesWith(Cond, ToVal);
  for (Instruction &I : reverse(*BB)) {
    // Reached the Cond whose uses we are trying to replace, so there are no
    // more uses.
    if (&I == Cond)
      break;
    // We only replace uses in instructions that are guaranteed to reach the end
    // of BB, where we know Cond is ToVal.
    if (!isGuaranteedToTransferExecutionToSuccessor(&I))
      break;
    I.replaceUsesOfWith(Cond, ToVal);
  }
  if (Cond->use_empty() && !Cond->mayHaveSideEffects())
    Cond->eraseFromParent();
}

/// HLS Specific
/// Return true if \p Call is a region ssdm call.
static bool isRegionSSDM(const CallInst *Call) {
  if (auto *F = Call->getCalledFunction()) {
    auto name = F->getName();
    return name.equals("_ssdm_RegionBegin") || name.equals("_ssdm_RegionEnd");
  }

  return false;
}
///

/// Return the cost of duplicating a piece of this block from first non-phi
/// and before StopAt instruction to thread across it. Stop scanning the block
/// when exceeding the threshold. If duplication is impossible, returns ~0U.
static unsigned getJumpThreadDuplicationCost(
    BasicBlock *BB, Instruction *StopAt, unsigned Threshold,
    const SmallPtrSetImpl<Instruction *> *MovedInsts = nullptr) {
  assert(StopAt->getParent() == BB && "Not an instruction from proper BB?");
  /// Ignore PHI nodes, these will be flattened when duplication happens.
  BasicBlock::const_iterator I(BB->getFirstNonPHI());

  // FIXME: THREADING will delete values that are just used to compute the
  // branch, so they shouldn't count against the duplication cost.

  unsigned Bonus = 0;
  if (BB->getTerminator() == StopAt) {
    // Threading through a switch statement is particularly profitable.  If this
    // block ends in a switch, decrease its cost to make it more likely to
    // happen.
    if (isa<SwitchInst>(StopAt))
      Bonus = 6;

    // The same holds for indirect branches, but slightly more so.
    if (isa<IndirectBrInst>(StopAt))
      Bonus = 8;
  }

  // Bump the threshold up so the early exit from the loop doesn't skip the
  // terminator-based Size adjustment at the end.
  Threshold += Bonus;

  // Sum up the cost of each instruction until we get to the terminator.  Don't
  // include the terminator because the copy won't include it.
  unsigned Size = 0;
  for (; &*I != StopAt; ++I) {

    // Stop scanning the block if we've reached the threshold.
    if (Size > Threshold)
      return Size;

    // Debugger intrinsics don't incur code size.
    if (isa<DbgInfoIntrinsic>(I)) continue;

    if (MovedInsts && MovedInsts->count(&*I))
      continue;

    // HLS BEGIN
    // consider load, GEP, cast is free for HLS 
    if(HLS && (isa<LoadInst>(I) || isa<GetElementPtrInst>(I) || isa<CastInst>(I))) continue;
    // HLS END

    // If this is a pointer->pointer bitcast, it is free.
    if (isa<BitCastInst>(I) && I->getType()->isPointerTy())
      continue;

    // Bail out if this instruction gives back a token type, it is not possible
    // to duplicate it if it is used outside this BB.
    if (I->getType()->isTokenTy() && I->isUsedOutsideOfBlock(BB))
      return ~0U;

    // All other instructions count for at least one unit.
    ++Size;

    // Calls are more expensive.  If they are non-intrinsic calls, we model them
    // as having cost of 4.  If they are a non-vector intrinsic, we model them
    // as having cost of 2 total, and if they are a vector intrinsic, we model
    // them as having cost 1.
    if (const CallInst *CI = dyn_cast<CallInst>(I)) {
      if (CI->cannotDuplicate() || CI->isConvergent() || isRegionSSDM(CI))
        // Blocks with NoDuplicate are modelled as having infinite cost, so they
        // are never duplicated. HLS NOTE: The directive.scope.begin/end
	// intrinsics are marked as no duplicate; Howerver, the
        // ssdm_RegionBegin/End is not understand by LLVM, check explicitly and
        // do not duplicate them.
        return ~0U;
      else if (!isa<IntrinsicInst>(CI))
        Size += 3;
      else if (!CI->getType()->isVectorTy())
        Size += 1;
    }
  }

  return Size > Bonus ? Size - Bonus : 0;
}

/// FindLoopHeaders - We do not want jump threading to turn proper loop
/// structures into irreducible loops.  Doing this breaks up the loop nesting
/// hierarchy and pessimizes later transformations.  To prevent this from
/// happening, we first have to find the loop headers.  Here we approximate this
/// by finding targets of backedges in the CFG.
///
/// Note that there definitely are cases when we want to allow threading of
/// edges across a loop header.  For example, threading a jump from outside the
/// loop (the preheader) to an exit block of the loop is definitely profitable.
/// It is also almost always profitable to thread backedges from within the loop
/// to exit blocks, and is often profitable to thread backedges to other blocks
/// within the loop (forming a nested loop).  This simple analysis is not rich
/// enough to track all of these properties and keep it up-to-date as the CFG
/// mutates, so we don't allow any of these transformations.
void JumpThreadingPass::FindLoopHeaders(Function &F) {
  SmallVector<std::pair<const BasicBlock*,const BasicBlock*>, 32> Edges;
  FindFunctionBackedges(F, Edges);

  for (const auto &Edge : Edges)
    LoopHeaders.insert(Edge.second);
}

/// getKnownConstant - Helper method to determine if we can thread over a
/// terminator with the given value as its condition, and if so what value to
/// use for that. What kind of value this is depends on whether we want an
/// integer or a block address, but an undef is always accepted.
/// Returns null if Val is null or not an appropriate constant.
static Constant *getKnownConstant(Value *Val, ConstantPreference Preference) {
  if (!Val)
    return nullptr;

  // Undef is "known" enough.
  if (UndefValue *U = dyn_cast<UndefValue>(Val))
    return U;

  if (Preference == WantBlockAddress)
    return dyn_cast<BlockAddress>(Val->stripPointerCasts());

  return dyn_cast<ConstantInt>(Val);
}

/// ComputeValueKnownInPredecessors - Given a basic block BB and a value V, see
/// if we can infer that the value is a known ConstantInt/BlockAddress or undef
/// in any of our predecessors.  If so, return the known list of value and pred
/// BB in the result vector.
///
/// This returns true if there were any known values.
bool JumpThreadingPass::ComputeValueKnownInPredecessors(
    Value *V, BasicBlock *BB, PredValueInfo &Result,
    ConstantPreference Preference, Instruction *CxtI) {
  // This method walks up use-def chains recursively.  Because of this, we could
  // get into an infinite loop going around loops in the use-def chain.  To
  // prevent this, keep track of what (value, block) pairs we've already visited
  // and terminate the search if we loop back to them
  if (!RecursionSet.insert(std::make_pair(V, BB)).second)
    return false;

  // An RAII help to remove this pair from the recursion set once the recursion
  // stack pops back out again.
  RecursionSetRemover remover(RecursionSet, std::make_pair(V, BB));

  // If V is a constant, then it is known in all predecessors.
  if (Constant *KC = getKnownConstant(V, Preference)) {
    for (BasicBlock *Pred : predecessors(BB))
      Result.push_back(std::make_pair(KC, Pred));

    return !Result.empty();
  }

  // If V is a non-instruction value, or an instruction in a different block,
  // then it can't be derived from a PHI.
  Instruction *I = dyn_cast<Instruction>(V);
  if (!I || I->getParent() != BB) {

    // Okay, if this is a live-in value, see if it has a known value at the end
    // of any of our predecessors.
    //
    // FIXME: This should be an edge property, not a block end property.
    /// TODO: Per PR2563, we could infer value range information about a
    /// predecessor based on its terminator.
    //
    // FIXME: change this to use the more-rich 'getPredicateOnEdge' method if
    // "I" is a non-local compare-with-a-constant instruction.  This would be
    // able to handle value inequalities better, for example if the compare is
    // "X < 4" and "X < 3" is known true but "X < 4" itself is not available.
    // Perhaps getConstantOnEdge should be smart enough to do this?

    for (BasicBlock *P : predecessors(BB)) {
      // If the value is known by LazyValueInfo to be a constant in a
      // predecessor, use that information to try to thread this block.
      Constant *PredCst = LVI->getConstantOnEdge(V, P, BB, CxtI);
      if (Constant *KC = getKnownConstant(PredCst, Preference))
        Result.push_back(std::make_pair(KC, P));
    }

    return !Result.empty();
  }

  /// If I is a PHI node, then we know the incoming values for any constants.
  if (PHINode *PN = dyn_cast<PHINode>(I)) {
    for (unsigned i = 0, e = PN->getNumIncomingValues(); i != e; ++i) {
      Value *InVal = PN->getIncomingValue(i);
      if (Constant *KC = getKnownConstant(InVal, Preference)) {
        Result.push_back(std::make_pair(KC, PN->getIncomingBlock(i)));
      } else {
        Constant *CI = LVI->getConstantOnEdge(InVal,
                                              PN->getIncomingBlock(i),
                                              BB, CxtI);
        if (Constant *KC = getKnownConstant(CI, Preference))
          Result.push_back(std::make_pair(KC, PN->getIncomingBlock(i)));
      }
    }

    return !Result.empty();
  }

  // Handle Cast instructions.  Only see through Cast when the source operand is
  // PHI or Cmp and the source type is i1 to save the compilation time.
  if (CastInst *CI = dyn_cast<CastInst>(I)) {
    Value *Source = CI->getOperand(0);
    if (!Source->getType()->isIntegerTy(1))
      return false;
    if (!isa<PHINode>(Source) && !isa<CmpInst>(Source))
      return false;
    ComputeValueKnownInPredecessors(Source, BB, Result, Preference, CxtI);
    if (Result.empty())
      return false;

    // Convert the known values.
    for (auto &R : Result)
      R.first = ConstantExpr::getCast(CI->getOpcode(), R.first, CI->getType());

    return true;
  }

  // Handle some boolean conditions.
  if (I->getType()->getPrimitiveSizeInBits() == 1) {
    assert(Preference == WantInteger && "One-bit non-integer type?");
    // X | true -> true
    // X & false -> false
    if (I->getOpcode() == Instruction::Or ||
        I->getOpcode() == Instruction::And) {
      PredValueInfoTy LHSVals, RHSVals;

      ComputeValueKnownInPredecessors(I->getOperand(0), BB, LHSVals,
                                      WantInteger, CxtI);
      ComputeValueKnownInPredecessors(I->getOperand(1), BB, RHSVals,
                                      WantInteger, CxtI);

      if (LHSVals.empty() && RHSVals.empty())
        return false;

      ConstantInt *InterestingVal;
      if (I->getOpcode() == Instruction::Or)
        InterestingVal = ConstantInt::getTrue(I->getContext());
      else
        InterestingVal = ConstantInt::getFalse(I->getContext());

      SmallPtrSet<BasicBlock*, 4> LHSKnownBBs;

      // Scan for the sentinel.  If we find an undef, force it to the
      // interesting value: x|undef -> true and x&undef -> false.
      for (const auto &LHSVal : LHSVals)
        if (LHSVal.first == InterestingVal || isa<UndefValue>(LHSVal.first)) {
          Result.emplace_back(InterestingVal, LHSVal.second);
          LHSKnownBBs.insert(LHSVal.second);
        }
      for (const auto &RHSVal : RHSVals)
        if (RHSVal.first == InterestingVal || isa<UndefValue>(RHSVal.first)) {
          // If we already inferred a value for this block on the LHS, don't
          // re-add it.
          if (!LHSKnownBBs.count(RHSVal.second))
            Result.emplace_back(InterestingVal, RHSVal.second);
        }

      return !Result.empty();
    }

    // Handle the NOT form of XOR.
    if (I->getOpcode() == Instruction::Xor &&
        isa<ConstantInt>(I->getOperand(1)) &&
        cast<ConstantInt>(I->getOperand(1))->isOne()) {
      ComputeValueKnownInPredecessors(I->getOperand(0), BB, Result,
                                      WantInteger, CxtI);
      if (Result.empty())
        return false;

      // Invert the known values.
      for (auto &R : Result)
        R.first = ConstantExpr::getNot(R.first);

      return true;
    }

  // Try to simplify some other binary operator values.
  } else if (BinaryOperator *BO = dyn_cast<BinaryOperator>(I)) {
    assert(Preference != WantBlockAddress
            && "A binary operator creating a block address?");
    if (ConstantInt *CI = dyn_cast<ConstantInt>(BO->getOperand(1))) {
      PredValueInfoTy LHSVals;
      ComputeValueKnownInPredecessors(BO->getOperand(0), BB, LHSVals,
                                      WantInteger, CxtI);

      // Try to use constant folding to simplify the binary operator.
      for (const auto &LHSVal : LHSVals) {
        Constant *V = LHSVal.first;
        Constant *Folded = ConstantExpr::get(BO->getOpcode(), V, CI);

        if (Constant *KC = getKnownConstant(Folded, WantInteger))
          Result.push_back(std::make_pair(KC, LHSVal.second));
      }
    }

    return !Result.empty();
  }

  // Handle compare with phi operand, where the PHI is defined in this block.
  if (CmpInst *Cmp = dyn_cast<CmpInst>(I)) {
    assert(Preference == WantInteger && "Compares only produce integers");
    Type *CmpType = Cmp->getType();
    Value *CmpLHS = Cmp->getOperand(0);
    Value *CmpRHS = Cmp->getOperand(1);
    CmpInst::Predicate Pred = Cmp->getPredicate();

    PHINode *PN = dyn_cast<PHINode>(CmpLHS);
    if (PN && PN->getParent() == BB) {
      const DataLayout &DL = PN->getModule()->getDataLayout();
      // We can do this simplification if any comparisons fold to true or false.
      // See if any do.
      for (unsigned i = 0, e = PN->getNumIncomingValues(); i != e; ++i) {
        BasicBlock *PredBB = PN->getIncomingBlock(i);
        Value *LHS = PN->getIncomingValue(i);
        Value *RHS = CmpRHS->DoPHITranslation(BB, PredBB);

        Value *Res = SimplifyCmpInst(Pred, LHS, RHS, {DL});
        if (!Res) {
          if (!isa<Constant>(RHS))
            continue;

          LazyValueInfo::Tristate
            ResT = LVI->getPredicateOnEdge(Pred, LHS,
                                           cast<Constant>(RHS), PredBB, BB,
                                           CxtI ? CxtI : Cmp);
          if (ResT == LazyValueInfo::Unknown)
            continue;
          Res = ConstantInt::get(Type::getInt1Ty(LHS->getContext()), ResT);
        }

        if (Constant *KC = getKnownConstant(Res, WantInteger))
          Result.push_back(std::make_pair(KC, PredBB));
      }

      return !Result.empty();
    }

    // If comparing a live-in value against a constant, see if we know the
    // live-in value on any predecessors.
    if (isa<Constant>(CmpRHS) && !CmpType->isVectorTy()) {
      Constant *CmpConst = cast<Constant>(CmpRHS);

      if (!isa<Instruction>(CmpLHS) ||
          cast<Instruction>(CmpLHS)->getParent() != BB) {
        for (BasicBlock *P : predecessors(BB)) {
          // If the value is known by LazyValueInfo to be a constant in a
          // predecessor, use that information to try to thread this block.
          LazyValueInfo::Tristate Res =
            LVI->getPredicateOnEdge(Pred, CmpLHS,
                                    CmpConst, P, BB, CxtI ? CxtI : Cmp);
          if (Res == LazyValueInfo::Unknown)
            continue;

          Constant *ResC = ConstantInt::get(CmpType, Res);
          Result.push_back(std::make_pair(ResC, P));
        }

        return !Result.empty();
      }

      // InstCombine can fold some forms of constant range checks into
      // (icmp (add (x, C1)), C2). See if we have we have such a thing with
      // x as a live-in.
      {
        using namespace PatternMatch;

        Value *AddLHS;
        ConstantInt *AddConst;
        if (isa<ConstantInt>(CmpConst) &&
            match(CmpLHS, m_Add(m_Value(AddLHS), m_ConstantInt(AddConst)))) {
          if (!isa<Instruction>(AddLHS) ||
              cast<Instruction>(AddLHS)->getParent() != BB) {
            for (BasicBlock *P : predecessors(BB)) {
              // If the value is known by LazyValueInfo to be a ConstantRange in
              // a predecessor, use that information to try to thread this
              // block.
              ConstantRange CR = LVI->getConstantRangeOnEdge(
                  AddLHS, P, BB, CxtI ? CxtI : cast<Instruction>(CmpLHS));
              // Propagate the range through the addition.
              CR = CR.add(AddConst->getValue());

              // Get the range where the compare returns true.
              ConstantRange CmpRange = ConstantRange::makeExactICmpRegion(
                  Pred, cast<ConstantInt>(CmpConst)->getValue());

              Constant *ResC;
              if (CmpRange.contains(CR))
                ResC = ConstantInt::getTrue(CmpType);
              else if (CmpRange.inverse().contains(CR))
                ResC = ConstantInt::getFalse(CmpType);
              else
                continue;

              Result.push_back(std::make_pair(ResC, P));
            }

            return !Result.empty();
          }
        }
      }

      // Try to find a constant value for the LHS of a comparison,
      // and evaluate it statically if we can.
      PredValueInfoTy LHSVals;
      ComputeValueKnownInPredecessors(I->getOperand(0), BB, LHSVals,
                                      WantInteger, CxtI);

      for (const auto &LHSVal : LHSVals) {
        Constant *V = LHSVal.first;
        Constant *Folded = ConstantExpr::getCompare(Pred, V, CmpConst);
        if (Constant *KC = getKnownConstant(Folded, WantInteger))
          Result.push_back(std::make_pair(KC, LHSVal.second));
      }

      return !Result.empty();
    }
  }

  if (SelectInst *SI = dyn_cast<SelectInst>(I)) {
    // Handle select instructions where at least one operand is a known constant
    // and we can figure out the condition value for any predecessor block.
    Constant *TrueVal = getKnownConstant(SI->getTrueValue(), Preference);
    Constant *FalseVal = getKnownConstant(SI->getFalseValue(), Preference);
    PredValueInfoTy Conds;
    if ((TrueVal || FalseVal) &&
        ComputeValueKnownInPredecessors(SI->getCondition(), BB, Conds,
                                        WantInteger, CxtI)) {
      for (auto &C : Conds) {
        Constant *Cond = C.first;

        // Figure out what value to use for the condition.
        bool KnownCond;
        if (ConstantInt *CI = dyn_cast<ConstantInt>(Cond)) {
          // A known boolean.
          KnownCond = CI->isOne();
        } else {
          assert(isa<UndefValue>(Cond) && "Unexpected condition value");
          // Either operand will do, so be sure to pick the one that's a known
          // constant.
          // FIXME: Do this more cleverly if both values are known constants?
          KnownCond = (TrueVal != nullptr);
        }

        // See if the select has a known constant value for this predecessor.
        if (Constant *Val = KnownCond ? TrueVal : FalseVal)
          Result.push_back(std::make_pair(Val, C.second));
      }

      return !Result.empty();
    }
  }

  // If all else fails, see if LVI can figure out a constant value for us.
  Constant *CI = LVI->getConstant(V, BB, CxtI);
  if (Constant *KC = getKnownConstant(CI, Preference)) {
    for (BasicBlock *Pred : predecessors(BB))
      Result.push_back(std::make_pair(KC, Pred));
  }

  return !Result.empty();
}

/// GetBestDestForBranchOnUndef - If we determine that the specified block ends
/// in an undefined jump, decide which block is best to revector to.
///
/// Since we can pick an arbitrary destination, we pick the successor with the
/// fewest predecessors.  This should reduce the in-degree of the others.
static unsigned GetBestDestForJumpOnUndef(BasicBlock *BB) {
  TerminatorInst *BBTerm = BB->getTerminator();
  unsigned MinSucc = 0;
  BasicBlock *TestBB = BBTerm->getSuccessor(MinSucc);
  // Compute the successor with the minimum number of predecessors.
  unsigned MinNumPreds = std::distance(pred_begin(TestBB), pred_end(TestBB));
  for (unsigned i = 1, e = BBTerm->getNumSuccessors(); i != e; ++i) {
    TestBB = BBTerm->getSuccessor(i);
    unsigned NumPreds = std::distance(pred_begin(TestBB), pred_end(TestBB));
    if (NumPreds < MinNumPreds) {
      MinSucc = i;
      MinNumPreds = NumPreds;
    }
  }

  return MinSucc;
}

static bool hasAddressTakenAndUsed(BasicBlock *BB) {
  if (!BB->hasAddressTaken()) return false;

  // If the block has its address taken, it may be a tree of dead constants
  // hanging off of it.  These shouldn't keep the block alive.
  BlockAddress *BA = BlockAddress::get(BB);
  BA->removeDeadConstantUsers();
  return !BA->use_empty();
}

/// ProcessBlock - If there are any predecessors whose control can be threaded
/// through to a successor, transform them now.
bool JumpThreadingPass::ProcessBlock(BasicBlock *BB) {
  // If the block is trivially dead, just return and let the caller nuke it.
  // This simplifies other transformations.
  if (DDT->pendingDeletedBB(BB) ||
      (pred_empty(BB) && BB != &BB->getParent()->getEntryBlock()))
    return false;

  // If this block has a single predecessor, and if that pred has a single
  // successor, merge the blocks.  This encourages recursive jump threading
  // because now the condition in this block can be threaded through
  // predecessors of our predecessor block.
  if (BasicBlock *SinglePred = BB->getSinglePredecessor()) {
    const TerminatorInst *TI = SinglePred->getTerminator();
    if (!TI->isExceptional() && TI->getNumSuccessors() == 1 &&
        SinglePred != BB && !hasAddressTakenAndUsed(BB)) {
      // If SinglePred was a loop header, BB becomes one.
      if (LoopHeaders.erase(SinglePred))
        LoopHeaders.insert(BB);

      LVI->eraseBlock(SinglePred);
      MergeBasicBlockIntoOnlyPred(BB, nullptr, DDT);

      // Now that BB is merged into SinglePred (i.e. SinglePred Code followed by
      // BB code within one basic block `BB`), we need to invalidate the LVI
      // information associated with BB, because the LVI information need not be
      // true for all of BB after the merge. For example,
      // Before the merge, LVI info and code is as follows:
      // SinglePred: <LVI info1 for %p val>
      // %y = use of %p
      // call @exit() // need not transfer execution to successor.
      // assume(%p) // from this point on %p is true
      // br label %BB
      // BB: <LVI info2 for %p val, i.e. %p is true>
      // %x = use of %p
      // br label exit
      //
      // Note that this LVI info for blocks BB and SinglPred is correct for %p
      // (info2 and info1 respectively). After the merge and the deletion of the
      // LVI info1 for SinglePred. We have the following code:
      // BB: <LVI info2 for %p val>
      // %y = use of %p
      // call @exit()
      // assume(%p)
      // %x = use of %p <-- LVI info2 is correct from here onwards.
      // br label exit
      // LVI info2 for BB is incorrect at the beginning of BB.

      // Invalidate LVI information for BB if the LVI is not provably true for
      // all of BB.
      if (any_of(*BB, [](Instruction &I) {
            return !isGuaranteedToTransferExecutionToSuccessor(&I);
          }))
        LVI->eraseBlock(BB);
      return true;
    }
  }

  if (TryToUnfoldSelectInCurrBB(BB))
    return true;

  // Look if we can propagate guards to predecessors.
  if (HasGuards && ProcessGuards(BB))
    return true;

  // What kind of constant we're looking for.
  ConstantPreference Preference = WantInteger;

  // Look to see if the terminator is a conditional branch, switch or indirect
  // branch, if not we can't thread it.
  Value *Condition;
  Instruction *Terminator = BB->getTerminator();
  if (BranchInst *BI = dyn_cast<BranchInst>(Terminator)) {
    // Can't thread an unconditional jump.
    if (BI->isUnconditional()) return false;
    Condition = BI->getCondition();
  } else if (SwitchInst *SI = dyn_cast<SwitchInst>(Terminator)) {
    Condition = SI->getCondition();
  } else if (IndirectBrInst *IB = dyn_cast<IndirectBrInst>(Terminator)) {
    // Can't thread indirect branch with no successors.
    if (IB->getNumSuccessors() == 0) return false;
    Condition = IB->getAddress()->stripPointerCasts();
    Preference = WantBlockAddress;
  } else {
    return false; // Must be an invoke.
  }

  // Run constant folding to see if we can reduce the condition to a simple
  // constant.
  if (Instruction *I = dyn_cast<Instruction>(Condition)) {
    Value *SimpleVal =
        ConstantFoldInstruction(I, BB->getModule()->getDataLayout(), TLI);
    if (SimpleVal) {
      I->replaceAllUsesWith(SimpleVal);
      if (isInstructionTriviallyDead(I, TLI))
        I->eraseFromParent();
      Condition = SimpleVal;
    }
  }

  // If the terminator is branching on an undef, we can pick any of the
  // successors to branch to.  Let GetBestDestForJumpOnUndef decide.
  if (isa<UndefValue>(Condition)) {
    unsigned BestSucc = GetBestDestForJumpOnUndef(BB);
    std::vector<DominatorTree::UpdateType> Updates;

    // Fold the branch/switch.
    TerminatorInst *BBTerm = BB->getTerminator();
    Updates.reserve(BBTerm->getNumSuccessors());
    for (unsigned i = 0, e = BBTerm->getNumSuccessors(); i != e; ++i) {
      if (i == BestSucc) continue;
      BasicBlock *Succ = BBTerm->getSuccessor(i);
      Succ->removePredecessor(BB, true);
      Updates.push_back({DominatorTree::Delete, BB, Succ});
    }

    DEBUG(dbgs() << "  In block '" << BB->getName()
          << "' folding undef terminator: " << *BBTerm << '\n');
    BranchInst::Create(BBTerm->getSuccessor(BestSucc), BBTerm);
    BBTerm->eraseFromParent();
    DDT->applyUpdates(Updates);
    return true;
  }

  // If the terminator of this block is branching on a constant, simplify the
  // terminator to an unconditional branch.  This can occur due to threading in
  // other blocks.
  if (getKnownConstant(Condition, Preference)) {
    DEBUG(dbgs() << "  In block '" << BB->getName()
          << "' folding terminator: " << *BB->getTerminator() << '\n');
    ++NumFolds;
    ConstantFoldTerminator(BB, true, nullptr, DDT);
    return true;
  }

  Instruction *CondInst = dyn_cast<Instruction>(Condition);

  // All the rest of our checks depend on the condition being an instruction.
  if (!CondInst) {
    // FIXME: Unify this with code below.
    if (ProcessThreadableEdges(Condition, BB, Preference, Terminator))
      return true;
    return false;
  }

  if (CmpInst *CondCmp = dyn_cast<CmpInst>(CondInst)) {
    // If we're branching on a conditional, LVI might be able to determine
    // it's value at the branch instruction.  We only handle comparisons
    // against a constant at this time.
    // TODO: This should be extended to handle switches as well.
    BranchInst *CondBr = dyn_cast<BranchInst>(BB->getTerminator());
    Constant *CondConst = dyn_cast<Constant>(CondCmp->getOperand(1));
    if (CondBr && CondConst) {
      // We should have returned as soon as we turn a conditional branch to
      // unconditional. Because its no longer interesting as far as jump
      // threading is concerned.
      assert(CondBr->isConditional() && "Threading on unconditional terminator");

      LazyValueInfo::Tristate Ret =
        LVI->getPredicateAt(CondCmp->getPredicate(), CondCmp->getOperand(0),
                            CondConst, CondBr);
      if (Ret != LazyValueInfo::Unknown) {
        unsigned ToRemove = Ret == LazyValueInfo::True ? 1 : 0;
        unsigned ToKeep = Ret == LazyValueInfo::True ? 0 : 1;
        BasicBlock *ToRemoveSucc = CondBr->getSuccessor(ToRemove);
        ToRemoveSucc->removePredecessor(BB, true);
        BranchInst::Create(CondBr->getSuccessor(ToKeep), CondBr);
        CondBr->eraseFromParent();
        if (CondCmp->use_empty())
          CondCmp->eraseFromParent();
        // We can safely replace *some* uses of the CondInst if it has
        // exactly one value as returned by LVI. RAUW is incorrect in the
        // presence of guards and assumes, that have the `Cond` as the use. This
        // is because we use the guards/assume to reason about the `Cond` value
        // at the end of block, but RAUW unconditionally replaces all uses
        // including the guards/assumes themselves and the uses before the
        // guard/assume.
        else if (CondCmp->getParent() == BB) {
          auto *CI = Ret == LazyValueInfo::True ?
            ConstantInt::getTrue(CondCmp->getType()) :
            ConstantInt::getFalse(CondCmp->getType());
          ReplaceFoldableUses(CondCmp, CI);
        }
        DDT->deleteEdge(BB, ToRemoveSucc);
        return true;
      }

      // We did not manage to simplify this branch, try to see whether
      // CondCmp depends on a known phi-select pattern.
      if (TryToUnfoldSelect(CondCmp, BB))
        return true;
    }
  }

  // Check for some cases that are worth simplifying.  Right now we want to look
  // for loads that are used by a switch or by the condition for the branch.  If
  // we see one, check to see if it's partially redundant.  If so, insert a PHI
  // which can then be used to thread the values.
  Value *SimplifyValue = CondInst;
  if (CmpInst *CondCmp = dyn_cast<CmpInst>(SimplifyValue))
    if (isa<Constant>(CondCmp->getOperand(1)))
      SimplifyValue = CondCmp->getOperand(0);

  // TODO: There are other places where load PRE would be profitable, such as
  // more complex comparisons.
  if (LoadInst *LI = dyn_cast<LoadInst>(SimplifyValue))
    if (SimplifyPartiallyRedundantLoad(LI))
      return true;

  // Before threading, try to propagate profile data backwards:
  if (PHINode *PN = dyn_cast<PHINode>(CondInst))
    if (PN->getParent() == BB && isa<BranchInst>(BB->getTerminator()))
      updatePredecessorProfileMetadata(PN, BB);

  // Handle a variety of cases where we are branching on something derived from
  // a PHI node in the current block.  If we can prove that any predecessors
  // compute a predictable value based on a PHI node, thread those predecessors.
  if (ProcessThreadableEdges(CondInst, BB, Preference, Terminator))
    return true;

  // If this is an otherwise-unfoldable branch on a phi node in the current
  // block, see if we can simplify.
  if (PHINode *PN = dyn_cast<PHINode>(CondInst))
    if (PN->getParent() == BB && isa<BranchInst>(BB->getTerminator()))
      return ProcessBranchOnPHI(PN);

  // If this is an otherwise-unfoldable branch on a XOR, see if we can simplify.
  if (CondInst->getOpcode() == Instruction::Xor &&
      CondInst->getParent() == BB && isa<BranchInst>(BB->getTerminator()))
    return ProcessBranchOnXOR(cast<BinaryOperator>(CondInst));
 
  // Search for a stronger dominating condition that can be used to simplify a
  // conditional branch leaving BB.
  if (ProcessImpliedCondition(BB))
    return true;

  return false;
}

bool JumpThreadingPass::ProcessImpliedCondition(BasicBlock *BB) {
  auto *BI = dyn_cast<BranchInst>(BB->getTerminator());
  if (!BI || !BI->isConditional())
    return false;

  Value *Cond = BI->getCondition();
  BasicBlock *CurrentBB = BB;
  BasicBlock *CurrentPred = BB->getSinglePredecessor();
  unsigned Iter = 0;

  auto &DL = BB->getModule()->getDataLayout();

  while (CurrentPred && Iter++ < ImplicationSearchThreshold) {
    auto *PBI = dyn_cast<BranchInst>(CurrentPred->getTerminator());
    if (!PBI || !PBI->isConditional())
      return false;
    if (PBI->getSuccessor(0) != CurrentBB && PBI->getSuccessor(1) != CurrentBB)
      return false;

    bool CondIsTrue = PBI->getSuccessor(0) == CurrentBB;
    Optional<bool> Implication =
        isImpliedCondition(PBI->getCondition(), Cond, DL, CondIsTrue);
    if (Implication) {
      BasicBlock *KeepSucc = BI->getSuccessor(*Implication ? 0 : 1);
      BasicBlock *RemoveSucc = BI->getSuccessor(*Implication ? 1 : 0);
      RemoveSucc->removePredecessor(BB);
      BranchInst::Create(KeepSucc, BI);
      BI->eraseFromParent();
      DDT->deleteEdge(BB, RemoveSucc);
      return true;
    }
    CurrentBB = CurrentPred;
    CurrentPred = CurrentBB->getSinglePredecessor();
  }

  return false;
}

/// Return true if Op is an instruction defined in the given block.
static bool isOpDefinedInBlock(Value *Op, BasicBlock *BB) {
  if (Instruction *OpInst = dyn_cast<Instruction>(Op))
    if (OpInst->getParent() == BB)
      return true;
  return false;
}

/// SimplifyPartiallyRedundantLoad - If LI is an obviously partially redundant
/// load instruction, eliminate it by replacing it with a PHI node.  This is an
/// important optimization that encourages jump threading, and needs to be run
/// interlaced with other jump threading tasks.
bool JumpThreadingPass::SimplifyPartiallyRedundantLoad(LoadInst *LI) {
  // Don't hack volatile and ordered loads.
  if (!LI->isUnordered()) return false;

  // If the load is defined in a block with exactly one predecessor, it can't be
  // partially redundant.
  BasicBlock *LoadBB = LI->getParent();
  if (LoadBB->getSinglePredecessor())
    return false;

  // If the load is defined in an EH pad, it can't be partially redundant,
  // because the edges between the invoke and the EH pad cannot have other
  // instructions between them.
  if (LoadBB->isEHPad())
    return false;

  Value *LoadedPtr = LI->getOperand(0);

  // If the loaded operand is defined in the LoadBB and its not a phi,
  // it can't be available in predecessors.
  if (isOpDefinedInBlock(LoadedPtr, LoadBB) && !isa<PHINode>(LoadedPtr))
    return false;

  // Scan a few instructions up from the load, to see if it is obviously live at
  // the entry to its block.
  BasicBlock::iterator BBIt(LI);
  bool IsLoadCSE;
  if (Value *AvailableVal = FindAvailableLoadedValue(
          LI, LoadBB, BBIt, DefMaxInstsToScan, AA, &IsLoadCSE)) {
    // If the value of the load is locally available within the block, just use
    // it.  This frequently occurs for reg2mem'd allocas.

    if (IsLoadCSE) {
      LoadInst *NLI = cast<LoadInst>(AvailableVal);
      combineMetadataForCSE(NLI, LI);
    };

    // If the returned value is the load itself, replace with an undef. This can
    // only happen in dead loops.
    if (AvailableVal == LI) AvailableVal = UndefValue::get(LI->getType());
    if (AvailableVal->getType() != LI->getType())
      AvailableVal =
          CastInst::CreateBitOrPointerCast(AvailableVal, LI->getType(), "", LI);
    LI->replaceAllUsesWith(AvailableVal);
    LI->eraseFromParent();
    return true;
  }

  // Otherwise, if we scanned the whole block and got to the top of the block,
  // we know the block is locally transparent to the load.  If not, something
  // might clobber its value.
  if (BBIt != LoadBB->begin())
    return false;

  // If all of the loads and stores that feed the value have the same AA tags,
  // then we can propagate them onto any newly inserted loads.
  AAMDNodes AATags;
  LI->getAAMetadata(AATags);

  SmallPtrSet<BasicBlock*, 8> PredsScanned;

  using AvailablePredsTy = SmallVector<std::pair<BasicBlock *, Value *>, 8>;

  AvailablePredsTy AvailablePreds;
  BasicBlock *OneUnavailablePred = nullptr;
  SmallVector<LoadInst*, 8> CSELoads;

  // If we got here, the loaded value is transparent through to the start of the
  // block.  Check to see if it is available in any of the predecessor blocks.
  for (BasicBlock *PredBB : predecessors(LoadBB)) {
    // If we already scanned this predecessor, skip it.
    if (!PredsScanned.insert(PredBB).second)
      continue;

    BBIt = PredBB->end();
    unsigned NumScanedInst = 0;
    Value *PredAvailable = nullptr;
    // NOTE: We don't CSE load that is volatile or anything stronger than
    // unordered, that should have been checked when we entered the function.
    assert(LI->isUnordered() && "Attempting to CSE volatile or atomic loads");
    // If this is a load on a phi pointer, phi-translate it and search
    // for available load/store to the pointer in predecessors.
    Value *Ptr = LoadedPtr->DoPHITranslation(LoadBB, PredBB);
    PredAvailable = FindAvailablePtrLoadStore(
        Ptr, LI->getType(), LI->isAtomic(), PredBB, BBIt, DefMaxInstsToScan,
        AA, &IsLoadCSE, &NumScanedInst);

    // If PredBB has a single predecessor, continue scanning through the
    // single precessor.
    BasicBlock *SinglePredBB = PredBB;
    while (!PredAvailable && SinglePredBB && BBIt == SinglePredBB->begin() &&
           NumScanedInst < DefMaxInstsToScan) {
      SinglePredBB = SinglePredBB->getSinglePredecessor();
      if (SinglePredBB) {
        BBIt = SinglePredBB->end();
        PredAvailable = FindAvailablePtrLoadStore(
            Ptr, LI->getType(), LI->isAtomic(), SinglePredBB, BBIt,
            (DefMaxInstsToScan - NumScanedInst), AA, &IsLoadCSE,
            &NumScanedInst);
      }
    }

    if (!PredAvailable) {
      OneUnavailablePred = PredBB;
      continue;
    }

    if (IsLoadCSE)
      CSELoads.push_back(cast<LoadInst>(PredAvailable));

    // If so, this load is partially redundant.  Remember this info so that we
    // can create a PHI node.
    AvailablePreds.push_back(std::make_pair(PredBB, PredAvailable));
  }

  // If the loaded value isn't available in any predecessor, it isn't partially
  // redundant.
  if (AvailablePreds.empty()) return false;

  // Okay, the loaded value is available in at least one (and maybe all!)
  // predecessors.  If the value is unavailable in more than one unique
  // predecessor, we want to insert a merge block for those common predecessors.
  // This ensures that we only have to insert one reload, thus not increasing
  // code size.
  BasicBlock *UnavailablePred = nullptr;

  // If the value is unavailable in one of predecessors, we will end up
  // inserting a new instruction into them. It is only valid if all the
  // instructions before LI are guaranteed to pass execution to its successor,
  // or if LI is safe to speculate.
  // TODO: If this logic becomes more complex, and we will perform PRE insertion
  // farther than to a predecessor, we need to reuse the code from GVN's PRE.
  // It requires domination tree analysis, so for this simple case it is an
  // overkill.
  if (PredsScanned.size() != AvailablePreds.size() &&
      !isSafeToSpeculativelyExecute(LI))
    for (auto I = LoadBB->begin(); &*I != LI; ++I)
      if (!isGuaranteedToTransferExecutionToSuccessor(&*I))
        return false;

  // If there is exactly one predecessor where the value is unavailable, the
  // already computed 'OneUnavailablePred' block is it.  If it ends in an
  // unconditional branch, we know that it isn't a critical edge.
  if (PredsScanned.size() == AvailablePreds.size()+1 &&
      OneUnavailablePred->getTerminator()->getNumSuccessors() == 1) {
    UnavailablePred = OneUnavailablePred;
  } else if (PredsScanned.size() != AvailablePreds.size()) {
    // Otherwise, we had multiple unavailable predecessors or we had a critical
    // edge from the one.
    SmallVector<BasicBlock*, 8> PredsToSplit;
    SmallPtrSet<BasicBlock*, 8> AvailablePredSet;

    for (const auto &AvailablePred : AvailablePreds)
      AvailablePredSet.insert(AvailablePred.first);

    // Add all the unavailable predecessors to the PredsToSplit list.
    for (BasicBlock *P : predecessors(LoadBB)) {
      // If the predecessor is an indirect goto, we can't split the edge.
      if (isa<IndirectBrInst>(P->getTerminator()))
        return false;

      if (!AvailablePredSet.count(P))
        PredsToSplit.push_back(P);
    }

    // Split them out to their own block.
    UnavailablePred = SplitBlockPreds(LoadBB, PredsToSplit, "thread-pre-split");
  }

  // If the value isn't available in all predecessors, then there will be
  // exactly one where it isn't available.  Insert a load on that edge and add
  // it to the AvailablePreds list.
  if (UnavailablePred) {
    assert(UnavailablePred->getTerminator()->getNumSuccessors() == 1 &&
           "Can't handle critical edge here!");
    LoadInst *NewVal = new LoadInst(
        LoadedPtr->DoPHITranslation(LoadBB, UnavailablePred),
        LI->getName() + ".pr", false, LI->getAlignment(), LI->getOrdering(),
        LI->getSyncScopeID(), UnavailablePred->getTerminator());
    NewVal->setDebugLoc(LI->getDebugLoc());
    if (AATags)
      NewVal->setAAMetadata(AATags);

    AvailablePreds.push_back(std::make_pair(UnavailablePred, NewVal));
  }

  // Now we know that each predecessor of this block has a value in
  // AvailablePreds, sort them for efficient access as we're walking the preds.
  array_pod_sort(AvailablePreds.begin(), AvailablePreds.end());

  // Create a PHI node at the start of the block for the PRE'd load value.
  pred_iterator PB = pred_begin(LoadBB), PE = pred_end(LoadBB);
  PHINode *PN = PHINode::Create(LI->getType(), std::distance(PB, PE), "",
                                &LoadBB->front());
  PN->takeName(LI);
  PN->setDebugLoc(LI->getDebugLoc());

  // Insert new entries into the PHI for each predecessor.  A single block may
  // have multiple entries here.
  for (pred_iterator PI = PB; PI != PE; ++PI) {
    BasicBlock *P = *PI;
    AvailablePredsTy::iterator I =
      std::lower_bound(AvailablePreds.begin(), AvailablePreds.end(),
                       std::make_pair(P, (Value*)nullptr));

    assert(I != AvailablePreds.end() && I->first == P &&
           "Didn't find entry for predecessor!");

    // If we have an available predecessor but it requires casting, insert the
    // cast in the predecessor and use the cast. Note that we have to update the
    // AvailablePreds vector as we go so that all of the PHI entries for this
    // predecessor use the same bitcast.
    Value *&PredV = I->second;
    if (PredV->getType() != LI->getType())
      PredV = CastInst::CreateBitOrPointerCast(PredV, LI->getType(), "",
                                               P->getTerminator());

    PN->addIncoming(PredV, I->first);
  }

  for (LoadInst *PredLI : CSELoads) {
    combineMetadataForCSE(PredLI, LI);
  }

  LI->replaceAllUsesWith(PN);
  LI->eraseFromParent();

  return true;
}

/// FindMostPopularDest - The specified list contains multiple possible
/// threadable destinations.  Pick the one that occurs the most frequently in
/// the list.
static BasicBlock *
FindMostPopularDest(BasicBlock *BB,
                    const SmallVectorImpl<std::pair<BasicBlock *,
                                          BasicBlock *>> &PredToDestList) {
  assert(!PredToDestList.empty());

  // Determine popularity.  If there are multiple possible destinations, we
  // explicitly choose to ignore 'undef' destinations.  We prefer to thread
  // blocks with known and real destinations to threading undef.  We'll handle
  // them later if interesting.
  DenseMap<BasicBlock*, unsigned> DestPopularity;
  for (const auto &PredToDest : PredToDestList)
    if (PredToDest.second)
      DestPopularity[PredToDest.second]++;

  // Find the most popular dest.
  DenseMap<BasicBlock*, unsigned>::iterator DPI = DestPopularity.begin();
  BasicBlock *MostPopularDest = DPI->first;
  unsigned Popularity = DPI->second;
  SmallVector<BasicBlock*, 4> SamePopularity;

  for (++DPI; DPI != DestPopularity.end(); ++DPI) {
    // If the popularity of this entry isn't higher than the popularity we've
    // seen so far, ignore it.
    if (DPI->second < Popularity)
      ; // ignore.
    else if (DPI->second == Popularity) {
      // If it is the same as what we've seen so far, keep track of it.
      SamePopularity.push_back(DPI->first);
    } else {
      // If it is more popular, remember it.
      SamePopularity.clear();
      MostPopularDest = DPI->first;
      Popularity = DPI->second;
    }
  }

  // Okay, now we know the most popular destination.  If there is more than one
  // destination, we need to determine one.  This is arbitrary, but we need
  // to make a deterministic decision.  Pick the first one that appears in the
  // successor list.
  if (!SamePopularity.empty()) {
    SamePopularity.push_back(MostPopularDest);
    TerminatorInst *TI = BB->getTerminator();
    for (unsigned i = 0; ; ++i) {
      assert(i != TI->getNumSuccessors() && "Didn't find any successor!");

      if (!is_contained(SamePopularity, TI->getSuccessor(i)))
        continue;

      MostPopularDest = TI->getSuccessor(i);
      break;
    }
  }

  // Okay, we have finally picked the most popular destination.
  return MostPopularDest;
}

bool JumpThreadingPass::ProcessThreadableEdges(Value *Cond, BasicBlock *BB,
                                               ConstantPreference Preference,
                                               Instruction *CxtI) {
  // If threading this would thread across a loop header, don't even try to
  // thread the edge.
  if (LoopHeaders.count(BB))
    return false;

  // HLS BEGIN
  // Don't thread across the loop preheader and loop latch
  for (BasicBlock *SuccBB : successors(BB)) {
    if (LoopHeaders.count(SuccBB))
      return false;
  }
  // HLS END

  PredValueInfoTy PredValues;
  if (!ComputeValueKnownInPredecessors(Cond, BB, PredValues, Preference, CxtI))
    return false;

  assert(!PredValues.empty() &&
         "ComputeValueKnownInPredecessors returned true with no values");

  DEBUG(dbgs() << "IN BB: " << *BB;
        for (const auto &PredValue : PredValues) {
          dbgs() << "  BB '" << BB->getName() << "': FOUND condition = "
            << *PredValue.first
            << " for pred '" << PredValue.second->getName() << "'.\n";
        });

  bool Changed = false;
  // HLS BEGIN
  if(HLS) {
    // we only do JT when BB's All Pred will jump to the only one BB's Succ,
    // this will keep the CFG more simple, and may enbale some access merge
    unsigned PredNum = std::distance(pred_begin(BB), pred_end(BB));
    if(PredNum != PredValues.size())
      return false;
    
    Changed |= JTSinkHoist->doSinkHoistOnBlock(BB);
  }
  // HLS END
  
  // Decide what we want to thread through.  Convert our list of known values to
  // a list of known destinations for each pred.  This also discards duplicate
  // predecessors and keeps track of the undefined inputs (which are represented
  // as a null dest in the PredToDestList).
  SmallPtrSet<BasicBlock*, 16> SeenPreds;
  SmallVector<std::pair<BasicBlock*, BasicBlock*>, 16> PredToDestList;

  BasicBlock *OnlyDest = nullptr;
  BasicBlock *MultipleDestSentinel = (BasicBlock*)(intptr_t)~0ULL;
  Constant *OnlyVal = nullptr;
  Constant *MultipleVal = (Constant *)(intptr_t)~0ULL;

  unsigned PredWithKnownDest = 0;
  for (const auto &PredValue : PredValues) {
    BasicBlock *Pred = PredValue.second;
    if (!SeenPreds.insert(Pred).second)
      continue;  // Duplicate predecessor entry.
  
    Constant *Val = PredValue.first;

    BasicBlock *DestBB;
    if (isa<UndefValue>(Val))
      DestBB = nullptr;
    else if (BranchInst *BI = dyn_cast<BranchInst>(BB->getTerminator())) {
      assert(isa<ConstantInt>(Val) && "Expecting a constant integer");
      DestBB = BI->getSuccessor(cast<ConstantInt>(Val)->isZero());
    } else if (SwitchInst *SI = dyn_cast<SwitchInst>(BB->getTerminator())) {
      assert(isa<ConstantInt>(Val) && "Expecting a constant integer");
      DestBB = SI->findCaseValue(cast<ConstantInt>(Val))->getCaseSuccessor();
    } else {
      assert(isa<IndirectBrInst>(BB->getTerminator())
              && "Unexpected terminator");
      assert(isa<BlockAddress>(Val) && "Expecting a constant blockaddress");
      DestBB = cast<BlockAddress>(Val)->getBasicBlock();
    }

    // If we have exactly one destination, remember it for efficiency below.
    if (PredToDestList.empty()) {
      OnlyDest = DestBB;
      OnlyVal = Val;
    } else {
      if (OnlyDest != DestBB)
        OnlyDest = MultipleDestSentinel;
      // It possible we have same destination, but different value, e.g. default
      // case in switchinst.
      if (Val != OnlyVal)
        OnlyVal = MultipleVal;
    }

    // We know where this predecessor is going.
    ++PredWithKnownDest;

    // If the predecessor ends with an indirect goto, we can't change its
    // destination.
    if (isa<IndirectBrInst>(Pred->getTerminator()))
      continue;

    PredToDestList.push_back(std::make_pair(Pred, DestBB));
  }

  // If all edges were unthreadable, we fail.
  if (PredToDestList.empty())
    return Changed;

  // If all the predecessors go to a single known successor, we want to fold,
  // not thread. By doing so, we do not need to duplicate the current block and
  // also miss potential opportunities in case we dont/cant duplicate.
  if (OnlyDest && OnlyDest != MultipleDestSentinel) {
    if (PredWithKnownDest ==
        (size_t)std::distance(pred_begin(BB), pred_end(BB))) {
      bool SeenFirstBranchToOnlyDest = false;
      std::vector <DominatorTree::UpdateType> Updates;
      Updates.reserve(BB->getTerminator()->getNumSuccessors() - 1);
      for (BasicBlock *SuccBB : successors(BB)) {
        if (SuccBB == OnlyDest && !SeenFirstBranchToOnlyDest) {
          SeenFirstBranchToOnlyDest = true; // Don't modify the first branch.
        } else {
          SuccBB->removePredecessor(BB, true); // This is unreachable successor.
          Updates.push_back({DominatorTree::Delete, BB, SuccBB});
        }
      }

      // Finally update the terminator.
      TerminatorInst *Term = BB->getTerminator();
      BranchInst::Create(OnlyDest, Term);
      Term->eraseFromParent();
      DDT->applyUpdates(Updates);

      // If the condition is now dead due to the removal of the old terminator,
      // erase it.
      if (auto *CondInst = dyn_cast<Instruction>(Cond)) {
        if (CondInst->use_empty() && !CondInst->mayHaveSideEffects())
          CondInst->eraseFromParent();
        // We can safely replace *some* uses of the CondInst if it has
        // exactly one value as returned by LVI. RAUW is incorrect in the
        // presence of guards and assumes, that have the `Cond` as the use. This
        // is because we use the guards/assume to reason about the `Cond` value
        // at the end of block, but RAUW unconditionally replaces all uses
        // including the guards/assumes themselves and the uses before the
        // guard/assume.
        else if (OnlyVal && OnlyVal != MultipleVal &&
                 CondInst->getParent() == BB)
          ReplaceFoldableUses(CondInst, OnlyVal);
      }
      return true;
    }
  }

  // Determine which is the most common successor.  If we have many inputs and
  // this block is a switch, we want to start by threading the batch that goes
  // to the most popular destination first.  If we only know about one
  // threadable destination (the common case) we can avoid this.
  BasicBlock *MostPopularDest = OnlyDest;

  if (MostPopularDest == MultipleDestSentinel)
    MostPopularDest = FindMostPopularDest(BB, PredToDestList);

  // Now that we know what the most popular destination is, factor all
  // predecessors that will jump to it into a single predecessor.
  SmallVector<BasicBlock*, 16> PredsToFactor;
  for (const auto &PredToDest : PredToDestList)
    if (PredToDest.second == MostPopularDest) {
      BasicBlock *Pred = PredToDest.first;

      // This predecessor may be a switch or something else that has multiple
      // edges to the block.  Factor each of these edges by listing them
      // according to # occurrences in PredsToFactor.
      for (BasicBlock *Succ : successors(Pred))
        if (Succ == BB)
          PredsToFactor.push_back(Pred);
    }

  // If the threadable edges are branching on an undefined value, we get to pick
  // the destination that these predecessors should get to.
  if (!MostPopularDest)
    MostPopularDest = BB->getTerminator()->
                            getSuccessor(GetBestDestForJumpOnUndef(BB));

  // Ok, try to thread it!
  return ThreadEdge(BB, PredsToFactor, MostPopularDest);
}

/// ProcessBranchOnPHI - We have an otherwise unthreadable conditional branch on
/// a PHI node in the current block.  See if there are any simplifications we
/// can do based on inputs to the phi node.
bool JumpThreadingPass::ProcessBranchOnPHI(PHINode *PN) {
  BasicBlock *BB = PN->getParent();

  // TODO: We could make use of this to do it once for blocks with common PHI
  // values.
  SmallVector<BasicBlock*, 1> PredBBs;
  PredBBs.resize(1);

  // If any of the predecessor blocks end in an unconditional branch, we can
  // *duplicate* the conditional branch into that block in order to further
  // encourage jump threading and to eliminate cases where we have branch on a
  // phi of an icmp (branch on icmp is much better).
  for (unsigned i = 0, e = PN->getNumIncomingValues(); i != e; ++i) {
    BasicBlock *PredBB = PN->getIncomingBlock(i);
    if (BranchInst *PredBr = dyn_cast<BranchInst>(PredBB->getTerminator()))
      if (PredBr->isUnconditional()) {
        PredBBs[0] = PredBB;
        // Try to duplicate BB into PredBB.
        if (DuplicateCondBranchOnPHIIntoPred(BB, PredBBs))
          return true;
      }
  }

  return false;
}

/// ProcessBranchOnXOR - We have an otherwise unthreadable conditional branch on
/// a xor instruction in the current block.  See if there are any
/// simplifications we can do based on inputs to the xor.
bool JumpThreadingPass::ProcessBranchOnXOR(BinaryOperator *BO) {
  BasicBlock *BB = BO->getParent();

  // If either the LHS or RHS of the xor is a constant, don't do this
  // optimization.
  if (isa<ConstantInt>(BO->getOperand(0)) ||
      isa<ConstantInt>(BO->getOperand(1)))
    return false;

  // If the first instruction in BB isn't a phi, we won't be able to infer
  // anything special about any particular predecessor.
  if (!isa<PHINode>(BB->front()))
    return false;

  // If this BB is a landing pad, we won't be able to split the edge into it.
  if (BB->isEHPad())
    return false;

  // If we have a xor as the branch input to this block, and we know that the
  // LHS or RHS of the xor in any predecessor is true/false, then we can clone
  // the condition into the predecessor and fix that value to true, saving some
  // logical ops on that path and encouraging other paths to simplify.
  //
  // This copies something like this:
  //
  //  BB:
  //    %X = phi i1 [1],  [%X']
  //    %Y = icmp eq i32 %A, %B
  //    %Z = xor i1 %X, %Y
  //    br i1 %Z, ...
  //
  // Into:
  //  BB':
  //    %Y = icmp ne i32 %A, %B
  //    br i1 %Y, ...

  PredValueInfoTy XorOpValues;
  bool isLHS = true;
  if (!ComputeValueKnownInPredecessors(BO->getOperand(0), BB, XorOpValues,
                                       WantInteger, BO)) {
    assert(XorOpValues.empty());
    if (!ComputeValueKnownInPredecessors(BO->getOperand(1), BB, XorOpValues,
                                         WantInteger, BO))
      return false;
    isLHS = false;
  }

  assert(!XorOpValues.empty() &&
         "ComputeValueKnownInPredecessors returned true with no values");

  // Scan the information to see which is most popular: true or false.  The
  // predecessors can be of the set true, false, or undef.
  unsigned NumTrue = 0, NumFalse = 0;
  for (const auto &XorOpValue : XorOpValues) {
    if (isa<UndefValue>(XorOpValue.first))
      // Ignore undefs for the count.
      continue;
    if (cast<ConstantInt>(XorOpValue.first)->isZero())
      ++NumFalse;
    else
      ++NumTrue;
  }

  // Determine which value to split on, true, false, or undef if neither.
  ConstantInt *SplitVal = nullptr;
  if (NumTrue > NumFalse)
    SplitVal = ConstantInt::getTrue(BB->getContext());
  else if (NumTrue != 0 || NumFalse != 0)
    SplitVal = ConstantInt::getFalse(BB->getContext());

  // Collect all of the blocks that this can be folded into so that we can
  // factor this once and clone it once.
  SmallVector<BasicBlock*, 8> BlocksToFoldInto;
  for (const auto &XorOpValue : XorOpValues) {
    if (XorOpValue.first != SplitVal && !isa<UndefValue>(XorOpValue.first))
      continue;

    BlocksToFoldInto.push_back(XorOpValue.second);
  }

  // If we inferred a value for all of the predecessors, then duplication won't
  // help us.  However, we can just replace the LHS or RHS with the constant.
  if (BlocksToFoldInto.size() ==
      cast<PHINode>(BB->front()).getNumIncomingValues()) {
    if (!SplitVal) {
      // If all preds provide undef, just nuke the xor, because it is undef too.
      BO->replaceAllUsesWith(UndefValue::get(BO->getType()));
      BO->eraseFromParent();
    } else if (SplitVal->isZero()) {
      // If all preds provide 0, replace the xor with the other input.
      BO->replaceAllUsesWith(BO->getOperand(isLHS));
      BO->eraseFromParent();
    } else {
      // If all preds provide 1, set the computed value to 1.
      BO->setOperand(!isLHS, SplitVal);
    }

    return true;
  }

  // Try to duplicate BB into PredBB.
  return DuplicateCondBranchOnPHIIntoPred(BB, BlocksToFoldInto);
}

/// AddPHINodeEntriesForMappedBlock - We're adding 'NewPred' as a new
/// predecessor to the PHIBB block.  If it has PHI nodes, add entries for
/// NewPred using the entries from OldPred (suitably mapped).
static void AddPHINodeEntriesForMappedBlock(BasicBlock *PHIBB,
                                            BasicBlock *OldPred,
                                            BasicBlock *NewPred,
                                     DenseMap<Instruction*, Value*> &ValueMap) {
  for (PHINode &PN : PHIBB->phis()) {
    // Ok, we have a PHI node.  Figure out what the incoming value was for the
    // DestBlock.
    Value *IV = PN.getIncomingValueForBlock(OldPred);

    // Remap the value if necessary.
    if (Instruction *Inst = dyn_cast<Instruction>(IV)) {
      DenseMap<Instruction*, Value*>::iterator I = ValueMap.find(Inst);
      if (I != ValueMap.end())
        IV = I->second;
    }

    PN.addIncoming(IV, NewPred);
  }
}

/// ThreadEdge - We have decided that it is safe and profitable to factor the
/// blocks in PredBBs to one predecessor, then thread an edge from it to SuccBB
/// across BB.  Transform the IR to reflect this change.
bool JumpThreadingPass::ThreadEdge(BasicBlock *BB,
                                   const SmallVectorImpl<BasicBlock *> &PredBBs,
                                   BasicBlock *SuccBB) {
  // If threading to the same block as we come from, we would infinite loop.
  if (SuccBB == BB) {
    DEBUG(dbgs() << "  Not threading across BB '" << BB->getName()
          << "' - would thread to self!\n");
    return false;
  }

  // If threading this would thread across a loop header, don't thread the edge.
  // See the comments above FindLoopHeaders for justifications and caveats.
  if (LoopHeaders.count(BB) || LoopHeaders.count(SuccBB)) {
    DEBUG({
      bool BBIsHeader = LoopHeaders.count(BB);
      bool SuccIsHeader = LoopHeaders.count(SuccBB);
      dbgs() << "  Not threading across "
          << (BBIsHeader ? "loop header BB '" : "block BB '") << BB->getName()
          << "' to dest " << (SuccIsHeader ? "loop header BB '" : "block BB '")
          << SuccBB->getName() << "' - it might create an irreducible loop!\n";
    });
    return false;
  }

  unsigned JumpThreadCost =
      getJumpThreadDuplicationCost(BB, BB->getTerminator(), BBDupThreshold);
  if (JumpThreadCost > BBDupThreshold) {
    DEBUG(dbgs() << "  Not threading BB '" << BB->getName()
          << "' - Cost is too high: " << JumpThreadCost << "\n");
    return false;
  }

  // And finally, do it!  Start by factoring the predecessors if needed.
  BasicBlock *PredBB;
  if (PredBBs.size() == 1)
    PredBB = PredBBs[0];
  else {
    DEBUG(dbgs() << "  Factoring out " << PredBBs.size()
          << " common predecessors.\n");
    PredBB = SplitBlockPreds(BB, PredBBs, ".thr_comm");
  }

  // And finally, do it!
  DEBUG(dbgs() << "  Threading edge from '" << PredBB->getName() << "' to '"
        << SuccBB->getName() << "' with cost: " << JumpThreadCost
        << ", across block:\n    "
        << *BB << "\n");

  LVI->threadEdge(PredBB, BB, SuccBB);

  // We are going to have to map operands from the original BB block to the new
  // copy of the block 'NewBB'.  If there are PHI nodes in BB, evaluate them to
  // account for entry from PredBB.
  DenseMap<Instruction*, Value*> ValueMapping;

  BasicBlock *NewBB = BasicBlock::Create(BB->getContext(),
                                         BB->getName()+".thread",
                                         BB->getParent(), BB);
  NewBB->moveAfter(PredBB);

  // Set the block frequency of NewBB.
  if (HasProfileData) {
    auto NewBBFreq =
        BFI->getBlockFreq(PredBB) * BPI->getEdgeProbability(PredBB, BB);
    BFI->setBlockFreq(NewBB, NewBBFreq.getFrequency());
  }

  BasicBlock::iterator BI = BB->begin();
  for (; PHINode *PN = dyn_cast<PHINode>(BI); ++BI)
    ValueMapping[PN] = PN->getIncomingValueForBlock(PredBB);

  // Clone the non-phi instructions of BB into NewBB, keeping track of the
  // mapping and using it to remap operands in the cloned instructions.
  for (; !isa<TerminatorInst>(BI); ++BI) {
    Instruction *New = BI->clone();
    New->setName(BI->getName());
    NewBB->getInstList().push_back(New);
    ValueMapping[&*BI] = New;

    // Remap operands to patch up intra-block references.
    for (unsigned i = 0, e = New->getNumOperands(); i != e; ++i)
      if (Instruction *Inst = dyn_cast<Instruction>(New->getOperand(i))) {
        DenseMap<Instruction*, Value*>::iterator I = ValueMapping.find(Inst);
        if (I != ValueMapping.end())
          New->setOperand(i, I->second);
      }
  }

  // We didn't copy the terminator from BB over to NewBB, because there is now
  // an unconditional jump to SuccBB.  Insert the unconditional jump.
  BranchInst *NewBI = BranchInst::Create(SuccBB, NewBB);
  NewBI->setDebugLoc(BB->getTerminator()->getDebugLoc());

  // Check to see if SuccBB has PHI nodes. If so, we need to add entries to the
  // PHI nodes for NewBB now.
  AddPHINodeEntriesForMappedBlock(SuccBB, BB, NewBB, ValueMapping);

  // If there were values defined in BB that are used outside the block, then we
  // now have to update all uses of the value to use either the original value,
  // the cloned value, or some PHI derived value.  This can require arbitrary
  // PHI insertion, of which we are prepared to do, clean these up now.
  SSAUpdater SSAUpdate;
  SmallVector<Use*, 16> UsesToRename;
  for (Instruction &I : *BB) {
    // Scan all uses of this instruction to see if it is used outside of its
    // block, and if so, record them in UsesToRename.
    for (Use &U : I.uses()) {
      Instruction *User = cast<Instruction>(U.getUser());
      if (PHINode *UserPN = dyn_cast<PHINode>(User)) {
        if (UserPN->getIncomingBlock(U) == BB)
          continue;
      } else if (User->getParent() == BB)
        continue;

      UsesToRename.push_back(&U);
    }

    // If there are no uses outside the block, we're done with this instruction.
    if (UsesToRename.empty())
      continue;

    DEBUG(dbgs() << "JT: Renaming non-local uses of: " << I << "\n");

    // We found a use of I outside of BB.  Rename all uses of I that are outside
    // its block to be uses of the appropriate PHI node etc.  See ValuesInBlocks
    // with the two values we know.
    SSAUpdate.Initialize(I.getType(), I.getName());
    SSAUpdate.AddAvailableValue(BB, &I);
    SSAUpdate.AddAvailableValue(NewBB, ValueMapping[&I]);

    while (!UsesToRename.empty())
      SSAUpdate.RewriteUse(*UsesToRename.pop_back_val());
    DEBUG(dbgs() << "\n");
  }

  // Ok, NewBB is good to go.  Update the terminator of PredBB to jump to
  // NewBB instead of BB.  This eliminates predecessors from BB, which requires
  // us to simplify any PHI nodes in BB.
  TerminatorInst *PredTerm = PredBB->getTerminator();
  for (unsigned i = 0, e = PredTerm->getNumSuccessors(); i != e; ++i)
    if (PredTerm->getSuccessor(i) == BB) {
      BB->removePredecessor(PredBB, true);
      PredTerm->setSuccessor(i, NewBB);
    }

  DDT->applyUpdates({{DominatorTree::Insert, NewBB, SuccBB},
                     {DominatorTree::Insert, PredBB, NewBB},
                     {DominatorTree::Delete, PredBB, BB}});

  // At this point, the IR is fully up to date and consistent.  Do a quick scan
  // over the new instructions and zap any that are constants or dead.  This
  // frequently happens because of phi translation.
  SimplifyInstructionsInBlock(NewBB, TLI);

  // Update the edge weight from BB to SuccBB, which should be less than before.
  UpdateBlockFreqAndEdgeWeight(PredBB, BB, NewBB, SuccBB);

  // Threaded an edge!
  ++NumThreads;
  return true;
}

/// Create a new basic block that will be the predecessor of BB and successor of
/// all blocks in Preds. When profile data is available, update the frequency of
/// this new block.
BasicBlock *JumpThreadingPass::SplitBlockPreds(BasicBlock *BB,
                                               ArrayRef<BasicBlock *> Preds,
                                               const char *Suffix) {
  SmallVector<BasicBlock *, 2> NewBBs;

  // Collect the frequencies of all predecessors of BB, which will be used to
  // update the edge weight of the result of splitting predecessors.
  DenseMap<BasicBlock *, BlockFrequency> FreqMap;
  if (HasProfileData)
    for (auto Pred : Preds)
      FreqMap.insert(std::make_pair(
          Pred, BFI->getBlockFreq(Pred) * BPI->getEdgeProbability(Pred, BB)));

  // In the case when BB is a LandingPad block we create 2 new predecessors
  // instead of just one.
  if (BB->isLandingPad()) {
    std::string NewName = std::string(Suffix) + ".split-lp";
    SplitLandingPadPredecessors(BB, Preds, Suffix, NewName.c_str(), NewBBs);
  } else {
    NewBBs.push_back(SplitBlockPredecessors(BB, Preds, Suffix));
  }

  std::vector<DominatorTree::UpdateType> Updates;
  Updates.reserve((2 * Preds.size()) + NewBBs.size());
  for (auto NewBB : NewBBs) {
    BlockFrequency NewBBFreq(0);
    Updates.push_back({DominatorTree::Insert, NewBB, BB});
    for (auto Pred : predecessors(NewBB)) {
      Updates.push_back({DominatorTree::Delete, Pred, BB});
      Updates.push_back({DominatorTree::Insert, Pred, NewBB});
      if (HasProfileData) // Update frequencies between Pred -> NewBB.
        NewBBFreq += FreqMap.lookup(Pred);
    }
    if (HasProfileData) // Apply the summed frequency to NewBB.
      BFI->setBlockFreq(NewBB, NewBBFreq.getFrequency());
  }

  DDT->applyUpdates(Updates);
  return NewBBs[0];
}

bool JumpThreadingPass::doesBlockHaveProfileData(BasicBlock *BB) {
  const TerminatorInst *TI = BB->getTerminator();
  assert(TI->getNumSuccessors() > 1 && "not a split");

  MDNode *WeightsNode = TI->getMetadata(LLVMContext::MD_prof);
  if (!WeightsNode)
    return false;

  MDString *MDName = cast<MDString>(WeightsNode->getOperand(0));
  if (MDName->getString() != "branch_weights")
    return false;

  // Ensure there are weights for all of the successors. Note that the first
  // operand to the metadata node is a name, not a weight.
  return WeightsNode->getNumOperands() == TI->getNumSuccessors() + 1;
}

/// Update the block frequency of BB and branch weight and the metadata on the
/// edge BB->SuccBB. This is done by scaling the weight of BB->SuccBB by 1 -
/// Freq(PredBB->BB) / Freq(BB->SuccBB).
void JumpThreadingPass::UpdateBlockFreqAndEdgeWeight(BasicBlock *PredBB,
                                                     BasicBlock *BB,
                                                     BasicBlock *NewBB,
                                                     BasicBlock *SuccBB) {
  if (!HasProfileData)
    return;

  assert(BFI && BPI && "BFI & BPI should have been created here");

  // As the edge from PredBB to BB is deleted, we have to update the block
  // frequency of BB.
  auto BBOrigFreq = BFI->getBlockFreq(BB);
  auto NewBBFreq = BFI->getBlockFreq(NewBB);
  auto BB2SuccBBFreq = BBOrigFreq * BPI->getEdgeProbability(BB, SuccBB);
  auto BBNewFreq = BBOrigFreq - NewBBFreq;
  BFI->setBlockFreq(BB, BBNewFreq.getFrequency());

  // Collect updated outgoing edges' frequencies from BB and use them to update
  // edge probabilities.
  SmallVector<uint64_t, 4> BBSuccFreq;
  for (BasicBlock *Succ : successors(BB)) {
    auto SuccFreq = (Succ == SuccBB)
                        ? BB2SuccBBFreq - NewBBFreq
                        : BBOrigFreq * BPI->getEdgeProbability(BB, Succ);
    BBSuccFreq.push_back(SuccFreq.getFrequency());
  }

  uint64_t MaxBBSuccFreq =
      *std::max_element(BBSuccFreq.begin(), BBSuccFreq.end());

  SmallVector<BranchProbability, 4> BBSuccProbs;
  if (MaxBBSuccFreq == 0)
    BBSuccProbs.assign(BBSuccFreq.size(),
                       {1, static_cast<unsigned>(BBSuccFreq.size())});
  else {
    for (uint64_t Freq : BBSuccFreq)
      BBSuccProbs.push_back(
          BranchProbability::getBranchProbability(Freq, MaxBBSuccFreq));
    // Normalize edge probabilities so that they sum up to one.
    BranchProbability::normalizeProbabilities(BBSuccProbs.begin(),
                                              BBSuccProbs.end());
  }

  // Update edge probabilities in BPI.
  for (int I = 0, E = BBSuccProbs.size(); I < E; I++)
    BPI->setEdgeProbability(BB, I, BBSuccProbs[I]);

  // Update the profile metadata as well.
  //
  // Don't do this if the profile of the transformed blocks was statically
  // estimated.  (This could occur despite the function having an entry
  // frequency in completely cold parts of the CFG.)
  //
  // In this case we don't want to suggest to subsequent passes that the
  // calculated weights are fully consistent.  Consider this graph:
  //
  //                 check_1
  //             50% /  |
  //             eq_1   | 50%
  //                 \  |
  //                 check_2
  //             50% /  |
  //             eq_2   | 50%
  //                 \  |
  //                 check_3
  //             50% /  |
  //             eq_3   | 50%
  //                 \  |
  //
  // Assuming the blocks check_* all compare the same value against 1, 2 and 3,
  // the overall probabilities are inconsistent; the total probability that the
  // value is either 1, 2 or 3 is 150%.
  //
  // As a consequence if we thread eq_1 -> check_2 to check_3, check_2->check_3
  // becomes 0%.  This is even worse if the edge whose probability becomes 0% is
  // the loop exit edge.  Then based solely on static estimation we would assume
  // the loop was extremely hot.
  //
  // FIXME this locally as well so that BPI and BFI are consistent as well.  We
  // shouldn't make edges extremely likely or unlikely based solely on static
  // estimation.
  if (BBSuccProbs.size() >= 2 && doesBlockHaveProfileData(BB)) {
    SmallVector<uint32_t, 4> Weights;
    for (auto Prob : BBSuccProbs)
      Weights.push_back(Prob.getNumerator());

    auto TI = BB->getTerminator();
    TI->setMetadata(
        LLVMContext::MD_prof,
        MDBuilder(TI->getParent()->getContext()).createBranchWeights(Weights));
  }
}

/// DuplicateCondBranchOnPHIIntoPred - PredBB contains an unconditional branch
/// to BB which contains an i1 PHI node and a conditional branch on that PHI.
/// If we can duplicate the contents of BB up into PredBB do so now, this
/// improves the odds that the branch will be on an analyzable instruction like
/// a compare.
bool JumpThreadingPass::DuplicateCondBranchOnPHIIntoPred(
    BasicBlock *BB, const SmallVectorImpl<BasicBlock *> &PredBBs) {
  assert(!PredBBs.empty() && "Can't handle an empty set");

  // If BB is a loop header, then duplicating this block outside the loop would
  // cause us to transform this into an irreducible loop, don't do this.
  // See the comments above FindLoopHeaders for justifications and caveats.
  if (LoopHeaders.count(BB)) {
    DEBUG(dbgs() << "  Not duplicating loop header '" << BB->getName()
          << "' into predecessor block '" << PredBBs[0]->getName()
          << "' - it might create an irreducible loop!\n");
    return false;
  }

  if (HLS) {
    // HLS BEGIN
    // Don't thread across the loop latch
    for (BasicBlock *SuccBB : successors(BB)) {
      if (LoopHeaders.count(SuccBB))
        return false;
    }
    // HLS END
  }

  unsigned DuplicationCost =
      getJumpThreadDuplicationCost(BB, BB->getTerminator(), BBDupThreshold);
  if (DuplicationCost > BBDupThreshold) {
    DEBUG(dbgs() << "  Not duplicating BB '" << BB->getName()
          << "' - Cost is too high: " << DuplicationCost << "\n");
    return false;
  }

  // And finally, do it!  Start by factoring the predecessors if needed.
  std::vector<DominatorTree::UpdateType> Updates;
  BasicBlock *PredBB;
  if (PredBBs.size() == 1)
    PredBB = PredBBs[0];
  else {
    DEBUG(dbgs() << "  Factoring out " << PredBBs.size()
          << " common predecessors.\n");
    PredBB = SplitBlockPreds(BB, PredBBs, ".thr_comm");
  }
  Updates.push_back({DominatorTree::Delete, PredBB, BB});

  // Okay, we decided to do this!  Clone all the instructions in BB onto the end
  // of PredBB.
  DEBUG(dbgs() << "  Duplicating block '" << BB->getName() << "' into end of '"
        << PredBB->getName() << "' to eliminate branch on phi.  Cost: "
        << DuplicationCost << " block is:" << *BB << "\n");

  // Unless PredBB ends with an unconditional branch, split the edge so that we
  // can just clone the bits from BB into the end of the new PredBB.
  BranchInst *OldPredBranch = dyn_cast<BranchInst>(PredBB->getTerminator());

  if (!OldPredBranch || !OldPredBranch->isUnconditional()) {
    BasicBlock *OldPredBB = PredBB;
    PredBB = SplitEdge(OldPredBB, BB);
    Updates.push_back({DominatorTree::Insert, OldPredBB, PredBB});
    Updates.push_back({DominatorTree::Insert, PredBB, BB});
    Updates.push_back({DominatorTree::Delete, OldPredBB, BB});
    OldPredBranch = cast<BranchInst>(PredBB->getTerminator());
  }

  // We are going to have to map operands from the original BB block into the
  // PredBB block.  Evaluate PHI nodes in BB.
  DenseMap<Instruction*, Value*> ValueMapping;

  BasicBlock::iterator BI = BB->begin();
  for (; PHINode *PN = dyn_cast<PHINode>(BI); ++BI)
    ValueMapping[PN] = PN->getIncomingValueForBlock(PredBB);
  // Clone the non-phi instructions of BB into PredBB, keeping track of the
  // mapping and using it to remap operands in the cloned instructions.
  for (; BI != BB->end(); ++BI) {
    Instruction *New = BI->clone();

    // Remap operands to patch up intra-block references.
    for (unsigned i = 0, e = New->getNumOperands(); i != e; ++i)
      if (Instruction *Inst = dyn_cast<Instruction>(New->getOperand(i))) {
        DenseMap<Instruction*, Value*>::iterator I = ValueMapping.find(Inst);
        if (I != ValueMapping.end())
          New->setOperand(i, I->second);
      }

    // If this instruction can be simplified after the operands are updated,
    // just use the simplified value instead.  This frequently happens due to
    // phi translation.
    if (Value *IV = SimplifyInstruction(
            New,
            {BB->getModule()->getDataLayout(), TLI, nullptr, nullptr, New})) {
      ValueMapping[&*BI] = IV;
      if (!New->mayHaveSideEffects()) {
        New->deleteValue();
        New = nullptr;
      }
    } else {
      ValueMapping[&*BI] = New;
    }
    if (New) {
      // Otherwise, insert the new instruction into the block.
      New->setName(BI->getName());
      PredBB->getInstList().insert(OldPredBranch->getIterator(), New);
      // Update Dominance from simplified New instruction operands.
      for (unsigned i = 0, e = New->getNumOperands(); i != e; ++i)
        if (BasicBlock *SuccBB = dyn_cast<BasicBlock>(New->getOperand(i)))
          Updates.push_back({DominatorTree::Insert, PredBB, SuccBB});
    }
  }

  // Check to see if the targets of the branch had PHI nodes. If so, we need to
  // add entries to the PHI nodes for branch from PredBB now.
  BranchInst *BBBranch = cast<BranchInst>(BB->getTerminator());
  AddPHINodeEntriesForMappedBlock(BBBranch->getSuccessor(0), BB, PredBB,
                                  ValueMapping);
  AddPHINodeEntriesForMappedBlock(BBBranch->getSuccessor(1), BB, PredBB,
                                  ValueMapping);

  // If there were values defined in BB that are used outside the block, then we
  // now have to update all uses of the value to use either the original value,
  // the cloned value, or some PHI derived value.  This can require arbitrary
  // PHI insertion, of which we are prepared to do, clean these up now.
  SSAUpdater SSAUpdate;
  SmallVector<Use*, 16> UsesToRename;
  for (Instruction &I : *BB) {
    // Scan all uses of this instruction to see if it is used outside of its
    // block, and if so, record them in UsesToRename.
    for (Use &U : I.uses()) {
      Instruction *User = cast<Instruction>(U.getUser());
      if (PHINode *UserPN = dyn_cast<PHINode>(User)) {
        if (UserPN->getIncomingBlock(U) == BB)
          continue;
      } else if (User->getParent() == BB)
        continue;

      UsesToRename.push_back(&U);
    }

    // If there are no uses outside the block, we're done with this instruction.
    if (UsesToRename.empty())
      continue;

    DEBUG(dbgs() << "JT: Renaming non-local uses of: " << I << "\n");

    // We found a use of I outside of BB.  Rename all uses of I that are outside
    // its block to be uses of the appropriate PHI node etc.  See ValuesInBlocks
    // with the two values we know.
    SSAUpdate.Initialize(I.getType(), I.getName());
    SSAUpdate.AddAvailableValue(BB, &I);
    SSAUpdate.AddAvailableValue(PredBB, ValueMapping[&I]);

    while (!UsesToRename.empty())
      SSAUpdate.RewriteUse(*UsesToRename.pop_back_val());
    DEBUG(dbgs() << "\n");
  }

  // PredBB no longer jumps to BB, remove entries in the PHI node for the edge
  // that we nuked.
  BB->removePredecessor(PredBB, true);

  // Remove the unconditional branch at the end of the PredBB block.
  OldPredBranch->eraseFromParent();
  DDT->applyUpdates(Updates);

  ++NumDupes;
  return true;
}

/// TryToUnfoldSelect - Look for blocks of the form
/// bb1:
///   %a = select
///   br bb2
///
/// bb2:
///   %p = phi [%a, %bb1] ...
///   %c = icmp %p
///   br i1 %c
///
/// And expand the select into a branch structure if one of its arms allows %c
/// to be folded. This later enables threading from bb1 over bb2.
bool JumpThreadingPass::TryToUnfoldSelect(CmpInst *CondCmp, BasicBlock *BB) {
  // HLS BEGIN
  // unfold select in latch may cause loop header has extra incoming edge
  if(HLS && LoopHeaders.count(BB))
    return false;
  // HLS END
 
  BranchInst *CondBr = dyn_cast<BranchInst>(BB->getTerminator());
  PHINode *CondLHS = dyn_cast<PHINode>(CondCmp->getOperand(0));
  Constant *CondRHS = cast<Constant>(CondCmp->getOperand(1));

  if (!CondBr || !CondBr->isConditional() || !CondLHS ||
      CondLHS->getParent() != BB)
    return false;

  for (unsigned I = 0, E = CondLHS->getNumIncomingValues(); I != E; ++I) {
    BasicBlock *Pred = CondLHS->getIncomingBlock(I);
    SelectInst *SI = dyn_cast<SelectInst>(CondLHS->getIncomingValue(I));

    // Look if one of the incoming values is a select in the corresponding
    // predecessor.
    if (!SI || SI->getParent() != Pred || !SI->hasOneUse())
      continue;

    BranchInst *PredTerm = dyn_cast<BranchInst>(Pred->getTerminator());
    if (!PredTerm || !PredTerm->isUnconditional())
      continue;

    // Now check if one of the select values would allow us to constant fold the
    // terminator in BB. We don't do the transform if both sides fold, those
    // cases will be threaded in any case.
    LazyValueInfo::Tristate LHSFolds =
        LVI->getPredicateOnEdge(CondCmp->getPredicate(), SI->getOperand(1),
                                CondRHS, Pred, BB, CondCmp);
    LazyValueInfo::Tristate RHSFolds =
        LVI->getPredicateOnEdge(CondCmp->getPredicate(), SI->getOperand(2),
                                CondRHS, Pred, BB, CondCmp);
    if ((LHSFolds != LazyValueInfo::Unknown ||
         RHSFolds != LazyValueInfo::Unknown) &&
        LHSFolds != RHSFolds) {
      // Expand the select.
      //
      // Pred --
      //  |    v
      //  |  NewBB
      //  |    |
      //  |-----
      //  v
      // BB
      BasicBlock *NewBB = BasicBlock::Create(BB->getContext(), "select.unfold",
                                             BB->getParent(), BB);
      // Move the unconditional branch to NewBB.
      PredTerm->removeFromParent();
      NewBB->getInstList().insert(NewBB->end(), PredTerm);
      // Create a conditional branch and update PHI nodes.
      BranchInst::Create(NewBB, BB, SI->getCondition(), Pred);
      CondLHS->setIncomingValue(I, SI->getFalseValue());
      CondLHS->addIncoming(SI->getTrueValue(), NewBB);
      // The select is now dead.
      SI->eraseFromParent();

      DDT->applyUpdates({{DominatorTree::Insert, NewBB, BB},
                         {DominatorTree::Insert, Pred, NewBB}});
      // Update any other PHI nodes in BB.
      for (BasicBlock::iterator BI = BB->begin();
           PHINode *Phi = dyn_cast<PHINode>(BI); ++BI)
        if (Phi != CondLHS)
          Phi->addIncoming(Phi->getIncomingValueForBlock(Pred), NewBB);
      return true;
    }
  }
  return false;
}

/// TryToUnfoldSelectInCurrBB - Look for PHI/Select or PHI/CMP/Select in the
/// same BB in the form
/// bb:
///   %p = phi [false, %bb1], [true, %bb2], [false, %bb3], [true, %bb4], ...
///   %s = select %p, trueval, falseval
///
/// or
///
/// bb:
///   %p = phi [0, %bb1], [1, %bb2], [0, %bb3], [1, %bb4], ...
///   %c = cmp %p, 0
///   %s = select %c, trueval, falseval
///
/// And expand the select into a branch structure. This later enables
/// jump-threading over bb in this pass.
///
/// Using the similar approach of SimplifyCFG::FoldCondBranchOnPHI(), unfold
/// select if the associated PHI has at least one constant.  If the unfolded
/// select is not jump-threaded, it will be folded again in the later
/// optimizations.
bool JumpThreadingPass::TryToUnfoldSelectInCurrBB(BasicBlock *BB) {
  // If threading this would thread across a loop header, don't thread the edge.
  // See the comments above FindLoopHeaders for justifications and caveats.
  if (LoopHeaders.count(BB))
    return false;

  for (BasicBlock::iterator BI = BB->begin();
       PHINode *PN = dyn_cast<PHINode>(BI); ++BI) {
    // Look for a Phi having at least one constant incoming value.
    if (llvm::all_of(PN->incoming_values(),
                     [](Value *V) { return !isa<ConstantInt>(V); }))
      continue;

    auto isUnfoldCandidate = [BB](SelectInst *SI, Value *V) {
      // Check if SI is in BB and use V as condition.
      if (SI->getParent() != BB)
        return false;
      Value *Cond = SI->getCondition();
      return (Cond && Cond == V && Cond->getType()->isIntegerTy(1));
    };

    SelectInst *SI = nullptr;
    for (Use &U : PN->uses()) {
      if (ICmpInst *Cmp = dyn_cast<ICmpInst>(U.getUser())) {
        // Look for a ICmp in BB that compares PN with a constant and is the
        // condition of a Select.
        if (Cmp->getParent() == BB && Cmp->hasOneUse() &&
            isa<ConstantInt>(Cmp->getOperand(1 - U.getOperandNo())))
          if (SelectInst *SelectI = dyn_cast<SelectInst>(Cmp->user_back()))
            if (isUnfoldCandidate(SelectI, Cmp->use_begin()->get())) {
              SI = SelectI;
              break;
            }
      } else if (SelectInst *SelectI = dyn_cast<SelectInst>(U.getUser())) {
        // Look for a Select in BB that uses PN as condtion.
        if (isUnfoldCandidate(SelectI, U.get())) {
          SI = SelectI;
          break;
        }
      }
    }

    if (!SI)
      continue;
    // Expand the select.
    TerminatorInst *Term =
        SplitBlockAndInsertIfThen(SI->getCondition(), SI, false);
    BasicBlock *SplitBB = SI->getParent();
    BasicBlock *NewBB = Term->getParent();
    PHINode *NewPN = PHINode::Create(SI->getType(), 2, "", SI);
    NewPN->addIncoming(SI->getTrueValue(), Term->getParent());
    NewPN->addIncoming(SI->getFalseValue(), BB);
    SI->replaceAllUsesWith(NewPN);
    SI->eraseFromParent();
    // NewBB and SplitBB are newly created blocks which require insertion.
    std::vector<DominatorTree::UpdateType> Updates;
    Updates.reserve((2 * SplitBB->getTerminator()->getNumSuccessors()) + 3);
    Updates.push_back({DominatorTree::Insert, BB, SplitBB});
    Updates.push_back({DominatorTree::Insert, BB, NewBB});
    Updates.push_back({DominatorTree::Insert, NewBB, SplitBB});
    // BB's successors were moved to SplitBB, update DDT accordingly.
    for (auto *Succ : successors(SplitBB)) {
      Updates.push_back({DominatorTree::Delete, BB, Succ});
      Updates.push_back({DominatorTree::Insert, SplitBB, Succ});
    }
    DDT->applyUpdates(Updates);
    return true;
  }
  return false;
}

/// Try to propagate a guard from the current BB into one of its predecessors
/// in case if another branch of execution implies that the condition of this
/// guard is always true. Currently we only process the simplest case that
/// looks like:
///
/// Start:
///   %cond = ...
///   br i1 %cond, label %T1, label %F1
/// T1:
///   br label %Merge
/// F1:
///   br label %Merge
/// Merge:
///   %condGuard = ...
///   call void(i1, ...) @llvm.experimental.guard( i1 %condGuard )[ "deopt"() ]
///
/// And cond either implies condGuard or !condGuard. In this case all the
/// instructions before the guard can be duplicated in both branches, and the
/// guard is then threaded to one of them.
bool JumpThreadingPass::ProcessGuards(BasicBlock *BB) {
  using namespace PatternMatch;

  // We only want to deal with two predecessors.
  BasicBlock *Pred1, *Pred2;
  auto PI = pred_begin(BB), PE = pred_end(BB);
  if (PI == PE)
    return false;
  Pred1 = *PI++;
  if (PI == PE)
    return false;
  Pred2 = *PI++;
  if (PI != PE)
    return false;
  if (Pred1 == Pred2)
    return false;

  // Try to thread one of the guards of the block.
  // TODO: Look up deeper than to immediate predecessor?
  auto *Parent = Pred1->getSinglePredecessor();
  if (!Parent || Parent != Pred2->getSinglePredecessor())
    return false;

  if (auto *BI = dyn_cast<BranchInst>(Parent->getTerminator()))
    for (auto &I : *BB)
      if (match(&I, m_Intrinsic<Intrinsic::experimental_guard>()))
        if (ThreadGuard(BB, cast<IntrinsicInst>(&I), BI))
          return true;

  return false;
}

/// Try to propagate the guard from BB which is the lower block of a diamond
/// to one of its branches, in case if diamond's condition implies guard's
/// condition.
bool JumpThreadingPass::ThreadGuard(BasicBlock *BB, IntrinsicInst *Guard,
                                    BranchInst *BI) {
  assert(BI->getNumSuccessors() == 2 && "Wrong number of successors?");
  assert(BI->isConditional() && "Unconditional branch has 2 successors?");
  Value *GuardCond = Guard->getArgOperand(0);
  Value *BranchCond = BI->getCondition();
  BasicBlock *TrueDest = BI->getSuccessor(0);
  BasicBlock *FalseDest = BI->getSuccessor(1);

  auto &DL = BB->getModule()->getDataLayout();
  bool TrueDestIsSafe = false;
  bool FalseDestIsSafe = false;

  // True dest is safe if BranchCond => GuardCond.
  auto Impl = isImpliedCondition(BranchCond, GuardCond, DL);
  if (Impl && *Impl)
    TrueDestIsSafe = true;
  else {
    // False dest is safe if !BranchCond => GuardCond.
    Impl = isImpliedCondition(BranchCond, GuardCond, DL, /* LHSIsTrue */ false);
    if (Impl && *Impl)
      FalseDestIsSafe = true;
  }

  if (!TrueDestIsSafe && !FalseDestIsSafe)
    return false;

  BasicBlock *PredUnguardedBlock = TrueDestIsSafe ? TrueDest : FalseDest;
  BasicBlock *PredGuardedBlock = FalseDestIsSafe ? TrueDest : FalseDest;

  ValueToValueMapTy UnguardedMapping, GuardedMapping;
  Instruction *AfterGuard = Guard->getNextNode();
  unsigned Cost = getJumpThreadDuplicationCost(BB, AfterGuard, BBDupThreshold);
  if (Cost > BBDupThreshold)
    return false;
  // Duplicate all instructions before the guard and the guard itself to the
  // branch where implication is not proved.
  BasicBlock *GuardedBlock = DuplicateInstructionsInSplitBetween(
      BB, PredGuardedBlock, AfterGuard, GuardedMapping);
  assert(GuardedBlock && "Could not create the guarded block?");
  // Duplicate all instructions before the guard in the unguarded branch.
  // Since we have successfully duplicated the guarded block and this block
  // has fewer instructions, we expect it to succeed.
  BasicBlock *UnguardedBlock = DuplicateInstructionsInSplitBetween(
      BB, PredUnguardedBlock, Guard, UnguardedMapping);
  assert(UnguardedBlock && "Could not create the unguarded block?");
  DEBUG(dbgs() << "Moved guard " << *Guard << " to block "
               << GuardedBlock->getName() << "\n");
  // DuplicateInstructionsInSplitBetween inserts a new block "BB.split" between
  // PredBB and BB. We need to perform two inserts and one delete for each of
  // the above calls to update Dominators.
  DDT->applyUpdates(
      {// Guarded block split.
       {DominatorTree::Delete, PredGuardedBlock, BB},
       {DominatorTree::Insert, PredGuardedBlock, GuardedBlock},
       {DominatorTree::Insert, GuardedBlock, BB},
       // Unguarded block split.
       {DominatorTree::Delete, PredUnguardedBlock, BB},
       {DominatorTree::Insert, PredUnguardedBlock, UnguardedBlock},
       {DominatorTree::Insert, UnguardedBlock, BB}});
  // Some instructions before the guard may still have uses. For them, we need
  // to create Phi nodes merging their copies in both guarded and unguarded
  // branches. Those instructions that have no uses can be just removed.
  SmallVector<Instruction *, 4> ToRemove;
  for (auto BI = BB->begin(); &*BI != AfterGuard; ++BI)
    if (!isa<PHINode>(&*BI))
      ToRemove.push_back(&*BI);

  Instruction *InsertionPoint = &*BB->getFirstInsertionPt();
  assert(InsertionPoint && "Empty block?");
  // Substitute with Phis & remove.
  for (auto *Inst : reverse(ToRemove)) {
    if (!Inst->use_empty()) {
      PHINode *NewPN = PHINode::Create(Inst->getType(), 2);
      NewPN->addIncoming(UnguardedMapping[Inst], UnguardedBlock);
      NewPN->addIncoming(GuardedMapping[Inst], GuardedBlock);
      NewPN->insertBefore(InsertionPoint);
      Inst->replaceAllUsesWith(NewPN);
    }
    Inst->eraseFromParent();
  }
  return true;
}


/************************************************************************************************
 *  Below implement the sink/hoist for jump threading pss
************************************************************************************************/
bool JumpThreadingSinkHoist::shouldMoveInst(Instruction *I) {
  if (isa<PHINode>(I))
    return false;

  if (isa<DbgInfoIntrinsic>(I))
    return false;

  if (isa<AllocaInst>(I))
    return false;

  if (isa<TerminatorInst>(I))
    return false;

  return true;
}

void JumpThreadingSinkHoist::collectForwardReachableBlocksSlowCase(
    BasicBlock *BB, BasicBlock *PostDomBB,
    SmallPtrSetImpl<BasicBlock *> &Blocks) {
  assert(PDT.properlyDominates(PostDomBB, BB) &&
         "PostDomBB should post dominate BB");

  for (BasicBlock *Succ : successors(BB)) {
    if (Succ == PostDomBB)
      continue;

    assert(PDT.properlyDominates(PostDomBB, Succ) &&
           "PostDomBB should post dominate Succ");
    if (Blocks.insert(Succ).second)
      collectForwardReachableBlocksSlowCase(Succ, PostDomBB, Blocks);
  }

  Blocks.insert(PostDomBB);
}

void JumpThreadingSinkHoist::collectForwardReachableBlocks(
    BasicBlock *BB, BasicBlock *PostDomBB,
    SmallPtrSetImpl<BasicBlock *> &Blocks) {
  assert(PDT.properlyDominates(PostDomBB, BB) &&
         "PostDomBB should post dominate BB");

  BlockCacheKeyTy Key = std::make_pair(BB, PostDomBB);
  if (ForwardBlockCache.count(Key)) {
    Blocks.insert(ForwardBlockCache[Key].begin(), ForwardBlockCache[Key].end());
  } else {
    DomTreeNode *PDomNode = PDT.getNode(BB);
    assert(PDomNode && "Post Dominator node not found??");
    DomTreeNode *IDomNode = PDomNode->getIDom();
    assert(IDomNode && IDomNode->getBlock() &&
           "IDomNode should not be nullptr");

    if (IDomNode->getBlock() == PostDomBB) {
      SmallPtrSet<BasicBlock *, 8> ReachableBlocks;
      collectForwardReachableBlocksSlowCase(BB, PostDomBB, ReachableBlocks);
      ForwardBlockCache[Key] = ReachableBlocks;
      Blocks.insert(ReachableBlocks.begin(), ReachableBlocks.end());
    } else {

      while (true) {
        BasicBlock *NewPostDomBB = IDomNode->getBlock();
        collectForwardReachableBlocks(BB, NewPostDomBB, Blocks);
        if (NewPostDomBB == PostDomBB)
          break;

        IDomNode = IDomNode->getIDom();
        assert(IDomNode && IDomNode->getBlock() &&
               "IDomNode should not be nullptr");
        BB = NewPostDomBB;
      }
    }
  }
}

void JumpThreadingSinkHoist::collectBackwardReachableBlocksSlowCase(
    BasicBlock *BB, BasicBlock *DomBB, SmallPtrSetImpl<BasicBlock *> &Blocks) {
  assert(DT.properlyDominates(DomBB, BB) && "DomBB should dominate BB");

  for (BasicBlock *Pred : predecessors(BB)) {
    if (Pred == DomBB)
      continue;

    assert(DT.properlyDominates(DomBB, Pred) && "DomBB should dominate Pred");
    if (Blocks.insert(Pred).second)
      collectBackwardReachableBlocksSlowCase(Pred, DomBB, Blocks);
  }

  Blocks.insert(DomBB);
}

void JumpThreadingSinkHoist::collectBackwardReachableBlocks(
    BasicBlock *BB, BasicBlock *DomBB, SmallPtrSetImpl<BasicBlock *> &Blocks) {
  assert(DT.properlyDominates(DomBB, BB) && "DomBB should dominate BB");

  BlockCacheKeyTy Key = std::make_pair(BB, DomBB);
  if (BackwardBlockCache.count(Key)) {
    Blocks.insert(BackwardBlockCache[Key].begin(),
                  BackwardBlockCache[Key].end());
  } else {
    DomTreeNode *DomNode = DT.getNode(BB);
    assert(DomNode && "Dominator node not found??");
    DomTreeNode *IDomNode = DomNode->getIDom();
    assert(IDomNode && IDomNode->getBlock() &&
           "IDomNode should not be nullptr");

    if (IDomNode->getBlock() == DomBB) {
      SmallPtrSet<BasicBlock *, 8> ReachableBlocks;
      collectBackwardReachableBlocksSlowCase(BB, DomBB, ReachableBlocks);
      BackwardBlockCache[Key] = ReachableBlocks;
      Blocks.insert(ReachableBlocks.begin(), ReachableBlocks.end());
    } else {

      while (true) {
        BasicBlock *NewDomBB = IDomNode->getBlock();
        collectBackwardReachableBlocks(BB, NewDomBB, Blocks);
        if (NewDomBB == DomBB)
          break;

        IDomNode = IDomNode->getIDom();
        assert(IDomNode && IDomNode->getBlock() &&
               "IDomNode should not be nullptr");
        BB = NewDomBB;
      }
    }
  }
}

// check if the dependence allows to move instruction from \p MovedFrom to \p
// MovedTo MovedTo must post dominate MovedFrom
bool JumpThreadingSinkHoist::canSinkInst(Instruction *I, BasicBlock *MovedFrom,
                                         BasicBlock *MovedTo) {
  assert(PDT.properlyDominates(MovedTo, MovedFrom) &&
         "MovedTo should post dominate MovedFrom");

  SmallPtrSet<BasicBlock *, 8> ReachableBlocks;
  collectForwardReachableBlocks(MovedFrom, MovedTo, ReachableBlocks);
  ReachableBlocks.erase(MovedTo);

  // 1. all the user should not in the path between MovedFrom to MovedTo
  for (User *U : I->users()) {
    if (Instruction *UI = dyn_cast<Instruction>(U)) {
      if (ReachableBlocks.count(UI->getParent()))
        return false;

      if (UI->getParent() == I->getParent())
        if (!MovedInsts.count(UI))
          return false;

      if (UI->getParent() == MovedTo)
        if (isa<PHINode>(UI))
          return false;
    }
  }

  if (!I->mayReadOrWriteMemory())
    return true;

  // 2. there is no memory dependence along this path
  DenseMap<Instruction *, unsigned> DFSNumber;
  unsigned InstLoc = 0;
  for (Instruction &I : *MovedFrom) {
    DFSNumber[&I] = InstLoc++;
  }

  assert(DFSNumber.count(I) && "Expect instruction inside map");
  Instruction *ScanPos = MovedFrom->getTerminator();
  while (true) {

    MemDepResult Res = getDependency(I, ScanPos, MDR, AA, TLI);
    if (Res.isClobber() || Res.isDef()) {
      Instruction *DepInst = Res.getInst();
      assert(DepInst && "Dependended on instruction should not be nullptr");
      if (MovedInsts.count(DepInst)) {
        ScanPos = DepInst;
        continue;
      }

      assert(DFSNumber.count(I) && DFSNumber.count(DepInst) &&
             "Expect instruction inside map");

      if (DFSNumber[I] >= DFSNumber[DepInst])
        break;
    }

    if (!Res.isNonLocal() && !Res.isNonFuncLocal())
      return false;

    break;
  }

  for (BasicBlock *BB : ReachableBlocks) {
    MemDepResult Res = getDependency(I, BB->getTerminator(), MDR, AA, TLI);
    if (!Res.isNonLocal() && !Res.isNonFuncLocal())
      return false;
  }

  return true;
}

bool JumpThreadingSinkHoist::canHoistInst(Instruction *I, BasicBlock *MovedFrom,
                                          BasicBlock *MovedTo) {

  assert(DT.properlyDominates(MovedTo, MovedFrom) &&
         "MovedTo should dominate MovedFrom");

  // 1. for instructions that may have sideeffects, we move it only if MovedFrom
  // post dominate MovedTo
  if (I->mayHaveSideEffects())
    if (!PDT.properlyDominates(MovedFrom, MovedTo))
      return false;

  SmallPtrSet<BasicBlock *, 8> ReachableBlocks;
  collectBackwardReachableBlocks(MovedFrom, MovedTo, ReachableBlocks);
  ReachableBlocks.erase(MovedTo);

  // 2. all the operand should not in the path between MovedFrom to MovedTo
  for (unsigned i = 0; i < I->getNumOperands(); ++i) {
    Value *Op = I->getOperand(i);
    if (Instruction *OpI = dyn_cast<Instruction>(Op)) {
      if (ReachableBlocks.count(OpI->getParent()))
        return false;

      if (OpI->getParent() == I->getParent())
        if (!MovedInsts.count(OpI))
          return false;
    }
  }

  if (!I->mayReadOrWriteMemory())
    return true;

  // 3. there is no memory dependence along this path
  Instruction *ScanPos = I;
  while (true) {
    MemDepResult Res = getDependency(I, ScanPos, MDR, AA, TLI);
    if (Res.isClobber() || Res.isDef()) {
      Instruction *DepInst = Res.getInst();
      assert(DepInst && "Dependended on instruction should not be nullptr");
      if (MovedInsts.count(DepInst)) {
        ScanPos = DepInst;
        continue;
      }
    }

    if (!Res.isNonLocal() && !Res.isNonFuncLocal())
      return false;

    break;
  }

  for (BasicBlock *BB : ReachableBlocks) {
    MemDepResult Res = getDependency(I, BB->getTerminator(), MDR, AA, TLI);
    if (!Res.isNonLocal() && !Res.isNonFuncLocal())
      return false;
  }

  return true;
}

BasicBlock *JumpThreadingSinkHoist::getSinkMoveBlock(BasicBlock *BB) {
  DomTreeNode *PDomNode = PDT.getNode(BB);
  assert(PDomNode && "Post Dominator node not found??");

  BasicBlock *MovedTo = nullptr;
  while (true) {
    PDomNode = PDomNode->getIDom();
    if (!PDomNode || !PDomNode->getBlock())
      break;

    BasicBlock *PDomBlock = PDomNode->getBlock();
    if (DT.properlyDominates(BB, PDomBlock)) {
      /// move should not accorss loop
      if (LI.getLoopFor(PDomBlock) != LI.getLoopFor(BB))
        continue;

      MovedTo = PDomNode->getBlock();
      break;
    }
  }

  return MovedTo;
}

BasicBlock *JumpThreadingSinkHoist::getHoistMoveBlock(BasicBlock *BB) {
  DomTreeNode *DomNode = DT.getNode(BB);
  assert(DomNode && "Dominator node not found??");

  BasicBlock *MovedTo = nullptr;
  while (true) {
    DomNode = DomNode->getIDom();
    if (!DomNode || !DomNode->getBlock())
      break;

    BasicBlock *DomBlock = DomNode->getBlock();
    if (PDT.properlyDominates(BB, DomBlock)) {
      /// move should not accorss loop
      if (LI.getLoopFor(DomBlock) != LI.getLoopFor(BB))
        continue;

      MovedTo = DomNode->getBlock();
      break;
    }
  }

  return MovedTo;
}

/// @brief try move instruction down to the immediate post dominator if dependence allows
/// @param BB the instructions that should do move 
void JumpThreadingSinkHoist::trySinkInstInBlock(BasicBlock *BB, BasicBlock *MovedTo) {
  if (!MovedTo)
    return;

  assert(DT.properlyDominates(BB, MovedTo) &&
         PDT.properlyDominates(MovedTo, BB) &&
         "Dominator relationship not satisfied??");

  for (BasicBlock::reverse_iterator it = BB->rbegin(), ie = BB->rend();
       it != ie; ++it) {
    Instruction *Inst = &*it;
    if (!shouldMoveInst(Inst))
      continue;

    if (MovedInsts.count(Inst))
      continue;

    if (!canSinkInst(Inst, BB, MovedTo))
      continue;

    SinkInsts.push_back(Inst);
    MovedInsts.insert(Inst);
  }
}

void JumpThreadingSinkHoist::tryHoistInstInBlock(BasicBlock *BB, BasicBlock *MovedTo) {
  if (!MovedTo)
    return;

  assert(DT.properlyDominates(MovedTo, BB) &&
         PDT.properlyDominates(BB, MovedTo) &&
         "Dominator relationship not satisfied??");

  for (BasicBlock::iterator it = BB->begin(), ie = BB->end(); it != ie; ++it) {
    Instruction *Inst = &*it;
    if (!shouldMoveInst(Inst))
      continue;

    if (MovedInsts.count(Inst))
      continue;

    if (!canHoistInst(Inst, BB, MovedTo))
      continue;

    HoistInsts.push_back(Inst);
    MovedInsts.insert(Inst);
  }
}

bool JumpThreadingSinkHoist::doSinkHoistOnBlock(BasicBlock *BB) {
  if (!VisitedBlocks.insert(BB).second)
    return false;

  DT.releaseMemory();
  PDT.releaseMemory();
  LI.releaseMemory();
  DT.recalculate(*BB->getParent());
  PDT.recalculate(*BB->getParent());
  LI.analyze(DT);

  DEBUG(dbgs() << "Do sink hoist on block: \n" << *BB << "\n");
  SinkInsts.clear();
  HoistInsts.clear();
  MovedInsts.clear();

  BasicBlock *HoistMovedTo = getHoistMoveBlock(BB);
  BasicBlock *SinkMovedTo = getSinkMoveBlock(BB);
  tryHoistInstInBlock(BB, HoistMovedTo);
  trySinkInstInBlock(BB, SinkMovedTo);

  unsigned JumpThreadCost = getJumpThreadDuplicationCost(
      BB, BB->getTerminator(), BBDupThreshold, &MovedInsts);
  if (JumpThreadCost > BBDupThreshold) {
    DEBUG(dbgs() << "Sink hoist on block can not enable more jump threading\n");
    return false;
  }

  if (!SinkInsts.empty()) {
    assert(SinkMovedTo && "MovedTo should not be nullptr");

    DEBUG(dbgs() << "Sink instruction to block: \n" << *SinkMovedTo << "\n");

    for (Instruction *I : SinkInsts) {
      DEBUG(dbgs() << "Sink instruction: " << *I << "\n");
      I->moveBefore(SinkMovedTo->getFirstNonPHI());
    }
  }

  if (!HoistInsts.empty()) {
    assert(HoistMovedTo && "MovedTo should not be nullptr");

    DEBUG(dbgs() << "Hoist instruction to block: \n" << *HoistMovedTo << "\n");
    for (Instruction *I : HoistInsts) {
      DEBUG(dbgs() << "Hoist instruction: " << *I << "\n");
      I->moveBefore(HoistMovedTo->getTerminator());
    }
  }

  return true;
}

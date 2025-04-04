//===- JumpThreading.h - thread control through conditional BBs -*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
/// \file
/// See the comments on JumpThreadingPass.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SCALAR_JUMPTHREADING_H
#define LLVM_TRANSFORMS_SCALAR_JUMPTHREADING_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Analysis/BranchProbabilityInfo.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/IR/ValueHandle.h"
#include <memory>
#include <utility>

namespace llvm {

class BasicBlock;
class BinaryOperator;
class BranchInst;
class CmpInst;
class Constant;
class DeferredDominance;
class Function;
class Instruction;
class IntrinsicInst;
class LazyValueInfo;
class LoadInst;
class PHINode;
class TargetLibraryInfo;
class Value;
class MemoryDependenceResults;

class JumpThreadingSinkHoist {
public:
  using BlockCacheKeyTy = std::pair<BasicBlock *, BasicBlock *>;
  using BlockCacheTy = DenseMap<BlockCacheKeyTy, SmallPtrSet<BasicBlock *, 8>>;

private:
  LoopInfo LI;
  DominatorTree DT;
  PostDominatorTree PDT;
  MemoryDependenceResults *MDR;
  AliasAnalysis *AA;
  TargetLibraryInfo *TLI;
  unsigned BBDupThreshold;
  SmallVector<Instruction *, 8> HoistInsts;
  SmallVector<Instruction *, 8> SinkInsts;
  SmallPtrSet<Instruction *, 8> MovedInsts;
  SmallPtrSet<BasicBlock *, 8> VisitedBlocks;
  BlockCacheTy ForwardBlockCache;
  BlockCacheTy BackwardBlockCache;

public:
  JumpThreadingSinkHoist(MemoryDependenceResults *MDR, AliasAnalysis *AA,
                         TargetLibraryInfo *TLI, unsigned BBDupThreshold)
      : MDR(MDR), AA(AA), TLI(TLI), BBDupThreshold(BBDupThreshold) {}

  bool doSinkHoistOnBlock(BasicBlock *BB);

private:
  void tryHoistInstInBlock(BasicBlock *BB, BasicBlock *MovedTo);
  void trySinkInstInBlock(BasicBlock *BB, BasicBlock *MovedTo);
  bool shouldMoveInst(Instruction *I);

  BasicBlock *getSinkMoveBlock(BasicBlock *BB);
  BasicBlock *getHoistMoveBlock(BasicBlock *BB);
  bool canSinkInst(Instruction *I, BasicBlock *MovedFrom, BasicBlock *MovedTo);
  bool canHoistInst(Instruction *I, BasicBlock *MovedFrom, BasicBlock *MovedTo);

  void collectForwardReachableBlocks(BasicBlock *BB, BasicBlock *PostDomBB,
                                     SmallPtrSetImpl<BasicBlock *> &Blocks);
  void
  collectForwardReachableBlocksSlowCase(BasicBlock *BB, BasicBlock *PostDomBB,
                                        SmallPtrSetImpl<BasicBlock *> &Blocks);
  void collectBackwardReachableBlocks(BasicBlock *BB, BasicBlock *DomBB,
                                      SmallPtrSetImpl<BasicBlock *> &Blocks);
  void
  collectBackwardReachableBlocksSlowCase(BasicBlock *BB, BasicBlock *DomBB,
                                         SmallPtrSetImpl<BasicBlock *> &Blocks);
};

/// A private "module" namespace for types and utilities used by
/// JumpThreading.
/// These are implementation details and should not be used by clients.
namespace jumpthreading {

// These are at global scope so static functions can use them too.
using PredValueInfo = SmallVectorImpl<std::pair<Constant *, BasicBlock *>>;
using PredValueInfoTy = SmallVector<std::pair<Constant *, BasicBlock *>, 8>;

// This is used to keep track of what kind of constant we're currently hoping
// to find.
enum ConstantPreference { WantInteger, WantBlockAddress };

} // end namespace jumpthreading

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
class JumpThreadingPass : public PassInfoMixin<JumpThreadingPass> {
  TargetLibraryInfo *TLI;
  LazyValueInfo *LVI;
  AliasAnalysis *AA;
  DeferredDominance *DDT;
  std::unique_ptr<BlockFrequencyInfo> BFI;
  std::unique_ptr<BranchProbabilityInfo> BPI;
  std::unique_ptr<JumpThreadingSinkHoist> JTSinkHoist;
  bool HasProfileData = false;
  bool HasGuards = false;
#ifdef NDEBUG
  SmallPtrSet<const BasicBlock *, 16> LoopHeaders;
#else
  SmallSet<AssertingVH<const BasicBlock>, 16> LoopHeaders;
#endif
  DenseSet<std::pair<Value *, BasicBlock *>> RecursionSet;

  unsigned BBDupThreshold;
  bool EnableSinkHoist;

  // RAII helper for updating the recursion stack.
  struct RecursionSetRemover {
    DenseSet<std::pair<Value *, BasicBlock *>> &TheSet;
    std::pair<Value *, BasicBlock *> ThePair;

    RecursionSetRemover(DenseSet<std::pair<Value *, BasicBlock *>> &S,
                        std::pair<Value *, BasicBlock *> P)
        : TheSet(S), ThePair(P) {}

    ~RecursionSetRemover() { TheSet.erase(ThePair); }
  };

public:
  JumpThreadingPass(int T = -1, bool enableSinkHoist = false);

  // Glue for old PM.
  bool runImpl(Function &F, TargetLibraryInfo *TLI_, LazyValueInfo *LVI_,
               AliasAnalysis *AA_, DeferredDominance *DDT_,
               bool HasProfileData_, std::unique_ptr<BlockFrequencyInfo> BFI_,
               std::unique_ptr<BranchProbabilityInfo> BPI_,
               MemoryDependenceResults *MDR_);

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);

  void releaseMemory() {
    BFI.reset();
    BPI.reset();
  }

  void FindLoopHeaders(Function &F);
  bool ProcessBlock(BasicBlock *BB);
  bool ThreadEdge(BasicBlock *BB, const SmallVectorImpl<BasicBlock *> &PredBBs,
                  BasicBlock *SuccBB);
  bool DuplicateCondBranchOnPHIIntoPred(
      BasicBlock *BB, const SmallVectorImpl<BasicBlock *> &PredBBs);

  bool
  ComputeValueKnownInPredecessors(Value *V, BasicBlock *BB,
                                  jumpthreading::PredValueInfo &Result,
                                  jumpthreading::ConstantPreference Preference,
                                  Instruction *CxtI = nullptr);
  bool ProcessThreadableEdges(Value *Cond, BasicBlock *BB,
                              jumpthreading::ConstantPreference Preference,
                              Instruction *CxtI = nullptr);

  bool ProcessBranchOnPHI(PHINode *PN);
  bool ProcessBranchOnXOR(BinaryOperator *BO);
  bool ProcessImpliedCondition(BasicBlock *BB);

  bool SimplifyPartiallyRedundantLoad(LoadInst *LI);
  bool TryToUnfoldSelect(CmpInst *CondCmp, BasicBlock *BB);
  bool TryToUnfoldSelectInCurrBB(BasicBlock *BB);

  bool ProcessGuards(BasicBlock *BB);
  bool ThreadGuard(BasicBlock *BB, IntrinsicInst *Guard, BranchInst *BI);

private:
  BasicBlock *SplitBlockPreds(BasicBlock *BB, ArrayRef<BasicBlock *> Preds,
                              const char *Suffix);
  void UpdateBlockFreqAndEdgeWeight(BasicBlock *PredBB, BasicBlock *BB,
                                    BasicBlock *NewBB, BasicBlock *SuccBB);
  /// Check if the block has profile metadata for its outgoing edges.
  bool doesBlockHaveProfileData(BasicBlock *BB);
};

} // end namespace llvm

#endif // LLVM_TRANSFORMS_SCALAR_JUMPTHREADING_H

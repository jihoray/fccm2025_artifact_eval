//===- Dominators.cpp - Dominator Calculation -----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
// And has the following additional copyright:
//
// Copyright (C) 2023, Advanced Micro Devices, Inc.
// All Rights Reserved.
//
//===----------------------------------------------------------------------===//
//
// This file implements simple dominator construction algorithms for finding
// forward dominators.  Postdominators are available in libanalysis, but are not
// included in libvmcore, because it's not needed.  Forward dominators are
// needed to support the Verifier pass.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/Dominators.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/GenericDomTreeConstruction.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
using namespace llvm;

// Always verify dominfo if expensive checking is enabled.
#ifdef EXPENSIVE_CHECKS
bool llvm::VerifyDomInfo = true;
#else
bool llvm::VerifyDomInfo = false;
#endif
static cl::opt<bool, true>
    VerifyDomInfoX("verify-dom-info", cl::location(VerifyDomInfo), cl::Hidden,
                   cl::desc("Verify dominator info (time consuming)"));

bool BasicBlockEdge::isSingleEdge() const {
  const TerminatorInst *TI = Start->getTerminator();
  unsigned NumEdgesToEnd = 0;
  for (unsigned int i = 0, n = TI->getNumSuccessors(); i < n; ++i) {
    if (TI->getSuccessor(i) == End)
      ++NumEdgesToEnd;
    if (NumEdgesToEnd >= 2)
      return false;
  }
  assert(NumEdgesToEnd == 1);
  return true;
}

//===----------------------------------------------------------------------===//
//  DominatorTree Implementation
//===----------------------------------------------------------------------===//
//
// Provide public access to DominatorTree information.  Implementation details
// can be found in Dominators.h, GenericDomTree.h, and
// GenericDomTreeConstruction.h.
//
//===----------------------------------------------------------------------===//

template class llvm::DomTreeNodeBase<BasicBlock>;
template class llvm::DominatorTreeBase<BasicBlock, false>; // DomTreeBase
template class llvm::DominatorTreeBase<BasicBlock, true>; // PostDomTreeBase

template struct llvm::DomTreeBuilder::Update<BasicBlock *>;

template void llvm::DomTreeBuilder::Calculate<DomTreeBuilder::BBDomTree>(
    DomTreeBuilder::BBDomTree &DT);
template void llvm::DomTreeBuilder::Calculate<DomTreeBuilder::BBPostDomTree>(
    DomTreeBuilder::BBPostDomTree &DT);

template void llvm::DomTreeBuilder::InsertEdge<DomTreeBuilder::BBDomTree>(
    DomTreeBuilder::BBDomTree &DT, BasicBlock *From, BasicBlock *To);
template void llvm::DomTreeBuilder::InsertEdge<DomTreeBuilder::BBPostDomTree>(
    DomTreeBuilder::BBPostDomTree &DT, BasicBlock *From, BasicBlock *To);

template void llvm::DomTreeBuilder::DeleteEdge<DomTreeBuilder::BBDomTree>(
    DomTreeBuilder::BBDomTree &DT, BasicBlock *From, BasicBlock *To);
template void llvm::DomTreeBuilder::DeleteEdge<DomTreeBuilder::BBPostDomTree>(
    DomTreeBuilder::BBPostDomTree &DT, BasicBlock *From, BasicBlock *To);

template void llvm::DomTreeBuilder::ApplyUpdates<DomTreeBuilder::BBDomTree>(
    DomTreeBuilder::BBDomTree &DT, DomTreeBuilder::BBUpdates);
template void llvm::DomTreeBuilder::ApplyUpdates<DomTreeBuilder::BBPostDomTree>(
    DomTreeBuilder::BBPostDomTree &DT, DomTreeBuilder::BBUpdates);

template bool llvm::DomTreeBuilder::Verify<DomTreeBuilder::BBDomTree>(
    const DomTreeBuilder::BBDomTree &DT);
template bool llvm::DomTreeBuilder::Verify<DomTreeBuilder::BBPostDomTree>(
    const DomTreeBuilder::BBPostDomTree &DT);

bool DominatorTree::invalidate(Function &F, const PreservedAnalyses &PA,
                               FunctionAnalysisManager::Invalidator &) {
  // Check whether the analysis, all analyses on functions, or the function's
  // CFG have been preserved.
  auto PAC = PA.getChecker<DominatorTreeAnalysis>();
  return !(PAC.preserved() || PAC.preservedSet<AllAnalysesOn<Function>>() ||
           PAC.preservedSet<CFGAnalyses>());
}

// dominates - Return true if Def dominates a use in User. This performs
// the special checks necessary if Def and User are in the same basic block.
// Note that Def doesn't dominate a use in Def itself!
bool DominatorTree::dominates(const Instruction *Def,
                              const Instruction *User) const {
  const BasicBlock *UseBB = User->getParent();
  const BasicBlock *DefBB = Def->getParent();

  // Any unreachable use is dominated, even if Def == User.
  if (!isReachableFromEntry(UseBB))
    return true;

  // Unreachable definitions don't dominate anything.
  if (!isReachableFromEntry(DefBB))
    return false;

  // An instruction doesn't dominate a use in itself.
  if (Def == User)
    return false;

  // The value defined by an invoke dominates an instruction only if it
  // dominates every instruction in UseBB.
  // A PHI is dominated only if the instruction dominates every possible use in
  // the UseBB.
  if (isa<InvokeInst>(Def) || isa<PHINode>(User))
    return dominates(Def, UseBB);

  if (DefBB != UseBB)
    return dominates(DefBB, UseBB);

  return Def->comesBefore(User);
}

// true if Def would dominate a use in any instruction in UseBB.
// note that dominates(Def, Def->getParent()) is false.
bool DominatorTree::dominates(const Instruction *Def,
                              const BasicBlock *UseBB) const {
  const BasicBlock *DefBB = Def->getParent();

  // Any unreachable use is dominated, even if DefBB == UseBB.
  if (!isReachableFromEntry(UseBB))
    return true;

  // Unreachable definitions don't dominate anything.
  if (!isReachableFromEntry(DefBB))
    return false;

  if (DefBB == UseBB)
    return false;

  // Invoke results are only usable in the normal destination, not in the
  // exceptional destination.
  if (const auto *II = dyn_cast<InvokeInst>(Def)) {
    BasicBlock *NormalDest = II->getNormalDest();
    BasicBlockEdge E(DefBB, NormalDest);
    return dominates(E, UseBB);
  }

  return dominates(DefBB, UseBB);
}

bool DominatorTree::dominates(const BasicBlockEdge &BBE,
                              const BasicBlock *UseBB) const {
  // If the BB the edge ends in doesn't dominate the use BB, then the
  // edge also doesn't.
  const BasicBlock *Start = BBE.getStart();
  const BasicBlock *End = BBE.getEnd();
  if (!dominates(End, UseBB))
    return false;

  // Simple case: if the end BB has a single predecessor, the fact that it
  // dominates the use block implies that the edge also does.
  if (End->getSinglePredecessor())
    return true;

  // The normal edge from the invoke is critical. Conceptually, what we would
  // like to do is split it and check if the new block dominates the use.
  // With X being the new block, the graph would look like:
  //
  //        DefBB
  //          /\      .  .
  //         /  \     .  .
  //        /    \    .  .
  //       /      \   |  |
  //      A        X  B  C
  //      |         \ | /
  //      .          \|/
  //      .      NormalDest
  //      .
  //
  // Given the definition of dominance, NormalDest is dominated by X iff X
  // dominates all of NormalDest's predecessors (X, B, C in the example). X
  // trivially dominates itself, so we only have to find if it dominates the
  // other predecessors. Since the only way out of X is via NormalDest, X can
  // only properly dominate a node if NormalDest dominates that node too.
  int IsDuplicateEdge = 0;
  for (const_pred_iterator PI = pred_begin(End), E = pred_end(End);
       PI != E; ++PI) {
    const BasicBlock *BB = *PI;
    if (BB == Start) {
      // If there are multiple edges between Start and End, by definition they
      // can't dominate anything.
      if (IsDuplicateEdge++)
        return false;
      continue;
    }

    if (!dominates(End, BB))
      return false;
  }
  return true;
}

bool DominatorTree::dominates(const BasicBlockEdge &BBE, const Use &U) const {
  Instruction *UserInst = cast<Instruction>(U.getUser());
  // A PHI in the end of the edge is dominated by it.
  PHINode *PN = dyn_cast<PHINode>(UserInst);
  if (PN && PN->getParent() == BBE.getEnd() &&
      PN->getIncomingBlock(U) == BBE.getStart())
    return true;

  // Otherwise use the edge-dominates-block query, which
  // handles the crazy critical edge cases properly.
  const BasicBlock *UseBB;
  if (PN)
    UseBB = PN->getIncomingBlock(U);
  else
    UseBB = UserInst->getParent();
  return dominates(BBE, UseBB);
}

bool DominatorTree::dominates(const Instruction *Def, const Use &U) const {
  Instruction *UserInst = cast<Instruction>(U.getUser());
  const BasicBlock *DefBB = Def->getParent();

  // Determine the block in which the use happens. PHI nodes use
  // their operands on edges; simulate this by thinking of the use
  // happening at the end of the predecessor block.
  const BasicBlock *UseBB;
  if (PHINode *PN = dyn_cast<PHINode>(UserInst))
    UseBB = PN->getIncomingBlock(U);
  else
    UseBB = UserInst->getParent();

  // Any unreachable use is dominated, even if Def == User.
  if (!isReachableFromEntry(UseBB))
    return true;

  // Unreachable definitions don't dominate anything.
  if (!isReachableFromEntry(DefBB))
    return false;

  // Invoke instructions define their return values on the edges to their normal
  // successors, so we have to handle them specially.
  // Among other things, this means they don't dominate anything in
  // their own block, except possibly a phi, so we don't need to
  // walk the block in any case.
  if (const InvokeInst *II = dyn_cast<InvokeInst>(Def)) {
    BasicBlock *NormalDest = II->getNormalDest();
    BasicBlockEdge E(DefBB, NormalDest);
    return dominates(E, U);
  }

  // If the def and use are in different blocks, do a simple CFG dominator
  // tree query.
  if (DefBB != UseBB)
    return dominates(DefBB, UseBB);

  // Ok, def and use are in the same block. If the def is an invoke, it
  // doesn't dominate anything in the block. If it's a PHI, it dominates
  // everything in the block.
  if (isa<PHINode>(UserInst))
    return true;

  return Def->comesBefore(UserInst);
}

bool DominatorTree::isReachableFromEntry(const Use &U) const {
  Instruction *I = dyn_cast<Instruction>(U.getUser());

  // ConstantExprs aren't really reachable from the entry block, but they
  // don't need to be treated like unreachable code either.
  if (!I) return true;

  // PHI nodes use their operands on their incoming edges.
  if (PHINode *PN = dyn_cast<PHINode>(I))
    return isReachableFromEntry(PN->getIncomingBlock(U));

  // Everything else uses their operands in their own block.
  return isReachableFromEntry(I->getParent());
}

void DominatorTree::verifyDomTree() const {
  // Perform the expensive checks only when VerifyDomInfo is set.
  if (VerifyDomInfo && !verify()) {
    errs() << "\n~~~~~~~~~~~\n\t\tDomTree verification failed!\n~~~~~~~~~~~\n";
    print(errs());
    abort();
  }

  Function &F = *getRoot()->getParent();

  DominatorTree OtherDT;
  OtherDT.recalculate(F);
  if (compare(OtherDT)) {
    errs() << "DominatorTree for function " << F.getName()
           << " is not up to date!\nComputed:\n";
    print(errs());
    errs() << "\nActual:\n";
    OtherDT.print(errs());
    errs() << "\nCFG:\n";
    F.print(errs());
    errs().flush();
    abort();
  }
}

//===----------------------------------------------------------------------===//
//  DominatorTreeAnalysis and related pass implementations
//===----------------------------------------------------------------------===//
//
// This implements the DominatorTreeAnalysis which is used with the new pass
// manager. It also implements some methods from utility passes.
//
//===----------------------------------------------------------------------===//

DominatorTree DominatorTreeAnalysis::run(Function &F,
                                         FunctionAnalysisManager &) {
  DominatorTree DT;
  DT.recalculate(F);
  return DT;
}

AnalysisKey DominatorTreeAnalysis::Key;

DominatorTreePrinterPass::DominatorTreePrinterPass(raw_ostream &OS) : OS(OS) {}

PreservedAnalyses DominatorTreePrinterPass::run(Function &F,
                                                FunctionAnalysisManager &AM) {
  OS << "DominatorTree for function: " << F.getName() << "\n";
  AM.getResult<DominatorTreeAnalysis>(F).print(OS);

  return PreservedAnalyses::all();
}

PreservedAnalyses DominatorTreeVerifierPass::run(Function &F,
                                                 FunctionAnalysisManager &AM) {
  AM.getResult<DominatorTreeAnalysis>(F).verifyDomTree();

  return PreservedAnalyses::all();
}

//===----------------------------------------------------------------------===//
//  DominatorTreeWrapperPass Implementation
//===----------------------------------------------------------------------===//
//
// The implementation details of the wrapper pass that holds a DominatorTree
// suitable for use with the legacy pass manager.
//
//===----------------------------------------------------------------------===//

char DominatorTreeWrapperPass::ID = 0;
INITIALIZE_PASS(DominatorTreeWrapperPass, "domtree",
                "Dominator Tree Construction", true, true)

bool DominatorTreeWrapperPass::runOnFunction(Function &F) {
  DT.recalculate(F);
  return false;
}

void DominatorTreeWrapperPass::verifyAnalysis() const {
    if (VerifyDomInfo)
      DT.verifyDomTree();
}

void DominatorTreeWrapperPass::print(raw_ostream &OS, const Module *) const {
  DT.print(OS);
}

//===----------------------------------------------------------------------===//
//  DeferredDominance Implementation
//===----------------------------------------------------------------------===//
//
// The implementation details of the DeferredDominance class which allows
// one to queue updates to a DominatorTree.
//
//===----------------------------------------------------------------------===//

/// \brief Queues multiple updates and discards duplicates.
void DeferredDominance::applyUpdates(
    ArrayRef<DominatorTree::UpdateType> Updates) {
  SmallVector<DominatorTree::UpdateType, 8> Seen;
  for (auto U : Updates)
    // Avoid duplicates to applyUpdate() to save on analysis.
    if (std::none_of(Seen.begin(), Seen.end(),
                     [U](DominatorTree::UpdateType S) { return S == U; })) {
      Seen.push_back(U);
      applyUpdate(U.getKind(), U.getFrom(), U.getTo());
    }
}

/// \brief Helper method for a single edge insertion. It's almost always better
/// to batch updates and call applyUpdates to quickly remove duplicate edges.
/// This is best used when there is only a single insertion needed to update
/// Dominators.
void DeferredDominance::insertEdge(BasicBlock *From, BasicBlock *To) {
  applyUpdate(DominatorTree::Insert, From, To);
}

/// \brief Helper method for a single edge deletion. It's almost always better
/// to batch updates and call applyUpdates to quickly remove duplicate edges.
/// This is best used when there is only a single deletion needed to update
/// Dominators.
void DeferredDominance::deleteEdge(BasicBlock *From, BasicBlock *To) {
  applyUpdate(DominatorTree::Delete, From, To);
}

/// \brief Delays the deletion of a basic block until a flush() event.
void DeferredDominance::deleteBB(BasicBlock *DelBB) {
  assert(DelBB && "Invalid push_back of nullptr DelBB.");
  assert(pred_empty(DelBB) && "DelBB has one or more predecessors.");
  // DelBB is unreachable and all its instructions are dead.
  while (!DelBB->empty()) {
    Instruction &I = DelBB->back();
    // Replace used instructions with an arbitrary value (undef).
    if (!I.use_empty())
      I.replaceAllUsesWith(llvm::UndefValue::get(I.getType()));
    DelBB->getInstList().pop_back();
  }
  // Make sure DelBB has a valid terminator instruction. As long as DelBB is a
  // Child of Function F it must contain valid IR.
  new UnreachableInst(DelBB->getContext(), DelBB);
  DeletedBBs.insert(DelBB);
}

/// \brief Returns true if DelBB is awaiting deletion at a flush() event.
bool DeferredDominance::pendingDeletedBB(BasicBlock *DelBB) {
  if (DeletedBBs.empty())
    return false;
  return DeletedBBs.count(DelBB) != 0;
}

/// \brief Flushes all pending updates and block deletions. Returns a
/// correct DominatorTree reference to be used by the caller for analysis.
DominatorTree &DeferredDominance::flush() {
  // Updates to DT must happen before blocks are deleted below. Otherwise the
  // DT traversal will encounter badref blocks and assert.
  if (!PendUpdates.empty()) {
    DT.applyUpdates(PendUpdates);
    PendUpdates.clear();
  }
  flushDelBB();
  return DT;
}

/// \brief Drops all internal state and forces a (slow) recalculation of the
/// DominatorTree based on the current state of the LLVM IR in F. This should
/// only be used in corner cases such as the Entry block of F being deleted.
void DeferredDominance::recalculate(Function &F) {
  // flushDelBB must be flushed before the recalculation. The state of the IR
  // must be consistent before the DT traversal algorithm determines the
  // actual DT.
  if (flushDelBB() || !PendUpdates.empty()) {
    DT.recalculate(F);
    PendUpdates.clear();
  }
}

/// \brief Debug method to help view the state of pending updates.
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD void DeferredDominance::dump() const {
  raw_ostream &OS = llvm::dbgs();
  OS << "PendUpdates:\n";
  int I = 0;
  for (auto U : PendUpdates) {
    OS << "  " << I << " : ";
    ++I;
    if (U.getKind() == DominatorTree::Insert)
      OS << "Insert, ";
    else
      OS << "Delete, ";
    BasicBlock *From = U.getFrom();
    if (From) {
      auto S = From->getName();
      if (!From->hasName())
        S = "(no name)";
      OS << S << "(" << From << "), ";
    } else {
      OS << "(badref), ";
    }
    BasicBlock *To = U.getTo();
    if (To) {
      auto S = To->getName();
      if (!To->hasName())
        S = "(no_name)";
      OS << S << "(" << To << ")\n";
    } else {
      OS << "(badref)\n";
    }
  }
  OS << "DeletedBBs:\n";
  I = 0;
  for (auto BB : DeletedBBs) {
    OS << "  " << I << " : ";
    ++I;
    if (BB->hasName())
      OS << BB->getName() << "(";
    else
      OS << "(no_name)(";
    OS << BB << ")\n";
  }
}
#endif

/// Apply an update (Kind, From, To) to the internal queued updates. The
/// update is only added when determined to be necessary. Checks for
/// self-domination, unnecessary updates, duplicate requests, and balanced
/// pairs of requests are all performed. Returns true if the update is
/// queued and false if it is discarded.
bool DeferredDominance::applyUpdate(DominatorTree::UpdateKind Kind,
                                    BasicBlock *From, BasicBlock *To) {
  if (From == To)
    return false; // Cannot dominate self; discard update.

  // Discard updates by inspecting the current state of successors of From.
  // Since applyUpdate() must be called *after* the Terminator of From is
  // altered we can determine if the update is unnecessary.
  bool HasEdge = std::any_of(succ_begin(From), succ_end(From),
                             [To](BasicBlock *B) { return B == To; });
  if (Kind == DominatorTree::Insert && !HasEdge)
    return false; // Unnecessary Insert: edge does not exist in IR.
  if (Kind == DominatorTree::Delete && HasEdge)
    return false; // Unnecessary Delete: edge still exists in IR.

  // Analyze pending updates to determine if the update is unnecessary.
  DominatorTree::UpdateType Update = {Kind, From, To};
  DominatorTree::UpdateType Invert = {Kind != DominatorTree::Insert
                                          ? DominatorTree::Insert
                                          : DominatorTree::Delete,
                                      From, To};
  for (auto I = PendUpdates.begin(), E = PendUpdates.end(); I != E; ++I) {
    if (Update == *I)
      return false; // Discard duplicate updates.
    if (Invert == *I) {
      // Update and Invert are both valid (equivalent to a no-op). Remove
      // Invert from PendUpdates and discard the Update.
      PendUpdates.erase(I);
      return false;
    }
  }
  PendUpdates.push_back(Update); // Save the valid update.
  return true;
}

/// Performs all pending basic block deletions. We have to defer the deletion
/// of these blocks until after the DominatorTree updates are applied. The
/// internal workings of the DominatorTree code expect every update's From
/// and To blocks to exist and to be a member of the same Function.
bool DeferredDominance::flushDelBB() {
  if (DeletedBBs.empty())
    return false;
  for (auto *BB : DeletedBBs)
    BB->eraseFromParent();
  DeletedBBs.clear();
  return true;
}

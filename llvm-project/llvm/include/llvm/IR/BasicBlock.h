//===- llvm/BasicBlock.h - Represent a basic block in the VM ----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the BasicBlock class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_IR_BASICBLOCK_H
#define LLVM_IR_BASICBLOCK_H

#include "llvm-c/Types.h"
#include "llvm/ADT/Twine.h"
#include "llvm/ADT/ilist.h"
#include "llvm/ADT/ilist_node.h"
#include "llvm/ADT/iterator.h"
#include "llvm/ADT/iterator_range.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/SymbolTableListTraits.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/CBindingWrapping.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Compiler.h"
#include <cassert>
#include <cstddef>
#include <iterator>

namespace llvm {

class CallInst;
class Function;
class LandingPadInst;
class LLVMContext;
class Module;
class PHINode;
class TerminatorInst;
class ValueSymbolTable;

/// \brief LLVM Basic Block Representation
///
/// This represents a single basic block in LLVM. A basic block is simply a
/// container of instructions that execute sequentially. Basic blocks are Values
/// because they are referenced by instructions such as branches and switch
/// tables. The type of a BasicBlock is "Type::LabelTy" because the basic block
/// represents a label to which a branch can jump.
///
/// A well formed basic block is formed of a list of non-terminating
/// instructions followed by a single TerminatorInst instruction.
/// TerminatorInst's may not occur in the middle of basic blocks, and must
/// terminate the blocks. The BasicBlock class allows malformed basic blocks to
/// occur because it may be useful in the intermediate stage of constructing or
/// modifying a program. However, the verifier will ensure that basic blocks
/// are "well formed".
class BasicBlock final : public Value, // Basic blocks are data objects also
                         public ilist_node_with_parent<BasicBlock, Function> {
public:
  using InstListType = SymbolTableList<Instruction>;

private:
  friend class BlockAddress;
  friend class SymbolTableListTraits<BasicBlock>;

  InstListType InstList;
  Function *Parent;

  void setParent(Function *parent);

  /// \brief Constructor.
  ///
  /// If the function parameter is specified, the basic block is automatically
  /// inserted at either the end of the function (if InsertBefore is null), or
  /// before the specified basic block.
  explicit BasicBlock(LLVMContext &C, const Twine &Name = "",
                      Function *Parent = nullptr,
                      BasicBlock *InsertBefore = nullptr);

public:
  BasicBlock(const BasicBlock &) = delete;
  BasicBlock &operator=(const BasicBlock &) = delete;
  ~BasicBlock();

  /// \brief Get the context in which this basic block lives.
  LLVMContext &getContext() const;

  /// Instruction iterators...
  using iterator = InstListType::iterator;
  using const_iterator = InstListType::const_iterator;
  using reverse_iterator = InstListType::reverse_iterator;
  using const_reverse_iterator = InstListType::const_reverse_iterator;

  /// \brief Creates a new BasicBlock.
  ///
  /// If the Parent parameter is specified, the basic block is automatically
  /// inserted at either the end of the function (if InsertBefore is 0), or
  /// before the specified basic block.
  static BasicBlock *Create(LLVMContext &Context, const Twine &Name = "",
                            Function *Parent = nullptr,
                            BasicBlock *InsertBefore = nullptr) {
    return new BasicBlock(Context, Name, Parent, InsertBefore);
  }

  /// \brief Return the enclosing method, or null if none.
  const Function *getParent() const { return Parent; }
        Function *getParent()       { return Parent; }

  /// \brief Return the module owning the function this basic block belongs to,
  /// or nullptr it the function does not have a module.
  ///
  /// Note: this is undefined behavior if the block does not have a parent.
  const Module *getModule() const;
  Module *getModule() {
    return const_cast<Module *>(
                            static_cast<const BasicBlock *>(this)->getModule());
  }

  /// \brief Returns the terminator instruction if the block is well formed or
  /// null if the block is not well formed.
  const TerminatorInst *getTerminator() const LLVM_READONLY;
  TerminatorInst *getTerminator() {
    return const_cast<TerminatorInst *>(
                        static_cast<const BasicBlock *>(this)->getTerminator());
  }

  /// \brief Returns the call instruction calling @llvm.experimental.deoptimize
  /// prior to the terminating return instruction of this basic block, if such a
  /// call is present.  Otherwise, returns null.
  const CallInst *getTerminatingDeoptimizeCall() const;
  CallInst *getTerminatingDeoptimizeCall() {
    return const_cast<CallInst *>(
         static_cast<const BasicBlock *>(this)->getTerminatingDeoptimizeCall());
  }

  /// \brief Returns the call instruction marked 'musttail' prior to the
  /// terminating return instruction of this basic block, if such a call is
  /// present.  Otherwise, returns null.
  const CallInst *getTerminatingMustTailCall() const;
  CallInst *getTerminatingMustTailCall() {
    return const_cast<CallInst *>(
           static_cast<const BasicBlock *>(this)->getTerminatingMustTailCall());
  }

  /// \brief Returns a pointer to the first instruction in this block that is
  /// not a PHINode instruction.
  ///
  /// When adding instructions to the beginning of the basic block, they should
  /// be added before the returned value, not before the first instruction,
  /// which might be PHI. Returns 0 is there's no non-PHI instruction.
  const Instruction* getFirstNonPHI() const;
  Instruction* getFirstNonPHI() {
    return const_cast<Instruction *>(
                       static_cast<const BasicBlock *>(this)->getFirstNonPHI());
  }

  /// \brief Returns a pointer to the first instruction in this block that is not
  /// a PHINode or a debug intrinsic.
  const Instruction* getFirstNonPHIOrDbg() const;
  Instruction* getFirstNonPHIOrDbg() {
    return const_cast<Instruction *>(
                  static_cast<const BasicBlock *>(this)->getFirstNonPHIOrDbg());
  }

  /// \brief Returns a pointer to the first instruction in this block that is not
  /// a PHINode, a debug intrinsic, or a lifetime intrinsic.
  const Instruction* getFirstNonPHIOrDbgOrLifetime() const;
  Instruction* getFirstNonPHIOrDbgOrLifetime() {
    return const_cast<Instruction *>(
        static_cast<const BasicBlock *>(this)->getFirstNonPHIOrDbgOrLifetime());
  }

  /// \brief Returns an iterator to the first instruction in this block that is
  /// suitable for inserting a non-PHI instruction.
  ///
  /// In particular, it skips all PHIs and LandingPad instructions.
  const_iterator getFirstInsertionPt() const;
  iterator getFirstInsertionPt() {
    return static_cast<const BasicBlock *>(this)
                                          ->getFirstInsertionPt().getNonConst();
  }

  /// \brief Unlink 'this' from the containing function, but do not delete it.
  void removeFromParent();

  /// \brief Unlink 'this' from the containing function and delete it.
  ///
  // \returns an iterator pointing to the element after the erased one.
  SymbolTableList<BasicBlock>::iterator eraseFromParent();

  /// \brief Unlink this basic block from its current function and insert it
  /// into the function that \p MovePos lives in, right before \p MovePos.
  void moveBefore(BasicBlock *MovePos);

  /// \brief Unlink this basic block from its current function and insert it
  /// right after \p MovePos in the function \p MovePos lives in.
  void moveAfter(BasicBlock *MovePos);

  /// \brief Insert unlinked basic block into a function.
  ///
  /// Inserts an unlinked basic block into \c Parent.  If \c InsertBefore is
  /// provided, inserts before that basic block, otherwise inserts at the end.
  ///
  /// \pre \a getParent() is \c nullptr.
  void insertInto(Function *Parent, BasicBlock *InsertBefore = nullptr);

  /// \brief Return the predecessor of this block if it has a single predecessor
  /// block. Otherwise return a null pointer.
  const BasicBlock *getSinglePredecessor() const;
  BasicBlock *getSinglePredecessor() {
    return const_cast<BasicBlock *>(
                 static_cast<const BasicBlock *>(this)->getSinglePredecessor());
  }

  /// \brief Return the predecessor of this block if it has a unique predecessor
  /// block. Otherwise return a null pointer.
  ///
  /// Note that unique predecessor doesn't mean single edge, there can be
  /// multiple edges from the unique predecessor to this block (for example a
  /// switch statement with multiple cases having the same destination).
  const BasicBlock *getUniquePredecessor() const;
  BasicBlock *getUniquePredecessor() {
    return const_cast<BasicBlock *>(
                 static_cast<const BasicBlock *>(this)->getUniquePredecessor());
  }

  /// \brief Return the successor of this block if it has a single successor.
  /// Otherwise return a null pointer.
  ///
  /// This method is analogous to getSinglePredecessor above.
  const BasicBlock *getSingleSuccessor() const;
  BasicBlock *getSingleSuccessor() {
    return const_cast<BasicBlock *>(
                   static_cast<const BasicBlock *>(this)->getSingleSuccessor());
  }

  /// \brief Return the successor of this block if it has a unique successor.
  /// Otherwise return a null pointer.
  ///
  /// This method is analogous to getUniquePredecessor above.
  const BasicBlock *getUniqueSuccessor() const;
  BasicBlock *getUniqueSuccessor() {
    return const_cast<BasicBlock *>(
                   static_cast<const BasicBlock *>(this)->getUniqueSuccessor());
  }

  /// Return true if this block has exactly N predecessors.
  bool hasNPredecessors(unsigned N) const;

  /// Return true if this block has N predecessors or more.
  bool hasNPredecessorsOrMore(unsigned N) const;

  //===--------------------------------------------------------------------===//
  /// Instruction iterator methods
  ///
  inline iterator                begin()       { return InstList.begin(); }
  inline const_iterator          begin() const { return InstList.begin(); }
  inline iterator                end  ()       { return InstList.end();   }
  inline const_iterator          end  () const { return InstList.end();   }

  inline reverse_iterator        rbegin()       { return InstList.rbegin(); }
  inline const_reverse_iterator  rbegin() const { return InstList.rbegin(); }
  inline reverse_iterator        rend  ()       { return InstList.rend();   }
  inline const_reverse_iterator  rend  () const { return InstList.rend();   }

  inline size_t                   size() const { return InstList.size();  }
  inline bool                    empty() const { return InstList.empty(); }
  inline const Instruction      &front() const { return InstList.front(); }
  inline       Instruction      &front()       { return InstList.front(); }
  inline const Instruction       &back() const { return InstList.back();  }
  inline       Instruction       &back()       { return InstList.back();  }

  /// Iterator to walk just the phi nodes in the basic block.
  template <typename PHINodeT = PHINode, typename BBIteratorT = iterator>
  class phi_iterator_impl
      : public iterator_facade_base<phi_iterator_impl<PHINodeT, BBIteratorT>,
                                    std::forward_iterator_tag, PHINodeT> {
    friend BasicBlock;

    PHINodeT *PN;

    phi_iterator_impl(PHINodeT *PN) : PN(PN) {}

  public:
    // Allow default construction to build variables, but this doesn't build
    // a useful iterator.
    phi_iterator_impl() = default;

    // Allow conversion between instantiations where valid.
    template <typename PHINodeU, typename BBIteratorU>
    phi_iterator_impl(const phi_iterator_impl<PHINodeU, BBIteratorU> &Arg)
        : PN(Arg.PN) {}

    bool operator==(const phi_iterator_impl &Arg) const { return PN == Arg.PN; }

    PHINodeT &operator*() const { return *PN; }

    using phi_iterator_impl::iterator_facade_base::operator++;
    phi_iterator_impl &operator++() {
      assert(PN && "Cannot increment the end iterator!");
      PN = dyn_cast<PHINodeT>(std::next(BBIteratorT(PN)));
      return *this;
    }
  };
  using phi_iterator = phi_iterator_impl<>;
  using const_phi_iterator =
      phi_iterator_impl<const PHINode, BasicBlock::const_iterator>;

  /// Returns a range that iterates over the phis in the basic block.
  ///
  /// Note that this cannot be used with basic blocks that have no terminator.
  iterator_range<const_phi_iterator> phis() const {
    return const_cast<BasicBlock *>(this)->phis();
  }
  iterator_range<phi_iterator> phis();

  /// \brief Return the underlying instruction list container.
  ///
  /// Currently you need to access the underlying instruction list container
  /// directly if you want to modify it.
  const InstListType &getInstList() const { return InstList; }
        InstListType &getInstList()       { return InstList; }

  /// \brief Returns a pointer to a member of the instruction list.
  static InstListType BasicBlock::*getSublistAccess(Instruction*) {
    return &BasicBlock::InstList;
  }

  /// \brief Returns a pointer to the symbol table if one exists.
  ValueSymbolTable *getValueSymbolTable();

  /// \brief Methods for support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const Value *V) {
    return V->getValueID() == Value::BasicBlockVal;
  }

  /// \brief Cause all subinstructions to "let go" of all the references that
  /// said subinstructions are maintaining.
  ///
  /// This allows one to 'delete' a whole class at a time, even though there may
  /// be circular references... first all references are dropped, and all use
  /// counts go to zero.  Then everything is delete'd for real.  Note that no
  /// operations are valid on an object that has "dropped all references",
  /// except operator delete.
  void dropAllReferences();

  /// \brief Notify the BasicBlock that the predecessor \p Pred is no longer
  /// able to reach it.
  ///
  /// This is actually not used to update the Predecessor list, but is actually
  /// used to update the PHI nodes that reside in the block.  Note that this
  /// should be called while the predecessor still refers to this block.
  void removePredecessor(BasicBlock *Pred, bool DontDeleteUselessPHIs = false);

  bool canSplitPredecessors() const;

  /// \brief Split the basic block into two basic blocks at the specified
  /// instruction.
  ///
  /// Note that all instructions BEFORE the specified iterator stay as part of
  /// the original basic block, an unconditional branch is added to the original
  /// BB, and the rest of the instructions in the BB are moved to the new BB,
  /// including the old terminator.  The newly formed BasicBlock is returned.
  /// This function invalidates the specified iterator.
  ///
  /// Note that this only works on well formed basic blocks (must have a
  /// terminator), and 'I' must not be the end of instruction list (which would
  /// cause a degenerate basic block to be formed, having a terminator inside of
  /// the basic block).
  ///
  /// Also note that this doesn't preserve any passes. To split blocks while
  /// keeping loop information consistent, use the SplitBlock utility function.
  BasicBlock *splitBasicBlock(iterator I, const Twine &BBName = "");
  BasicBlock *splitBasicBlock(Instruction *I, const Twine &BBName = "") {
    return splitBasicBlock(I->getIterator(), BBName);
  }

  /// \brief Returns true if there are any uses of this basic block other than
  /// direct branches, switches, etc. to it.
  bool hasAddressTaken() const {
    return getBasicBlockBits().BlockAddressRefCount != 0;
  }

  /// \brief Update all phi nodes in this basic block's successors to refer to
  /// basic block \p New instead of to it.
  void replaceSuccessorsPhiUsesWith(BasicBlock *New);

  /// \brief Return true if this basic block is an exception handling block.
  bool isEHPad() const { return getFirstNonPHI()->isEHPad(); }

  /// \brief Return true if this basic block is a landing pad.
  ///
  /// Being a ``landing pad'' means that the basic block is the destination of
  /// the 'unwind' edge of an invoke instruction.
  bool isLandingPad() const;

  /// \brief Return the landingpad instruction associated with the landing pad.
  const LandingPadInst *getLandingPadInst() const;
  LandingPadInst *getLandingPadInst() {
    return const_cast<LandingPadInst *>(
                    static_cast<const BasicBlock *>(this)->getLandingPadInst());
  }

  /// \brief Return true if it is legal to hoist instructions into this block.
  bool isLegalToHoistInto() const;

  Optional<uint64_t> getIrrLoopHeaderWeight() const;

  bool isInstrOrderValid() const {
    return getBasicBlockBits().InstrOrderValid;
  }

  /// Mark instruction ordering invalid. Done on every instruction insert.
  void invalidateOrders() {
    validateInstrOrdering();
    BasicBlockBits Bits = getBasicBlockBits();
    Bits.InstrOrderValid = false;
    setBasicBlockBits(Bits);
  }

  /// Renumber instructions and mark the ordering as valid.
  void renumberInstructions();

  /// Asserts that instruction order numbers are marked invalid, or that they
  /// are in ascending order. This is constant time if the ordering is invalid,
  /// and linear in the number of instructions if the ordering is valid. Callers
  /// should be careful not to call this in ways that make common operations
  /// O(n^2). For example, it takes O(n) time to assign order numbers to
  /// instructions, so the order should be validated no more than once after
  /// each ordering to ensure that transforms have the same algorithmic
  /// complexity when asserts are enabled as when they are disabled.
  void validateInstrOrdering() const;

private:
  /// Bitfield to help interpret the bits in Value::SubclassData.
  struct BasicBlockBits {
    unsigned short BlockAddressRefCount : 15;
    unsigned short InstrOrderValid : 1;
  };

  /// Safely reinterpret the subclass data bits to a more useful form.
  BasicBlockBits getBasicBlockBits() const {
    static_assert(sizeof(BasicBlockBits) == sizeof(unsigned short),
                  "too many bits for Value::SubclassData");
    unsigned short ValueData = getSubclassDataFromValue();
    BasicBlockBits AsBits;
    memcpy(&AsBits, &ValueData, sizeof(AsBits));
    return AsBits;
  }

  /// Reinterpret our subclass bits and store them back into Value.
  void setBasicBlockBits(BasicBlockBits AsBits) {
    unsigned short D;
    memcpy(&D, &AsBits, sizeof(D));
    Value::setValueSubclassData(D);
  }

  /// \brief Increment the internal refcount of the number of BlockAddresses
  /// referencing this BasicBlock by \p Amt.
  ///
  /// This is almost always 0, sometimes one possibly, but almost never 2, and
  /// inconceivably 3 or more.
  void AdjustBlockAddressRefCount(int Amt) {
    BasicBlockBits Bits = getBasicBlockBits();
    Bits.BlockAddressRefCount += Amt;
    setBasicBlockBits(Bits);
    assert(Bits.BlockAddressRefCount < 255 && "Refcount wrap-around");
  }

  /// \brief Shadow Value::setValueSubclassData with a private forwarding method
  /// so that any future subclasses cannot accidentally use it.
  void setValueSubclassData(unsigned short D) {
    Value::setValueSubclassData(D);
  }
};

// Create wrappers for C Binding types (see CBindingWrapping.h).
DEFINE_SIMPLE_CONVERSION_FUNCTIONS(BasicBlock, LLVMBasicBlockRef)

#ifdef NDEBUG
/// In release builds, this is a no-op. For !NDEBUG builds, the checks are
/// implemented in the .cpp file to avoid circular header deps.
inline void BasicBlock::validateInstrOrdering() const {}
#endif

} // end namespace llvm

#endif // LLVM_IR_BASICBLOCK_H

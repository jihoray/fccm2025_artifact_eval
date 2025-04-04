//===- Decl.h - Classes for representing declarations -----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
// And has the following additional copyright:
//
// (C) Copyright 2016-2020 Xilinx, Inc.
// All Rights Reserved.
//
//===----------------------------------------------------------------------===//
//
//  This file defines the Decl subclasses.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_AST_DECL_H
#define LLVM_CLANG_AST_DECL_H

#include "clang/AST/APValue.h"
#include "clang/AST/DeclBase.h"
#include "clang/AST/DeclarationName.h"
#include "clang/AST/ExternalASTSource.h"
#include "clang/AST/NestedNameSpecifier.h"
#include "clang/AST/Redeclarable.h"
#include "clang/AST/Type.h"
#include "clang/Basic/AddressSpaces.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/IdentifierTable.h"
#include "clang/Basic/LLVM.h"
#include "clang/Basic/Linkage.h"
#include "clang/Basic/OperatorKinds.h"
#include "clang/Basic/PartialDiagnostic.h"
#include "clang/Basic/PragmaKinds.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/Specifiers.h"
#include "clang/Basic/Visibility.h"
#include "llvm/ADT/APSInt.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/PointerIntPair.h"
#include "llvm/ADT/PointerUnion.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/iterator_range.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/TrailingObjects.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>

namespace clang {

class ASTContext;
struct ASTTemplateArgumentListInfo;
class Attr;
class CompoundStmt;
class DependentFunctionTemplateSpecializationInfo;
class EnumDecl;
class Expr;
class FunctionTemplateDecl;
class FunctionTemplateSpecializationInfo;
class LabelStmt;
class MemberSpecializationInfo;
class Module;
class NamespaceDecl;
class ParmVarDecl;
class RecordDecl;
class Stmt;
class StringLiteral;
class TagDecl;
class TemplateArgumentList;
class TemplateArgumentListInfo;
class TemplateParameterList;
class TypeAliasTemplateDecl;
class TypeLoc;
class UnresolvedSetImpl;
class VarTemplateDecl;

/// \brief A container of type source information.
///
/// A client can read the relevant info using TypeLoc wrappers, e.g:
/// @code
/// TypeLoc TL = TypeSourceInfo->getTypeLoc();
/// TL.getStartLoc().print(OS, SrcMgr);
/// @endcode
class TypeSourceInfo {
  // Contains a memory block after the class, used for type source information,
  // allocated by ASTContext.
  friend class ASTContext;

  QualType Ty;

  TypeSourceInfo(QualType ty) : Ty(ty) {}

public:
  /// \brief Return the type wrapped by this type source info.
  QualType getType() const { return Ty; }

  /// \brief Return the TypeLoc wrapper for the type source info.
  TypeLoc getTypeLoc() const; // implemented in TypeLoc.h
  
  /// \brief Override the type stored in this TypeSourceInfo. Use with caution!
  void overrideType(QualType T) { Ty = T; }
};

/// TranslationUnitDecl - The top declaration context.
class TranslationUnitDecl : public Decl, public DeclContext {
  ASTContext &Ctx;

  /// The (most recently entered) anonymous namespace for this
  /// translation unit, if one has been created.
  NamespaceDecl *AnonymousNamespace = nullptr;

  explicit TranslationUnitDecl(ASTContext &ctx);

  virtual void anchor();

public:
  ASTContext &getASTContext() const { return Ctx; }

  NamespaceDecl *getAnonymousNamespace() const { return AnonymousNamespace; }
  void setAnonymousNamespace(NamespaceDecl *D) { AnonymousNamespace = D; }

  static TranslationUnitDecl *Create(ASTContext &C);

  // Implement isa/cast/dyncast/etc.
  static bool classof(const Decl *D) { return classofKind(D->getKind()); }
  static bool classofKind(Kind K) { return K == TranslationUnit; }
  static DeclContext *castToDeclContext(const TranslationUnitDecl *D) {
    return static_cast<DeclContext *>(const_cast<TranslationUnitDecl*>(D));
  }
  static TranslationUnitDecl *castFromDeclContext(const DeclContext *DC) {
    return static_cast<TranslationUnitDecl *>(const_cast<DeclContext*>(DC));
  }
};

/// \brief Represents a `#pragma comment` line. Always a child of
/// TranslationUnitDecl.
class PragmaCommentDecl final
    : public Decl,
      private llvm::TrailingObjects<PragmaCommentDecl, char> {
  friend class ASTDeclReader;
  friend class ASTDeclWriter;
  friend TrailingObjects;

  PragmaMSCommentKind CommentKind;

  PragmaCommentDecl(TranslationUnitDecl *TU, SourceLocation CommentLoc,
                    PragmaMSCommentKind CommentKind)
      : Decl(PragmaComment, TU, CommentLoc), CommentKind(CommentKind) {}

  virtual void anchor();

public:
  static PragmaCommentDecl *Create(const ASTContext &C, TranslationUnitDecl *DC,
                                   SourceLocation CommentLoc,
                                   PragmaMSCommentKind CommentKind,
                                   StringRef Arg);
  static PragmaCommentDecl *CreateDeserialized(ASTContext &C, unsigned ID,
                                               unsigned ArgSize);

  PragmaMSCommentKind getCommentKind() const { return CommentKind; }

  StringRef getArg() const { return getTrailingObjects<char>(); }

  // Implement isa/cast/dyncast/etc.
  static bool classof(const Decl *D) { return classofKind(D->getKind()); }
  static bool classofKind(Kind K) { return K == PragmaComment; }
};

/// \brief Represents a `#pragma detect_mismatch` line. Always a child of
/// TranslationUnitDecl.
class PragmaDetectMismatchDecl final
    : public Decl,
      private llvm::TrailingObjects<PragmaDetectMismatchDecl, char> {
  friend class ASTDeclReader;
  friend class ASTDeclWriter;
  friend TrailingObjects;

  size_t ValueStart;

  PragmaDetectMismatchDecl(TranslationUnitDecl *TU, SourceLocation Loc,
                           size_t ValueStart)
      : Decl(PragmaDetectMismatch, TU, Loc), ValueStart(ValueStart) {}

  virtual void anchor();

public:
  static PragmaDetectMismatchDecl *Create(const ASTContext &C,
                                          TranslationUnitDecl *DC,
                                          SourceLocation Loc, StringRef Name,
                                          StringRef Value);
  static PragmaDetectMismatchDecl *
  CreateDeserialized(ASTContext &C, unsigned ID, unsigned NameValueSize);

  StringRef getName() const { return getTrailingObjects<char>(); }
  StringRef getValue() const { return getTrailingObjects<char>() + ValueStart; }

  // Implement isa/cast/dyncast/etc.
  static bool classof(const Decl *D) { return classofKind(D->getKind()); }
  static bool classofKind(Kind K) { return K == PragmaDetectMismatch; }
};

/// \brief Declaration context for names declared as extern "C" in C++. This
/// is neither the semantic nor lexical context for such declarations, but is
/// used to check for conflicts with other extern "C" declarations. Example:
///
/// \code
///   namespace N { extern "C" void f(); } // #1
///   void N::f() {}                       // #2
///   namespace M { extern "C" void f(); } // #3
/// \endcode
///
/// The semantic context of #1 is namespace N and its lexical context is the
/// LinkageSpecDecl; the semantic context of #2 is namespace N and its lexical
/// context is the TU. However, both declarations are also visible in the
/// extern "C" context.
///
/// The declaration at #3 finds it is a redeclaration of \c N::f through
/// lookup in the extern "C" context.
class ExternCContextDecl : public Decl, public DeclContext {
  explicit ExternCContextDecl(TranslationUnitDecl *TU)
    : Decl(ExternCContext, TU, SourceLocation()),
      DeclContext(ExternCContext) {}

  virtual void anchor();

public:
  static ExternCContextDecl *Create(const ASTContext &C,
                                    TranslationUnitDecl *TU);

  // Implement isa/cast/dyncast/etc.
  static bool classof(const Decl *D) { return classofKind(D->getKind()); }
  static bool classofKind(Kind K) { return K == ExternCContext; }
  static DeclContext *castToDeclContext(const ExternCContextDecl *D) {
    return static_cast<DeclContext *>(const_cast<ExternCContextDecl*>(D));
  }
  static ExternCContextDecl *castFromDeclContext(const DeclContext *DC) {
    return static_cast<ExternCContextDecl *>(const_cast<DeclContext*>(DC));
  }
};

/// NamedDecl - This represents a decl with a name.  Many decls have names such
/// as ObjCMethodDecl, but not \@class, etc.
class NamedDecl : public Decl {
  /// Name - The name of this declaration, which is typically a normal
  /// identifier but may also be a special kind of name (C++
  /// constructor, Objective-C selector, etc.)
  DeclarationName Name;

  virtual void anchor();

private:
  NamedDecl *getUnderlyingDeclImpl() LLVM_READONLY;

protected:
  NamedDecl(Kind DK, DeclContext *DC, SourceLocation L, DeclarationName N)
      : Decl(DK, DC, L), Name(N) {}

public:
  /// getIdentifier - Get the identifier that names this declaration,
  /// if there is one. This will return NULL if this declaration has
  /// no name (e.g., for an unnamed class) or if the name is a special
  /// name (C++ constructor, Objective-C selector, etc.).
  IdentifierInfo *getIdentifier() const { return Name.getAsIdentifierInfo(); }

  /// getName - Get the name of identifier for this declaration as a StringRef.
  /// This requires that the declaration have a name and that it be a simple
  /// identifier.
  StringRef getName() const {
    assert(Name.isIdentifier() && "Name is not a simple identifier");
    return getIdentifier() ? getIdentifier()->getName() : "";
  }

  /// getNameAsString - Get a human-readable name for the declaration, even if
  /// it is one of the special kinds of names (C++ constructor, Objective-C
  /// selector, etc).  Creating this name requires expensive string
  /// manipulation, so it should be called only when performance doesn't matter.
  /// For simple declarations, getNameAsCString() should suffice.
  //
  // FIXME: This function should be renamed to indicate that it is not just an
  // alternate form of getName(), and clients should move as appropriate.
  //
  // FIXME: Deprecated, move clients to getName().
  std::string getNameAsString() const { return Name.getAsString(); }

  virtual void printName(raw_ostream &os) const;

  /// getDeclName - Get the actual, stored name of the declaration,
  /// which may be a special name.
  DeclarationName getDeclName() const { return Name; }

  /// \brief Set the name of this declaration.
  void setDeclName(DeclarationName N) { Name = N; }

  /// printQualifiedName - Returns human-readable qualified name for
  /// declaration, like A::B::i, for i being member of namespace A::B.
  /// If declaration is not member of context which can be named (record,
  /// namespace), it will return same result as printName().
  /// Creating this name is expensive, so it should be called only when
  /// performance doesn't matter.
  void printQualifiedName(raw_ostream &OS) const;
  void printQualifiedName(raw_ostream &OS, const PrintingPolicy &Policy) const;

  // FIXME: Remove string version.
  std::string getQualifiedNameAsString() const;

  /// Appends a human-readable name for this declaration into the given stream.
  ///
  /// This is the method invoked by Sema when displaying a NamedDecl
  /// in a diagnostic.  It does not necessarily produce the same
  /// result as printName(); for example, class template
  /// specializations are printed with their template arguments.
  virtual void getNameForDiagnostic(raw_ostream &OS,
                                    const PrintingPolicy &Policy,
                                    bool Qualified) const;

  /// \brief Determine whether this declaration, if
  /// known to be well-formed within its context, will replace the
  /// declaration OldD if introduced into scope. A declaration will
  /// replace another declaration if, for example, it is a
  /// redeclaration of the same variable or function, but not if it is
  /// a declaration of a different kind (function vs. class) or an
  /// overloaded function.
  ///
  /// \param IsKnownNewer \c true if this declaration is known to be newer
  /// than \p OldD (for instance, if this declaration is newly-created).
  bool declarationReplaces(NamedDecl *OldD, bool IsKnownNewer = true) const;

  /// \brief Determine whether this declaration has linkage.
  bool hasLinkage() const;

  using Decl::isModulePrivate;
  using Decl::setModulePrivate;

  /// \brief Determine whether this declaration is a C++ class member.
  bool isCXXClassMember() const {
    const DeclContext *DC = getDeclContext();

    // C++0x [class.mem]p1:
    //   The enumerators of an unscoped enumeration defined in
    //   the class are members of the class.
    if (isa<EnumDecl>(DC))
      DC = DC->getRedeclContext();

    return DC->isRecord();
  }

  /// \brief Determine whether the given declaration is an instance member of
  /// a C++ class.
  bool isCXXInstanceMember() const;

  /// \brief Determine what kind of linkage this entity has.
  /// This is not the linkage as defined by the standard or the codegen notion
  /// of linkage. It is just an implementation detail that is used to compute
  /// those.
  Linkage getLinkageInternal() const;

  /// \brief Get the linkage from a semantic point of view. Entities in
  /// anonymous namespaces are external (in c++98).
  Linkage getFormalLinkage() const {
    return clang::getFormalLinkage(getLinkageInternal());
  }

  /// \brief True if this decl has external linkage.
  bool hasExternalFormalLinkage() const {
    return isExternalFormalLinkage(getLinkageInternal());
  }

  bool isExternallyVisible() const {
    return clang::isExternallyVisible(getLinkageInternal());
  }

  /// Determine whether this declaration can be redeclared in a
  /// different translation unit.
  bool isExternallyDeclarable() const {
    return isExternallyVisible() && !getOwningModuleForLinkage();
  }

  /// \brief Determines the visibility of this entity.
  Visibility getVisibility() const {
    return getLinkageAndVisibility().getVisibility();
  }

  /// \brief Determines the linkage and visibility of this entity.
  LinkageInfo getLinkageAndVisibility() const;

  /// Kinds of explicit visibility.
  enum ExplicitVisibilityKind {
    /// Do an LV computation for, ultimately, a type.
    /// Visibility may be restricted by type visibility settings and
    /// the visibility of template arguments.
    VisibilityForType,

    /// Do an LV computation for, ultimately, a non-type declaration.
    /// Visibility may be restricted by value visibility settings and
    /// the visibility of template arguments.
    VisibilityForValue
  };

  /// \brief If visibility was explicitly specified for this
  /// declaration, return that visibility.
  Optional<Visibility>
  getExplicitVisibility(ExplicitVisibilityKind kind) const;

  /// \brief True if the computed linkage is valid. Used for consistency
  /// checking. Should always return true.
  bool isLinkageValid() const;

  /// \brief True if something has required us to compute the linkage
  /// of this declaration.
  ///
  /// Language features which can retroactively change linkage (like a
  /// typedef name for linkage purposes) may need to consider this,
  /// but hopefully only in transitory ways during parsing.
  bool hasLinkageBeenComputed() const {
    return hasCachedLinkage();
  }

  /// \brief Looks through UsingDecls and ObjCCompatibleAliasDecls for
  /// the underlying named decl.
  NamedDecl *getUnderlyingDecl() {
    // Fast-path the common case.
    if (this->getKind() != UsingShadow &&
        this->getKind() != ConstructorUsingShadow &&
        this->getKind() != ObjCCompatibleAlias &&
        this->getKind() != NamespaceAlias)
      return this;

    return getUnderlyingDeclImpl();
  }
  const NamedDecl *getUnderlyingDecl() const {
    return const_cast<NamedDecl*>(this)->getUnderlyingDecl();
  }

  NamedDecl *getMostRecentDecl() {
    return cast<NamedDecl>(static_cast<Decl *>(this)->getMostRecentDecl());
  }
  const NamedDecl *getMostRecentDecl() const {
    return const_cast<NamedDecl*>(this)->getMostRecentDecl();
  }

  ObjCStringFormatFamily getObjCFStringFormattingFamily() const;

  static bool classof(const Decl *D) { return classofKind(D->getKind()); }
  static bool classofKind(Kind K) { return K >= firstNamed && K <= lastNamed; }
};

inline raw_ostream &operator<<(raw_ostream &OS, const NamedDecl &ND) {
  ND.printName(OS);
  return OS;
}

/// LabelDecl - Represents the declaration of a label.  Labels also have a
/// corresponding LabelStmt, which indicates the position that the label was
/// defined at.  For normal labels, the location of the decl is the same as the
/// location of the statement.  For GNU local labels (__label__), the decl
/// location is where the __label__ is.
class LabelDecl : public NamedDecl {
  LabelStmt *TheStmt;
  StringRef MSAsmName;
  bool MSAsmNameResolved = false;

  /// LocStart - For normal labels, this is the same as the main declaration
  /// label, i.e., the location of the identifier; for GNU local labels,
  /// this is the location of the __label__ keyword.
  SourceLocation LocStart;

  LabelDecl(DeclContext *DC, SourceLocation IdentL, IdentifierInfo *II,
            LabelStmt *S, SourceLocation StartL)
      : NamedDecl(Label, DC, IdentL, II), TheStmt(S), LocStart(StartL) {}

  void anchor() override;

public:
  static LabelDecl *Create(ASTContext &C, DeclContext *DC,
                           SourceLocation IdentL, IdentifierInfo *II);
  static LabelDecl *Create(ASTContext &C, DeclContext *DC,
                           SourceLocation IdentL, IdentifierInfo *II,
                           SourceLocation GnuLabelL);
  static LabelDecl *CreateDeserialized(ASTContext &C, unsigned ID);
  
  LabelStmt *getStmt() const { return TheStmt; }
  void setStmt(LabelStmt *T) { TheStmt = T; }

  bool isGnuLocal() const { return LocStart != getLocation(); }
  void setLocStart(SourceLocation L) { LocStart = L; }

  SourceRange getSourceRange() const override LLVM_READONLY {
    return SourceRange(LocStart, getLocation());
  }

  bool isMSAsmLabel() const { return !MSAsmName.empty(); }
  bool isResolvedMSAsmLabel() const { return isMSAsmLabel() && MSAsmNameResolved; }
  void setMSAsmLabel(StringRef Name);
  StringRef getMSAsmLabel() const { return MSAsmName; }
  void setMSAsmLabelResolved() { MSAsmNameResolved = true; }

  // Implement isa/cast/dyncast/etc.
  static bool classof(const Decl *D) { return classofKind(D->getKind()); }
  static bool classofKind(Kind K) { return K == Label; }
};

/// NamespaceDecl - Represent a C++ namespace.
class NamespaceDecl : public NamedDecl, public DeclContext, 
                      public Redeclarable<NamespaceDecl> 
{
  /// LocStart - The starting location of the source range, pointing
  /// to either the namespace or the inline keyword.
  SourceLocation LocStart;

  /// RBraceLoc - The ending location of the source range.
  SourceLocation RBraceLoc;

  /// \brief A pointer to either the anonymous namespace that lives just inside
  /// this namespace or to the first namespace in the chain (the latter case
  /// only when this is not the first in the chain), along with a 
  /// boolean value indicating whether this is an inline namespace.
  llvm::PointerIntPair<NamespaceDecl *, 1, bool> AnonOrFirstNamespaceAndInline;

  NamespaceDecl(ASTContext &C, DeclContext *DC, bool Inline,
                SourceLocation StartLoc, SourceLocation IdLoc,
                IdentifierInfo *Id, NamespaceDecl *PrevDecl);

  using redeclarable_base = Redeclarable<NamespaceDecl>;

  NamespaceDecl *getNextRedeclarationImpl() override;
  NamespaceDecl *getPreviousDeclImpl() override;
  NamespaceDecl *getMostRecentDeclImpl() override;

public:
  friend class ASTDeclReader;
  friend class ASTDeclWriter;

  static NamespaceDecl *Create(ASTContext &C, DeclContext *DC,
                               bool Inline, SourceLocation StartLoc,
                               SourceLocation IdLoc, IdentifierInfo *Id,
                               NamespaceDecl *PrevDecl);

  static NamespaceDecl *CreateDeserialized(ASTContext &C, unsigned ID);

  using redecl_range = redeclarable_base::redecl_range;
  using redecl_iterator = redeclarable_base::redecl_iterator;

  using redeclarable_base::redecls_begin;
  using redeclarable_base::redecls_end;
  using redeclarable_base::redecls;
  using redeclarable_base::getPreviousDecl;
  using redeclarable_base::getMostRecentDecl;
  using redeclarable_base::isFirstDecl;

  /// \brief Returns true if this is an anonymous namespace declaration.
  ///
  /// For example:
  /// \code
  ///   namespace {
  ///     ...
  ///   };
  /// \endcode
  /// q.v. C++ [namespace.unnamed]
  bool isAnonymousNamespace() const {
    return !getIdentifier();
  }

  /// \brief Returns true if this is an inline namespace declaration.
  bool isInline() const {
    return AnonOrFirstNamespaceAndInline.getInt();
  }

  /// \brief Set whether this is an inline namespace declaration.
  void setInline(bool Inline) {
    AnonOrFirstNamespaceAndInline.setInt(Inline);
  }

  /// \brief Get the original (first) namespace declaration.
  NamespaceDecl *getOriginalNamespace();

  /// \brief Get the original (first) namespace declaration.
  const NamespaceDecl *getOriginalNamespace() const;

  /// \brief Return true if this declaration is an original (first) declaration
  /// of the namespace. This is false for non-original (subsequent) namespace
  /// declarations and anonymous namespaces.
  bool isOriginalNamespace() const;

  /// \brief Retrieve the anonymous namespace nested inside this namespace,
  /// if any.
  NamespaceDecl *getAnonymousNamespace() const {
    return getOriginalNamespace()->AnonOrFirstNamespaceAndInline.getPointer();
  }

  void setAnonymousNamespace(NamespaceDecl *D) {
    getOriginalNamespace()->AnonOrFirstNamespaceAndInline.setPointer(D);
  }

  /// Retrieves the canonical declaration of this namespace.
  NamespaceDecl *getCanonicalDecl() override {
    return getOriginalNamespace();
  }
  const NamespaceDecl *getCanonicalDecl() const {
    return getOriginalNamespace();
  }

  SourceRange getSourceRange() const override LLVM_READONLY {
    return SourceRange(LocStart, RBraceLoc);
  }

  SourceLocation getLocStart() const LLVM_READONLY { return LocStart; }
  SourceLocation getRBraceLoc() const { return RBraceLoc; }
  void setLocStart(SourceLocation L) { LocStart = L; }
  void setRBraceLoc(SourceLocation L) { RBraceLoc = L; }

  // Implement isa/cast/dyncast/etc.
  static bool classof(const Decl *D) { return classofKind(D->getKind()); }
  static bool classofKind(Kind K) { return K == Namespace; }
  static DeclContext *castToDeclContext(const NamespaceDecl *D) {
    return static_cast<DeclContext *>(const_cast<NamespaceDecl*>(D));
  }
  static NamespaceDecl *castFromDeclContext(const DeclContext *DC) {
    return static_cast<NamespaceDecl *>(const_cast<DeclContext*>(DC));
  }
};

/// ValueDecl - Represent the declaration of a variable (in which case it is
/// an lvalue) a function (in which case it is a function designator) or
/// an enum constant.
class ValueDecl : public NamedDecl {
  QualType DeclType;

  void anchor() override;

protected:
  ValueDecl(Kind DK, DeclContext *DC, SourceLocation L,
            DeclarationName N, QualType T)
    : NamedDecl(DK, DC, L, N), DeclType(T) {}

public:
  QualType getType() const { return DeclType; }
  void setType(QualType newType) { DeclType = newType; }

  /// \brief Determine whether this symbol is weakly-imported,
  ///        or declared with the weak or weak-ref attr.
  bool isWeak() const;

  // Implement isa/cast/dyncast/etc.
  static bool classof(const Decl *D) { return classofKind(D->getKind()); }
  static bool classofKind(Kind K) { return K >= firstValue && K <= lastValue; }
};

/// QualifierInfo - A struct with extended info about a syntactic
/// name qualifier, to be used for the case of out-of-line declarations.
struct QualifierInfo {
  NestedNameSpecifierLoc QualifierLoc;

  /// NumTemplParamLists - The number of "outer" template parameter lists.
  /// The count includes all of the template parameter lists that were matched
  /// against the template-ids occurring into the NNS and possibly (in the
  /// case of an explicit specialization) a final "template <>".
  unsigned NumTemplParamLists = 0;

  /// TemplParamLists - A new-allocated array of size NumTemplParamLists,
  /// containing pointers to the "outer" template parameter lists.
  /// It includes all of the template parameter lists that were matched
  /// against the template-ids occurring into the NNS and possibly (in the
  /// case of an explicit specialization) a final "template <>".
  TemplateParameterList** TemplParamLists = nullptr;

  QualifierInfo() = default;
  QualifierInfo(const QualifierInfo &) = delete;
  QualifierInfo& operator=(const QualifierInfo &) = delete;

  /// setTemplateParameterListsInfo - Sets info about "outer" template
  /// parameter lists.
  void setTemplateParameterListsInfo(ASTContext &Context,
                                     ArrayRef<TemplateParameterList *> TPLists);
};

/// \brief Represents a ValueDecl that came out of a declarator.
/// Contains type source information through TypeSourceInfo.
class DeclaratorDecl : public ValueDecl {
  // A struct representing both a TInfo and a syntactic qualifier,
  // to be used for the (uncommon) case of out-of-line declarations.
  struct ExtInfo : public QualifierInfo {
    TypeSourceInfo *TInfo;
  };

  llvm::PointerUnion<TypeSourceInfo *, ExtInfo *> DeclInfo;

  /// InnerLocStart - The start of the source range for this declaration,
  /// ignoring outer template declarations.
  SourceLocation InnerLocStart;

  bool hasExtInfo() const { return DeclInfo.is<ExtInfo*>(); }
  ExtInfo *getExtInfo() { return DeclInfo.get<ExtInfo*>(); }
  const ExtInfo *getExtInfo() const { return DeclInfo.get<ExtInfo*>(); }

protected:
  DeclaratorDecl(Kind DK, DeclContext *DC, SourceLocation L,
                 DeclarationName N, QualType T, TypeSourceInfo *TInfo,
                 SourceLocation StartL)
      : ValueDecl(DK, DC, L, N, T), DeclInfo(TInfo), InnerLocStart(StartL) {}

public:
  friend class ASTDeclReader;
  friend class ASTDeclWriter;

  TypeSourceInfo *getTypeSourceInfo() const {
    return hasExtInfo()
      ? getExtInfo()->TInfo
      : DeclInfo.get<TypeSourceInfo*>();
  }

  void setTypeSourceInfo(TypeSourceInfo *TI) {
    if (hasExtInfo())
      getExtInfo()->TInfo = TI;
    else
      DeclInfo = TI;
  }

  /// getInnerLocStart - Return SourceLocation representing start of source
  /// range ignoring outer template declarations.
  SourceLocation getInnerLocStart() const { return InnerLocStart; }
  void setInnerLocStart(SourceLocation L) { InnerLocStart = L; }

  /// getOuterLocStart - Return SourceLocation representing start of source
  /// range taking into account any outer template declarations.
  SourceLocation getOuterLocStart() const;

  SourceRange getSourceRange() const override LLVM_READONLY;

  SourceLocation getLocStart() const LLVM_READONLY {
    return getOuterLocStart();
  }

  /// \brief Retrieve the nested-name-specifier that qualifies the name of this
  /// declaration, if it was present in the source.
  NestedNameSpecifier *getQualifier() const {
    return hasExtInfo() ? getExtInfo()->QualifierLoc.getNestedNameSpecifier()
                        : nullptr;
  }

  /// \brief Retrieve the nested-name-specifier (with source-location
  /// information) that qualifies the name of this declaration, if it was
  /// present in the source.
  NestedNameSpecifierLoc getQualifierLoc() const {
    return hasExtInfo() ? getExtInfo()->QualifierLoc
                        : NestedNameSpecifierLoc();
  }

  void setQualifierInfo(NestedNameSpecifierLoc QualifierLoc);

  unsigned getNumTemplateParameterLists() const {
    return hasExtInfo() ? getExtInfo()->NumTemplParamLists : 0;
  }

  TemplateParameterList *getTemplateParameterList(unsigned index) const {
    assert(index < getNumTemplateParameterLists());
    return getExtInfo()->TemplParamLists[index];
  }

  void setTemplateParameterListsInfo(ASTContext &Context,
                                     ArrayRef<TemplateParameterList *> TPLists);

  SourceLocation getTypeSpecStartLoc() const;

  // Implement isa/cast/dyncast/etc.
  static bool classof(const Decl *D) { return classofKind(D->getKind()); }
  static bool classofKind(Kind K) {
    return K >= firstDeclarator && K <= lastDeclarator;
  }
};

/// \brief Structure used to store a statement, the constant value to
/// which it was evaluated (if any), and whether or not the statement
/// is an integral constant expression (if known).
struct EvaluatedStmt {
  /// \brief Whether this statement was already evaluated.
  bool WasEvaluated : 1;

  /// \brief Whether this statement is being evaluated.
  bool IsEvaluating : 1;

  /// \brief Whether we already checked whether this statement was an
  /// integral constant expression.
  bool CheckedICE : 1;

  /// \brief Whether we are checking whether this statement is an
  /// integral constant expression.
  bool CheckingICE : 1;

  /// \brief Whether this statement is an integral constant expression,
  /// or in C++11, whether the statement is a constant expression. Only
  /// valid if CheckedICE is true.
  bool IsICE : 1;

  Stmt *Value;
  APValue Evaluated;

  EvaluatedStmt() : WasEvaluated(false), IsEvaluating(false), CheckedICE(false),
                    CheckingICE(false), IsICE(false) {}

};

/// VarDecl - An instance of this class is created to represent a variable
/// declaration or definition.
class VarDecl : public DeclaratorDecl, public Redeclarable<VarDecl> {
public:
  /// \brief Initialization styles.
  enum InitializationStyle {
    /// C-style initialization with assignment
    CInit,

    /// Call-style initialization (C++98)
    CallInit,

    /// Direct list-initialization (C++11)
    ListInit
  };

  /// \brief Kinds of thread-local storage.
  enum TLSKind {
    /// Not a TLS variable.
    TLS_None,

    /// TLS with a known-constant initializer.
    TLS_Static,

    /// TLS with a dynamic initializer.
    TLS_Dynamic
  };

  /// getStorageClassSpecifierString - Return the string used to
  /// specify the storage class \p SC.
  ///
  /// It is illegal to call this function with SC == None.
  static const char *getStorageClassSpecifierString(StorageClass SC);

protected:
  // A pointer union of Stmt * and EvaluatedStmt *. When an EvaluatedStmt, we
  // have allocated the auxiliary struct of information there.
  //
  // TODO: It is a bit unfortunate to use a PointerUnion inside the VarDecl for
  // this as *many* VarDecls are ParmVarDecls that don't have default
  // arguments. We could save some space by moving this pointer union to be
  // allocated in trailing space when necessary.
  using InitType = llvm::PointerUnion<Stmt *, EvaluatedStmt *>;

  /// \brief The initializer for this variable or, for a ParmVarDecl, the
  /// C++ default argument.
  mutable InitType Init;

private:
  friend class ASTDeclReader;
  friend class ASTNodeImporter;
  friend class StmtIteratorBase;

  class VarDeclBitfields {
    friend class ASTDeclReader;
    friend class VarDecl;

    unsigned SClass : 3;
    unsigned TSCSpec : 2;
    unsigned InitStyle : 2;
  };
  enum { NumVarDeclBits = 7 };

protected:
  enum { NumParameterIndexBits = 8 };

  enum DefaultArgKind {
    DAK_None,
    DAK_Unparsed,
    DAK_Uninstantiated,
    DAK_Normal
  };

  class ParmVarDeclBitfields {
    friend class ASTDeclReader;
    friend class ParmVarDecl;

    unsigned : NumVarDeclBits;

    /// Whether this parameter inherits a default argument from a
    /// prior declaration.
    unsigned HasInheritedDefaultArg : 1;

    /// Describes the kind of default argument for this parameter. By default
    /// this is none. If this is normal, then the default argument is stored in
    /// the \c VarDecl initializer expression unless we were unable to parse
    /// (even an invalid) expression for the default argument.
    unsigned DefaultArgKind : 2;

    /// Whether this parameter undergoes K&R argument promotion.
    unsigned IsKNRPromoted : 1;

    /// Whether this parameter is an ObjC method parameter or not.
    unsigned IsObjCMethodParam : 1;

    /// If IsObjCMethodParam, a Decl::ObjCDeclQualifier.
    /// Otherwise, the number of function parameter scopes enclosing
    /// the function parameter scope in which this parameter was
    /// declared.
    unsigned ScopeDepthOrObjCQuals : 7;

    /// The number of parameters preceding this parameter in the
    /// function parameter scope in which it was declared.
    unsigned ParameterIndex : NumParameterIndexBits;
  };

  class NonParmVarDeclBitfields {
    friend class ASTDeclReader;
    friend class ImplicitParamDecl;
    friend class VarDecl;

    unsigned : NumVarDeclBits;

    // FIXME: We need something similar to CXXRecordDecl::DefinitionData.
    /// \brief Whether this variable is a definition which was demoted due to
    /// module merge.
    unsigned IsThisDeclarationADemotedDefinition : 1;

    /// \brief Whether this variable is the exception variable in a C++ catch
    /// or an Objective-C @catch statement.
    unsigned ExceptionVar : 1;

    /// \brief Whether this local variable could be allocated in the return
    /// slot of its function, enabling the named return value optimization
    /// (NRVO).
    unsigned NRVOVariable : 1;

    /// \brief Whether this variable is the for-range-declaration in a C++0x
    /// for-range statement.
    unsigned CXXForRangeDecl : 1;

    /// \brief Whether this variable is an ARC pseudo-__strong
    /// variable;  see isARCPseudoStrong() for details.
    unsigned ARCPseudoStrong : 1;

    /// \brief Whether this variable is (C++1z) inline.
    unsigned IsInline : 1;

    /// \brief Whether this variable has (C++1z) inline explicitly specified.
    unsigned IsInlineSpecified : 1;

    /// \brief Whether this variable is (C++0x) constexpr.
    unsigned IsConstexpr : 1;

    /// \brief Whether this variable is the implicit variable for a lambda
    /// init-capture.
    unsigned IsInitCapture : 1;

    /// \brief Whether this local extern variable's previous declaration was
    /// declared in the same block scope. This controls whether we should merge
    /// the type of this declaration with its previous declaration.
    unsigned PreviousDeclInSameBlockScope : 1;

    /// Defines kind of the ImplicitParamDecl: 'this', 'self', 'vtt', '_cmd' or
    /// something else.
    unsigned ImplicitParamKind : 3;
  };

  union {
    unsigned AllBits;
    VarDeclBitfields VarDeclBits;
    ParmVarDeclBitfields ParmVarDeclBits;
    NonParmVarDeclBitfields NonParmVarDeclBits;
  };

  VarDecl(Kind DK, ASTContext &C, DeclContext *DC, SourceLocation StartLoc,
          SourceLocation IdLoc, IdentifierInfo *Id, QualType T,
          TypeSourceInfo *TInfo, StorageClass SC);

  using redeclarable_base = Redeclarable<VarDecl>;

  VarDecl *getNextRedeclarationImpl() override {
    return getNextRedeclaration();
  }

  VarDecl *getPreviousDeclImpl() override {
    return getPreviousDecl();
  }

  VarDecl *getMostRecentDeclImpl() override {
    return getMostRecentDecl();
  }

public:
  using redecl_range = redeclarable_base::redecl_range;
  using redecl_iterator = redeclarable_base::redecl_iterator;

  using redeclarable_base::redecls_begin;
  using redeclarable_base::redecls_end;
  using redeclarable_base::redecls;
  using redeclarable_base::getPreviousDecl;
  using redeclarable_base::getMostRecentDecl;
  using redeclarable_base::isFirstDecl;

  static VarDecl *Create(ASTContext &C, DeclContext *DC,
                         SourceLocation StartLoc, SourceLocation IdLoc,
                         IdentifierInfo *Id, QualType T, TypeSourceInfo *TInfo,
                         StorageClass S);

  static VarDecl *CreateDeserialized(ASTContext &C, unsigned ID);

  SourceRange getSourceRange() const override LLVM_READONLY;

  /// \brief Returns the storage class as written in the source. For the
  /// computed linkage of symbol, see getLinkage.
  StorageClass getStorageClass() const {
    return (StorageClass) VarDeclBits.SClass;
  }
  void setStorageClass(StorageClass SC);

  void setTSCSpec(ThreadStorageClassSpecifier TSC) {
    VarDeclBits.TSCSpec = TSC;
    assert(VarDeclBits.TSCSpec == TSC && "truncation");
  }
  ThreadStorageClassSpecifier getTSCSpec() const {
    return static_cast<ThreadStorageClassSpecifier>(VarDeclBits.TSCSpec);
  }
  TLSKind getTLSKind() const;

  /// hasLocalStorage - Returns true if a variable with function scope
  ///  is a non-static local variable.
  bool hasLocalStorage() const {
    if (getStorageClass() == SC_None) {
      // OpenCL v1.2 s6.5.3: The __constant or constant address space name is
      // used to describe variables allocated in global memory and which are
      // accessed inside a kernel(s) as read-only variables. As such, variables
      // in constant address space cannot have local storage.
      if (getType().getAddressSpace() == LangAS::opencl_constant)
        return false;
      // Second check is for C++11 [dcl.stc]p4.
      return !isFileVarDecl() && getTSCSpec() == TSCS_unspecified;
    }

    // Global Named Register (GNU extension)
    if (getStorageClass() == SC_Register && !isLocalVarDeclOrParm())
      return false;

    // Return true for:  Auto, Register.
    // Return false for: Extern, Static, PrivateExtern, OpenCLWorkGroupLocal.

    return getStorageClass() >= SC_Auto;
  }

  /// isStaticLocal - Returns true if a variable with function scope is a
  /// static local variable.
  bool isStaticLocal() const {
    return (getStorageClass() == SC_Static ||
            // C++11 [dcl.stc]p4
            (getStorageClass() == SC_None && getTSCSpec() == TSCS_thread_local))
      && !isFileVarDecl();
  }

  /// \brief Returns true if a variable has extern or __private_extern__
  /// storage.
  bool hasExternalStorage() const {
    return getStorageClass() == SC_Extern ||
           getStorageClass() == SC_PrivateExtern;
  }

  /// \brief Returns true for all variables that do not have local storage.
  ///
  /// This includes all global variables as well as static variables declared
  /// within a function.
  bool hasGlobalStorage() const { return !hasLocalStorage(); }

  /// \brief Get the storage duration of this variable, per C++ [basic.stc].
  StorageDuration getStorageDuration() const {
    return hasLocalStorage() ? SD_Automatic :
           getTSCSpec() ? SD_Thread : SD_Static;
  }

  /// \brief Compute the language linkage.
  LanguageLinkage getLanguageLinkage() const;

  /// \brief Determines whether this variable is a variable with
  /// external, C linkage.
  bool isExternC() const;

  /// \brief Determines whether this variable's context is, or is nested within,
  /// a C++ extern "C" linkage spec.
  bool isInExternCContext() const;

  /// \brief Determines whether this variable's context is, or is nested within,
  /// a C++ extern "C++" linkage spec.
  bool isInExternCXXContext() const;

  /// isLocalVarDecl - Returns true for local variable declarations
  /// other than parameters.  Note that this includes static variables
  /// inside of functions. It also includes variables inside blocks.
  ///
  ///   void foo() { int x; static int y; extern int z; }
  bool isLocalVarDecl() const {
    if (getKind() != Decl::Var && getKind() != Decl::Decomposition)
      return false;
    if (const DeclContext *DC = getLexicalDeclContext())
      return DC->getRedeclContext()->isFunctionOrMethod();
    return false;
  }

  /// \brief Similar to isLocalVarDecl but also includes parameters.
  bool isLocalVarDeclOrParm() const {
    return isLocalVarDecl() || getKind() == Decl::ParmVar;
  }

  /// isFunctionOrMethodVarDecl - Similar to isLocalVarDecl, but
  /// excludes variables declared in blocks.
  bool isFunctionOrMethodVarDecl() const {
    if (getKind() != Decl::Var && getKind() != Decl::Decomposition)
      return false;
    const DeclContext *DC = getLexicalDeclContext()->getRedeclContext();
    return DC->isFunctionOrMethod() && DC->getDeclKind() != Decl::Block;
  }

  /// \brief Determines whether this is a static data member.
  ///
  /// This will only be true in C++, and applies to, e.g., the
  /// variable 'x' in:
  /// \code
  /// struct S {
  ///   static int x;
  /// };
  /// \endcode
  bool isStaticDataMember() const {
    // If it wasn't static, it would be a FieldDecl.
    return getKind() != Decl::ParmVar && getDeclContext()->isRecord();
  }

  VarDecl *getCanonicalDecl() override;
  const VarDecl *getCanonicalDecl() const {
    return const_cast<VarDecl*>(this)->getCanonicalDecl();
  }

  enum DefinitionKind {
    /// This declaration is only a declaration.
    DeclarationOnly,

    /// This declaration is a tentative definition.
    TentativeDefinition,

    /// This declaration is definitely a definition.
    Definition
  };

  /// \brief Check whether this declaration is a definition. If this could be
  /// a tentative definition (in C), don't check whether there's an overriding
  /// definition.
  DefinitionKind isThisDeclarationADefinition(ASTContext &) const;
  DefinitionKind isThisDeclarationADefinition() const {
    return isThisDeclarationADefinition(getASTContext());
  }

  /// \brief Check whether this variable is defined in this
  /// translation unit.
  DefinitionKind hasDefinition(ASTContext &) const;
  DefinitionKind hasDefinition() const {
    return hasDefinition(getASTContext());
  }

  /// \brief Get the tentative definition that acts as the real definition in
  /// a TU. Returns null if there is a proper definition available.
  VarDecl *getActingDefinition();
  const VarDecl *getActingDefinition() const {
    return const_cast<VarDecl*>(this)->getActingDefinition();
  }

  /// \brief Get the real (not just tentative) definition for this declaration.
  VarDecl *getDefinition(ASTContext &);
  const VarDecl *getDefinition(ASTContext &C) const {
    return const_cast<VarDecl*>(this)->getDefinition(C);
  }
  VarDecl *getDefinition() {
    return getDefinition(getASTContext());
  }
  const VarDecl *getDefinition() const {
    return const_cast<VarDecl*>(this)->getDefinition();
  }

  /// \brief Determine whether this is or was instantiated from an out-of-line
  /// definition of a static data member.
  bool isOutOfLine() const override;

  /// isFileVarDecl - Returns true for file scoped variable declaration.
  bool isFileVarDecl() const {
    Kind K = getKind();
    if (K == ParmVar || K == ImplicitParam)
      return false;

    if (getLexicalDeclContext()->getRedeclContext()->isFileContext())
      return true;

    if (isStaticDataMember())
      return true;

    return false;
  }

  /// getAnyInitializer - Get the initializer for this variable, no matter which
  /// declaration it is attached to.
  const Expr *getAnyInitializer() const {
    const VarDecl *D;
    return getAnyInitializer(D);
  }

  /// getAnyInitializer - Get the initializer for this variable, no matter which
  /// declaration it is attached to. Also get that declaration.
  const Expr *getAnyInitializer(const VarDecl *&D) const;

  bool hasInit() const;
  const Expr *getInit() const {
    return const_cast<VarDecl *>(this)->getInit();
  }
  Expr *getInit();

  /// \brief Retrieve the address of the initializer expression.
  Stmt **getInitAddress();

  void setInit(Expr *I);

  /// \brief Determine whether this variable's value can be used in a
  /// constant expression, according to the relevant language standard.
  /// This only checks properties of the declaration, and does not check
  /// whether the initializer is in fact a constant expression.
  bool isUsableInConstantExpressions(ASTContext &C) const;

  EvaluatedStmt *ensureEvaluatedStmt() const;

  /// \brief Attempt to evaluate the value of the initializer attached to this
  /// declaration, and produce notes explaining why it cannot be evaluated or is
  /// not a constant expression. Returns a pointer to the value if evaluation
  /// succeeded, 0 otherwise.
  APValue *evaluateValue() const;
  APValue *evaluateValue(SmallVectorImpl<PartialDiagnosticAt> &Notes) const;

  /// \brief Return the already-evaluated value of this variable's
  /// initializer, or NULL if the value is not yet known. Returns pointer
  /// to untyped APValue if the value could not be evaluated.
  APValue *getEvaluatedValue() const;

  /// \brief Determines whether it is already known whether the
  /// initializer is an integral constant expression or not.
  bool isInitKnownICE() const;

  /// \brief Determines whether the initializer is an integral constant
  /// expression, or in C++11, whether the initializer is a constant
  /// expression.
  ///
  /// \pre isInitKnownICE()
  bool isInitICE() const;

  /// \brief Determine whether the value of the initializer attached to this
  /// declaration is an integral constant expression.
  bool checkInitIsICE() const;

  void setInitStyle(InitializationStyle Style) {
    VarDeclBits.InitStyle = Style;
  }

  /// \brief The style of initialization for this declaration.
  ///
  /// C-style initialization is "int x = 1;". Call-style initialization is
  /// a C++98 direct-initializer, e.g. "int x(1);". The Init expression will be
  /// the expression inside the parens or a "ClassType(a,b,c)" class constructor
  /// expression for class types. List-style initialization is C++11 syntax,
  /// e.g. "int x{1};". Clients can distinguish between different forms of
  /// initialization by checking this value. In particular, "int x = {1};" is
  /// C-style, "int x({1})" is call-style, and "int x{1};" is list-style; the
  /// Init expression in all three cases is an InitListExpr.
  InitializationStyle getInitStyle() const {
    return static_cast<InitializationStyle>(VarDeclBits.InitStyle);
  }

  /// \brief Whether the initializer is a direct-initializer (list or call).
  bool isDirectInit() const {
    return getInitStyle() != CInit;
  }

  /// \brief If this definition should pretend to be a declaration.
  bool isThisDeclarationADemotedDefinition() const {
    return isa<ParmVarDecl>(this) ? false :
      NonParmVarDeclBits.IsThisDeclarationADemotedDefinition;
  }

  /// \brief This is a definition which should be demoted to a declaration.
  ///
  /// In some cases (mostly module merging) we can end up with two visible
  /// definitions one of which needs to be demoted to a declaration to keep
  /// the AST invariants.
  void demoteThisDefinitionToDeclaration() {
    assert(isThisDeclarationADefinition() && "Not a definition!");
    assert(!isa<ParmVarDecl>(this) && "Cannot demote ParmVarDecls!");
    NonParmVarDeclBits.IsThisDeclarationADemotedDefinition = 1;
  }

  /// \brief Determine whether this variable is the exception variable in a
  /// C++ catch statememt or an Objective-C \@catch statement.
  bool isExceptionVariable() const {
    return isa<ParmVarDecl>(this) ? false : NonParmVarDeclBits.ExceptionVar;
  }
  void setExceptionVariable(bool EV) {
    assert(!isa<ParmVarDecl>(this));
    NonParmVarDeclBits.ExceptionVar = EV;
  }

  /// \brief Determine whether this local variable can be used with the named
  /// return value optimization (NRVO).
  ///
  /// The named return value optimization (NRVO) works by marking certain
  /// non-volatile local variables of class type as NRVO objects. These
  /// locals can be allocated within the return slot of their containing
  /// function, in which case there is no need to copy the object to the
  /// return slot when returning from the function. Within the function body,
  /// each return that returns the NRVO object will have this variable as its
  /// NRVO candidate.
  bool isNRVOVariable() const {
    return isa<ParmVarDecl>(this) ? false : NonParmVarDeclBits.NRVOVariable;
  }
  void setNRVOVariable(bool NRVO) {
    assert(!isa<ParmVarDecl>(this));
    NonParmVarDeclBits.NRVOVariable = NRVO;
  }

  /// \brief Determine whether this variable is the for-range-declaration in
  /// a C++0x for-range statement.
  bool isCXXForRangeDecl() const {
    return isa<ParmVarDecl>(this) ? false : NonParmVarDeclBits.CXXForRangeDecl;
  }
  void setCXXForRangeDecl(bool FRD) {
    assert(!isa<ParmVarDecl>(this));
    NonParmVarDeclBits.CXXForRangeDecl = FRD;
  }

  /// \brief Determine whether this variable is an ARC pseudo-__strong
  /// variable.  A pseudo-__strong variable has a __strong-qualified
  /// type but does not actually retain the object written into it.
  /// Generally such variables are also 'const' for safety.
  bool isARCPseudoStrong() const {
    return isa<ParmVarDecl>(this) ? false : NonParmVarDeclBits.ARCPseudoStrong;
  }
  void setARCPseudoStrong(bool ps) {
    assert(!isa<ParmVarDecl>(this));
    NonParmVarDeclBits.ARCPseudoStrong = ps;
  }

  /// Whether this variable is (C++1z) inline.
  bool isInline() const {
    return isa<ParmVarDecl>(this) ? false : NonParmVarDeclBits.IsInline;
  }
  bool isInlineSpecified() const {
    return isa<ParmVarDecl>(this) ? false
                                  : NonParmVarDeclBits.IsInlineSpecified;
  }
  void setInlineSpecified() {
    assert(!isa<ParmVarDecl>(this));
    NonParmVarDeclBits.IsInline = true;
    NonParmVarDeclBits.IsInlineSpecified = true;
  }
  void setImplicitlyInline() {
    assert(!isa<ParmVarDecl>(this));
    NonParmVarDeclBits.IsInline = true;
  }

  /// Whether this variable is (C++11) constexpr.
  bool isConstexpr() const {
    return isa<ParmVarDecl>(this) ? false : NonParmVarDeclBits.IsConstexpr;
  }
  void setConstexpr(bool IC) {
    assert(!isa<ParmVarDecl>(this));
    NonParmVarDeclBits.IsConstexpr = IC;
  }

  /// Whether this variable is the implicit variable for a lambda init-capture.
  bool isInitCapture() const {
    return isa<ParmVarDecl>(this) ? false : NonParmVarDeclBits.IsInitCapture;
  }
  void setInitCapture(bool IC) {
    assert(!isa<ParmVarDecl>(this));
    NonParmVarDeclBits.IsInitCapture = IC;
  }

  /// Whether this local extern variable declaration's previous declaration
  /// was declared in the same block scope. Only correct in C++.
  bool isPreviousDeclInSameBlockScope() const {
    return isa<ParmVarDecl>(this)
               ? false
               : NonParmVarDeclBits.PreviousDeclInSameBlockScope;
  }
  void setPreviousDeclInSameBlockScope(bool Same) {
    assert(!isa<ParmVarDecl>(this));
    NonParmVarDeclBits.PreviousDeclInSameBlockScope = Same;
  }

  /// \brief Retrieve the variable declaration from which this variable could
  /// be instantiated, if it is an instantiation (rather than a non-template).
  VarDecl *getTemplateInstantiationPattern() const;

  /// \brief If this variable is an instantiated static data member of a
  /// class template specialization, returns the templated static data member
  /// from which it was instantiated.
  VarDecl *getInstantiatedFromStaticDataMember() const;

  /// \brief If this variable is an instantiation of a variable template or a
  /// static data member of a class template, determine what kind of
  /// template specialization or instantiation this is.
  TemplateSpecializationKind getTemplateSpecializationKind() const;

  /// \brief If this variable is an instantiation of a variable template or a
  /// static data member of a class template, determine its point of
  /// instantiation.
  SourceLocation getPointOfInstantiation() const;

  /// \brief If this variable is an instantiation of a static data member of a
  /// class template specialization, retrieves the member specialization
  /// information.
  MemberSpecializationInfo *getMemberSpecializationInfo() const;

  /// \brief For a static data member that was instantiated from a static
  /// data member of a class template, set the template specialiation kind.
  void setTemplateSpecializationKind(TemplateSpecializationKind TSK,
                        SourceLocation PointOfInstantiation = SourceLocation());

  /// \brief Specify that this variable is an instantiation of the
  /// static data member VD.
  void setInstantiationOfStaticDataMember(VarDecl *VD,
                                          TemplateSpecializationKind TSK);

  /// \brief Retrieves the variable template that is described by this
  /// variable declaration.
  ///
  /// Every variable template is represented as a VarTemplateDecl and a
  /// VarDecl. The former contains template properties (such as
  /// the template parameter lists) while the latter contains the
  /// actual description of the template's
  /// contents. VarTemplateDecl::getTemplatedDecl() retrieves the
  /// VarDecl that from a VarTemplateDecl, while
  /// getDescribedVarTemplate() retrieves the VarTemplateDecl from
  /// a VarDecl.
  VarTemplateDecl *getDescribedVarTemplate() const;

  void setDescribedVarTemplate(VarTemplateDecl *Template);

  // Implement isa/cast/dyncast/etc.
  static bool classof(const Decl *D) { return classofKind(D->getKind()); }
  static bool classofKind(Kind K) { return K >= firstVar && K <= lastVar; }
};

class ImplicitParamDecl : public VarDecl {
  void anchor() override;

public:
  /// Defines the kind of the implicit parameter: is this an implicit parameter
  /// with pointer to 'this', 'self', '_cmd', virtual table pointers, captured
  /// context or something else.
  enum ImplicitParamKind : unsigned {
    /// Parameter for Objective-C 'self' argument
    ObjCSelf,

    /// Parameter for Objective-C '_cmd' argument
    ObjCCmd,

    /// Parameter for C++ 'this' argument
    CXXThis,

    /// Parameter for C++ virtual table pointers
    CXXVTT,

    /// Parameter for captured context
    CapturedContext,

    /// Other implicit parameter
    Other,
  };

  /// Create implicit parameter.
  static ImplicitParamDecl *Create(ASTContext &C, DeclContext *DC,
                                   SourceLocation IdLoc, IdentifierInfo *Id,
                                   QualType T, ImplicitParamKind ParamKind);
  static ImplicitParamDecl *Create(ASTContext &C, QualType T,
                                   ImplicitParamKind ParamKind);

  static ImplicitParamDecl *CreateDeserialized(ASTContext &C, unsigned ID);

  ImplicitParamDecl(ASTContext &C, DeclContext *DC, SourceLocation IdLoc,
                    IdentifierInfo *Id, QualType Type,
                    ImplicitParamKind ParamKind)
      : VarDecl(ImplicitParam, C, DC, IdLoc, IdLoc, Id, Type,
                /*TInfo=*/nullptr, SC_None) {
    NonParmVarDeclBits.ImplicitParamKind = ParamKind;
    setImplicit();
  }

  ImplicitParamDecl(ASTContext &C, QualType Type, ImplicitParamKind ParamKind)
      : VarDecl(ImplicitParam, C, /*DC=*/nullptr, SourceLocation(),
                SourceLocation(), /*Id=*/nullptr, Type,
                /*TInfo=*/nullptr, SC_None) {
    NonParmVarDeclBits.ImplicitParamKind = ParamKind;
    setImplicit();
  }

  /// Returns the implicit parameter kind.
  ImplicitParamKind getParameterKind() const {
    return static_cast<ImplicitParamKind>(NonParmVarDeclBits.ImplicitParamKind);
  }

  // Implement isa/cast/dyncast/etc.
  static bool classof(const Decl *D) { return classofKind(D->getKind()); }
  static bool classofKind(Kind K) { return K == ImplicitParam; }
};

/// ParmVarDecl - Represents a parameter to a function.
class ParmVarDecl : public VarDecl {
public:
  enum { MaxFunctionScopeDepth = 255 };
  enum { MaxFunctionScopeIndex = 255 };

protected:
  ParmVarDecl(Kind DK, ASTContext &C, DeclContext *DC, SourceLocation StartLoc,
              SourceLocation IdLoc, IdentifierInfo *Id, QualType T,
              TypeSourceInfo *TInfo, StorageClass S, Expr *DefArg)
      : VarDecl(DK, C, DC, StartLoc, IdLoc, Id, T, TInfo, S) {
    assert(ParmVarDeclBits.HasInheritedDefaultArg == false);
    assert(ParmVarDeclBits.DefaultArgKind == DAK_None);
    assert(ParmVarDeclBits.IsKNRPromoted == false);
    assert(ParmVarDeclBits.IsObjCMethodParam == false);
    setDefaultArg(DefArg);
  }

public:
  static ParmVarDecl *Create(ASTContext &C, DeclContext *DC,
                             SourceLocation StartLoc,
                             SourceLocation IdLoc, IdentifierInfo *Id,
                             QualType T, TypeSourceInfo *TInfo,
                             StorageClass S, Expr *DefArg);

  static ParmVarDecl *CreateDeserialized(ASTContext &C, unsigned ID);

  SourceRange getSourceRange() const override LLVM_READONLY;

  void setObjCMethodScopeInfo(unsigned parameterIndex) {
    ParmVarDeclBits.IsObjCMethodParam = true;
    setParameterIndex(parameterIndex);
  }

  void setScopeInfo(unsigned scopeDepth, unsigned parameterIndex) {
    assert(!ParmVarDeclBits.IsObjCMethodParam);

    ParmVarDeclBits.ScopeDepthOrObjCQuals = scopeDepth;
    assert(ParmVarDeclBits.ScopeDepthOrObjCQuals == scopeDepth
           && "truncation!");

    setParameterIndex(parameterIndex);
  }

  bool isObjCMethodParameter() const {
    return ParmVarDeclBits.IsObjCMethodParam;
  }

  unsigned getFunctionScopeDepth() const {
    if (ParmVarDeclBits.IsObjCMethodParam) return 0;
    return ParmVarDeclBits.ScopeDepthOrObjCQuals;
  }

  /// Returns the index of this parameter in its prototype or method scope.
  unsigned getFunctionScopeIndex() const {
    return getParameterIndex();
  }

  ObjCDeclQualifier getObjCDeclQualifier() const {
    if (!ParmVarDeclBits.IsObjCMethodParam) return OBJC_TQ_None;
    return ObjCDeclQualifier(ParmVarDeclBits.ScopeDepthOrObjCQuals);
  }
  void setObjCDeclQualifier(ObjCDeclQualifier QTVal) {
    assert(ParmVarDeclBits.IsObjCMethodParam);
    ParmVarDeclBits.ScopeDepthOrObjCQuals = QTVal;
  }

  /// True if the value passed to this parameter must undergo
  /// K&R-style default argument promotion:
  ///
  /// C99 6.5.2.2.
  ///   If the expression that denotes the called function has a type
  ///   that does not include a prototype, the integer promotions are
  ///   performed on each argument, and arguments that have type float
  ///   are promoted to double.
  bool isKNRPromoted() const {
    return ParmVarDeclBits.IsKNRPromoted;
  }
  void setKNRPromoted(bool promoted) {
    ParmVarDeclBits.IsKNRPromoted = promoted;
  }

  Expr *getDefaultArg();
  const Expr *getDefaultArg() const {
    return const_cast<ParmVarDecl *>(this)->getDefaultArg();
  }

  void setDefaultArg(Expr *defarg);

  /// \brief Retrieve the source range that covers the entire default
  /// argument.
  SourceRange getDefaultArgRange() const;
  void setUninstantiatedDefaultArg(Expr *arg);
  Expr *getUninstantiatedDefaultArg();
  const Expr *getUninstantiatedDefaultArg() const {
    return const_cast<ParmVarDecl *>(this)->getUninstantiatedDefaultArg();
  }

  /// hasDefaultArg - Determines whether this parameter has a default argument,
  /// either parsed or not.
  bool hasDefaultArg() const;

  /// hasUnparsedDefaultArg - Determines whether this parameter has a
  /// default argument that has not yet been parsed. This will occur
  /// during the processing of a C++ class whose member functions have
  /// default arguments, e.g.,
  /// @code
  ///   class X {
  ///   public:
  ///     void f(int x = 17); // x has an unparsed default argument now
  ///   }; // x has a regular default argument now
  /// @endcode
  bool hasUnparsedDefaultArg() const {
    return ParmVarDeclBits.DefaultArgKind == DAK_Unparsed;
  }

  bool hasUninstantiatedDefaultArg() const {
    return ParmVarDeclBits.DefaultArgKind == DAK_Uninstantiated;
  }

  /// setUnparsedDefaultArg - Specify that this parameter has an
  /// unparsed default argument. The argument will be replaced with a
  /// real default argument via setDefaultArg when the class
  /// definition enclosing the function declaration that owns this
  /// default argument is completed.
  void setUnparsedDefaultArg() {
    ParmVarDeclBits.DefaultArgKind = DAK_Unparsed;
  }

  bool hasInheritedDefaultArg() const {
    return ParmVarDeclBits.HasInheritedDefaultArg;
  }

  void setHasInheritedDefaultArg(bool I = true) {
    ParmVarDeclBits.HasInheritedDefaultArg = I;
  }

  QualType getOriginalType() const;

  /// \brief Determine whether this parameter is actually a function
  /// parameter pack.
  bool isParameterPack() const;

  /// setOwningFunction - Sets the function declaration that owns this
  /// ParmVarDecl. Since ParmVarDecls are often created before the
  /// FunctionDecls that own them, this routine is required to update
  /// the DeclContext appropriately.
  void setOwningFunction(DeclContext *FD) { setDeclContext(FD); }

  // Implement isa/cast/dyncast/etc.
  static bool classof(const Decl *D) { return classofKind(D->getKind()); }
  static bool classofKind(Kind K) { return K == ParmVar; }

private:
  enum { ParameterIndexSentinel = (1 << NumParameterIndexBits) - 1 };

  void setParameterIndex(unsigned parameterIndex) {
    if (parameterIndex >= ParameterIndexSentinel) {
      setParameterIndexLarge(parameterIndex);
      return;
    }

    ParmVarDeclBits.ParameterIndex = parameterIndex;
    assert(ParmVarDeclBits.ParameterIndex == parameterIndex && "truncation!");
  }
  unsigned getParameterIndex() const {
    unsigned d = ParmVarDeclBits.ParameterIndex;
    return d == ParameterIndexSentinel ? getParameterIndexLarge() : d;
  }

  void setParameterIndexLarge(unsigned parameterIndex);
  unsigned getParameterIndexLarge() const;
};

/// An instance of this class is created to represent a function declaration or
/// definition.
///
/// Since a given function can be declared several times in a program,
/// there may be several FunctionDecls that correspond to that
/// function. Only one of those FunctionDecls will be found when
/// traversing the list of declarations in the context of the
/// FunctionDecl (e.g., the translation unit); this FunctionDecl
/// contains all of the information known about the function. Other,
/// previous declarations of the function are available via the
/// getPreviousDecl() chain.
class FunctionDecl : public DeclaratorDecl, public DeclContext,
                     public Redeclarable<FunctionDecl> {
public:
  /// \brief The kind of templated function a FunctionDecl can be.
  enum TemplatedKind {
    TK_NonTemplate,
    TK_FunctionTemplate,
    TK_MemberSpecialization,
    TK_FunctionTemplateSpecialization,
    TK_DependentFunctionTemplateSpecialization
  };

private:
  /// ParamInfo - new[]'d array of pointers to VarDecls for the formal
  /// parameters of this function.  This is null if a prototype or if there are
  /// no formals.
  ParmVarDecl **ParamInfo = nullptr;

  LazyDeclStmtPtr Body;

  // FIXME: This can be packed into the bitfields in DeclContext.
  // NOTE: VC++ packs bitfields poorly if the types differ.
  unsigned SClass : 3;
  unsigned IsInline : 1;
  unsigned IsInlineSpecified : 1;

protected:
  // This is shared by CXXConstructorDecl, CXXConversionDecl, and
  // CXXDeductionGuideDecl.
  unsigned IsExplicitSpecified : 1;

private:
  unsigned IsVirtualAsWritten : 1;
  unsigned IsPure : 1;
  unsigned HasInheritedPrototype : 1;
  unsigned HasWrittenPrototype : 1;
  unsigned IsDeleted : 1;
  unsigned IsTrivial : 1; // sunk from CXXMethodDecl
  unsigned IsDefaulted : 1; // sunk from CXXMethoDecl
  unsigned IsExplicitlyDefaulted : 1; //sunk from CXXMethodDecl
  unsigned HasImplicitReturnZero : 1;
  unsigned IsLateTemplateParsed : 1;
  unsigned IsConstexpr : 1;
  unsigned InstantiationIsPending : 1;

  /// \brief Indicates if the function uses __try.
  unsigned UsesSEHTry : 1;

  /// \brief Indicates if the function was a definition but its body was
  /// skipped.
  unsigned HasSkippedBody : 1;

  /// Indicates if the function declaration will have a body, once we're done
  /// parsing it.
  unsigned WillHaveBody : 1;

  /// Indicates that this function is a multiversioned function using attribute
  /// 'target'.
  unsigned IsMultiVersion : 1;

protected:
  /// [C++17] Only used by CXXDeductionGuideDecl. Declared here to avoid
  /// increasing the size of CXXDeductionGuideDecl by the size of an unsigned
  /// int as opposed to adding a single bit to FunctionDecl.
  /// Indicates that the Deduction Guide is the implicitly generated 'copy
  /// deduction candidate' (is used during overload resolution).
  unsigned IsCopyDeductionCandidate : 1;

private:

  /// Store the ODRHash after first calculation.
  unsigned HasODRHash : 1;
  unsigned ODRHash;

  /// \brief End part of this FunctionDecl's source range.
  ///
  /// We could compute the full range in getSourceRange(). However, when we're
  /// dealing with a function definition deserialized from a PCH/AST file,
  /// we can only compute the full range once the function body has been
  /// de-serialized, so it's far better to have the (sometimes-redundant)
  /// EndRangeLoc.
  SourceLocation EndRangeLoc;

  /// \brief The template or declaration that this declaration
  /// describes or was instantiated from, respectively.
  ///
  /// For non-templates, this value will be NULL. For function
  /// declarations that describe a function template, this will be a
  /// pointer to a FunctionTemplateDecl. For member functions
  /// of class template specializations, this will be a MemberSpecializationInfo
  /// pointer containing information about the specialization.
  /// For function template specializations, this will be a
  /// FunctionTemplateSpecializationInfo, which contains information about
  /// the template being specialized and the template arguments involved in
  /// that specialization.
  llvm::PointerUnion4<FunctionTemplateDecl *,
                      MemberSpecializationInfo *,
                      FunctionTemplateSpecializationInfo *,
                      DependentFunctionTemplateSpecializationInfo *>
    TemplateOrSpecialization;

  /// Provides source/type location info for the declaration name embedded in
  /// the DeclaratorDecl base class.
  DeclarationNameLoc DNLoc;

  /// \brief Specify that this function declaration is actually a function
  /// template specialization.
  ///
  /// \param C the ASTContext.
  ///
  /// \param Template the function template that this function template
  /// specialization specializes.
  ///
  /// \param TemplateArgs the template arguments that produced this
  /// function template specialization from the template.
  ///
  /// \param InsertPos If non-NULL, the position in the function template
  /// specialization set where the function template specialization data will
  /// be inserted.
  ///
  /// \param TSK the kind of template specialization this is.
  ///
  /// \param TemplateArgsAsWritten location info of template arguments.
  ///
  /// \param PointOfInstantiation point at which the function template
  /// specialization was first instantiated.
  void setFunctionTemplateSpecialization(ASTContext &C,
                                         FunctionTemplateDecl *Template,
                                       const TemplateArgumentList *TemplateArgs,
                                         void *InsertPos,
                                         TemplateSpecializationKind TSK,
                          const TemplateArgumentListInfo *TemplateArgsAsWritten,
                                         SourceLocation PointOfInstantiation);

  /// \brief Specify that this record is an instantiation of the
  /// member function FD.
  void setInstantiationOfMemberFunction(ASTContext &C, FunctionDecl *FD,
                                        TemplateSpecializationKind TSK);

  void setParams(ASTContext &C, ArrayRef<ParmVarDecl *> NewParamInfo);

protected:
  FunctionDecl(Kind DK, ASTContext &C, DeclContext *DC, SourceLocation StartLoc,
               const DeclarationNameInfo &NameInfo, QualType T,
               TypeSourceInfo *TInfo, StorageClass S, bool isInlineSpecified,
               bool isConstexprSpecified)
      : DeclaratorDecl(DK, DC, NameInfo.getLoc(), NameInfo.getName(), T, TInfo,
                       StartLoc),
        DeclContext(DK), redeclarable_base(C), SClass(S),
        IsInline(isInlineSpecified), IsInlineSpecified(isInlineSpecified),
        IsExplicitSpecified(false), IsVirtualAsWritten(false), IsPure(false),
        HasInheritedPrototype(false), HasWrittenPrototype(true),
        IsDeleted(false), IsTrivial(false), IsDefaulted(false),
        IsExplicitlyDefaulted(false), HasImplicitReturnZero(false),
        IsLateTemplateParsed(false), IsConstexpr(isConstexprSpecified),
        InstantiationIsPending(false), UsesSEHTry(false), HasSkippedBody(false),
        WillHaveBody(false), IsMultiVersion(false),
        IsCopyDeductionCandidate(false), HasODRHash(false), ODRHash(0),
        EndRangeLoc(NameInfo.getEndLoc()), DNLoc(NameInfo.getInfo()) {}

  using redeclarable_base = Redeclarable<FunctionDecl>;

  FunctionDecl *getNextRedeclarationImpl() override {
    return getNextRedeclaration();
  }

  FunctionDecl *getPreviousDeclImpl() override {
    return getPreviousDecl();
  }

  FunctionDecl *getMostRecentDeclImpl() override {
    return getMostRecentDecl();
  }

public:
  friend class ASTDeclReader;
  friend class ASTDeclWriter;

  using redecl_range = redeclarable_base::redecl_range;
  using redecl_iterator = redeclarable_base::redecl_iterator;

  using redeclarable_base::redecls_begin;
  using redeclarable_base::redecls_end;
  using redeclarable_base::redecls;
  using redeclarable_base::getPreviousDecl;
  using redeclarable_base::getMostRecentDecl;
  using redeclarable_base::isFirstDecl;

  static FunctionDecl *Create(ASTContext &C, DeclContext *DC,
                              SourceLocation StartLoc, SourceLocation NLoc,
                              DeclarationName N, QualType T,
                              TypeSourceInfo *TInfo,
                              StorageClass SC,
                              bool isInlineSpecified = false,
                              bool hasWrittenPrototype = true,
                              bool isConstexprSpecified = false) {
    DeclarationNameInfo NameInfo(N, NLoc);
    return FunctionDecl::Create(C, DC, StartLoc, NameInfo, T, TInfo,
                                SC,
                                isInlineSpecified, hasWrittenPrototype,
                                isConstexprSpecified);
  }

  static FunctionDecl *Create(ASTContext &C, DeclContext *DC,
                              SourceLocation StartLoc,
                              const DeclarationNameInfo &NameInfo,
                              QualType T, TypeSourceInfo *TInfo,
                              StorageClass SC,
                              bool isInlineSpecified,
                              bool hasWrittenPrototype,
                              bool isConstexprSpecified = false);

  static FunctionDecl *CreateDeserialized(ASTContext &C, unsigned ID);
                       
  DeclarationNameInfo getNameInfo() const {
    return DeclarationNameInfo(getDeclName(), getLocation(), DNLoc);
  }

  void getNameForDiagnostic(raw_ostream &OS, const PrintingPolicy &Policy,
                            bool Qualified) const override;

  void setRangeEnd(SourceLocation E) { EndRangeLoc = E; }

  SourceRange getSourceRange() const override LLVM_READONLY;

  /// \brief Returns true if the function has a body (definition). The
  /// function body might be in any of the (re-)declarations of this
  /// function. The variant that accepts a FunctionDecl pointer will
  /// set that function declaration to the actual declaration
  /// containing the body (if there is one).
  bool hasBody(const FunctionDecl *&Definition) const;

  bool hasBody() const override {
    const FunctionDecl* Definition;
    return hasBody(Definition);
  }

  /// Returns whether the function has a trivial body that does not require any
  /// specific codegen.
  bool hasTrivialBody() const;

  /// Returns true if the function is defined at all, including a deleted
  /// definition. Except for the behavior when the function is deleted, behaves
  /// like hasBody.
  bool isDefined(const FunctionDecl *&Definition) const;

  virtual bool isDefined() const {
    const FunctionDecl* Definition;
    return isDefined(Definition);
  }

  /// \brief Get the definition for this declaration.
  FunctionDecl *getDefinition() {
    const FunctionDecl *Definition;
    if (isDefined(Definition))
      return const_cast<FunctionDecl *>(Definition);
    return nullptr;
  }
  const FunctionDecl *getDefinition() const {
    return const_cast<FunctionDecl *>(this)->getDefinition();
  }

  /// Retrieve the body (definition) of the function. The function body might be
  /// in any of the (re-)declarations of this function. The variant that accepts
  /// a FunctionDecl pointer will set that function declaration to the actual
  /// declaration containing the body (if there is one).
  /// NOTE: For checking if there is a body, use hasBody() instead, to avoid
  /// unnecessary AST de-serialization of the body.
  Stmt *getBody(const FunctionDecl *&Definition) const;

  Stmt *getBody() const override {
    const FunctionDecl* Definition;
    return getBody(Definition);
  }

  /// Returns whether this specific declaration of the function is also a
  /// definition that does not contain uninstantiated body.
  ///
  /// This does not determine whether the function has been defined (e.g., in a
  /// previous definition); for that information, use isDefined.
  bool isThisDeclarationADefinition() const {
    return IsDeleted || IsDefaulted || Body || HasSkippedBody ||
           IsLateTemplateParsed || WillHaveBody || hasDefiningAttr();
  }

  /// Returns whether this specific declaration of the function has a body -
  /// that is, if it is a non-deleted definition.
  bool doesThisDeclarationHaveABody() const {
    return Body || IsLateTemplateParsed;
  }

  void setBody(Stmt *B);
  void setLazyBody(uint64_t Offset) { Body = Offset; }

  /// Whether this function is variadic.
  bool isVariadic() const;

  /// Whether this function is marked as virtual explicitly.
  bool isVirtualAsWritten() const { return IsVirtualAsWritten; }
  void setVirtualAsWritten(bool V) { IsVirtualAsWritten = V; }

  /// Whether this virtual function is pure, i.e. makes the containing class
  /// abstract.
  bool isPure() const { return IsPure; }
  void setPure(bool P = true);

  /// Whether this templated function will be late parsed.
  bool isLateTemplateParsed() const { return IsLateTemplateParsed; }
  void setLateTemplateParsed(bool ILT = true) { IsLateTemplateParsed = ILT; }

  /// Whether this function is "trivial" in some specialized C++ senses.
  /// Can only be true for default constructors, copy constructors,
  /// copy assignment operators, and destructors.  Not meaningful until
  /// the class has been fully built by Sema.
  bool isTrivial() const { return IsTrivial; }
  void setTrivial(bool IT) { IsTrivial = IT; }

  /// Whether this function is defaulted per C++0x. Only valid for
  /// special member functions.
  bool isDefaulted() const { return IsDefaulted; }
  void setDefaulted(bool D = true) { IsDefaulted = D; }

  /// Whether this function is explicitly defaulted per C++0x. Only valid
  /// for special member functions.
  bool isExplicitlyDefaulted() const { return IsExplicitlyDefaulted; }
  void setExplicitlyDefaulted(bool ED = true) { IsExplicitlyDefaulted = ED; }

  /// Whether falling off this function implicitly returns null/zero.
  /// If a more specific implicit return value is required, front-ends
  /// should synthesize the appropriate return statements.
  bool hasImplicitReturnZero() const { return HasImplicitReturnZero; }
  void setHasImplicitReturnZero(bool IRZ) { HasImplicitReturnZero = IRZ; }

  /// \brief Whether this function has a prototype, either because one
  /// was explicitly written or because it was "inherited" by merging
  /// a declaration without a prototype with a declaration that has a
  /// prototype.
  bool hasPrototype() const {
    return HasWrittenPrototype || HasInheritedPrototype;
  }

  bool hasWrittenPrototype() const { return HasWrittenPrototype; }

  /// \brief Whether this function inherited its prototype from a
  /// previous declaration.
  bool hasInheritedPrototype() const { return HasInheritedPrototype; }
  void setHasInheritedPrototype(bool P = true) { HasInheritedPrototype = P; }

  /// Whether this is a (C++11) constexpr function or constexpr constructor.
  bool isConstexpr() const { return IsConstexpr; }
  void setConstexpr(bool IC) { IsConstexpr = IC; }

  /// \brief Whether the instantiation of this function is pending.
  /// This bit is set when the decision to instantiate this function is made
  /// and unset if and when the function body is created. That leaves out
  /// cases where instantiation did not happen because the template definition
  /// was not seen in this TU. This bit remains set in those cases, under the
  /// assumption that the instantiation will happen in some other TU.
  bool instantiationIsPending() const { return InstantiationIsPending; }
  void setInstantiationIsPending(bool IC) { InstantiationIsPending = IC; }

  /// \brief Indicates the function uses __try.
  bool usesSEHTry() const { return UsesSEHTry; }
  void setUsesSEHTry(bool UST) { UsesSEHTry = UST; }

  /// \brief Whether this function has been deleted.
  ///
  /// A function that is "deleted" (via the C++0x "= delete" syntax)
  /// acts like a normal function, except that it cannot actually be
  /// called or have its address taken. Deleted functions are
  /// typically used in C++ overload resolution to attract arguments
  /// whose type or lvalue/rvalue-ness would permit the use of a
  /// different overload that would behave incorrectly. For example,
  /// one might use deleted functions to ban implicit conversion from
  /// a floating-point number to an Integer type:
  ///
  /// @code
  /// struct Integer {
  ///   Integer(long); // construct from a long
  ///   Integer(double) = delete; // no construction from float or double
  ///   Integer(long double) = delete; // no construction from long double
  /// };
  /// @endcode
  // If a function is deleted, its first declaration must be.
  bool isDeleted() const { return getCanonicalDecl()->IsDeleted; }
  bool isDeletedAsWritten() const { return IsDeleted && !IsDefaulted; }
  void setDeletedAsWritten(bool D = true) { IsDeleted = D; }

  /// \brief Determines whether this function is "main", which is the
  /// entry point into an executable program.
  bool isMain() const;

  /// \brief Determines whether this function is a MSVCRT user defined entry
  /// point.
  bool isMSVCRTEntryPoint() const;

  /// \brief Determines whether this operator new or delete is one
  /// of the reserved global placement operators:
  ///    void *operator new(size_t, void *);
  ///    void *operator new[](size_t, void *);
  ///    void operator delete(void *, void *);
  ///    void operator delete[](void *, void *);
  /// These functions have special behavior under [new.delete.placement]:
  ///    These functions are reserved, a C++ program may not define
  ///    functions that displace the versions in the Standard C++ library.
  ///    The provisions of [basic.stc.dynamic] do not apply to these
  ///    reserved placement forms of operator new and operator delete.
  ///
  /// This function must be an allocation or deallocation function.
  bool isReservedGlobalPlacementOperator() const;

  /// \brief Determines whether this function is one of the replaceable
  /// global allocation functions:
  ///    void *operator new(size_t);
  ///    void *operator new(size_t, const std::nothrow_t &) noexcept;
  ///    void *operator new[](size_t);
  ///    void *operator new[](size_t, const std::nothrow_t &) noexcept;
  ///    void operator delete(void *) noexcept;
  ///    void operator delete(void *, std::size_t) noexcept;      [C++1y]
  ///    void operator delete(void *, const std::nothrow_t &) noexcept;
  ///    void operator delete[](void *) noexcept;
  ///    void operator delete[](void *, std::size_t) noexcept;    [C++1y]
  ///    void operator delete[](void *, const std::nothrow_t &) noexcept;
  /// These functions have special behavior under C++1y [expr.new]:
  ///    An implementation is allowed to omit a call to a replaceable global
  ///    allocation function. [...]
  ///
  /// If this function is an aligned allocation/deallocation function, return
  /// true through IsAligned.
  bool isReplaceableGlobalAllocationFunction(bool *IsAligned = nullptr) const;

  /// \brief Determine whether this is a destroying operator delete.
  bool isDestroyingOperatorDelete() const;

  /// Compute the language linkage.
  LanguageLinkage getLanguageLinkage() const;

  /// \brief Determines whether this function is a function with
  /// external, C linkage.
  bool isExternC() const;

  /// \brief Determines whether this function's context is, or is nested within,
  /// a C++ extern "C" linkage spec.
  bool isInExternCContext() const;

  /// \brief Determines whether this function's context is, or is nested within,
  /// a C++ extern "C++" linkage spec.
  bool isInExternCXXContext() const;

  /// \brief Determines whether this is a global function.
  bool isGlobal() const;

  /// \brief Determines whether this function is known to be 'noreturn', through
  /// an attribute on its declaration or its type.
  bool isNoReturn() const;

  /// \brief True if the function was a definition but its body was skipped.
  bool hasSkippedBody() const { return HasSkippedBody; }
  void setHasSkippedBody(bool Skipped = true) { HasSkippedBody = Skipped; }

  /// True if this function will eventually have a body, once it's fully parsed.
  bool willHaveBody() const { return WillHaveBody; }
  void setWillHaveBody(bool V = true) { WillHaveBody = V; }

  /// True if this function is considered a multiversioned function.
  bool isMultiVersion() const { return getCanonicalDecl()->IsMultiVersion; }

  /// Sets the multiversion state for this declaration and all of its
  /// redeclarations.
  void setIsMultiVersion(bool V = true) {
    getCanonicalDecl()->IsMultiVersion = V;
  }

  void setPreviousDeclaration(FunctionDecl * PrevDecl);

  FunctionDecl *getCanonicalDecl() override;
  const FunctionDecl *getCanonicalDecl() const {
    return const_cast<FunctionDecl*>(this)->getCanonicalDecl();
  }

  unsigned getBuiltinID() const;

  // ArrayRef interface to parameters.
  ArrayRef<ParmVarDecl *> parameters() const {
    return {ParamInfo, getNumParams()};
  }
  MutableArrayRef<ParmVarDecl *> parameters() {
    return {ParamInfo, getNumParams()};
  }

  // Iterator access to formal parameters.
  using param_iterator = MutableArrayRef<ParmVarDecl *>::iterator;
  using param_const_iterator = ArrayRef<ParmVarDecl *>::const_iterator;

  bool param_empty() const { return parameters().empty(); }
  param_iterator param_begin() { return parameters().begin(); }
  param_iterator param_end() { return parameters().end(); }
  param_const_iterator param_begin() const { return parameters().begin(); }
  param_const_iterator param_end() const { return parameters().end(); }
  size_t param_size() const { return parameters().size(); }

  /// Return the number of parameters this function must have based on its
  /// FunctionType.  This is the length of the ParamInfo array after it has been
  /// created.
  unsigned getNumParams() const;

  const ParmVarDecl *getParamDecl(unsigned i) const {
    assert(i < getNumParams() && "Illegal param #");
    return ParamInfo[i];
  }
  ParmVarDecl *getParamDecl(unsigned i) {
    assert(i < getNumParams() && "Illegal param #");
    return ParamInfo[i];
  }
  void setParams(ArrayRef<ParmVarDecl *> NewParamInfo) {
    setParams(getASTContext(), NewParamInfo);
  }

  /// Returns the minimum number of arguments needed to call this function. This
  /// may be fewer than the number of function parameters, if some of the
  /// parameters have default arguments (in C++).
  unsigned getMinRequiredArguments() const;

  QualType getReturnType() const {
    assert(getType()->getAs<FunctionType>() && "Expected a FunctionType!");
    return getType()->getAs<FunctionType>()->getReturnType();
  }

  /// \brief Attempt to compute an informative source range covering the
  /// function return type. This may omit qualifiers and other information with
  /// limited representation in the AST.
  SourceRange getReturnTypeSourceRange() const;

  /// \brief Attempt to compute an informative source range covering the
  /// function exception specification, if any.
  SourceRange getExceptionSpecSourceRange() const;

  /// \brief Determine the type of an expression that calls this function.
  QualType getCallResultType() const {
    assert(getType()->getAs<FunctionType>() && "Expected a FunctionType!");
    return getType()->getAs<FunctionType>()->getCallResultType(getASTContext());
  }

  /// \brief Returns the WarnUnusedResultAttr that is either declared on this
  /// function, or its return type declaration.
  const Attr *getUnusedResultAttr() const;

  /// \brief Returns true if this function or its return type has the
  /// warn_unused_result attribute.
  bool hasUnusedResultAttr() const { return getUnusedResultAttr() != nullptr; }

  /// \brief Returns the storage class as written in the source. For the
  /// computed linkage of symbol, see getLinkage.
  StorageClass getStorageClass() const { return StorageClass(SClass); }

  /// \brief Determine whether the "inline" keyword was specified for this
  /// function.
  bool isInlineSpecified() const { return IsInlineSpecified; }

  /// Set whether the "inline" keyword was specified for this function.
  void setInlineSpecified(bool I) {
    IsInlineSpecified = false;
    IsInline = false;
  }

  /// Flag that this function is implicitly inline.
  void setImplicitlyInline() {
    IsInline = false;
  }

  /// \brief Determine whether this function should be inlined, because it is
  /// either marked "inline" or "constexpr" or is a member function of a class
  /// that was defined in the class body.
  bool isInlined() const { return IsInline; }

  bool isInlineDefinitionExternallyVisible() const;

  bool isMSExternInline() const;

  bool doesDeclarationForceExternallyVisibleDefinition() const;

  /// isOverloadedOperator - Whether this function declaration
  /// represents an C++ overloaded operator, e.g., "operator+".
  bool isOverloadedOperator() const {
    return getOverloadedOperator() != OO_None;
  }

  OverloadedOperatorKind getOverloadedOperator() const;

  const IdentifierInfo *getLiteralIdentifier() const;

  /// \brief If this function is an instantiation of a member function
  /// of a class template specialization, retrieves the function from
  /// which it was instantiated.
  ///
  /// This routine will return non-NULL for (non-templated) member
  /// functions of class templates and for instantiations of function
  /// templates. For example, given:
  ///
  /// \code
  /// template<typename T>
  /// struct X {
  ///   void f(T);
  /// };
  /// \endcode
  ///
  /// The declaration for X<int>::f is a (non-templated) FunctionDecl
  /// whose parent is the class template specialization X<int>. For
  /// this declaration, getInstantiatedFromFunction() will return
  /// the FunctionDecl X<T>::A. When a complete definition of
  /// X<int>::A is required, it will be instantiated from the
  /// declaration returned by getInstantiatedFromMemberFunction().
  FunctionDecl *getInstantiatedFromMemberFunction() const;

  /// \brief What kind of templated function this is.
  TemplatedKind getTemplatedKind() const;

  /// \brief If this function is an instantiation of a member function of a
  /// class template specialization, retrieves the member specialization
  /// information.
  MemberSpecializationInfo *getMemberSpecializationInfo() const;

  /// \brief Specify that this record is an instantiation of the
  /// member function FD.
  void setInstantiationOfMemberFunction(FunctionDecl *FD,
                                        TemplateSpecializationKind TSK) {
    setInstantiationOfMemberFunction(getASTContext(), FD, TSK);
  }

  /// \brief Retrieves the function template that is described by this
  /// function declaration.
  ///
  /// Every function template is represented as a FunctionTemplateDecl
  /// and a FunctionDecl (or something derived from FunctionDecl). The
  /// former contains template properties (such as the template
  /// parameter lists) while the latter contains the actual
  /// description of the template's
  /// contents. FunctionTemplateDecl::getTemplatedDecl() retrieves the
  /// FunctionDecl that describes the function template,
  /// getDescribedFunctionTemplate() retrieves the
  /// FunctionTemplateDecl from a FunctionDecl.
  FunctionTemplateDecl *getDescribedFunctionTemplate() const;

  void setDescribedFunctionTemplate(FunctionTemplateDecl *Template);

  /// \brief Determine whether this function is a function template
  /// specialization.
  bool isFunctionTemplateSpecialization() const {
    return getPrimaryTemplate() != nullptr;
  }

  /// \brief Retrieve the class scope template pattern that this function
  ///  template specialization is instantiated from.
  FunctionDecl *getClassScopeSpecializationPattern() const;

  /// \brief If this function is actually a function template specialization,
  /// retrieve information about this function template specialization.
  /// Otherwise, returns NULL.
  FunctionTemplateSpecializationInfo *getTemplateSpecializationInfo() const;

  /// \brief Determines whether this function is a function template
  /// specialization or a member of a class template specialization that can
  /// be implicitly instantiated.
  bool isImplicitlyInstantiable() const;

  /// \brief Determines if the given function was instantiated from a
  /// function template.
  bool isTemplateInstantiation() const;

  /// \brief Retrieve the function declaration from which this function could
  /// be instantiated, if it is an instantiation (rather than a non-template
  /// or a specialization, for example).
  FunctionDecl *getTemplateInstantiationPattern() const;

  /// \brief Retrieve the primary template that this function template
  /// specialization either specializes or was instantiated from.
  ///
  /// If this function declaration is not a function template specialization,
  /// returns NULL.
  FunctionTemplateDecl *getPrimaryTemplate() const;

  /// \brief Retrieve the template arguments used to produce this function
  /// template specialization from the primary template.
  ///
  /// If this function declaration is not a function template specialization,
  /// returns NULL.
  const TemplateArgumentList *getTemplateSpecializationArgs() const;

  /// \brief Retrieve the template argument list as written in the sources,
  /// if any.
  ///
  /// If this function declaration is not a function template specialization
  /// or if it had no explicit template argument list, returns NULL.
  /// Note that it an explicit template argument list may be written empty,
  /// e.g., template<> void foo<>(char* s);
  const ASTTemplateArgumentListInfo*
  getTemplateSpecializationArgsAsWritten() const;

  /// \brief Specify that this function declaration is actually a function
  /// template specialization.
  ///
  /// \param Template the function template that this function template
  /// specialization specializes.
  ///
  /// \param TemplateArgs the template arguments that produced this
  /// function template specialization from the template.
  ///
  /// \param InsertPos If non-NULL, the position in the function template
  /// specialization set where the function template specialization data will
  /// be inserted.
  ///
  /// \param TSK the kind of template specialization this is.
  ///
  /// \param TemplateArgsAsWritten location info of template arguments.
  ///
  /// \param PointOfInstantiation point at which the function template
  /// specialization was first instantiated.
  void setFunctionTemplateSpecialization(FunctionTemplateDecl *Template,
                const TemplateArgumentList *TemplateArgs,
                void *InsertPos,
                TemplateSpecializationKind TSK = TSK_ImplicitInstantiation,
                const TemplateArgumentListInfo *TemplateArgsAsWritten = nullptr,
                SourceLocation PointOfInstantiation = SourceLocation()) {
    setFunctionTemplateSpecialization(getASTContext(), Template, TemplateArgs,
                                      InsertPos, TSK, TemplateArgsAsWritten,
                                      PointOfInstantiation);
  }

  /// \brief Specifies that this function declaration is actually a
  /// dependent function template specialization.
  void setDependentTemplateSpecialization(ASTContext &Context,
                             const UnresolvedSetImpl &Templates,
                      const TemplateArgumentListInfo &TemplateArgs);

  DependentFunctionTemplateSpecializationInfo *
  getDependentSpecializationInfo() const;

  /// \brief Determine what kind of template instantiation this function
  /// represents.
  TemplateSpecializationKind getTemplateSpecializationKind() const;

  /// \brief Determine what kind of template instantiation this function
  /// represents.
  void setTemplateSpecializationKind(TemplateSpecializationKind TSK,
                        SourceLocation PointOfInstantiation = SourceLocation());

  /// \brief Retrieve the (first) point of instantiation of a function template
  /// specialization or a member of a class template specialization.
  ///
  /// \returns the first point of instantiation, if this function was
  /// instantiated from a template; otherwise, returns an invalid source
  /// location.
  SourceLocation getPointOfInstantiation() const;

  /// \brief Determine whether this is or was instantiated from an out-of-line
  /// definition of a member function.
  bool isOutOfLine() const override;

  /// \brief Identify a memory copying or setting function.
  /// If the given function is a memory copy or setting function, returns
  /// the corresponding Builtin ID. If the function is not a memory function,
  /// returns 0.
  unsigned getMemoryFunctionKind() const;

  /// \brief Returns ODRHash of the function.  This value is calculated and
  /// stored on first call, then the stored value returned on the other calls.
  unsigned getODRHash();

  // XMC extensions
  friend class ASTDeclReader;
  friend class ASTDeclWriter;

  std::map<std::string, std::string> XMCPortDirection;

  // Implement isa/cast/dyncast/etc.
  static bool classof(const Decl *D) { return classofKind(D->getKind()); }
  static bool classofKind(Kind K) {
    return K >= firstFunction && K <= lastFunction;
  }
  static DeclContext *castToDeclContext(const FunctionDecl *D) {
    return static_cast<DeclContext *>(const_cast<FunctionDecl*>(D));
  }
  static FunctionDecl *castFromDeclContext(const DeclContext *DC) {
    return static_cast<FunctionDecl *>(const_cast<DeclContext*>(DC));
  }
};

/// FieldDecl - An instance of this class is created by Sema::ActOnField to
/// represent a member of a struct/union/class.
class FieldDecl : public DeclaratorDecl, public Mergeable<FieldDecl> {
  unsigned BitField : 1;
  unsigned Mutable : 1;
  mutable unsigned CachedFieldIndex : 30;

  /// The kinds of value we can store in InitializerOrBitWidth.
  ///
  /// Note that this is compatible with InClassInitStyle except for
  /// ISK_CapturedVLAType.
  enum InitStorageKind {
    /// If the pointer is null, there's nothing special.  Otherwise,
    /// this is a bitfield and the pointer is the Expr* storing the
    /// bit-width.
    ISK_NoInit = (unsigned) ICIS_NoInit,

    /// The pointer is an (optional due to delayed parsing) Expr*
    /// holding the copy-initializer.
    ISK_InClassCopyInit = (unsigned) ICIS_CopyInit,

    /// The pointer is an (optional due to delayed parsing) Expr*
    /// holding the list-initializer.
    ISK_InClassListInit = (unsigned) ICIS_ListInit,

    /// The pointer is a VariableArrayType* that's been captured;
    /// the enclosing context is a lambda or captured statement.
    ISK_CapturedVLAType,
  };

  /// If this is a bitfield with a default member initializer, this
  /// structure is used to represent the two expressions.
  struct InitAndBitWidth {
    Expr *Init;
    Expr *BitWidth;
  };

  /// \brief Storage for either the bit-width, the in-class initializer, or
  /// both (via InitAndBitWidth), or the captured variable length array bound.
  ///
  /// If the storage kind is ISK_InClassCopyInit or
  /// ISK_InClassListInit, but the initializer is null, then this
  /// field has an in-class initializer that has not yet been parsed
  /// and attached.
  // FIXME: Tail-allocate this to reduce the size of FieldDecl in the
  // overwhelmingly common case that we have none of these things.
  llvm::PointerIntPair<void *, 2, InitStorageKind> InitStorage;

protected:
  FieldDecl(Kind DK, DeclContext *DC, SourceLocation StartLoc,
            SourceLocation IdLoc, IdentifierInfo *Id,
            QualType T, TypeSourceInfo *TInfo, Expr *BW, bool Mutable,
            InClassInitStyle InitStyle)
    : DeclaratorDecl(DK, DC, IdLoc, Id, T, TInfo, StartLoc),
      BitField(false), Mutable(Mutable), CachedFieldIndex(0),
      InitStorage(nullptr, (InitStorageKind) InitStyle) {
    if (BW)
      setBitWidth(BW);
  }

public:
  friend class ASTDeclReader;
  friend class ASTDeclWriter;

  static FieldDecl *Create(const ASTContext &C, DeclContext *DC,
                           SourceLocation StartLoc, SourceLocation IdLoc,
                           IdentifierInfo *Id, QualType T,
                           TypeSourceInfo *TInfo, Expr *BW, bool Mutable,
                           InClassInitStyle InitStyle);

  static FieldDecl *CreateDeserialized(ASTContext &C, unsigned ID);
  
  /// getFieldIndex - Returns the index of this field within its record,
  /// as appropriate for passing to ASTRecordLayout::getFieldOffset.
  unsigned getFieldIndex() const;

  /// isMutable - Determines whether this field is mutable (C++ only).
  bool isMutable() const { return Mutable; }

  /// \brief Determines whether this field is a bitfield.
  bool isBitField() const { return BitField; }

  /// @brief Determines whether this is an unnamed bitfield.
  bool isUnnamedBitfield() const { return isBitField() && !getDeclName(); }

  /// isAnonymousStructOrUnion - Determines whether this field is a
  /// representative for an anonymous struct or union. Such fields are
  /// unnamed and are implicitly generated by the implementation to
  /// store the data for the anonymous union or struct.
  bool isAnonymousStructOrUnion() const;

  Expr *getBitWidth() const {
    if (!BitField)
      return nullptr;
    void *Ptr = InitStorage.getPointer();
    if (getInClassInitStyle())
      return static_cast<InitAndBitWidth*>(Ptr)->BitWidth;
    return static_cast<Expr*>(Ptr);
  }

  unsigned getBitWidthValue(const ASTContext &Ctx) const;

  /// setBitWidth - Set the bit-field width for this member.
  // Note: used by some clients (i.e., do not remove it).
  void setBitWidth(Expr *Width) {
    assert(!hasCapturedVLAType() && !BitField &&
           "bit width or captured type already set");
    assert(Width && "no bit width specified");
    InitStorage.setPointer(
        InitStorage.getInt()
            ? new (getASTContext())
                  InitAndBitWidth{getInClassInitializer(), Width}
            : static_cast<void*>(Width));
    BitField = true;
  }

  /// removeBitWidth - Remove the bit-field width from this member.
  // Note: used by some clients (i.e., do not remove it).
  void removeBitWidth() {
    assert(isBitField() && "no bitfield width to remove");
    InitStorage.setPointer(getInClassInitializer());
    BitField = false;
  }

  /// Get the kind of (C++11) default member initializer that this field has.
  InClassInitStyle getInClassInitStyle() const {
    InitStorageKind storageKind = InitStorage.getInt();
    return (storageKind == ISK_CapturedVLAType
              ? ICIS_NoInit : (InClassInitStyle) storageKind);
  }

  /// Determine whether this member has a C++11 default member initializer.
  bool hasInClassInitializer() const {
    return getInClassInitStyle() != ICIS_NoInit;
  }

  /// Get the C++11 default member initializer for this member, or null if one
  /// has not been set. If a valid declaration has a default member initializer,
  /// but this returns null, then we have not parsed and attached it yet.
  Expr *getInClassInitializer() const {
    if (!hasInClassInitializer())
      return nullptr;
    void *Ptr = InitStorage.getPointer();
    if (BitField)
      return static_cast<InitAndBitWidth*>(Ptr)->Init;
    return static_cast<Expr*>(Ptr);
  }

  /// setInClassInitializer - Set the C++11 in-class initializer for this
  /// member.
  void setInClassInitializer(Expr *Init) {
    assert(hasInClassInitializer() && !getInClassInitializer());
    if (BitField)
      static_cast<InitAndBitWidth*>(InitStorage.getPointer())->Init = Init;
    else
      InitStorage.setPointer(Init);
  }

  /// removeInClassInitializer - Remove the C++11 in-class initializer from this
  /// member.
  void removeInClassInitializer() {
    assert(hasInClassInitializer() && "no initializer to remove");
    InitStorage.setPointerAndInt(getBitWidth(), ISK_NoInit);
  }

  /// \brief Determine whether this member captures the variable length array
  /// type.
  bool hasCapturedVLAType() const {
    return InitStorage.getInt() == ISK_CapturedVLAType;
  }

  /// \brief Get the captured variable length array type.
  const VariableArrayType *getCapturedVLAType() const {
    return hasCapturedVLAType() ? static_cast<const VariableArrayType *>(
                                      InitStorage.getPointer())
                                : nullptr;
  }

  /// \brief Set the captured variable length array type for this field.
  void setCapturedVLAType(const VariableArrayType *VLAType);

  /// getParent - Returns the parent of this field declaration, which
  /// is the struct in which this field is defined.
  const RecordDecl *getParent() const {
    return cast<RecordDecl>(getDeclContext());
  }

  RecordDecl *getParent() {
    return cast<RecordDecl>(getDeclContext());
  }

  SourceRange getSourceRange() const override LLVM_READONLY;

  /// Retrieves the canonical declaration of this field.
  FieldDecl *getCanonicalDecl() override { return getFirstDecl(); }
  const FieldDecl *getCanonicalDecl() const { return getFirstDecl(); }

  // Implement isa/cast/dyncast/etc.
  static bool classof(const Decl *D) { return classofKind(D->getKind()); }
  static bool classofKind(Kind K) { return K >= firstField && K <= lastField; }
};

/// EnumConstantDecl - An instance of this object exists for each enum constant
/// that is defined.  For example, in "enum X {a,b}", each of a/b are
/// EnumConstantDecl's, X is an instance of EnumDecl, and the type of a/b is a
/// TagType for the X EnumDecl.
class EnumConstantDecl : public ValueDecl, public Mergeable<EnumConstantDecl> {
  Stmt *Init; // an integer constant expression
  llvm::APSInt Val; // The value.

protected:
  EnumConstantDecl(DeclContext *DC, SourceLocation L,
                   IdentifierInfo *Id, QualType T, Expr *E,
                   const llvm::APSInt &V)
    : ValueDecl(EnumConstant, DC, L, Id, T), Init((Stmt*)E), Val(V) {}

public:
  friend class StmtIteratorBase;

  static EnumConstantDecl *Create(ASTContext &C, EnumDecl *DC,
                                  SourceLocation L, IdentifierInfo *Id,
                                  QualType T, Expr *E,
                                  const llvm::APSInt &V);
  static EnumConstantDecl *CreateDeserialized(ASTContext &C, unsigned ID);
  
  const Expr *getInitExpr() const { return (const Expr*) Init; }
  Expr *getInitExpr() { return (Expr*) Init; }
  const llvm::APSInt &getInitVal() const { return Val; }

  void setInitExpr(Expr *E) { Init = (Stmt*) E; }
  void setInitVal(const llvm::APSInt &V) { Val = V; }

  SourceRange getSourceRange() const override LLVM_READONLY;

  /// Retrieves the canonical declaration of this enumerator.
  EnumConstantDecl *getCanonicalDecl() override { return getFirstDecl(); }
  const EnumConstantDecl *getCanonicalDecl() const { return getFirstDecl(); }

  // Implement isa/cast/dyncast/etc.
  static bool classof(const Decl *D) { return classofKind(D->getKind()); }
  static bool classofKind(Kind K) { return K == EnumConstant; }
};

/// IndirectFieldDecl - An instance of this class is created to represent a
/// field injected from an anonymous union/struct into the parent scope.
/// IndirectFieldDecl are always implicit.
class IndirectFieldDecl : public ValueDecl,
                          public Mergeable<IndirectFieldDecl> {
  NamedDecl **Chaining;
  unsigned ChainingSize;

  IndirectFieldDecl(ASTContext &C, DeclContext *DC, SourceLocation L,
                    DeclarationName N, QualType T,
                    MutableArrayRef<NamedDecl *> CH);

  void anchor() override;

public:
  friend class ASTDeclReader;

  static IndirectFieldDecl *Create(ASTContext &C, DeclContext *DC,
                                   SourceLocation L, IdentifierInfo *Id,
                                   QualType T, llvm::MutableArrayRef<NamedDecl *> CH);

  static IndirectFieldDecl *CreateDeserialized(ASTContext &C, unsigned ID);

  using chain_iterator = ArrayRef<NamedDecl *>::const_iterator;

  ArrayRef<NamedDecl *> chain() const {
    return llvm::makeArrayRef(Chaining, ChainingSize);
  }
  chain_iterator chain_begin() const { return chain().begin(); }
  chain_iterator chain_end() const { return chain().end(); }

  unsigned getChainingSize() const { return ChainingSize; }

  FieldDecl *getAnonField() const {
    assert(chain().size() >= 2);
    return cast<FieldDecl>(chain().back());
  }

  VarDecl *getVarDecl() const {
    assert(chain().size() >= 2);
    return dyn_cast<VarDecl>(chain().front());
  }

  IndirectFieldDecl *getCanonicalDecl() override { return getFirstDecl(); }
  const IndirectFieldDecl *getCanonicalDecl() const { return getFirstDecl(); }

  // Implement isa/cast/dyncast/etc.
  static bool classof(const Decl *D) { return classofKind(D->getKind()); }
  static bool classofKind(Kind K) { return K == IndirectField; }
};

/// TypeDecl - Represents a declaration of a type.
class TypeDecl : public NamedDecl {
  friend class ASTContext;

  /// TypeForDecl - This indicates the Type object that represents
  /// this TypeDecl.  It is a cache maintained by
  /// ASTContext::getTypedefType, ASTContext::getTagDeclType, and
  /// ASTContext::getTemplateTypeParmType, and TemplateTypeParmDecl.
  mutable const Type *TypeForDecl = nullptr;

  /// LocStart - The start of the source range for this declaration.
  SourceLocation LocStart;

  void anchor() override;

protected:
  TypeDecl(Kind DK, DeclContext *DC, SourceLocation L, IdentifierInfo *Id,
           SourceLocation StartL = SourceLocation())
    : NamedDecl(DK, DC, L, Id), LocStart(StartL) {}

public:
  // Low-level accessor. If you just want the type defined by this node,
  // check out ASTContext::getTypeDeclType or one of
  // ASTContext::getTypedefType, ASTContext::getRecordType, etc. if you
  // already know the specific kind of node this is.
  const Type *getTypeForDecl() const { return TypeForDecl; }
  void setTypeForDecl(const Type *TD) { TypeForDecl = TD; }

  SourceLocation getLocStart() const LLVM_READONLY { return LocStart; }
  void setLocStart(SourceLocation L) { LocStart = L; }
  SourceRange getSourceRange() const override LLVM_READONLY {
    if (LocStart.isValid())
      return SourceRange(LocStart, getLocation());
    else
      return SourceRange(getLocation());
  }

  // Implement isa/cast/dyncast/etc.
  static bool classof(const Decl *D) { return classofKind(D->getKind()); }
  static bool classofKind(Kind K) { return K >= firstType && K <= lastType; }
};

/// Base class for declarations which introduce a typedef-name.
class TypedefNameDecl : public TypeDecl, public Redeclarable<TypedefNameDecl> {
  using ModedTInfo = std::pair<TypeSourceInfo *, QualType>;
  llvm::PointerUnion<TypeSourceInfo *, ModedTInfo *> MaybeModedTInfo;

  // FIXME: This can be packed into the bitfields in Decl.
  /// If 0, we have not computed IsTransparentTag.
  /// Otherwise, IsTransparentTag is (CacheIsTransparentTag >> 1).
  mutable unsigned CacheIsTransparentTag : 2;

  void anchor() override;

protected:
  TypedefNameDecl(Kind DK, ASTContext &C, DeclContext *DC,
                  SourceLocation StartLoc, SourceLocation IdLoc,
                  IdentifierInfo *Id, TypeSourceInfo *TInfo)
      : TypeDecl(DK, DC, IdLoc, Id, StartLoc), redeclarable_base(C),
        MaybeModedTInfo(TInfo), CacheIsTransparentTag(0) {}

  using redeclarable_base = Redeclarable<TypedefNameDecl>;

  TypedefNameDecl *getNextRedeclarationImpl() override {
    return getNextRedeclaration();
  }

  TypedefNameDecl *getPreviousDeclImpl() override {
    return getPreviousDecl();
  }

  TypedefNameDecl *getMostRecentDeclImpl() override {
    return getMostRecentDecl();
  }

public:
  using redecl_range = redeclarable_base::redecl_range;
  using redecl_iterator = redeclarable_base::redecl_iterator;

  using redeclarable_base::redecls_begin;
  using redeclarable_base::redecls_end;
  using redeclarable_base::redecls;
  using redeclarable_base::getPreviousDecl;
  using redeclarable_base::getMostRecentDecl;
  using redeclarable_base::isFirstDecl;

  bool isModed() const { return MaybeModedTInfo.is<ModedTInfo*>(); }

  TypeSourceInfo *getTypeSourceInfo() const {
    return isModed()
      ? MaybeModedTInfo.get<ModedTInfo*>()->first
      : MaybeModedTInfo.get<TypeSourceInfo*>();
  }

  QualType getUnderlyingType() const {
    return isModed()
      ? MaybeModedTInfo.get<ModedTInfo*>()->second
      : MaybeModedTInfo.get<TypeSourceInfo*>()->getType();
  }

  void setTypeSourceInfo(TypeSourceInfo *newType) {
    MaybeModedTInfo = newType;
  }

  void setModedTypeSourceInfo(TypeSourceInfo *unmodedTSI, QualType modedTy) {
    MaybeModedTInfo = new (getASTContext()) ModedTInfo(unmodedTSI, modedTy);
  }

  /// Retrieves the canonical declaration of this typedef-name.
  TypedefNameDecl *getCanonicalDecl() override { return getFirstDecl(); }
  const TypedefNameDecl *getCanonicalDecl() const { return getFirstDecl(); }

  /// Retrieves the tag declaration for which this is the typedef name for
  /// linkage purposes, if any.
  ///
  /// \param AnyRedecl Look for the tag declaration in any redeclaration of
  /// this typedef declaration.
  TagDecl *getAnonDeclWithTypedefName(bool AnyRedecl = false) const;

  /// Determines if this typedef shares a name and spelling location with its
  /// underlying tag type, as is the case with the NS_ENUM macro.
  bool isTransparentTag() const {
    if (CacheIsTransparentTag)
      return CacheIsTransparentTag & 0x2;
    return isTransparentTagSlow();
  }

  // Implement isa/cast/dyncast/etc.
  static bool classof(const Decl *D) { return classofKind(D->getKind()); }
  static bool classofKind(Kind K) {
    return K >= firstTypedefName && K <= lastTypedefName;
  }

private:
  bool isTransparentTagSlow() const;
};

/// TypedefDecl - Represents the declaration of a typedef-name via the 'typedef'
/// type specifier.
class TypedefDecl : public TypedefNameDecl {
  TypedefDecl(ASTContext &C, DeclContext *DC, SourceLocation StartLoc,
              SourceLocation IdLoc, IdentifierInfo *Id, TypeSourceInfo *TInfo)
      : TypedefNameDecl(Typedef, C, DC, StartLoc, IdLoc, Id, TInfo) {}

public:
  static TypedefDecl *Create(ASTContext &C, DeclContext *DC,
                             SourceLocation StartLoc, SourceLocation IdLoc,
                             IdentifierInfo *Id, TypeSourceInfo *TInfo);
  static TypedefDecl *CreateDeserialized(ASTContext &C, unsigned ID);

  SourceRange getSourceRange() const override LLVM_READONLY;

  // Implement isa/cast/dyncast/etc.
  static bool classof(const Decl *D) { return classofKind(D->getKind()); }
  static bool classofKind(Kind K) { return K == Typedef; }
};

/// TypeAliasDecl - Represents the declaration of a typedef-name via a C++0x
/// alias-declaration.
class TypeAliasDecl : public TypedefNameDecl {
  /// The template for which this is the pattern, if any.
  TypeAliasTemplateDecl *Template;

  TypeAliasDecl(ASTContext &C, DeclContext *DC, SourceLocation StartLoc,
                SourceLocation IdLoc, IdentifierInfo *Id, TypeSourceInfo *TInfo)
      : TypedefNameDecl(TypeAlias, C, DC, StartLoc, IdLoc, Id, TInfo),
        Template(nullptr) {}

public:
  static TypeAliasDecl *Create(ASTContext &C, DeclContext *DC,
                               SourceLocation StartLoc, SourceLocation IdLoc,
                               IdentifierInfo *Id, TypeSourceInfo *TInfo);
  static TypeAliasDecl *CreateDeserialized(ASTContext &C, unsigned ID);

  SourceRange getSourceRange() const override LLVM_READONLY;

  TypeAliasTemplateDecl *getDescribedAliasTemplate() const { return Template; }
  void setDescribedAliasTemplate(TypeAliasTemplateDecl *TAT) { Template = TAT; }

  // Implement isa/cast/dyncast/etc.
  static bool classof(const Decl *D) { return classofKind(D->getKind()); }
  static bool classofKind(Kind K) { return K == TypeAlias; }
};

/// TagDecl - Represents the declaration of a struct/union/class/enum.
class TagDecl
  : public TypeDecl, public DeclContext, public Redeclarable<TagDecl> {
public:
  // This is really ugly.
  using TagKind = TagTypeKind;

private:
  // FIXME: This can be packed into the bitfields in Decl.
  /// TagDeclKind - The TagKind enum.
  unsigned TagDeclKind : 3;

  /// IsCompleteDefinition - True if this is a definition ("struct foo
  /// {};"), false if it is a declaration ("struct foo;").  It is not
  /// a definition until the definition has been fully processed.
  unsigned IsCompleteDefinition : 1;

protected:
  /// IsBeingDefined - True if this is currently being defined.
  unsigned IsBeingDefined : 1;

private:
  /// IsEmbeddedInDeclarator - True if this tag declaration is
  /// "embedded" (i.e., defined or declared for the very first time)
  /// in the syntax of a declarator.
  unsigned IsEmbeddedInDeclarator : 1;

  /// \brief True if this tag is free standing, e.g. "struct foo;".
  unsigned IsFreeStanding : 1;

protected:
  // These are used by (and only defined for) EnumDecl.
  unsigned NumPositiveBits : 8;
  unsigned NumNegativeBits : 8;

  /// IsScoped - True if this tag declaration is a scoped enumeration. Only
  /// possible in C++11 mode.
  unsigned IsScoped : 1;

  /// IsScopedUsingClassTag - If this tag declaration is a scoped enum,
  /// then this is true if the scoped enum was declared using the class
  /// tag, false if it was declared with the struct tag. No meaning is
  /// associated if this tag declaration is not a scoped enum.
  unsigned IsScopedUsingClassTag : 1;

  /// IsFixed - True if this is an enumeration with fixed underlying type. Only
  /// possible in C++11, Microsoft extensions, or Objective C mode.
  unsigned IsFixed : 1;

  /// \brief Indicates whether it is possible for declarations of this kind
  /// to have an out-of-date definition.
  ///
  /// This option is only enabled when modules are enabled.
  unsigned MayHaveOutOfDateDef : 1;

  /// Has the full definition of this type been required by a use somewhere in
  /// the TU.
  unsigned IsCompleteDefinitionRequired : 1;

private:
  SourceRange BraceRange;

  // A struct representing syntactic qualifier info,
  // to be used for the (uncommon) case of out-of-line declarations.
  using ExtInfo = QualifierInfo;

  /// \brief If the (out-of-line) tag declaration name
  /// is qualified, it points to the qualifier info (nns and range);
  /// otherwise, if the tag declaration is anonymous and it is part of
  /// a typedef or alias, it points to the TypedefNameDecl (used for mangling);
  /// otherwise, if the tag declaration is anonymous and it is used as a
  /// declaration specifier for variables, it points to the first VarDecl (used
  /// for mangling);
  /// otherwise, it is a null (TypedefNameDecl) pointer.
  llvm::PointerUnion<TypedefNameDecl *, ExtInfo *> TypedefNameDeclOrQualifier;

  bool hasExtInfo() const { return TypedefNameDeclOrQualifier.is<ExtInfo *>(); }
  ExtInfo *getExtInfo() { return TypedefNameDeclOrQualifier.get<ExtInfo *>(); }
  const ExtInfo *getExtInfo() const {
    return TypedefNameDeclOrQualifier.get<ExtInfo *>();
  }

protected:
  TagDecl(Kind DK, TagKind TK, const ASTContext &C, DeclContext *DC,
          SourceLocation L, IdentifierInfo *Id, TagDecl *PrevDecl,
          SourceLocation StartL)
      : TypeDecl(DK, DC, L, Id, StartL), DeclContext(DK), redeclarable_base(C),
        TagDeclKind(TK), IsCompleteDefinition(false), IsBeingDefined(false),
        IsEmbeddedInDeclarator(false), IsFreeStanding(false),
        IsCompleteDefinitionRequired(false),
        TypedefNameDeclOrQualifier((TypedefNameDecl *)nullptr) {
    assert((DK != Enum || TK == TTK_Enum) &&
           "EnumDecl not matched with TTK_Enum");
    setPreviousDecl(PrevDecl);
  }

  using redeclarable_base = Redeclarable<TagDecl>;

  TagDecl *getNextRedeclarationImpl() override {
    return getNextRedeclaration();
  }

  TagDecl *getPreviousDeclImpl() override {
    return getPreviousDecl();
  }

  TagDecl *getMostRecentDeclImpl() override {
    return getMostRecentDecl();
  }

  /// @brief Completes the definition of this tag declaration.
  ///
  /// This is a helper function for derived classes.
  void completeDefinition();

public:
  friend class ASTDeclReader;
  friend class ASTDeclWriter;

  using redecl_range = redeclarable_base::redecl_range;
  using redecl_iterator = redeclarable_base::redecl_iterator;

  using redeclarable_base::redecls_begin;
  using redeclarable_base::redecls_end;
  using redeclarable_base::redecls;
  using redeclarable_base::getPreviousDecl;
  using redeclarable_base::getMostRecentDecl;
  using redeclarable_base::isFirstDecl;

  SourceRange getBraceRange() const { return BraceRange; }
  void setBraceRange(SourceRange R) { BraceRange = R; }

  /// getInnerLocStart - Return SourceLocation representing start of source
  /// range ignoring outer template declarations.
  SourceLocation getInnerLocStart() const { return getLocStart(); }

  /// getOuterLocStart - Return SourceLocation representing start of source
  /// range taking into account any outer template declarations.
  SourceLocation getOuterLocStart() const;
  SourceRange getSourceRange() const override LLVM_READONLY;

  TagDecl *getCanonicalDecl() override;
  const TagDecl *getCanonicalDecl() const {
    return const_cast<TagDecl*>(this)->getCanonicalDecl();
  }

  /// isThisDeclarationADefinition() - Return true if this declaration
  /// is a completion definition of the type.  Provided for consistency.
  bool isThisDeclarationADefinition() const {
    return isCompleteDefinition();
  }

  /// isCompleteDefinition - Return true if this decl has its body
  /// fully specified.
  bool isCompleteDefinition() const {
    return IsCompleteDefinition;
  }

  /// \brief Return true if this complete decl is
  /// required to be complete for some existing use.
  bool isCompleteDefinitionRequired() const {
    return IsCompleteDefinitionRequired;
  }

  /// isBeingDefined - Return true if this decl is currently being defined.
  bool isBeingDefined() const {
    return IsBeingDefined;
  }

  bool isEmbeddedInDeclarator() const {
    return IsEmbeddedInDeclarator;
  }
  void setEmbeddedInDeclarator(bool isInDeclarator) {
    IsEmbeddedInDeclarator = isInDeclarator;
  }

  bool isFreeStanding() const { return IsFreeStanding; }
  void setFreeStanding(bool isFreeStanding = true) {
    IsFreeStanding = isFreeStanding;
  }

  /// \brief Whether this declaration declares a type that is
  /// dependent, i.e., a type that somehow depends on template
  /// parameters.
  bool isDependentType() const { return isDependentContext(); }

  /// @brief Starts the definition of this tag declaration.
  ///
  /// This method should be invoked at the beginning of the definition
  /// of this tag declaration. It will set the tag type into a state
  /// where it is in the process of being defined.
  void startDefinition();

  /// getDefinition - Returns the TagDecl that actually defines this
  ///  struct/union/class/enum.  When determining whether or not a
  ///  struct/union/class/enum has a definition, one should use this
  ///  method as opposed to 'isDefinition'.  'isDefinition' indicates
  ///  whether or not a specific TagDecl is defining declaration, not
  ///  whether or not the struct/union/class/enum type is defined.
  ///  This method returns NULL if there is no TagDecl that defines
  ///  the struct/union/class/enum.
  TagDecl *getDefinition() const;

  void setCompleteDefinition(bool V) { IsCompleteDefinition = V; }

  void setCompleteDefinitionRequired(bool V = true) {
    IsCompleteDefinitionRequired = V;
  }

  StringRef getKindName() const {
    return TypeWithKeyword::getTagTypeKindName(getTagKind());
  }

  TagKind getTagKind() const {
    return TagKind(TagDeclKind);
  }

  void setTagKind(TagKind TK) { TagDeclKind = TK; }

  bool isStruct() const { return getTagKind() == TTK_Struct; }
  bool isInterface() const { return getTagKind() == TTK_Interface; }
  bool isClass()  const { return getTagKind() == TTK_Class; }
  bool isUnion()  const { return getTagKind() == TTK_Union; }
  bool isEnum()   const { return getTagKind() == TTK_Enum; }

  /// Is this tag type named, either directly or via being defined in
  /// a typedef of this type?
  ///
  /// C++11 [basic.link]p8:
  ///   A type is said to have linkage if and only if:
  ///     - it is a class or enumeration type that is named (or has a
  ///       name for linkage purposes) and the name has linkage; ...
  /// C++11 [dcl.typedef]p9:
  ///   If the typedef declaration defines an unnamed class (or enum),
  ///   the first typedef-name declared by the declaration to be that
  ///   class type (or enum type) is used to denote the class type (or
  ///   enum type) for linkage purposes only.
  ///
  /// C does not have an analogous rule, but the same concept is
  /// nonetheless useful in some places.
  bool hasNameForLinkage() const {
    return (getDeclName() || getTypedefNameForAnonDecl());
  }

  TypedefNameDecl *getTypedefNameForAnonDecl() const {
    return hasExtInfo() ? nullptr
                        : TypedefNameDeclOrQualifier.get<TypedefNameDecl *>();
  }

  void setTypedefNameForAnonDecl(TypedefNameDecl *TDD);

  /// \brief Retrieve the nested-name-specifier that qualifies the name of this
  /// declaration, if it was present in the source.
  NestedNameSpecifier *getQualifier() const {
    return hasExtInfo() ? getExtInfo()->QualifierLoc.getNestedNameSpecifier()
                        : nullptr;
  }

  /// \brief Retrieve the nested-name-specifier (with source-location
  /// information) that qualifies the name of this declaration, if it was
  /// present in the source.
  NestedNameSpecifierLoc getQualifierLoc() const {
    return hasExtInfo() ? getExtInfo()->QualifierLoc
                        : NestedNameSpecifierLoc();
  }

  void setQualifierInfo(NestedNameSpecifierLoc QualifierLoc);

  unsigned getNumTemplateParameterLists() const {
    return hasExtInfo() ? getExtInfo()->NumTemplParamLists : 0;
  }

  TemplateParameterList *getTemplateParameterList(unsigned i) const {
    assert(i < getNumTemplateParameterLists());
    return getExtInfo()->TemplParamLists[i];
  }

  void setTemplateParameterListsInfo(ASTContext &Context,
                                     ArrayRef<TemplateParameterList *> TPLists);

  // Implement isa/cast/dyncast/etc.
  static bool classof(const Decl *D) { return classofKind(D->getKind()); }
  static bool classofKind(Kind K) { return K >= firstTag && K <= lastTag; }

  static DeclContext *castToDeclContext(const TagDecl *D) {
    return static_cast<DeclContext *>(const_cast<TagDecl*>(D));
  }

  static TagDecl *castFromDeclContext(const DeclContext *DC) {
    return static_cast<TagDecl *>(const_cast<DeclContext*>(DC));
  }
};

/// EnumDecl - Represents an enum.  In C++11, enums can be forward-declared
/// with a fixed underlying type, and in C we allow them to be forward-declared
/// with no underlying type as an extension.
class EnumDecl : public TagDecl {
  /// IntegerType - This represent the integer type that the enum corresponds
  /// to for code generation purposes.  Note that the enumerator constants may
  /// have a different type than this does.
  ///
  /// If the underlying integer type was explicitly stated in the source
  /// code, this is a TypeSourceInfo* for that type. Otherwise this type
  /// was automatically deduced somehow, and this is a Type*.
  ///
  /// Normally if IsFixed(), this would contain a TypeSourceInfo*, but in
  /// some cases it won't.
  ///
  /// The underlying type of an enumeration never has any qualifiers, so
  /// we can get away with just storing a raw Type*, and thus save an
  /// extra pointer when TypeSourceInfo is needed.
  llvm::PointerUnion<const Type *, TypeSourceInfo *> IntegerType;

  /// PromotionType - The integer type that values of this type should
  /// promote to.  In C, enumerators are generally of an integer type
  /// directly, but gcc-style large enumerators (and all enumerators
  /// in C++) are of the enum type instead.
  QualType PromotionType;

  /// \brief If this enumeration is an instantiation of a member enumeration
  /// of a class template specialization, this is the member specialization
  /// information.
  MemberSpecializationInfo *SpecializationInfo = nullptr;

  EnumDecl(ASTContext &C, DeclContext *DC, SourceLocation StartLoc,
           SourceLocation IdLoc, IdentifierInfo *Id, EnumDecl *PrevDecl,
           bool Scoped, bool ScopedUsingClassTag, bool Fixed)
      : TagDecl(Enum, TTK_Enum, C, DC, IdLoc, Id, PrevDecl, StartLoc) {
    assert(Scoped || !ScopedUsingClassTag);
    IntegerType = (const Type *)nullptr;
    NumNegativeBits = 0;
    NumPositiveBits = 0;
    IsScoped = Scoped;
    IsScopedUsingClassTag = ScopedUsingClassTag;
    IsFixed = Fixed;
  }

  void anchor() override;

  void setInstantiationOfMemberEnum(ASTContext &C, EnumDecl *ED,
                                    TemplateSpecializationKind TSK);
public:
  friend class ASTDeclReader;

  EnumDecl *getCanonicalDecl() override {
    return cast<EnumDecl>(TagDecl::getCanonicalDecl());
  }
  const EnumDecl *getCanonicalDecl() const {
    return const_cast<EnumDecl*>(this)->getCanonicalDecl();
  }

  EnumDecl *getPreviousDecl() {
    return cast_or_null<EnumDecl>(
            static_cast<TagDecl *>(this)->getPreviousDecl());
  }
  const EnumDecl *getPreviousDecl() const {
    return const_cast<EnumDecl*>(this)->getPreviousDecl();
  }

  EnumDecl *getMostRecentDecl() {
    return cast<EnumDecl>(static_cast<TagDecl *>(this)->getMostRecentDecl());
  }
  const EnumDecl *getMostRecentDecl() const {
    return const_cast<EnumDecl*>(this)->getMostRecentDecl();
  }

  EnumDecl *getDefinition() const {
    return cast_or_null<EnumDecl>(TagDecl::getDefinition());
  }

  static EnumDecl *Create(ASTContext &C, DeclContext *DC,
                          SourceLocation StartLoc, SourceLocation IdLoc,
                          IdentifierInfo *Id, EnumDecl *PrevDecl,
                          bool IsScoped, bool IsScopedUsingClassTag,
                          bool IsFixed);
  static EnumDecl *CreateDeserialized(ASTContext &C, unsigned ID);

  /// completeDefinition - When created, the EnumDecl corresponds to a
  /// forward-declared enum. This method is used to mark the
  /// declaration as being defined; it's enumerators have already been
  /// added (via DeclContext::addDecl). NewType is the new underlying
  /// type of the enumeration type.
  void completeDefinition(QualType NewType,
                          QualType PromotionType,
                          unsigned NumPositiveBits,
                          unsigned NumNegativeBits);

  // enumerator_iterator - Iterates through the enumerators of this
  // enumeration.
  using enumerator_iterator = specific_decl_iterator<EnumConstantDecl>;
  using enumerator_range =
      llvm::iterator_range<specific_decl_iterator<EnumConstantDecl>>;

  enumerator_range enumerators() const {
    return enumerator_range(enumerator_begin(), enumerator_end());
  }

  enumerator_iterator enumerator_begin() const {
    const EnumDecl *E = getDefinition();
    if (!E)
      E = this;
    return enumerator_iterator(E->decls_begin());
  }

  enumerator_iterator enumerator_end() const {
    const EnumDecl *E = getDefinition();
    if (!E)
      E = this;
    return enumerator_iterator(E->decls_end());
  }

  /// getPromotionType - Return the integer type that enumerators
  /// should promote to.
  QualType getPromotionType() const { return PromotionType; }

  /// \brief Set the promotion type.
  void setPromotionType(QualType T) { PromotionType = T; }

  /// getIntegerType - Return the integer type this enum decl corresponds to.
  /// This returns a null QualType for an enum forward definition with no fixed
  /// underlying type.
  QualType getIntegerType() const {
    if (!IntegerType)
      return QualType();
    if (const Type *T = IntegerType.dyn_cast<const Type*>())
      return QualType(T, 0);
    return IntegerType.get<TypeSourceInfo*>()->getType().getUnqualifiedType();
  }

  /// \brief Set the underlying integer type.
  void setIntegerType(QualType T) { IntegerType = T.getTypePtrOrNull(); }

  /// \brief Set the underlying integer type source info.
  void setIntegerTypeSourceInfo(TypeSourceInfo *TInfo) { IntegerType = TInfo; }

  /// \brief Return the type source info for the underlying integer type,
  /// if no type source info exists, return 0.
  TypeSourceInfo *getIntegerTypeSourceInfo() const {
    return IntegerType.dyn_cast<TypeSourceInfo*>();
  }

  /// \brief Retrieve the source range that covers the underlying type if
  /// specified.
  SourceRange getIntegerTypeRange() const LLVM_READONLY;

  /// \brief Returns the width in bits required to store all the
  /// non-negative enumerators of this enum.
  unsigned getNumPositiveBits() const {
    return NumPositiveBits;
  }
  void setNumPositiveBits(unsigned Num) {
    NumPositiveBits = Num;
    assert(NumPositiveBits == Num && "can't store this bitcount");
  }

  /// \brief Returns the width in bits required to store all the
  /// negative enumerators of this enum.  These widths include
  /// the rightmost leading 1;  that is:
  ///
  /// MOST NEGATIVE ENUMERATOR     PATTERN     NUM NEGATIVE BITS
  /// ------------------------     -------     -----------------
  ///                       -1     1111111                     1
  ///                      -10     1110110                     5
  ///                     -101     1001011                     8
  unsigned getNumNegativeBits() const {
    return NumNegativeBits;
  }
  void setNumNegativeBits(unsigned Num) {
    NumNegativeBits = Num;
  }

  /// \brief Returns true if this is a C++11 scoped enumeration.
  bool isScoped() const {
    return IsScoped;
  }

  /// \brief Returns true if this is a C++11 scoped enumeration.
  bool isScopedUsingClassTag() const {
    return IsScopedUsingClassTag;
  }

  /// \brief Returns true if this is an Objective-C, C++11, or
  /// Microsoft-style enumeration with a fixed underlying type.
  bool isFixed() const {
    return IsFixed;
  }

  /// \brief Returns true if this can be considered a complete type.
  bool isComplete() const {
    return isCompleteDefinition() || isFixed();
  }

  /// Returns true if this enum is either annotated with
  /// enum_extensibility(closed) or isn't annotated with enum_extensibility.
  bool isClosed() const;

  /// Returns true if this enum is annotated with flag_enum and isn't annotated
  /// with enum_extensibility(open).
  bool isClosedFlag() const;

  /// Returns true if this enum is annotated with neither flag_enum nor
  /// enum_extensibility(open).
  bool isClosedNonFlag() const;

  /// \brief Retrieve the enum definition from which this enumeration could
  /// be instantiated, if it is an instantiation (rather than a non-template).
  EnumDecl *getTemplateInstantiationPattern() const;

  /// \brief Returns the enumeration (declared within the template)
  /// from which this enumeration type was instantiated, or NULL if
  /// this enumeration was not instantiated from any template.
  EnumDecl *getInstantiatedFromMemberEnum() const;

  /// \brief If this enumeration is a member of a specialization of a
  /// templated class, determine what kind of template specialization
  /// or instantiation this is.
  TemplateSpecializationKind getTemplateSpecializationKind() const;

  /// \brief For an enumeration member that was instantiated from a member
  /// enumeration of a templated class, set the template specialiation kind.
  void setTemplateSpecializationKind(TemplateSpecializationKind TSK,
                        SourceLocation PointOfInstantiation = SourceLocation());

  /// \brief If this enumeration is an instantiation of a member enumeration of
  /// a class template specialization, retrieves the member specialization
  /// information.
  MemberSpecializationInfo *getMemberSpecializationInfo() const {
    return SpecializationInfo;
  }

  /// \brief Specify that this enumeration is an instantiation of the
  /// member enumeration ED.
  void setInstantiationOfMemberEnum(EnumDecl *ED,
                                    TemplateSpecializationKind TSK) {
    setInstantiationOfMemberEnum(getASTContext(), ED, TSK);
  }

  static bool classof(const Decl *D) { return classofKind(D->getKind()); }
  static bool classofKind(Kind K) { return K == Enum; }
};

/// RecordDecl - Represents a struct/union/class.  For example:
///   struct X;                  // Forward declaration, no "body".
///   union Y { int A, B; };     // Has body with members A and B (FieldDecls).
/// This decl will be marked invalid if *any* members are invalid.
class RecordDecl : public TagDecl {
  friend class DeclContext;

  // FIXME: This can be packed into the bitfields in Decl.
  /// HasFlexibleArrayMember - This is true if this struct ends with a flexible
  /// array member (e.g. int X[]) or if this union contains a struct that does.
  /// If so, this cannot be contained in arrays or other structs as a member.
  bool HasFlexibleArrayMember : 1;

  /// AnonymousStructOrUnion - Whether this is the type of an anonymous struct
  /// or union.
  bool AnonymousStructOrUnion : 1;

  /// HasObjectMember - This is true if this struct has at least one member
  /// containing an Objective-C object pointer type.
  bool HasObjectMember : 1;
  
  /// HasVolatileMember - This is true if struct has at least one member of
  /// 'volatile' type.
  bool HasVolatileMember : 1;

  /// \brief Whether the field declarations of this record have been loaded
  /// from external storage. To avoid unnecessary deserialization of
  /// methods/nested types we allow deserialization of just the fields
  /// when needed.
  mutable bool LoadedFieldsFromExternalStorage : 1;

protected:
  RecordDecl(Kind DK, TagKind TK, const ASTContext &C, DeclContext *DC,
             SourceLocation StartLoc, SourceLocation IdLoc,
             IdentifierInfo *Id, RecordDecl *PrevDecl);

public:
  static RecordDecl *Create(const ASTContext &C, TagKind TK, DeclContext *DC,
                            SourceLocation StartLoc, SourceLocation IdLoc,
                            IdentifierInfo *Id, RecordDecl* PrevDecl = nullptr);
  static RecordDecl *CreateDeserialized(const ASTContext &C, unsigned ID);

  RecordDecl *getPreviousDecl() {
    return cast_or_null<RecordDecl>(
            static_cast<TagDecl *>(this)->getPreviousDecl());
  }
  const RecordDecl *getPreviousDecl() const {
    return const_cast<RecordDecl*>(this)->getPreviousDecl();
  }

  RecordDecl *getMostRecentDecl() {
    return cast<RecordDecl>(static_cast<TagDecl *>(this)->getMostRecentDecl());
  }
  const RecordDecl *getMostRecentDecl() const {
    return const_cast<RecordDecl*>(this)->getMostRecentDecl();
  }

  bool hasFlexibleArrayMember() const { return HasFlexibleArrayMember; }
  void setHasFlexibleArrayMember(bool V) { HasFlexibleArrayMember = V; }

  /// isAnonymousStructOrUnion - Whether this is an anonymous struct
  /// or union. To be an anonymous struct or union, it must have been
  /// declared without a name and there must be no objects of this
  /// type declared, e.g.,
  /// @code
  ///   union { int i; float f; };
  /// @endcode
  /// is an anonymous union but neither of the following are:
  /// @code
  ///  union X { int i; float f; };
  ///  union { int i; float f; } obj;
  /// @endcode
  bool isAnonymousStructOrUnion() const { return AnonymousStructOrUnion; }
  void setAnonymousStructOrUnion(bool Anon) {
    AnonymousStructOrUnion = Anon;
  }

  bool hasObjectMember() const { return HasObjectMember; }
  void setHasObjectMember (bool val) { HasObjectMember = val; }

  bool hasVolatileMember() const { return HasVolatileMember; }
  void setHasVolatileMember (bool val) { HasVolatileMember = val; }

  bool hasLoadedFieldsFromExternalStorage() const {
    return LoadedFieldsFromExternalStorage;
  }
  void setHasLoadedFieldsFromExternalStorage(bool val) {
    LoadedFieldsFromExternalStorage = val;
  }

  /// \brief Determines whether this declaration represents the
  /// injected class name.
  ///
  /// The injected class name in C++ is the name of the class that
  /// appears inside the class itself. For example:
  ///
  /// \code
  /// struct C {
  ///   // C is implicitly declared here as a synonym for the class name.
  /// };
  ///
  /// C::C c; // same as "C c;"
  /// \endcode
  bool isInjectedClassName() const;

  /// \brief Determine whether this record is a class describing a lambda
  /// function object.
  bool isLambda() const;

  /// \brief Determine whether this record is a record for captured variables in
  /// CapturedStmt construct.
  bool isCapturedRecord() const;

  /// \brief Mark the record as a record for captured variables in CapturedStmt
  /// construct.
  void setCapturedRecord();

  /// getDefinition - Returns the RecordDecl that actually defines
  ///  this struct/union/class.  When determining whether or not a
  ///  struct/union/class is completely defined, one should use this
  ///  method as opposed to 'isCompleteDefinition'.
  ///  'isCompleteDefinition' indicates whether or not a specific
  ///  RecordDecl is a completed definition, not whether or not the
  ///  record type is defined.  This method returns NULL if there is
  ///  no RecordDecl that defines the struct/union/tag.
  RecordDecl *getDefinition() const {
    return cast_or_null<RecordDecl>(TagDecl::getDefinition());
  }

  // Iterator access to field members. The field iterator only visits
  // the non-static data members of this class, ignoring any static
  // data members, functions, constructors, destructors, etc.
  using field_iterator = specific_decl_iterator<FieldDecl>;
  using field_range = llvm::iterator_range<specific_decl_iterator<FieldDecl>>;

  field_range fields() const { return field_range(field_begin(), field_end()); }
  field_iterator field_begin() const;

  field_iterator field_end() const {
    return field_iterator(decl_iterator());
  }

  // field_empty - Whether there are any fields (non-static data
  // members) in this record.
  bool field_empty() const {
    return field_begin() == field_end();
  }

  /// completeDefinition - Notes that the definition of this type is
  /// now complete.
  virtual void completeDefinition();

  static bool classof(const Decl *D) { return classofKind(D->getKind()); }
  static bool classofKind(Kind K) {
    return K >= firstRecord && K <= lastRecord;
  }

  /// \brief Get whether or not this is an ms_struct which can
  /// be turned on with an attribute, pragma, or -mms-bitfields
  /// commandline option.
  bool isMsStruct(const ASTContext &C) const;

  /// \brief Whether we are allowed to insert extra padding between fields.
  /// These padding are added to help AddressSanitizer detect
  /// intra-object-overflow bugs.
  bool mayInsertExtraPadding(bool EmitRemark = false) const;

  /// Finds the first data member which has a name.
  /// nullptr is returned if no named data member exists.
  const FieldDecl *findFirstNamedDataMember() const;  

private:
  /// \brief Deserialize just the fields.
  void LoadFieldsFromExternalStorage() const;
};

class FileScopeAsmDecl : public Decl {
  StringLiteral *AsmString;
  SourceLocation RParenLoc;

  FileScopeAsmDecl(DeclContext *DC, StringLiteral *asmstring,
                   SourceLocation StartL, SourceLocation EndL)
    : Decl(FileScopeAsm, DC, StartL), AsmString(asmstring), RParenLoc(EndL) {}

  virtual void anchor();

public:
  static FileScopeAsmDecl *Create(ASTContext &C, DeclContext *DC,
                                  StringLiteral *Str, SourceLocation AsmLoc,
                                  SourceLocation RParenLoc);

  static FileScopeAsmDecl *CreateDeserialized(ASTContext &C, unsigned ID);
  
  SourceLocation getAsmLoc() const { return getLocation(); }
  SourceLocation getRParenLoc() const { return RParenLoc; }
  void setRParenLoc(SourceLocation L) { RParenLoc = L; }
  SourceRange getSourceRange() const override LLVM_READONLY {
    return SourceRange(getAsmLoc(), getRParenLoc());
  }

  const StringLiteral *getAsmString() const { return AsmString; }
  StringLiteral *getAsmString() { return AsmString; }
  void setAsmString(StringLiteral *Asm) { AsmString = Asm; }

  static bool classof(const Decl *D) { return classofKind(D->getKind()); }
  static bool classofKind(Kind K) { return K == FileScopeAsm; }
};

/// BlockDecl - This represents a block literal declaration, which is like an
/// unnamed FunctionDecl.  For example:
/// ^{ statement-body }   or   ^(int arg1, float arg2){ statement-body }
class BlockDecl : public Decl, public DeclContext {
public:
  /// A class which contains all the information about a particular
  /// captured value.
  class Capture {
    enum {
      flag_isByRef = 0x1,
      flag_isNested = 0x2
    };

    /// The variable being captured.
    llvm::PointerIntPair<VarDecl*, 2> VariableAndFlags;

    /// The copy expression, expressed in terms of a DeclRef (or
    /// BlockDeclRef) to the captured variable.  Only required if the
    /// variable has a C++ class type.
    Expr *CopyExpr;

  public:
    Capture(VarDecl *variable, bool byRef, bool nested, Expr *copy)
      : VariableAndFlags(variable,
                  (byRef ? flag_isByRef : 0) | (nested ? flag_isNested : 0)),
        CopyExpr(copy) {}

    /// The variable being captured.
    VarDecl *getVariable() const { return VariableAndFlags.getPointer(); }

    /// Whether this is a "by ref" capture, i.e. a capture of a __block
    /// variable.
    bool isByRef() const { return VariableAndFlags.getInt() & flag_isByRef; }

    /// Whether this is a nested capture, i.e. the variable captured
    /// is not from outside the immediately enclosing function/block.
    bool isNested() const { return VariableAndFlags.getInt() & flag_isNested; }

    bool hasCopyExpr() const { return CopyExpr != nullptr; }
    Expr *getCopyExpr() const { return CopyExpr; }
    void setCopyExpr(Expr *e) { CopyExpr = e; }
  };

private:
  // FIXME: This can be packed into the bitfields in Decl.
  bool IsVariadic : 1;
  bool CapturesCXXThis : 1;
  bool BlockMissingReturnType : 1;
  bool IsConversionFromLambda : 1;

  /// ParamInfo - new[]'d array of pointers to ParmVarDecls for the formal
  /// parameters of this function.  This is null if a prototype or if there are
  /// no formals.
  ParmVarDecl **ParamInfo = nullptr;
  unsigned NumParams = 0;

  Stmt *Body = nullptr;
  TypeSourceInfo *SignatureAsWritten = nullptr;

  const Capture *Captures = nullptr;
  unsigned NumCaptures = 0;

  unsigned ManglingNumber = 0;
  Decl *ManglingContextDecl = nullptr;

protected:
  BlockDecl(DeclContext *DC, SourceLocation CaretLoc)
      : Decl(Block, DC, CaretLoc), DeclContext(Block), IsVariadic(false),
        CapturesCXXThis(false), BlockMissingReturnType(true),
        IsConversionFromLambda(false) {}

public:
  static BlockDecl *Create(ASTContext &C, DeclContext *DC, SourceLocation L); 
  static BlockDecl *CreateDeserialized(ASTContext &C, unsigned ID);
  
  SourceLocation getCaretLocation() const { return getLocation(); }

  bool isVariadic() const { return IsVariadic; }
  void setIsVariadic(bool value) { IsVariadic = value; }

  CompoundStmt *getCompoundBody() const { return (CompoundStmt*) Body; }
  Stmt *getBody() const override { return (Stmt*) Body; }
  void setBody(CompoundStmt *B) { Body = (Stmt*) B; }

  void setSignatureAsWritten(TypeSourceInfo *Sig) { SignatureAsWritten = Sig; }
  TypeSourceInfo *getSignatureAsWritten() const { return SignatureAsWritten; }

  // ArrayRef access to formal parameters.
  ArrayRef<ParmVarDecl *> parameters() const {
    return {ParamInfo, getNumParams()};
  }
  MutableArrayRef<ParmVarDecl *> parameters() {
    return {ParamInfo, getNumParams()};
  }

  // Iterator access to formal parameters.
  using param_iterator = MutableArrayRef<ParmVarDecl *>::iterator;
  using param_const_iterator = ArrayRef<ParmVarDecl *>::const_iterator;

  bool param_empty() const { return parameters().empty(); }
  param_iterator param_begin() { return parameters().begin(); }
  param_iterator param_end() { return parameters().end(); }
  param_const_iterator param_begin() const { return parameters().begin(); }
  param_const_iterator param_end() const { return parameters().end(); }
  size_t param_size() const { return parameters().size(); }

  unsigned getNumParams() const { return NumParams; }

  const ParmVarDecl *getParamDecl(unsigned i) const {
    assert(i < getNumParams() && "Illegal param #");
    return ParamInfo[i];
  }
  ParmVarDecl *getParamDecl(unsigned i) {
    assert(i < getNumParams() && "Illegal param #");
    return ParamInfo[i];
  }

  void setParams(ArrayRef<ParmVarDecl *> NewParamInfo);

  /// hasCaptures - True if this block (or its nested blocks) captures
  /// anything of local storage from its enclosing scopes.
  bool hasCaptures() const { return NumCaptures != 0 || CapturesCXXThis; }

  /// getNumCaptures - Returns the number of captured variables.
  /// Does not include an entry for 'this'.
  unsigned getNumCaptures() const { return NumCaptures; }

  using capture_const_iterator = ArrayRef<Capture>::const_iterator;

  ArrayRef<Capture> captures() const { return {Captures, NumCaptures}; }

  capture_const_iterator capture_begin() const { return captures().begin(); }
  capture_const_iterator capture_end() const { return captures().end(); }

  bool capturesCXXThis() const { return CapturesCXXThis; }
  bool blockMissingReturnType() const { return BlockMissingReturnType; }
  void setBlockMissingReturnType(bool val) { BlockMissingReturnType = val; }

  bool isConversionFromLambda() const { return IsConversionFromLambda; }
  void setIsConversionFromLambda(bool val) { IsConversionFromLambda = val; }

  bool capturesVariable(const VarDecl *var) const;

  void setCaptures(ASTContext &Context, ArrayRef<Capture> Captures,
                   bool CapturesCXXThis);

   unsigned getBlockManglingNumber() const {
     return ManglingNumber;
   }

   Decl *getBlockManglingContextDecl() const {
     return ManglingContextDecl;    
   }

  void setBlockMangling(unsigned Number, Decl *Ctx) {
    ManglingNumber = Number;
    ManglingContextDecl = Ctx;
  }

  SourceRange getSourceRange() const override LLVM_READONLY;

  // Implement isa/cast/dyncast/etc.
  static bool classof(const Decl *D) { return classofKind(D->getKind()); }
  static bool classofKind(Kind K) { return K == Block; }
  static DeclContext *castToDeclContext(const BlockDecl *D) {
    return static_cast<DeclContext *>(const_cast<BlockDecl*>(D));
  }
  static BlockDecl *castFromDeclContext(const DeclContext *DC) {
    return static_cast<BlockDecl *>(const_cast<DeclContext*>(DC));
  }
};

/// \brief This represents the body of a CapturedStmt, and serves as its
/// DeclContext.
class CapturedDecl final
    : public Decl,
      public DeclContext,
      private llvm::TrailingObjects<CapturedDecl, ImplicitParamDecl *> {
protected:
  size_t numTrailingObjects(OverloadToken<ImplicitParamDecl>) {
    return NumParams;
  }

private:
  /// \brief The number of parameters to the outlined function.
  unsigned NumParams;

  /// \brief The position of context parameter in list of parameters.
  unsigned ContextParam;

  /// \brief The body of the outlined function.
  llvm::PointerIntPair<Stmt *, 1, bool> BodyAndNothrow;

  explicit CapturedDecl(DeclContext *DC, unsigned NumParams);

  ImplicitParamDecl *const *getParams() const {
    return getTrailingObjects<ImplicitParamDecl *>();
  }

  ImplicitParamDecl **getParams() {
    return getTrailingObjects<ImplicitParamDecl *>();
  }

public:
  friend class ASTDeclReader;
  friend class ASTDeclWriter;
  friend TrailingObjects;

  static CapturedDecl *Create(ASTContext &C, DeclContext *DC,
                              unsigned NumParams);
  static CapturedDecl *CreateDeserialized(ASTContext &C, unsigned ID,
                                          unsigned NumParams);

  Stmt *getBody() const override;
  void setBody(Stmt *B);

  bool isNothrow() const;
  void setNothrow(bool Nothrow = true);

  unsigned getNumParams() const { return NumParams; }

  ImplicitParamDecl *getParam(unsigned i) const {
    assert(i < NumParams);
    return getParams()[i];
  }
  void setParam(unsigned i, ImplicitParamDecl *P) {
    assert(i < NumParams);
    getParams()[i] = P;
  }

  // ArrayRef interface to parameters.
  ArrayRef<ImplicitParamDecl *> parameters() const {
    return {getParams(), getNumParams()};
  }
  MutableArrayRef<ImplicitParamDecl *> parameters() {
    return {getParams(), getNumParams()};
  }

  /// \brief Retrieve the parameter containing captured variables.
  ImplicitParamDecl *getContextParam() const {
    assert(ContextParam < NumParams);
    return getParam(ContextParam);
  }
  void setContextParam(unsigned i, ImplicitParamDecl *P) {
    assert(i < NumParams);
    ContextParam = i;
    setParam(i, P);
  }
  unsigned getContextParamPosition() const { return ContextParam; }

  using param_iterator = ImplicitParamDecl *const *;
  using param_range = llvm::iterator_range<param_iterator>;

  /// \brief Retrieve an iterator pointing to the first parameter decl.
  param_iterator param_begin() const { return getParams(); }
  /// \brief Retrieve an iterator one past the last parameter decl.
  param_iterator param_end() const { return getParams() + NumParams; }

  // Implement isa/cast/dyncast/etc.
  static bool classof(const Decl *D) { return classofKind(D->getKind()); }
  static bool classofKind(Kind K) { return K == Captured; }
  static DeclContext *castToDeclContext(const CapturedDecl *D) {
    return static_cast<DeclContext *>(const_cast<CapturedDecl *>(D));
  }
  static CapturedDecl *castFromDeclContext(const DeclContext *DC) {
    return static_cast<CapturedDecl *>(const_cast<DeclContext *>(DC));
  }
};

/// \brief Describes a module import declaration, which makes the contents
/// of the named module visible in the current translation unit.
///
/// An import declaration imports the named module (or submodule). For example:
/// \code
///   @import std.vector;
/// \endcode
///
/// Import declarations can also be implicitly generated from
/// \#include/\#import directives.
class ImportDecl final : public Decl,
                         llvm::TrailingObjects<ImportDecl, SourceLocation> {
  friend class ASTContext;
  friend class ASTDeclReader;
  friend class ASTReader;
  friend TrailingObjects;

  /// \brief The imported module, along with a bit that indicates whether
  /// we have source-location information for each identifier in the module
  /// name. 
  ///
  /// When the bit is false, we only have a single source location for the
  /// end of the import declaration.
  llvm::PointerIntPair<Module *, 1, bool> ImportedAndComplete;
  
  /// \brief The next import in the list of imports local to the translation
  /// unit being parsed (not loaded from an AST file).
  ImportDecl *NextLocalImport = nullptr;
  
  ImportDecl(DeclContext *DC, SourceLocation StartLoc, Module *Imported,
             ArrayRef<SourceLocation> IdentifierLocs);

  ImportDecl(DeclContext *DC, SourceLocation StartLoc, Module *Imported,
             SourceLocation EndLoc);

  ImportDecl(EmptyShell Empty) : Decl(Import, Empty) {}
  
public:
  /// \brief Create a new module import declaration.
  static ImportDecl *Create(ASTContext &C, DeclContext *DC, 
                            SourceLocation StartLoc, Module *Imported,
                            ArrayRef<SourceLocation> IdentifierLocs);
  
  /// \brief Create a new module import declaration for an implicitly-generated
  /// import.
  static ImportDecl *CreateImplicit(ASTContext &C, DeclContext *DC, 
                                    SourceLocation StartLoc, Module *Imported, 
                                    SourceLocation EndLoc);
  
  /// \brief Create a new, deserialized module import declaration.
  static ImportDecl *CreateDeserialized(ASTContext &C, unsigned ID, 
                                        unsigned NumLocations);
  
  /// \brief Retrieve the module that was imported by the import declaration.
  Module *getImportedModule() const { return ImportedAndComplete.getPointer(); }
  
  /// \brief Retrieves the locations of each of the identifiers that make up
  /// the complete module name in the import declaration.
  ///
  /// This will return an empty array if the locations of the individual
  /// identifiers aren't available.
  ArrayRef<SourceLocation> getIdentifierLocs() const;

  SourceRange getSourceRange() const override LLVM_READONLY;

  static bool classof(const Decl *D) { return classofKind(D->getKind()); }
  static bool classofKind(Kind K) { return K == Import; }
};

/// \brief Represents a C++ Modules TS module export declaration.
///
/// For example:
/// \code
///   export void foo();
/// \endcode
class ExportDecl final : public Decl, public DeclContext {
  virtual void anchor();

private:
  friend class ASTDeclReader;

  /// \brief The source location for the right brace (if valid).
  SourceLocation RBraceLoc;

  ExportDecl(DeclContext *DC, SourceLocation ExportLoc)
      : Decl(Export, DC, ExportLoc), DeclContext(Export),
        RBraceLoc(SourceLocation()) {}

public:
  static ExportDecl *Create(ASTContext &C, DeclContext *DC,
                            SourceLocation ExportLoc);
  static ExportDecl *CreateDeserialized(ASTContext &C, unsigned ID);
  
  SourceLocation getExportLoc() const { return getLocation(); }
  SourceLocation getRBraceLoc() const { return RBraceLoc; }
  void setRBraceLoc(SourceLocation L) { RBraceLoc = L; }

  SourceLocation getLocEnd() const LLVM_READONLY {
    if (RBraceLoc.isValid())
      return RBraceLoc;
    // No braces: get the end location of the (only) declaration in context
    // (if present).
    return decls_empty() ? getLocation() : decls_begin()->getLocEnd();
  }

  SourceRange getSourceRange() const override LLVM_READONLY {
    return SourceRange(getLocation(), getLocEnd());
  }

  static bool classof(const Decl *D) { return classofKind(D->getKind()); }
  static bool classofKind(Kind K) { return K == Export; }
  static DeclContext *castToDeclContext(const ExportDecl *D) {
    return static_cast<DeclContext *>(const_cast<ExportDecl*>(D));
  }
  static ExportDecl *castFromDeclContext(const DeclContext *DC) {
    return static_cast<ExportDecl *>(const_cast<DeclContext*>(DC));
  }
};

/// \brief Represents an empty-declaration.
class EmptyDecl : public Decl {
  EmptyDecl(DeclContext *DC, SourceLocation L) : Decl(Empty, DC, L) {}

  virtual void anchor();

public:
  static EmptyDecl *Create(ASTContext &C, DeclContext *DC,
                           SourceLocation L);
  static EmptyDecl *CreateDeserialized(ASTContext &C, unsigned ID);

  static bool classof(const Decl *D) { return classofKind(D->getKind()); }
  static bool classofKind(Kind K) { return K == Empty; }
};

/// Insertion operator for diagnostics.  This allows sending NamedDecl's
/// into a diagnostic with <<.
inline const DiagnosticBuilder &operator<<(const DiagnosticBuilder &DB,
                                           const NamedDecl* ND) {
  DB.AddTaggedVal(reinterpret_cast<intptr_t>(ND),
                  DiagnosticsEngine::ak_nameddecl);
  return DB;
}
inline const PartialDiagnostic &operator<<(const PartialDiagnostic &PD,
                                           const NamedDecl* ND) {
  PD.AddTaggedVal(reinterpret_cast<intptr_t>(ND),
                  DiagnosticsEngine::ak_nameddecl);
  return PD;
}

template<typename decl_type>
void Redeclarable<decl_type>::setPreviousDecl(decl_type *PrevDecl) {
  // Note: This routine is implemented here because we need both NamedDecl
  // and Redeclarable to be defined.
  assert(RedeclLink.NextIsLatest() &&
         "setPreviousDecl on a decl already in a redeclaration chain");

  if (PrevDecl) {
    // Point to previous. Make sure that this is actually the most recent
    // redeclaration, or we can build invalid chains. If the most recent
    // redeclaration is invalid, it won't be PrevDecl, but we want it anyway.
    First = PrevDecl->getFirstDecl();
    assert(First->RedeclLink.NextIsLatest() && "Expected first");
    decl_type *MostRecent = First->getNextRedeclaration();
    RedeclLink = PreviousDeclLink(cast<decl_type>(MostRecent));

    // If the declaration was previously visible, a redeclaration of it remains
    // visible even if it wouldn't be visible by itself.
    static_cast<decl_type*>(this)->IdentifierNamespace |=
      MostRecent->getIdentifierNamespace() &
      (Decl::IDNS_Ordinary | Decl::IDNS_Tag | Decl::IDNS_Type);
  } else {
    // Make this first.
    First = static_cast<decl_type*>(this);
  }

  // First one will point to this one as latest.
  First->RedeclLink.setLatest(static_cast<decl_type*>(this));

  assert(!isa<NamedDecl>(static_cast<decl_type*>(this)) ||
         cast<NamedDecl>(static_cast<decl_type*>(this))->isLinkageValid());
}

// Inline function definitions.

/// \brief Check if the given decl is complete.
///
/// We use this function to break a cycle between the inline definitions in
/// Type.h and Decl.h.
inline bool IsEnumDeclComplete(EnumDecl *ED) {
  return ED->isComplete();
}

/// \brief Check if the given decl is scoped.
///
/// We use this function to break a cycle between the inline definitions in
/// Type.h and Decl.h.
inline bool IsEnumDeclScoped(EnumDecl *ED) {
  return ED->isScoped();
}

} // namespace clang

#endif // LLVM_CLANG_AST_DECL_H

//===--- tools/extra/clang-tidy/ClangTidyDiagnosticConsumer.cpp ----------=== //
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
// And has the following additional copyright:
//
// (C) Copyright 2016-2022 Xilinx, Inc.
// All Rights Reserved.
//
//===----------------------------------------------------------------------===//
///
///  \file This file implements ClangTidyDiagnosticConsumer, ClangTidyContext
///  and ClangTidyError classes.
///
///  This tool uses the Clang Tooling infrastructure, see
///    http://clang.llvm.org/docs/HowToSetupToolingForLLVM.html
///  for details on setting it up with LLVM source tree.
///
//===----------------------------------------------------------------------===//

#include "ClangTidyDiagnosticConsumer.h"
#include "ClangTidyOptions.h"
#include "clang/AST/ASTDiagnostic.h"
#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Frontend/DiagnosticRenderer.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/Path.h"
#include <tuple>
#include <vector>
using namespace clang;
using namespace tidy;

namespace {
class ClangTidyDiagnosticRenderer : public DiagnosticRenderer {
public:
  ClangTidyDiagnosticRenderer(const LangOptions &LangOpts,
                              DiagnosticOptions *DiagOpts,
                              ClangTidyError &Error)
      : DiagnosticRenderer(LangOpts, DiagOpts), Error(Error) {}

protected:
  void emitDiagnosticMessage(FullSourceLoc Loc, PresumedLoc PLoc,
                             DiagnosticsEngine::Level Level, StringRef Message,
                             ArrayRef<CharSourceRange> Ranges,
                             DiagOrStoredDiag Info) override {
    // Remove check name from the message.
    // FIXME: Remove this once there's a better way to pass check names than
    // appending the check name to the message in ClangTidyContext::diag and
    // using getCustomDiagID.
    std::string CheckNameInMessage = " [" + Error.DiagnosticName + "]";
    if (Message.endswith(CheckNameInMessage))
      Message = Message.substr(0, Message.size() - CheckNameInMessage.size());

    auto TidyMessage =
        Loc.isValid()
            ? tooling::DiagnosticMessage(Message, Loc.getManager(), Loc)
            : tooling::DiagnosticMessage(Message);
    if (Level == DiagnosticsEngine::Note) {
      Error.Notes.push_back(TidyMessage);
      return;
    }
    assert(Error.Message.Message.empty() && "Overwriting a diagnostic message");
    Error.Message = TidyMessage;
  }

  void emitDiagnosticLoc(FullSourceLoc Loc, PresumedLoc PLoc,
                         DiagnosticsEngine::Level Level,
                         ArrayRef<CharSourceRange> Ranges) override {}

  void emitCodeContext(FullSourceLoc Loc, DiagnosticsEngine::Level Level,
                       SmallVectorImpl<CharSourceRange> &Ranges,
                       ArrayRef<FixItHint> Hints) override {
    assert(Loc.isValid());
    for (const auto &FixIt : Hints) {
      CharSourceRange Range = FixIt.RemoveRange;
      assert(Range.getBegin().isValid() && Range.getEnd().isValid() &&
             "Invalid range in the fix-it hint.");
      assert(Range.getBegin().isFileID() && Range.getEnd().isFileID() &&
             "Only file locations supported in fix-it hints.");

      tooling::Replacement Replacement(Loc.getManager(), Range,
                                       FixIt.CodeToInsert);
      llvm::Error Err = Error.Fix[Replacement.getFilePath()].add(Replacement);
      if (Err) {
        // FIXME: better error handling. Now if there are conflicted fixes,
        //        neither one will be applied.
        llvm::errs() << "Fix conflicts with existing fix! "
                     << llvm::toString(std::move(Err)) << "\n";
        Error.HasConflictedFixes = true;
      }
    }
  }

  void emitIncludeLocation(FullSourceLoc Loc, PresumedLoc PLoc) override {}

  void emitImportLocation(FullSourceLoc Loc, PresumedLoc PLoc,
                          StringRef ModuleName) override {}

  void emitBuildingModuleLocation(FullSourceLoc Loc, PresumedLoc PLoc,
                                  StringRef ModuleName) override {}

  void endDiagnostic(DiagOrStoredDiag D,
                     DiagnosticsEngine::Level Level) override {
    assert(!Error.Message.Message.empty() && "Message has not been set");
  }

private:
  ClangTidyError &Error;
};
} // end anonymous namespace

ClangTidyError::ClangTidyError(StringRef CheckName,
                               ClangTidyError::Level DiagLevel,
                               StringRef BuildDirectory, bool IsWarningAsError,
                               bool HasConflictedFixes = false,
                               PresumedLoc Loc = PresumedLoc(),
                               std::string FilePath = std::string())
    : tooling::Diagnostic(CheckName, DiagLevel, BuildDirectory),
      IsWarningAsError(IsWarningAsError),
      HasConflictedFixes(HasConflictedFixes), RealLoc(Loc),
      RealFilePath(FilePath) {
}

// Returns true if GlobList starts with the negative indicator ('-'), removes it
// from the GlobList.
static bool ConsumeNegativeIndicator(StringRef &GlobList) {
  GlobList = GlobList.trim(" \r\n");
  if (GlobList.startswith("-")) {
    GlobList = GlobList.substr(1);
    return true;
  }
  return false;
}
// Converts first glob from the comma-separated list of globs to Regex and
// removes it and the trailing comma from the GlobList.
static llvm::Regex ConsumeGlob(StringRef &GlobList) {
  StringRef UntrimmedGlob = GlobList.substr(0, GlobList.find(','));
  StringRef Glob = UntrimmedGlob.trim(' ');
  GlobList = GlobList.substr(UntrimmedGlob.size() + 1);
  SmallString<128> RegexText("^");
  StringRef MetaChars("()^$|*+?.[]\\{}");
  for (char C : Glob) {
    if (C == '*')
      RegexText.push_back('.');
    else if (MetaChars.find(C) != StringRef::npos)
      RegexText.push_back('\\');
    RegexText.push_back(C);
  }
  RegexText.push_back('$');
  return llvm::Regex(RegexText);
}

GlobList::GlobList(StringRef Globs)
    : Positive(!ConsumeNegativeIndicator(Globs)), Regex(ConsumeGlob(Globs)),
      NextGlob(Globs.empty() ? nullptr : new GlobList(Globs)) {}

bool GlobList::contains(StringRef S, bool Contains) {
  if (Regex.match(S))
    Contains = Positive;

  if (NextGlob)
    Contains = NextGlob->contains(S, Contains);
  return Contains;
}

class ClangTidyContext::CachedGlobList {
public:
  CachedGlobList(StringRef Globs) : Globs(Globs) {}

  bool contains(StringRef S) {
    switch (auto &Result = Cache[S]) {
      case Yes: return true;
      case No: return false;
      case None:
        Result = Globs.contains(S) ? Yes : No;
        return Result == Yes;
    }
    llvm_unreachable("invalid enum");
  }

private:
  GlobList Globs;
  enum Tristate { None, Yes, No };
  llvm::StringMap<Tristate> Cache;
};

ClangTidyContext::ClangTidyContext(
    std::unique_ptr<ClangTidyOptionsProvider> OptionsProvider)
    : DiagEngine(nullptr), OptionsProvider(std::move(OptionsProvider)),
      Profile(nullptr) {
  // Before the first translation unit we can get errors related to command-line
  // parsing, use empty string for the file name in this case.
  setCurrentFile("");
}

ClangTidyContext::~ClangTidyContext() = default;

DiagnosticBuilder ClangTidyContext::diag(
    StringRef CheckName, SourceLocation Loc, StringRef Description,
    DiagnosticIDs::Level Level /* = DiagnosticIDs::Warning*/) {
  assert(Loc.isValid());
  unsigned ID = DiagEngine->getDiagnosticIDs()->getCustomDiagID(
      Level, (Description + " [" + CheckName + "]").str());
  CheckNamesByDiagnosticID.try_emplace(ID, CheckName);
  return DiagEngine->Report(Loc, ID);
}

void ClangTidyContext::setDiagnosticsEngine(DiagnosticsEngine *Engine) {
  DiagEngine = Engine;
}

void ClangTidyContext::setSourceManager(SourceManager *SourceMgr) {
  DiagEngine->setSourceManager(SourceMgr);
  this->SourceMgr = SourceMgr; 
}

void ClangTidyContext::setCurrentFile(StringRef File) {
  CurrentFile = File;
  CurrentOptions = getOptionsForFile(CurrentFile);
  CheckFilter = llvm::make_unique<CachedGlobList>(*getOptions().Checks);
  WarningAsErrorFilter =
      llvm::make_unique<CachedGlobList>(*getOptions().WarningsAsErrors);
}

void ClangTidyContext::setASTContext(ASTContext *Context) {
  DiagEngine->SetArgToStringFn(&FormatASTNodeDiagnosticArgument, Context);
  LangOpts = Context->getLangOpts();
  this->astContext = Context; 
}

const ClangTidyGlobalOptions &ClangTidyContext::getGlobalOptions() const {
  return OptionsProvider->getGlobalOptions();
}

const ClangTidyOptions &ClangTidyContext::getOptions() const {
  return CurrentOptions;
}

ClangTidyOptions ClangTidyContext::getOptionsForFile(StringRef File) const {
  // Merge options on top of getDefaults() as a safeguard against options with
  // unset values.
  return ClangTidyOptions::getDefaults().mergeWith(
      OptionsProvider->getOptions(File));
}

void ClangTidyContext::setCheckProfileData(ProfileData *P) { Profile = P; }

bool ClangTidyContext::isCheckEnabled(StringRef CheckName) const {
  assert(CheckFilter != nullptr);
  return CheckFilter->contains(CheckName);
}

bool ClangTidyContext::treatAsError(StringRef CheckName) const {
  assert(WarningAsErrorFilter != nullptr);
  return WarningAsErrorFilter->contains(CheckName);
}

/// \brief Store a \c ClangTidyError.
void ClangTidyContext::storeError(const ClangTidyError &Error) {
  Errors.push_back(Error);
}

StringRef ClangTidyContext::getCheckName(unsigned DiagnosticID) const {
  llvm::DenseMap<unsigned, std::string>::const_iterator I =
      CheckNamesByDiagnosticID.find(DiagnosticID);
  if (I != CheckNamesByDiagnosticID.end())
    return I->second;
  return "";
}

ClangTidyDiagnosticConsumer::ClangTidyDiagnosticConsumer(
    ClangTidyContext &Ctx, bool RemoveIncompatibleErrors)
    : Context(Ctx), RemoveIncompatibleErrors(RemoveIncompatibleErrors),
      LastErrorRelatesToUserCode(false), LastErrorPassesLineFilter(false),
      LastErrorWasIgnored(false) {
  IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts = new DiagnosticOptions();
  Diags = llvm::make_unique<DiagnosticsEngine>(
      IntrusiveRefCntPtr<DiagnosticIDs>(new DiagnosticIDs), &*DiagOpts, this,
      /*ShouldOwnClient=*/false);
  DiagOpts->ShowPresumedLoc = 1;
  Context.setDiagnosticsEngine(Diags.get());
}

void ClangTidyDiagnosticConsumer::finalizeLastError() {
  if (!Errors.empty()) {
    ClangTidyError &Error = Errors.back();
    if (!Context.isCheckEnabled(Error.DiagnosticName) &&
        Error.DiagLevel != ClangTidyError::Error) {
      ++Context.Stats.ErrorsIgnoredCheckFilter;
      Errors.pop_back();
    } else if (!LastErrorRelatesToUserCode) {
      ++Context.Stats.ErrorsIgnoredNonUserCode;
      Errors.pop_back();
    } else if (!LastErrorPassesLineFilter) {
      ++Context.Stats.ErrorsIgnoredLineFilter;
      Errors.pop_back();
    } else {
      ++Context.Stats.ErrorsDisplayed;
    }
  }
  LastErrorRelatesToUserCode = false;
  LastErrorPassesLineFilter = false;
}

static bool IsNOLINTFound(StringRef NolintDirectiveText, StringRef Line,
                          unsigned DiagID, const ClangTidyContext &Context) {
  const size_t NolintIndex = Line.find(NolintDirectiveText);
  if (NolintIndex == StringRef::npos)
    return false;

  size_t BracketIndex = NolintIndex + NolintDirectiveText.size();
  // Check if the specific checks are specified in brackets.
  if (BracketIndex < Line.size() && Line[BracketIndex] == '(') {
    ++BracketIndex;
    const size_t BracketEndIndex = Line.find(')', BracketIndex);
    if (BracketEndIndex != StringRef::npos) {
      StringRef ChecksStr =
          Line.substr(BracketIndex, BracketEndIndex - BracketIndex);
      // Allow disabling all the checks with "*".
      if (ChecksStr != "*") {
        StringRef CheckName = Context.getCheckName(DiagID);
        // Allow specifying a few check names, delimited with comma.
        SmallVector<StringRef, 1> Checks;
        ChecksStr.split(Checks, ',', -1, false);
        llvm::transform(Checks, Checks.begin(),
                        [](StringRef S) { return S.trim(); });
        return llvm::find(Checks, CheckName) != Checks.end();
      }
    }
  }
  return true;
}

static bool LineIsMarkedWithNOLINT(SourceManager &SM, SourceLocation Loc,
                                   unsigned DiagID,
                                   const ClangTidyContext &Context) {
  bool Invalid;
  const char *CharacterData = SM.getCharacterData(Loc, &Invalid);
  if (Invalid)
    return false;

  // Check if there's a NOLINT on this line.
  const char *P = CharacterData;
  while (*P != '\0' && *P != '\r' && *P != '\n')
    ++P;
  StringRef RestOfLine(CharacterData, P - CharacterData + 1);
  if (IsNOLINTFound("NOLINT", RestOfLine, DiagID, Context))
    return true;

  // Check if there's a NOLINTNEXTLINE on the previous line.
  const char *BufBegin =
      SM.getCharacterData(SM.getLocForStartOfFile(SM.getFileID(Loc)), &Invalid);
  if (Invalid || P == BufBegin)
    return false;

  // Scan backwards over the current line.
  P = CharacterData;
  while (P != BufBegin && *P != '\n')
    --P;

  // If we reached the begin of the file there is no line before it.
  if (P == BufBegin)
    return false;

  // Skip over the newline.
  --P;
  const char *LineEnd = P;

  // Now we're on the previous line. Skip to the beginning of it.
  while (P != BufBegin && *P != '\n')
    --P;

  RestOfLine = StringRef(P, LineEnd - P + 1);
  if (IsNOLINTFound("NOLINTNEXTLINE", RestOfLine, DiagID, Context))
    return true;

  return false;
}

static bool LineIsMarkedWithNOLINTinMacro(SourceManager &SM, SourceLocation Loc,
                                          unsigned DiagID,
                                          const ClangTidyContext &Context) {
  while (true) {
    if (LineIsMarkedWithNOLINT(SM, Loc, DiagID, Context))
      return true;
    if (!Loc.isMacroID())
      return false;
    Loc = SM.getImmediateExpansionRange(Loc).first;
  }
  return false;
}

static std::string GetAbsoluteFilename(const Diagnostic &Info,
                                       PresumedLoc PLoc) {
  if (PLoc.isInvalid() || !Info.hasSourceManager())
    return std::string();
  const auto &SM = Info.getSourceManager();
  StringRef Filename = PLoc.getFilename();
  SmallVector<char, 128> AbsoluteFilename;
  const DirectoryEntry *Dir =
      SM.getFileManager().getDirectory(llvm::sys::path::parent_path(Filename));
  if (Dir) {
    StringRef DirName = SM.getFileManager().getCanonicalName(Dir);
    llvm::sys::path::append(AbsoluteFilename, DirName,
                            llvm::sys::path::filename(Filename));
    Filename = StringRef(AbsoluteFilename.data(), AbsoluteFilename.size());
  }
  return Filename.str();
}

void ClangTidyDiagnosticConsumer::HandleDiagnostic(
    DiagnosticsEngine::Level DiagLevel, const Diagnostic &Info) {
  if (LastErrorWasIgnored && DiagLevel == DiagnosticsEngine::Note)
    return;

  if (Info.getLocation().isValid() && DiagLevel != DiagnosticsEngine::Error &&
      DiagLevel != DiagnosticsEngine::Fatal &&
      LineIsMarkedWithNOLINTinMacro(Diags->getSourceManager(),
                                    Info.getLocation(), Info.getID(),
                                    Context)) {
    ++Context.Stats.ErrorsIgnoredNOLINT;
    // Ignored a warning, should ignore related notes as well
    LastErrorWasIgnored = true;
    return;
  }

  LastErrorWasIgnored = false;
  // Count warnings/errors.
  DiagnosticConsumer::HandleDiagnostic(DiagLevel, Info);
  FullSourceLoc Loc =
      (Info.getLocation().isInvalid())
          ? FullSourceLoc()
          : FullSourceLoc(Info.getLocation(), Info.getSourceManager());
  PresumedLoc PLoc = Loc.getPresumedLoc();
  auto FilePath = GetAbsoluteFilename(Info, PLoc);

  if (DiagLevel == DiagnosticsEngine::Note) {
    assert(!Errors.empty() &&
           "A diagnostic note can only be appended to a message.");
  } else {
    finalizeLastError();
    StringRef WarningOption =
        Context.DiagEngine->getDiagnosticIDs()->getWarningOptionForDiag(
            Info.getID());
    std::string CheckName = !WarningOption.empty()
                                ? ("clang-diagnostic-" + WarningOption).str()
                                : Context.getCheckName(Info.getID()).str();

    if (CheckName.empty()) {
      // This is a compiler diagnostic without a warning option. Assign check
      // name based on its level.
      switch (DiagLevel) {
      case DiagnosticsEngine::Error:
      case DiagnosticsEngine::Fatal:
        CheckName = "clang-diagnostic-error";
        break;
      case DiagnosticsEngine::Warning:
        CheckName = "clang-diagnostic-warning";
        break;
      default:
        CheckName = "clang-diagnostic-unknown";
        break;
      }
    }

    ClangTidyError::Level Level = ClangTidyError::Warning;
    if (DiagLevel == DiagnosticsEngine::Error ||
        DiagLevel == DiagnosticsEngine::Fatal) {
      // Force reporting of Clang errors regardless of filters and non-user
      // code.
      Level = ClangTidyError::Error;
      LastErrorRelatesToUserCode = true;
      LastErrorPassesLineFilter = true;
    }
    bool IsWarningAsError = DiagLevel == DiagnosticsEngine::Warning &&
                            Context.treatAsError(CheckName);
    Errors.emplace_back(CheckName, Level, Context.getCurrentBuildDirectory(),
                        IsWarningAsError, false /* HasConflictedFixes */, PLoc,
                        FilePath);
  }

  ClangTidyDiagnosticRenderer Converter(
      Context.getLangOpts(), &Context.DiagEngine->getDiagnosticOptions(),
      Errors.back());
  SmallString<100> Message;
  Info.FormatDiagnostic(Message);

  Converter.emitDiagnostic(Loc, DiagLevel, Message, Info.getRanges(),
                           Info.getFixItHints());

  checkFilters(Info.getLocation());
}

bool ClangTidyDiagnosticConsumer::passesLineFilter(StringRef FileName,
                                                   unsigned LineNumber) const {
  if (Context.getGlobalOptions().LineFilter.empty())
    return true;
  for (const FileFilter &Filter : Context.getGlobalOptions().LineFilter) {
    if (FileName.endswith(Filter.Name)) {
      if (Filter.LineRanges.empty())
        return true;
      for (const FileFilter::LineRange &Range : Filter.LineRanges) {
        if (Range.first <= LineNumber && LineNumber <= Range.second)
          return true;
      }
      return false;
    }
  }
  return false;
}

void ClangTidyDiagnosticConsumer::checkFilters(SourceLocation Location) {
  // Invalid location may mean a diagnostic in a command line, don't skip these.
  if (!Location.isValid()) {
    LastErrorRelatesToUserCode = true;
    LastErrorPassesLineFilter = true;
    return;
  }

  const SourceManager &Sources = Diags->getSourceManager();
  if (!*Context.getOptions().SystemHeaders &&
      Sources.isInSystemHeader(Location))
    return;

  // FIXME: We start with a conservative approach here, but the actual type of
  // location needed depends on the check (in particular, where this check wants
  // to apply fixes).
  FileID FID = Sources.getDecomposedExpansionLoc(Location).first;
  const FileEntry *File = Sources.getFileEntryForID(FID);

  // -DMACRO definitions on the command line have locations in a virtual buffer
  // that doesn't have a FileEntry. Don't skip these as well.
  if (!File) {
    LastErrorRelatesToUserCode = true;
    LastErrorPassesLineFilter = true;
    return;
  }

  StringRef FileName(File->getName());
  LastErrorRelatesToUserCode = LastErrorRelatesToUserCode ||
                               Sources.isInMainFile(Location) ||
                               getHeaderFilter()->match(FileName);

  unsigned LineNumber = Sources.getExpansionLineNumber(Location);
  LastErrorPassesLineFilter =
      LastErrorPassesLineFilter || passesLineFilter(FileName, LineNumber);
}

llvm::Regex *ClangTidyDiagnosticConsumer::getHeaderFilter() {
  if (!HeaderFilter)
    HeaderFilter =
        llvm::make_unique<llvm::Regex>(*Context.getOptions().HeaderFilterRegex);
  return HeaderFilter.get();
}

void ClangTidyDiagnosticConsumer::removeIncompatibleErrors(
    SmallVectorImpl<ClangTidyError> &Errors) const {
  // Each error is modelled as the set of intervals in which it applies
  // replacements. To detect overlapping replacements, we use a sweep line
  // algorithm over these sets of intervals.
  // An event here consists of the opening or closing of an interval. During the
  // process, we maintain a counter with the amount of open intervals. If we
  // find an endpoint of an interval and this counter is different from 0, it
  // means that this interval overlaps with another one, so we set it as
  // inapplicable.
  struct Event {
    // An event can be either the begin or the end of an interval.
    enum EventType {
      ET_Begin = 1,
      ET_End = -1,
    };

    Event(unsigned Begin, unsigned End, EventType Type, unsigned ErrorId,
          unsigned ErrorSize)
        : Type(Type), ErrorId(ErrorId) {
      // The events are going to be sorted by their position. In case of draw:
      //
      // * If an interval ends at the same position at which other interval
      //   begins, this is not an overlapping, so we want to remove the ending
      //   interval before adding the starting one: end events have higher
      //   priority than begin events.
      //
      // * If we have several begin points at the same position, we will mark as
      //   inapplicable the ones that we process later, so the first one has to
      //   be the one with the latest end point, because this one will contain
      //   all the other intervals. For the same reason, if we have several end
      //   points in the same position, the last one has to be the one with the
      //   earliest begin point. In both cases, we sort non-increasingly by the
      //   position of the complementary.
      //
      // * In case of two equal intervals, the one whose error is bigger can
      //   potentially contain the other one, so we want to process its begin
      //   points before and its end points later.
      //
      // * Finally, if we have two equal intervals whose errors have the same
      //   size, none of them will be strictly contained inside the other.
      //   Sorting by ErrorId will guarantee that the begin point of the first
      //   one will be processed before, disallowing the second one, and the
      //   end point of the first one will also be processed before,
      //   disallowing the first one.
      if (Type == ET_Begin)
        Priority = std::make_tuple(Begin, Type, -End, -ErrorSize, ErrorId);
      else
        Priority = std::make_tuple(End, Type, -Begin, ErrorSize, ErrorId);
    }

    bool operator<(const Event &Other) const {
      return Priority < Other.Priority;
    }

    // Determines if this event is the begin or the end of an interval.
    EventType Type;
    // The index of the error to which the interval that generated this event
    // belongs.
    unsigned ErrorId;
    // The events will be sorted based on this field.
    std::tuple<unsigned, EventType, int, int, unsigned> Priority;
  };

  // Compute error sizes.
  std::vector<int> Sizes;
  for (const auto &Error : Errors) {
    int Size = 0;
    for (const auto &FileAndReplaces : Error.Fix) {
      for (const auto &Replace : FileAndReplaces.second)
        Size += Replace.getLength();
    }
    Sizes.push_back(Size);
  }

  // Build events from error intervals.
  std::map<std::string, std::vector<Event>> FileEvents;
  for (unsigned I = 0; I < Errors.size(); ++I) {
    for (const auto &FileAndReplace : Errors[I].Fix) {
      for (const auto &Replace : FileAndReplace.second) {
        unsigned Begin = Replace.getOffset();
        unsigned End = Begin + Replace.getLength();
        const std::string &FilePath = Replace.getFilePath();
        // FIXME: Handle empty intervals, such as those from insertions.
        if (Begin == End)
          continue;
        auto &Events = FileEvents[FilePath];
        Events.emplace_back(Begin, End, Event::ET_Begin, I, Sizes[I]);
        Events.emplace_back(Begin, End, Event::ET_End, I, Sizes[I]);
      }
    }
  }

  std::vector<bool> Apply(Errors.size(), true);
  for (auto &FileAndEvents : FileEvents) {
    std::vector<Event> &Events = FileAndEvents.second;
    // Sweep.
    std::sort(Events.begin(), Events.end());
    int OpenIntervals = 0;
    for (const auto &Event : Events) {
      if (Event.Type == Event::ET_End)
        --OpenIntervals;
      // This has to be checked after removing the interval from the count if it
      // is an end event, or before adding it if it is a begin event.
      if (OpenIntervals != 0)
        Apply[Event.ErrorId] = false;
      if (Event.Type == Event::ET_Begin)
        ++OpenIntervals;
    }
    assert(OpenIntervals == 0 && "Amount of begin/end points doesn't match");
  }

  for (unsigned I = 0; I < Errors.size(); ++I) {
    if (Errors[I].HasConflictedFixes) {
      Errors[I].Fix.clear();
      Errors[I].Notes.emplace_back(
          "Found multiple fixes which conflict to each other and can not be "
          "resolved. Fixes will not be applied.");
    }

    if (!Apply[I]) {
      Errors[I].Fix.clear();
      Errors[I].Notes.emplace_back(
          "this fix will not be applied because it overlaps with another fix");
    }
  }
}

namespace {
struct LessClangTidyError {
  bool operator()(const ClangTidyError &LHS, const ClangTidyError &RHS) const {
    const tooling::DiagnosticMessage &M1 = LHS.Message;
    const tooling::DiagnosticMessage &M2 = RHS.Message;

    return std::tie(M1.FilePath, M1.FileOffset, M1.Message) <
           std::tie(M2.FilePath, M2.FileOffset, M2.Message);
  }
};
struct EqualClangTidyError {
  bool operator()(const ClangTidyError &LHS, const ClangTidyError &RHS) const {
    LessClangTidyError Less;
    return !Less(LHS, RHS) && !Less(RHS, LHS);
  }
};
} // end anonymous namespace

// Flushes the internal diagnostics buffer to the ClangTidyContext.
void ClangTidyDiagnosticConsumer::finish() {
  finalizeLastError();

  std::sort(Errors.begin(), Errors.end(), LessClangTidyError());
  Errors.erase(std::unique(Errors.begin(), Errors.end(), EqualClangTidyError()),
               Errors.end());

  if (RemoveIncompatibleErrors)
    removeIncompatibleErrors(Errors);

  for (const ClangTidyError &Error : Errors)
    Context.storeError(Error);
  Errors.clear();
}

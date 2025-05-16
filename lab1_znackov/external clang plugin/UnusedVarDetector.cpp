#include "clang/AST/AST.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Lex/Lexer.h"

using namespace clang;

namespace {

class MaybeUnusedRewriter : public RecursiveASTVisitor<MaybeUnusedRewriter> {
public:
  MaybeUnusedRewriter(ASTContext &Ctx, Rewriter &R) : Ctx(Ctx), Rw(R) {}

  bool VisitFunctionDecl(FunctionDecl *FD) {
    if (FD->isImplicit())
      return true;
    for (ParmVarDecl *P : FD->parameters())
      if (!P->isUsed() && !hasAttr(P))
        insertAttr(P->getLocation());
    return true;
  }

  bool VisitVarDecl(VarDecl *VD) {
    if (VD->isLocalVarDecl() && !VD->isImplicit() && !VD->isUsed() && !hasAttr(VD))
      insertAttr(VD->getLocation());
    return true;
  }

private:
  ASTContext &Ctx;
  Rewriter    &Rw;

  void insertAttr(SourceLocation Loc) {
    Loc = Ctx.getSourceManager().getSpellingLoc(Loc);
    Rw.InsertText(Loc, "[[maybe_unused]] ", /*InsertAfter=*/false, /*Indent=*/true);
  }

  bool hasAttr(const Decl *D) {
    auto &SM = Ctx.getSourceManager();
    SourceLocation L = SM.getSpellingLoc(D->getBeginLoc());
    auto TokBegin    = Lexer::GetBeginningOfToken(L, SM, Ctx.getLangOpts());
    auto Range       = CharSourceRange::getCharRange(TokBegin, L.getLocWithOffset(64));
    StringRef Text   = Lexer::getSourceText(Range, SM, Ctx.getLangOpts());
    return Text.contains("[[maybe_unused]]");
  }
};

class Consumer : public ASTConsumer {
  MaybeUnusedRewriter Visitor;
public:
  Consumer(ASTContext &Ctx, Rewriter &R) : Visitor(Ctx, R) {}
  void HandleTranslationUnit(ASTContext &Ctx) override {
    Visitor.TraverseDecl(Ctx.getTranslationUnitDecl());
  }
};

class Action : public PluginASTAction {
  Rewriter Rw;
protected:
  bool ParseArgs(const CompilerInstance &, const std::vector<std::string>&) override { return true; }

  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, llvm::StringRef) override {
    Rw.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
    return std::make_unique<Consumer>(CI.getASTContext(), Rw);
  }

  void EndSourceFileAction() override {
    Rw.getEditBuffer(Rw.getSourceMgr().getMainFileID()).write(llvm::outs());
  }
};

} // namespace

static FrontendPluginRegistry::Add<Action>
  X("mark-unused", "add [[maybe_unused]] to unused parameters/locals");


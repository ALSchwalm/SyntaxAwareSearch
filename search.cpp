#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif

#include <set>

#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/TextDiagnosticBuffer.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/AST/ASTContext.h"
#include "clang/Parse/ParseAST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Lex/Lexer.h"

#include "search.hpp"

using namespace clang;
using namespace clang::tooling;

template <typename T>
class ASTSearcher : public ASTConsumer,
                    public RecursiveASTVisitor<ASTSearcher<T>> {

    const CompilerInstance* m_compiler;
    const ASTContext* m_context;
    const T& m_term;

    static std::set<Decl::Kind> declSkipList;
    static std::set<Stmt::StmtClass> stmtSkipList;

    ///  Adapted from clang CIndex.cpp
    ///
    /// Clang internally represents ranges where the end location points to the
    /// start of the token at the end. However, for external clients it is more
    /// useful to have a CXSourceRange be a proper half-open interval. This
    /// routine does the appropriate translation.
    SourceRange translateSourceRange(const SourceRange& range) const {
        // We want the last character in this location, so we will adjust the
        // location accordingly.
        auto R = CharSourceRange::getTokenRange(range);
        auto& LangOpts = m_context->getLangOpts();
        auto& SM = m_compiler->getSourceManager();
        SourceLocation EndLoc = R.getEnd();
        if (EndLoc.isValid() && EndLoc.isMacroID() &&
            !SM.isMacroArgExpansion(EndLoc))
            EndLoc = SM.getExpansionRange(EndLoc).second;
        if (R.isTokenRange() && EndLoc.isValid()) {
            unsigned Length =
                Lexer::MeasureTokenLength(SM.getSpellingLoc(EndLoc), SM,
                                          LangOpts);
            EndLoc = EndLoc.getLocWithOffset(Length - 1);
        }

        SourceRange Result = {R.getBegin(), EndLoc};
        return Result;
    }

public:
    bool VisitStmt(const Stmt* S) {
        if (stmtSkipList.count(S->getStmtClass())) {
            return true;
        }
        return true;
    }

    virtual bool HandleTopLevelDecl(DeclGroupRef DR) override {
        for (DeclGroupRef::iterator d = DR.begin(), e = DR.end(); d != e; ++d) {
            if (m_compiler->getSourceManager().isInMainFile(
                    (*d)->getLocStart())) {
                RecursiveASTVisitor<ASTSearcher>::TraverseDecl(*d);
            }
        }
        return true;
    }

    virtual void Initialize(clang::ASTContext& Context) override {
        this->m_context = &Context;
    }

    ASTSearcher(const CompilerInstance* C, const T& term)
        : m_compiler{C}, m_term{term} {}
};

template <typename T>
std::set<Decl::Kind> ASTSearcher<T>::declSkipList = {};

// Currently just the implicit expressions from Stmt::IgnoreImplicit
template <typename T>
std::set<Stmt::StmtClass> ASTSearcher<T>::stmtSkipList =
    {Stmt::MaterializeTemporaryExprClass, Stmt::ExprWithCleanupsClass,
     Stmt::ImplicitCastExprClass, Stmt::CXXBindTemporaryExprClass};

template <typename T>
class BuildSearcherFrontendAction : public clang::ASTFrontendAction {
public:
    BuildSearcherFrontendAction(const T& term) : m_term{term} {}
    virtual std::unique_ptr<clang::ASTConsumer>
    CreateASTConsumer(clang::CompilerInstance& Compiler, llvm::StringRef) {
        return std::unique_ptr<clang::ASTConsumer>(
            new ASTSearcher<T>(&Compiler, m_term));
    }

private:
    const T& m_term;
};

class CPPParser {
public:
    template <typename T>
    CPPParser(std::string str, const T& term) {
        auto action = new BuildSearcherFrontendAction<T>(term);
        runToolOnCodeWithArgs(action, str,
                              {"-std=c++14", "-I/usr/lib/clang/3.7.1/include"});
    }
};

/**
 * Run the search for a given 'term' on 'file'. The 'term' will be visited
 * using the 'TermSearchVisitor' type.
 */
void search_file(const char* file, Term& term,
                 const po::variables_map& config) {
    auto p = boost::filesystem::path(file);
    auto extension = boost::filesystem::extension(p);
    if (!config.count("search-all-extensions") && extension != ".cpp" &&
        extension != ".c" && extension != ".h" && extension != ".hpp") {
        return;
    }

    boost::apply_visitor(TermSearchVisitor(file, config), term);
}

void TermSearchVisitor::operator()(Function& f) const {
    CPPParser(this->m_root_filename, f);
}

void TermSearchVisitor::operator()(Variable& v) const {
    CPPParser(this->m_root_filename, v);
}

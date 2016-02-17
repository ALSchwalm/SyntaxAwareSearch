#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif

#include <set>
#include <fstream>

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

namespace {

template <typename T, typename Reg>
bool match(const T& t, const Reg& reg) {
    return std::regex_match(t, std::regex(reg));
}
}

using namespace clang;
using namespace clang::tooling;

// Forward declaration for CRTP
template <typename T>
class TermSearcher;

// Base type that implements utility functions for the specific
// term searchers.
template <typename T>
class BaseTermSearcher : public ASTConsumer,
                         public RecursiveASTVisitor<TermSearcher<T>> {
protected:
    const CompilerInstance* m_compiler;
    const ASTContext* m_context;
    const T& m_term;

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

    template <typename U>
    void print_node(const U& node) {
        auto& SM = m_compiler->getSourceManager();
        auto range = node->getSourceRange();
        range = translateSourceRange(range);

        if (!range.isValid())
            return;

        auto start_loc = SM.getExpansionLoc(range.getBegin());
        auto end_loc = SM.getExpansionLoc(range.getEnd());

        auto start_row = SM.getExpansionLineNumber(start_loc);
        auto start_column = SM.getExpansionColumnNumber(start_loc);
        auto end_row = SM.getExpansionLineNumber(end_loc);
        auto end_column = SM.getExpansionColumnNumber(end_loc);

        std::cout << start_row << ":" << start_column << std::endl;
    }

public:
    BaseTermSearcher(const CompilerInstance* C, const T& term)
        : m_compiler{C}, m_term{term} {
        auto& diagEngine = C->getDiagnostics();
        diagEngine.setSuppressAllDiagnostics();
    }

    virtual bool HandleTopLevelDecl(DeclGroupRef DR) override {
        for (DeclGroupRef::iterator d = DR.begin(), e = DR.end(); d != e; ++d) {
            if (this->m_compiler->getSourceManager().isInMainFile(
                    (*d)->getLocStart())) {
                this->TraverseDecl(*d);
            }
        }
        return true;
    }

    virtual void Initialize(clang::ASTContext& Context) override {
        this->m_context = &Context;
    }
};

// TermSearcher for functions
template <>
class TermSearcher<Function> : public BaseTermSearcher<Function> {
public:
    TermSearcher(const CompilerInstance* C, const Function& term)
        : BaseTermSearcher<Function>(C, term) {}
};

// TermSearcher for variables
template <>
class TermSearcher<Variable> : public BaseTermSearcher<Variable> {
public:
    TermSearcher(const CompilerInstance* C, const Variable& term)
        : BaseTermSearcher<Variable>(C, term) {}

    bool VisitVarDecl(const VarDecl* v) {
        if (match(v->getNameAsString(), m_term.name)) {
            this->print_node(v);
        }
        return true;
    }
};

template <typename T>
class BuildSearcherFrontendAction : public clang::ASTFrontendAction {
public:
    BuildSearcherFrontendAction(const T& term) : m_term{term} {}

    virtual std::unique_ptr<clang::ASTConsumer>
    CreateASTConsumer(clang::CompilerInstance& Compiler, llvm::StringRef) {
        return std::unique_ptr<clang::ASTConsumer>(
            new TermSearcher<T>(&Compiler, m_term));
    }

private:
    const T& m_term;
};

class CPPParser {
public:
    template <typename T>
    CPPParser(std::string path, const T& term) {
        std::ifstream file{path};
        std::stringstream buffer;
        buffer << file.rdbuf();
        auto action = new BuildSearcherFrontendAction<T>(term);
        runToolOnCodeWithArgs(action, buffer.str(),
                              {"-std=c++14", "-I/usr/lib/clang/3.7.1/include"});
    }
};

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

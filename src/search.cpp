#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif

#include <fstream>

#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Lex/Lexer.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

#include "search.hpp"

using namespace clang;
using namespace clang::tooling;
using namespace clang::ast_matchers;

namespace {

std::string get_line_from_offset(StringRef buffer, std::size_t offset) {
    assert(buffer.size() > offset);

    auto line_start = buffer.find_last_of("\r\n", offset) + 1;
    auto line_end = buffer.find_first_of("\r\n", offset);
    return std::string(buffer.begin() + line_start, buffer.begin() + line_end);
}

///  Adapted from clang CIndex.cpp
///
/// Clang internally represents ranges where the end location points to the
/// start of the token at the end. However, for external clients it is more
/// useful to have a CXSourceRange be a proper half-open interval. This
/// routine does the appropriate translation.
SourceRange translateSourceRange(const ASTContext* context,
                                 const SourceManager* sm,
                                 const SourceRange& range) {
    // We want the last character in this location, so we will adjust the
    // location accordingly.
    auto R = CharSourceRange::getTokenRange(range);
    auto& LangOpts = context->getLangOpts();
    SourceLocation EndLoc = R.getEnd();
    if (EndLoc.isValid() && EndLoc.isMacroID() &&
        !sm->isMacroArgExpansion(EndLoc))
        EndLoc = sm->getExpansionRange(EndLoc).second;
    if (R.isTokenRange() && EndLoc.isValid()) {
        unsigned Length = Lexer::MeasureTokenLength(sm->getSpellingLoc(EndLoc),
                                                    *sm, LangOpts);
        EndLoc = EndLoc.getLocWithOffset(Length - 1);
    }
    SourceRange Result = {R.getBegin(), EndLoc};
    return Result;
}

template <typename U>
void print_node(const ASTContext* context, const SourceManager* sm,
                const U* node) {
    auto range = node->getSourceRange();
    range = translateSourceRange(context, sm, range);

    if (!range.isValid())
        return;

    auto start_loc = sm->getExpansionLoc(range.getBegin());
    auto end_loc = sm->getExpansionLoc(range.getEnd());

    auto start_row = sm->getExpansionLineNumber(start_loc);
    auto start_column = sm->getExpansionColumnNumber(start_loc);
    auto end_row = sm->getExpansionLineNumber(end_loc);
    auto end_column = sm->getExpansionColumnNumber(end_loc);

    auto file_id = sm->getFileID(start_loc);
    auto buffer = sm->getBufferData(file_id);
    auto offset = sm->getDecomposedLoc(start_loc).second;

    std::cout << start_row << ":" << start_column << ":"
              << get_line_from_offset(buffer, offset) << std::endl;
}

AST_MATCHER_P(NamedDecl, matchesUnqualifiedName, std::string, RegExp) {
    assert(!RegExp.empty());
    std::string FullNameString = Node.getNameAsString();
    llvm::Regex RE(RegExp);
    return RE.match(FullNameString);
}

AST_MATCHER_P(BuiltinType, matchesBuiltinType, std::string, RegExp) {
    assert(!RegExp.empty());
    LangOptions opts;
    opts.CPlusPlus14 = 1;
    PrintingPolicy p(opts);

    std::string FullNameString = Node.getName(p);
    llvm::Regex RE(RegExp);
    return RE.match(FullNameString);
}
}

template <typename T>
class Printer;

template <>
class Printer<Variable> : public MatchFinder::MatchCallback {
public:
    virtual void run(const MatchFinder::MatchResult& Result) {
        auto d = Result.Nodes.getNodeAs<VarDecl>("varDecl");
        print_node(Result.Context, Result.SourceManager, d);
    }
};

template <>
class Printer<Function> : public MatchFinder::MatchCallback {
public:
    virtual void run(const MatchFinder::MatchResult& Result) {
        auto d = Result.Nodes.getNodeAs<FunctionDecl>("funcDecl");
        if (d) {
            print_node(Result.Context, Result.SourceManager, d);
        } else {
            auto e = Result.Nodes.getNodeAs<CallExpr>("funcCall");
            print_node(Result.Context, Result.SourceManager, e);
        }
    }
};

void addMatchersForTerm(const Variable& v, MatchFinder& finder,
                        Printer<Variable>* printer) {
    auto type_matches =
        anyOf(hasType(recordDecl(matchesUnqualifiedName(v.type))),
              hasType(builtinType(matchesBuiltinType(v.type))));

    auto varDeclMatcher =
        varDecl(allOf(matchesUnqualifiedName(v.name), type_matches))
            .bind("varDecl");

    finder.addMatcher(varDeclMatcher, printer);
}

void addMatchersForTerm(const Function& f, MatchFinder& finder,
                        Printer<Function>* printer) {
    auto funcDeclMatcher =
        functionDecl(matchesUnqualifiedName(f.name)).bind("funcDecl");

    auto funcCallMatcher =
        callExpr(hasDeclaration(namedDecl(matchesUnqualifiedName(f.name))))
            .bind("funcCall");

    finder.addMatcher(funcDeclMatcher, printer);
    finder.addMatcher(funcCallMatcher, printer);
}

class CPPParser {
public:
    template <typename T>
    CPPParser(std::string path, const T& term) {
        std::ifstream file{path};
        std::stringstream buffer;
        buffer << file.rdbuf();

        Printer<T> Printer;
        MatchFinder Finder;
        addMatchersForTerm(term, Finder, &Printer);

        auto action_factory = newFrontendActionFactory(&Finder);
        auto action = action_factory->create();

        runToolOnCodeWithArgs(action, buffer.str(),
                              {"-w", "-std=c++14",
                               "-I/usr/lib/clang/3.7.1/include"});
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

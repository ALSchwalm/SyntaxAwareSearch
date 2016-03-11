#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif

#include <fstream>
#include <type_traits>

#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Lex/Lexer.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

#include "search.hpp"
#include "matchers.hpp"

using namespace clang;
using namespace clang::tooling;
using namespace clang::ast_matchers;

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

using node_context_t = std::tuple<match_t, StringRef, std::size_t>;

template <typename U>
node_context_t node_context(const ASTContext* context, const SourceManager* sm,
                            const U* node) {
    auto range = node->getSourceRange();
    range = translateSourceRange(context, sm, range);

    auto start_loc = sm->getExpansionLoc(range.getBegin());
    auto end_loc = sm->getExpansionLoc(range.getEnd());

    auto start_row = sm->getExpansionLineNumber(start_loc);
    auto start_column = sm->getExpansionColumnNumber(start_loc);
    auto end_row = sm->getExpansionLineNumber(end_loc);
    auto end_column = sm->getExpansionColumnNumber(end_loc);

    auto file_id = sm->getFileID(start_loc);
    auto buffer = sm->getBufferData(file_id);
    auto offset = sm->getDecomposedLoc(start_loc).second;

    return node_context_t{{{start_row, start_column}, {end_row, end_column}},
                          buffer,
                          offset};
}

void print_context(const node_context_t& context) {
    const auto& bounds = std::get<0>(context);
    std::cout << bounds.first.first << ":" << bounds.first.second << ":"
              << get_line_from_offset(std::get<1>(context),
                                      std::get<2>(context))
              << std::endl;
}

node_context_t get_variable_context(const MatchFinder::MatchResult& Result) {
    auto d = Result.Nodes.getNodeAs<VarDecl>("varDecl");
    return node_context(Result.Context, Result.SourceManager, d);
}

node_context_t get_function_context(const MatchFinder::MatchResult& Result) {
    if (auto d = Result.Nodes.getNodeAs<FunctionDecl>("funcDecl")) {
        return node_context(Result.Context, Result.SourceManager, d);
    }
    if (auto e = Result.Nodes.getNodeAs<CallExpr>("funcCall")) {
        return node_context(Result.Context, Result.SourceManager, e);
    }
    assert(false && "Function matcher matched invalid function");
}

node_context_t get_type_context(const MatchFinder::MatchResult& Result) {
    auto d = Result.Nodes.getNodeAs<RecordDecl>("typeDecl");
    return node_context(Result.Context, Result.SourceManager, d);
}

template <typename T>
class Printer : public MatchFinder::MatchCallback {
public:
    virtual void run(const MatchFinder::MatchResult& Result) {
        node_context_t context;
        if (std::is_same<T, Variable>::value) {
            context = get_variable_context(Result);
        } else if (std::is_same<T, Function>::value) {
            context = get_function_context(Result);
        } else if (std::is_same<T, Class>::value) {
            context = get_type_context(Result);
        }
        print_context(context);
    }
};

template <typename T>
class MatchListBuilder : public MatchFinder::MatchCallback {
public:
    virtual void run(const MatchFinder::MatchResult& Result) {
        node_context_t context;
        if (std::is_same<T, Variable>::value) {
            context = get_variable_context(Result);
        } else if (std::is_same<T, Function>::value) {
            context = get_function_context(Result);
        } else if (std::is_same<T, Class>::value) {
            context = get_type_context(Result);
        }
        matches.push_back(std::get<0>(context));
    }

    std::vector<match_t> matches;
};

template <typename Callback>
void addMatchersForTerm(const Variable& v, MatchFinder& finder,
                        Callback* callback) {

    auto varDeclMatcher =
        varDecl(allOf(isExpansionInMainFile(), matchesUnqualifiedName(v.name),
                      hasType(matchesType(v.type)),
                      matchesQualifiers(v.qualifiers), unless(isImplicit())))
            .bind("varDecl");

    finder.addMatcher(varDeclMatcher, callback);
}

template <typename Callback>
void addMatchersForTerm(const Function& f, MatchFinder& finder,
                        Callback* callback) {

    auto declMatcher = functionDecl(
        allOf(isExpansionInMainFile(), matchesUnqualifiedName(f.name),
              returns(matchesType(f.return_type)), unless(isImplicit()),
              matchesQualifiers(f.qualifiers),
              matchesParameters(f.parameters)));

    auto funcDeclMatcher = declMatcher.bind("funcDecl");

    auto funcCallMatcher =
        callExpr(hasDeclaration(declMatcher)).bind("funcCall");

    finder.addMatcher(funcDeclMatcher, callback);
    finder.addMatcher(funcCallMatcher, callback);
}

template <typename Callback>
void addMatchersForTerm(const Class& c, MatchFinder& finder,
                        Callback* callback) {

    auto typeDeclMatcher =
        recordDecl(allOf(matchesClass(c), unless(isImplicit())))
            .bind("typeDecl");
    finder.addMatcher(typeDeclMatcher, callback);
}

bool should_search_path(const std::string& file,
                        const po::variables_map& config) {
    auto p = boost::filesystem::path(file);
    auto extension = boost::filesystem::extension(p);
    if (!config.count("search-all-extensions") && extension != ".cpp" &&
        extension != ".c" && extension != ".h" && extension != ".hpp") {
        return false;
    }
    return true;
}

void print_matches(const std::string& file, Term& term,
                   const po::variables_map& config) {
    if (!should_search_path(file, config)) {
        return;
    }
    boost::apply_visitor(MatchPrintVisitor(file, config), term);
}

std::vector<match_t> find_matches(const std::string& file, Term& term,
                                  const po::variables_map& config) {
    if (!should_search_path(file, config)) {
        return {};
    }
    return boost::apply_visitor(MatchBuildListVisitor(file, config), term);
}

template <typename T>
void MatchPrintVisitor::operator()(T& term) const {
    std::ifstream ifs{this->m_root_filename};
    std::stringstream buffer;
    buffer << ifs.rdbuf();

    MatchFinder finder;
    Printer<T> printer;

    addMatchersForTerm(term, finder, &printer);

    auto action_factory = newFrontendActionFactory(&finder);
    auto action = action_factory->create();

    runToolOnCodeWithArgs(action, buffer.str(),
                          {"-w", "-std=c++14",
                           "-I/usr/lib/clang/3.7.1/include"});
}

template <typename T>
std::vector<match_t> MatchBuildListVisitor::operator()(T& term) const {
    std::ifstream ifs{this->m_root_filename};
    std::stringstream buffer;
    buffer << ifs.rdbuf();

    MatchFinder finder;
    MatchListBuilder<T> builder;

    addMatchersForTerm(term, finder, &builder);

    auto action_factory = newFrontendActionFactory(&finder);
    auto action = action_factory->create();

    runToolOnCodeWithArgs(action, buffer.str(),
                          {"-w", "-std=c++14",
                           "-I/usr/lib/clang/3.7.1/include"});
    return builder.matches;
}

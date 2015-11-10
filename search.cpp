#include <regex>
#include <set>
#include <functional>

#include "search.hpp"

namespace {

void print_match(const char* name) { std::cout << name << std::endl; }

struct ScopedString {
    ScopedString(CXString _str) : cx_str{_str} {}
    ~ScopedString() { clang_disposeString(cx_str); }

    const char* str() const { return clang_getCString(cx_str); }

    CXString cx_str;
};
}

void TermSearchVisitor::operator()(Function& func) const {
    std::regex name_regex{func.name};
    std::regex return_type_regex{func.return_type};
    std::set<CXCursorKind> allowed_kinds = {CXCursor_FunctionDecl,
                                            CXCursor_CXXMethod,
                                            CXCursor_FunctionTemplate,
                                            CXCursor_Constructor,
                                            CXCursor_Destructor};

    auto data = std::make_tuple(name_regex, return_type_regex, allowed_kinds);

    auto visitor = [](CXCursor cursor, CXCursor,
                      CXClientData cdata) -> CXChildVisitResult {
        auto spelling = ScopedString(clang_getCursorSpelling(cursor));
        auto kind = clang_getCursorKind(cursor);
        auto result_type = clang_getCursorResultType(cursor);
        auto type_spelling = ScopedString(clang_getTypeSpelling(result_type));

        auto data_tuple = static_cast<decltype(data)*>(cdata);

        auto& name_regex = std::get<0>(*data_tuple);
        auto& return_type_regex = std::get<1>(*data_tuple);
        auto& allowed_kinds = std::get<2>(*data_tuple);

        if (allowed_kinds.count(kind) &&
            std::regex_match(spelling.str(), name_regex) &&
            std::regex_match(type_spelling.str(), return_type_regex)) {
            print_match(spelling.str());
        }

        return CXChildVisit_Continue;
    };

    clang_visitChildren(clang_getTranslationUnitCursor(m_TU), visitor, &data);
}

void TermSearchVisitor::operator()(Variable& var) const {
    std::regex name_regex{var.name};
    std::regex type_regex{var.type};
    std::set<CXCursorKind> allowed_kinds = {CXCursor_VarDecl};

    auto data = std::make_tuple(name_regex, type_regex, allowed_kinds);

    auto visitor = [](CXCursor cursor, CXCursor,
                      CXClientData cdata) -> CXChildVisitResult {
        auto spelling = ScopedString(clang_getCursorSpelling(cursor));
        auto kind = clang_getCursorKind(cursor);
        auto result_type = clang_getCursorType(cursor);
        auto type_spelling = ScopedString(clang_getTypeSpelling(result_type));

        auto data_tuple = static_cast<decltype(data)*>(cdata);

        auto& name_regex = std::get<0>(*data_tuple);
        auto& type_regex = std::get<1>(*data_tuple);
        auto& allowed_kinds = std::get<2>(*data_tuple);

        if (allowed_kinds.count(kind) &&
            std::regex_match(spelling.str(), name_regex) &&
            std::regex_match(type_spelling.str(), type_regex)) {
            print_match(spelling.str());
        }

        return CXChildVisit_Continue;
    };

    clang_visitChildren(clang_getTranslationUnitCursor(m_TU), visitor, &data);
}

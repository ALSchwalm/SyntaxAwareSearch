#include <regex>
#include <set>
#include <fstream>

#include "search.hpp"

namespace {
std::string get_line_from_offset(std::fstream& file, unsigned int offset) {
    file.seekg(offset);
    std::fstream::int_type c = 0;

    while (c != '\n' && c != '\r') {
        file.seekg(-1, std::ios_base::cur);
        c = file.peek();
        if (c == std::fstream::traits_type::eof()) {
            break;
        }
    }
    file.seekg(1, std::ios_base::cur);
    std::string line;
    std::getline(file, line);
    return line;
}

void print_match(const char* filename, CXSourceRange extent) {
    auto start = clang_getRangeStart(extent);
    unsigned line_number, column_number, offset;
    clang_getSpellingLocation(start, NULL, &line_number, &column_number,
                              &offset);
    std::fstream file;
    file.open(filename, std::fstream::in | std::fstream::binary);

    std::string line = get_line_from_offset(file, offset);

    std::cout << filename << ":" << line_number << ":" << line << std::endl;
    file.close();
}

struct ScopedString {
    ScopedString(CXString _str) : cx_str{_str} {}
    ~ScopedString() { clang_disposeString(cx_str); }

    const char* str() const { return clang_getCString(cx_str); }

    CXString cx_str;
};

bool is_within_header(CXCursor cursor, const std::string& root_filename) {
    auto location = clang_getCursorLocation(cursor);
    CXFile file;
    unsigned int line, column, offset;
    clang_getSpellingLocation(location, &file, &line, &column, &offset);
    auto filename = ScopedString(clang_getFileName(file));
    if (filename.str() && root_filename != filename.str()) {
        return true;
    }
    return false;
}

template <typename T>
bool matches_by_kind(CXCursor cursor, const T& variable,
                     std::set<CXCursorKind> kinds) {
    // TODO: store this in the parameter?
    std::regex type_regex{variable.type};
    std::regex name_regex{variable.name};
    auto type_spelling =
        ScopedString(clang_getTypeSpelling(clang_getCursorType(cursor)));
    auto spelling = ScopedString(clang_getCursorSpelling(cursor));
    return kinds.count(clang_getCursorKind(cursor)) &&
           std::regex_match(type_spelling.str(), type_regex) &&
           std::regex_match(spelling.str(), name_regex);
}

bool matches_parameters(CXCursor cursor,
                        const std::vector<FunctionParameter>& parameters) {
    bool ellipses_active = false;
    const int total_cursor_arguments = clang_Cursor_getNumArguments(cursor);
    int num_cursor_argument = 0;

    unsigned int i = 0;
    while (i < parameters.size()) {
        if (parameters[i].type() == typeid(Ellipses)) {
            ellipses_active = true;
            ++i;
            continue;
        }
        const auto& parameter = boost::get<ExplicitParameter>(parameters[i]);

        if (num_cursor_argument == total_cursor_arguments) {
            return false;
        }

        if (!matches_by_kind(clang_Cursor_getArgument(cursor, i), parameter,
                             {CXCursor_ParmDecl})) {
            if (!ellipses_active) {
                return false;
            } else {
                ++num_cursor_argument;
                continue;
            }
        } else {
            ++num_cursor_argument;
            ellipses_active = false;
        }
        ++i;
    }
    if (num_cursor_argument != total_cursor_arguments && !ellipses_active) {
        return false;
    }
    return true;
}
}

void search_file(const char* file, Term& term) {
    auto p = boost::filesystem::path(file);
    auto extension = boost::filesystem::extension(p);
    if (extension != ".cpp" && extension != ".c" && extension != ".h" &&
        extension != ".hpp") {
        return;
    }
    auto Idx = clang_createIndex(0, 0);
    const char* args[] = {""};
    auto TU =
        clang_createTranslationUnitFromSourceFile(Idx, file, 0, args, 0, 0);

    boost::apply_visitor(TermSearchVisitor(TU, file), term);
    clang_disposeTranslationUnit(TU);
}

void TermSearchVisitor::operator()(Function& func) const {
    std::regex name_regex{func.name};
    std::regex return_type_regex{func.return_type};
    std::set<CXCursorKind> allowed_kinds = {CXCursor_FunctionDecl,
                                            CXCursor_CXXMethod,
                                            CXCursor_FunctionTemplate,
                                            CXCursor_Constructor,
                                            CXCursor_Destructor};

    auto data = std::make_tuple(this, func, name_regex, return_type_regex,
                                allowed_kinds);

    auto visitor = [](CXCursor cursor, CXCursor,
                      CXClientData cdata) -> CXChildVisitResult {

        auto data_tuple = static_cast<decltype(data)*>(cdata);
        auto pthis = std::get<0>(*data_tuple);

        if (is_within_header(cursor, pthis->m_root_filename)) {
            return CXChildVisit_Continue;
        }

        auto& func = std::get<1>(*data_tuple);
        auto& name_regex = std::get<2>(*data_tuple);
        auto& return_type_regex = std::get<3>(*data_tuple);
        auto& allowed_kinds = std::get<4>(*data_tuple);

        auto spelling = ScopedString(clang_getCursorSpelling(cursor));
        auto kind = clang_getCursorKind(cursor);
        auto result_type = clang_getCursorResultType(cursor);
        auto type_spelling = ScopedString(clang_getTypeSpelling(result_type));

        if (allowed_kinds.count(kind) &&
            std::regex_match(spelling.str(), name_regex) &&
            std::regex_match(type_spelling.str(), return_type_regex) &&
            matches_parameters(cursor, func.parameters)) {
            print_match(pthis->m_root_filename.c_str(),
                        clang_getCursorExtent(cursor));
        }

        return CXChildVisit_Recurse;
    };

    clang_visitChildren(clang_getTranslationUnitCursor(m_TU), visitor, &data);
}

void TermSearchVisitor::operator()(Variable& var) const {
    std::regex name_regex{var.name};
    std::regex type_regex{var.type};
    std::set<CXCursorKind> allowed_kinds = {CXCursor_VarDecl};

    auto data =
        std::make_tuple(this, var, name_regex, type_regex, allowed_kinds);

    auto visitor = [](CXCursor cursor, CXCursor,
                      CXClientData cdata) -> CXChildVisitResult {

        auto data_tuple = static_cast<decltype(data)*>(cdata);
        auto pthis = std::get<0>(*data_tuple);

        if (is_within_header(cursor, pthis->m_root_filename)) {
            return CXChildVisit_Continue;
        }

        auto& var = std::get<1>(*data_tuple);
        auto& name_regex = std::get<2>(*data_tuple);
        auto& type_regex = std::get<3>(*data_tuple);
        auto& allowed_kinds = std::get<4>(*data_tuple);

        auto spelling = ScopedString(clang_getCursorSpelling(cursor));
        auto kind = clang_getCursorKind(cursor);
        auto result_type = clang_getCursorType(cursor);
        auto type_spelling = ScopedString(clang_getTypeSpelling(result_type));

        if (allowed_kinds.count(kind) &&
            std::regex_match(spelling.str(), name_regex) &&
            std::regex_match(type_spelling.str(), type_regex)) {
            print_match(pthis->m_root_filename.c_str(),
                        clang_getCursorExtent(cursor));
        }
        return CXChildVisit_Recurse;
    };

    clang_visitChildren(clang_getTranslationUnitCursor(m_TU), visitor, &data);
}

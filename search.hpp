#ifndef SAS_SEARCH
#define SAS_SEARCH

#include <boost/filesystem.hpp>
#include <clang-c/Platform.h>
#include <clang-c/Index.h>
#include <vector>
#include <regex>

#include "parser.hpp"

class TermSearchVisitor : public boost::static_visitor<> {
public:
    TermSearchVisitor(CXTranslationUnit TU, const std::string& root_filename)
        : m_TU{TU}, m_root_filename{root_filename} {}
    void operator()(Function&) const;

    void operator()(Variable&) const;

private:
    CXTranslationUnit m_TU;
    std::string m_root_filename;
};

void search_file(const char* file, Term&);
inline void search_file(const std::string& file, Term& term) {
    return search_file(file.c_str(), term);
}
inline void search_file(const boost::filesystem::path& file, Term& term) {
    return search_file(file.string(), term);
}

#endif

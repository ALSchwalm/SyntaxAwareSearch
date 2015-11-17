#ifndef SAS_SEARCH
#define SAS_SEARCH

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <clang-c/Platform.h>
#include <clang-c/Index.h>
#include <vector>
#include <regex>

#include "parser.hpp"

namespace po = boost::program_options;

class TermSearchVisitor : public boost::static_visitor<> {
public:
    TermSearchVisitor(CXTranslationUnit TU, const std::string& root_filename,
                      const po::variables_map& config)
        : m_TU{TU}, m_root_filename{root_filename}, m_config{config} {}
    void operator()(Function&) const;

    void operator()(Variable&) const;

private:
    CXTranslationUnit m_TU;
    std::string m_root_filename;
    po::variables_map m_config;
};

void search_file(const char* file, Term&, const po::variables_map& config);
inline void search_file(const std::string& file, Term& term,
                        const po::variables_map& config) {
    return search_file(file.c_str(), term, config);
}
inline void search_file(const boost::filesystem::path& file, Term& term,
                        const po::variables_map& config) {
    return search_file(file.string(), term, config);
}

#endif

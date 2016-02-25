#ifndef SAS_SEARCH
#define SAS_SEARCH

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <vector>
#include <regex>

#include "parser.hpp"

namespace clang {
namespace ast_matchers {
class MatchFinder;
}
}

namespace po = boost::program_options;
using match_t = std::pair<std::pair<int, int>, std::pair<int, int>>;

class MatchPrintVisitor : public boost::static_visitor<> {
public:
    MatchPrintVisitor(const std::string& root_filename,
                      const po::variables_map& config)
        : m_root_filename{root_filename}, m_config{config} {}
    template <typename T>
    void operator()(T&) const;

private:
    std::string m_root_filename;
    po::variables_map m_config;
};

class MatchBuildListVisitor
    : public boost::static_visitor<std::vector<match_t>> {
public:
    MatchBuildListVisitor(const std::string& root_filename,
                          const po::variables_map& config)
        : m_root_filename{root_filename}, m_config{config} {}
    template <typename T>
    std::vector<match_t> operator()(T&) const;

private:
    std::string m_root_filename;
    po::variables_map m_config;
};

void print_matches(const std::string& file, Term& term,
                   const po::variables_map& config);

std::vector<match_t> find_matches(const std::string& file, Term& term,
                                  const po::variables_map& config);

#endif

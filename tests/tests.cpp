#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "boost/filesystem/path.hpp"

#include "parser.hpp"
#include "search.hpp"

void test_case(const std::string& filename) {
    boost::program_options::variables_map vm;
    std::ifstream test_cases(filename);

    std::string line;
    while (std::getline(test_cases, line)) {
        // Ignore empty lines and comments
        if (line.size() < 4 || line.substr(0, 3) != "// ") {
            continue;
        }

        std::string statement = line.substr(3, std::string::npos);

        if (!std::getline(test_cases, line)) {
            std::cerr << "Unexpected end of test cases." << std::endl;
            exit(-1);
        }

        std::istringstream ss{line};
        ss.ignore(std::numeric_limits<std::streamsize>::max(), ' ');
        std::vector<int> matches;
        int match_line;

        while (ss >> match_line) {
            matches.push_back(match_line);
        }

        std::cout << "Testing: " << statement << std::endl;

        auto term = parse_search_string(statement, vm);
        auto found = find_matches(filename, term, vm);

        assert(found.size() == matches.size());
        for (std::size_t s = 0; s < found.size(); ++s) {
            assert(found[s].first.first == matches[s]);
        }

        std::cout << "  Passed" << std::endl;
    }
}

namespace fs = boost::filesystem;

int main() {
    fs::directory_iterator end_iter;
    for (fs::directory_iterator dir_itr("tests/cases"); dir_itr != end_iter;
         ++dir_itr) {
        std::string path = dir_itr->path().string();
        std::cout << "Case: " << path << std::endl;
        test_case(path);
    }
}

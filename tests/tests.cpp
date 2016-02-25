#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "parser.hpp"
#include "search.hpp"

int main() {
    boost::program_options::variables_map vm;
    std::ifstream test_cases("tests/test_cases.txt");

    std::string line;
    while (std::getline(test_cases, line)) {
        // Ignore empty lines and comments
        if (!line.size() || line[0] == '#') {
            continue;
        }

        std::istringstream ss{line};
        std::string statement;
        ss >> statement;

        if (!std::getline(test_cases, line)) {
            std::cerr << "Unexpected end of test cases." << std::endl;
            exit(-1);
        }

        ss = std::istringstream{line};
        std::vector<int> matches;
        int match_line;

        while (ss >> match_line) {
            matches.push_back(match_line);
        }

        std::cout << "Testing: " << statement << std::endl;

        auto term = parse_search_string(statement, vm);
        auto found = find_matches("tests/file1.cpp", term, vm);

        assert(found.size() == matches.size());
        for (std::size_t s = 0; s < found.size(); ++s) {
            assert(found[s].first.first == matches[s]);
        }

        std::cout << "  Passed" << std::endl;
    }
}

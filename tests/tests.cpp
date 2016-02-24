#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

int main() {
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
    }
}

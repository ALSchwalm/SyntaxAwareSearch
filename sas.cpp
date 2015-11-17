#include <boost/filesystem.hpp>

#include "parser.hpp"
#include "search.hpp"

namespace fs = boost::filesystem;

int main(int argc, char** argv) {
    SASParser g;

    po::options_description desc(
        "Usage: sas [options] search-string path [path...]");
    desc.add_options()                                                      //
        ("help", "Print help messages")                                     //
        ("search-string", po::value<std::string>(), "The search string")    //
        ("paths", po::value<std::vector<std::string>>(), "Paths to search") //
        ("search-all-extensions",                                           //
         "By default only file with h, hpp, c or cpp are searched."         //
         " Use this flag to disable this behavior.")                        //
        ("expressions,e", "Match expressions")                              //
        ("declarations,d", "Match declaratons")                             //
        ("definitions,D",                                                   //
         "Only match declarations that are also definitions (implies -d)")  //
        ("recursive,r", "Read all files under each directory recursively");

    po::positional_options_description p;
    p.add("search-string", 1);
    p.add("paths", -1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv)
                  .options(desc)
                  .positional(p)
                  .run(),
              vm);

    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 0;
    }

    // If no match-mode is specified, look for anything
    if (!vm.count("expressions") && !vm.count("declarations") &&
        !vm.count("definitions")) {
        vm.insert(
            std::make_pair("expressions", po::variable_value(true, true)));
        vm.insert(
            std::make_pair("declarations", po::variable_value(true, true)));
    }
    // Matching definitions implies matching declarations
    else if (!vm.count("declarations") && vm.count("definitions")) {
        vm.insert(
            std::make_pair("declarations", po::variable_value(true, true)));
    }

    po::notify(vm);

    Term term;
    const auto search_string = vm["search-string"].as<std::string>();
    auto iter = search_string.begin();
    auto end = search_string.end();
    bool r = phrase_parse(iter, end, g, ascii::space, term);

    auto paths = vm["paths"].as<std::vector<std::string>>();

    if (r && iter == end) {
        for (const auto& path : paths) {
            if (fs::is_directory(path)) {
                if (!vm.count("recursive")) {
                    std::cerr << "sas: " << path << ": Is a directory"
                              << std::endl;
                    continue;
                } else {
                    for (fs::recursive_directory_iterator iter(path), end;
                         iter != end; ++iter) {
                        search_file(*iter, term, vm);
                    }
                }
            } else {
                search_file(path, term, vm);
            }
        }
    } else {
        std::cout << "-------------------------\n";
        std::cout << "Parsing failed\n";
        std::cout << "-------------------------\n";
    }
    return 0;
}

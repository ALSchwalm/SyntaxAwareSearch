#include <boost/filesystem.hpp>

#include "parser.hpp"
#include "search.hpp"

namespace fs = boost::filesystem;

int main(int argc, char** argv) {
    po::options_description desc(
        "Usage: sas [options] search-string path [path...]");
    desc.add_options()                                                      //
        ("help,h", "Print help messages")                                   //
        ("debug", "Enable debugging output")                                //
        ("search-string", po::value<std::string>(), "The search string")    //
        ("paths", po::value<std::vector<std::string>>(), "Paths to search") //
        ("search-all-extensions,a",                                         //
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

    const auto search_string = vm["search-string"].as<std::string>();
    auto term = parse_search_string(search_string, vm);

    auto paths = vm["paths"].as<std::vector<std::string>>();

    for (const auto& path : paths) {
        if (fs::is_directory(path)) {
            if (!vm.count("recursive")) {
                std::cerr << "sas: " << path << ": Is a directory" << std::endl;
                continue;
            } else {
                for (fs::recursive_directory_iterator iter(path), end;
                     iter != end; ++iter) {
                    print_matches(iter->path().string(), term, vm);
                }
            }
        } else {
            print_matches(path, term, vm);
        }
    }

    return 0;
}

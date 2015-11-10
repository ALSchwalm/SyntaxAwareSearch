#include <boost/program_options.hpp>

#include "parser.hpp"
#include "search.hpp"

namespace po = boost::program_options;

int main(int argc, char** argv) {
    SASParser g;

    po::options_description desc("Allowed options");
    desc.add_options()                                                   //
        ("help", "Print help messages")                                  //
        ("search-string", po::value<std::string>(), "The search string") //
        ("paths", po::value<std::vector<std::string>>(), "Paths to search");
    ;

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

    po::notify(vm);

    Term term;
    const auto search_string = vm["search-string"].as<std::string>();
    auto iter = search_string.begin();
    auto end = search_string.end();
    bool r = phrase_parse(iter, end, g, ascii::space, term);

    auto paths = vm["paths"].as<std::vector<std::string>>();

    if (r && iter == end) {
        for (const auto& path : paths) {
            auto Idx = clang_createIndex(0, 0);
            const char* args[] = {""};
            auto TU =
                clang_createTranslationUnitFromSourceFile(Idx, path.c_str(), 0,
                                                          args, 0, 0);

            boost::apply_visitor(TermSearchVisitor(TU), term);

            clang_disposeTranslationUnit(TU);
        }
    } else {
        std::cout << "-------------------------\n";
        std::cout << "Parsing failed\n";
        std::cout << "-------------------------\n";
    }
    return 0;
}

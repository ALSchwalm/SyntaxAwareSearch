/*
  The current grammer in EBNF:

   L_PAREN = '('
   R_PAREN = ')'
   COLON = ':'
   ELLIPSES = '...'
   COMMA = ','

   name = ? [^():,{}#<>[\]]+ ? ;
   list_contents = ELLIPSES, [COMMA, list_contents] |
                   variable, [COMMA, list_contents]
   parameter_list = L_PAREN, [ list_contents ], R_PAREN ;
   function = name, COLON, name, [ parameter_list ] ;
 */

#define BOOST_SPIRIT_DEBUG

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <clang-c/Platform.h>
#include <clang-c/Index.h>
#include <vector>
#include <string>

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

struct ExplicitParameter {
    std::string type;
    std::string name;
};

struct Ellipses {};

using FunctionParameter = boost::variant<ExplicitParameter, Ellipses>;

struct Function {
    std::string return_type;
    std::string name;
    std::vector<FunctionParameter> parameters;
};

struct Variable {
    std::string type;
    std::string name;
};

using Term = boost::variant<Variable, Function>;

std::ostream& operator<<(std::ostream& stream, const Function& func) {
    stream << func.return_type << " " << func.name << "(";
    if (!func.parameters.empty()) {
        for (std::size_t i = 0; i < func.parameters.size() - 1; ++i)
            stream << func.parameters[i] << ", ";
        stream << func.parameters.back();
    }
    stream << ")";
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const ExplicitParameter& param) {
    stream << param.type + " " + param.name;
    return stream;
}

std::ostream& operator<<(std::ostream& stream, Ellipses) {
    stream << "...";
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const Variable& var) {
    stream << var.type << " " << var.name;
    return stream;
}

BOOST_FUSION_ADAPT_STRUCT( //
    ExplicitParameter,     //
    (std::string, type)    //
    (std::string, name))

BOOST_FUSION_ADAPT_STRUCT( //
    Variable,              //
    (std::string, type)    //
    (std::string, name))

BOOST_FUSION_ADAPT_STRUCT(     //
    Function,                  //
    (std::string, return_type) //
    (std::string, name)        //
    (std::vector<FunctionParameter>, parameters))

template <typename Iterator>
struct sas_parser : qi::grammar<Iterator, Term(), ascii::space_type> {
    sas_parser() : sas_parser::base_type(term) {
        using ascii::char_;
        using qi::lit;
        using namespace qi::labels; // _val

        name %= +(char_ - char_("/:(),")) | '/' >> +(char_ - '/') >> '/';
        parameter %= (-name >> ':' >> -name) | lit("...")[_val = Ellipses{}];
        function %= -name >> ':' >> -name >> '(' >> -(parameter % ',') >> ')';
        variable %= -name >> ':' >> -name;

        term %= function | variable;

        BOOST_SPIRIT_DEBUG_NODES((term)(function)(variable)(name)(parameter));
    }

    qi::rule<Iterator, Term(), ascii::space_type> term;
    qi::rule<Iterator, Function(), ascii::space_type> function;
    qi::rule<Iterator, Variable(), ascii::space_type> variable;
    qi::rule<Iterator, FunctionParameter(), ascii::space_type> parameter;
    qi::rule<Iterator, std::string(), ascii::space_type> name;
};

int main(int argc, char** argv) {
    using sas_parser = sas_parser<std::string::const_iterator>;

    sas_parser g; // Our grammar
    const std::string search = argv[1];
    const std::string path = argv[2];

    // This will load all the symbols from 'IndexTest.c', excluding symbols
    // from 'IndexTest.pch'.
    auto Idx = clang_createIndex(1, 1);
    char* args[] = {""};
    auto TU = clang_createTranslationUnitFromSourceFile(Idx, path.c_str(), 2,
                                                        args, 0, 0);

    auto visitor = [](CXCursor cursor, CXCursor parent,
                      CXClientData client_data) -> CXChildVisitResult {
        auto cx_spelling = clang_getCursorSpelling(cursor);
        auto spelling = clang_getCString(cx_spelling);
        std::cout << spelling << std::endl;
        clang_disposeString(cx_spelling);
        return CXChildVisit_Continue;
    };
    clang_visitChildren(clang_getTranslationUnitCursor(TU), visitor, 0);
    clang_disposeTranslationUnit(TU);

    Term term;
    auto iter = search.begin();
    auto end = search.end();
    bool r = phrase_parse(iter, end, g, ascii::space, term);

    if (r && iter == end) {
        std::cout << term << std::endl;
    } else {
        std::cout << "-------------------------\n";
        std::cout << "Parsing failed\n";
        std::cout << "-------------------------\n";
    }
    return 0;
}

#ifndef SAS_PARSER
#define SAS_PARSER

#define BOOST_SPIRIT_DEBUG

#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/program_options.hpp>
#include <string>

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

struct Namespace {
    Namespace() = default;
    explicit Namespace(const std::string& _name) : name{_name} {}
    std::string name;
};

struct Class {
    Class() = default;
    explicit Class(const std::string& _name) : name{_name} {}
    std::string name;
};

using Qualifier = boost::variant<Namespace, Class>;

struct ExplicitParameter {
    std::string type;
    std::string name;
};

struct Ellipses {};

using FunctionParameter = boost::variant<ExplicitParameter, Ellipses>;

struct Function {
    std::vector<Qualifier> qualifiers;
    std::string return_type;
    std::string name;
    std::vector<FunctionParameter> parameters;
};

struct Variable {
    std::vector<Qualifier> qualifiers;
    std::string type;
    std::string name;
};

using Term = boost::variant<Variable, Function>;

std::ostream& operator<<(std::ostream& stream, const Function&);
std::ostream& operator<<(std::ostream& stream, const ExplicitParameter&);
std::ostream& operator<<(std::ostream& stream, Ellipses);
std::ostream& operator<<(std::ostream& stream, const Variable&);
std::ostream& operator<<(std::ostream& stream, const Namespace&);
std::ostream& operator<<(std::ostream& stream, const Class&);

BOOST_FUSION_ADAPT_STRUCT( //
    Namespace,             //
    (std::string, name))

BOOST_FUSION_ADAPT_STRUCT( //
    Class,                 //
    (std::string, name))

BOOST_FUSION_ADAPT_STRUCT( //
    ExplicitParameter,     //
    (std::string, type)    //
    (std::string, name))

BOOST_FUSION_ADAPT_STRUCT(               //
    Variable,                            //
    (std::vector<Qualifier>, qualifiers) //
    (std::string, type)                  //
    (std::string, name))

BOOST_FUSION_ADAPT_STRUCT(               //
    Function,                            //
    (std::vector<Qualifier>, qualifiers) //
    (std::string, return_type)           //
    (std::string, name)                  //
    (std::vector<FunctionParameter>, parameters))

struct SASParser
    : qi::grammar<std::string::const_iterator, Term(), ascii::space_type> {
    using Iterator = std::string::const_iterator;
    SASParser(const boost::program_options::variables_map&);

    boost::program_options::variables_map config;
    qi::rule<Iterator, Term(), ascii::space_type> term;
    qi::rule<Iterator, Function(), ascii::space_type> function;
    qi::rule<Iterator, Variable(), ascii::space_type> variable;
    qi::rule<Iterator, FunctionParameter(), ascii::space_type> parameter;
    qi::rule<Iterator, Qualifier(), ascii::space_type> qualifier;
    qi::rule<Iterator, std::vector<Qualifier>(), ascii::space_type> qualifiers;
    qi::rule<Iterator, std::string(), ascii::space_type> data;
    qi::rule<Iterator, std::string(), ascii::space_type> required_data;
};

#endif

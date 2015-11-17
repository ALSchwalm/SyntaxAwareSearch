#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>

#include "parser.hpp"

SASParser::SASParser() : SASParser::base_type(term) {
    using ascii::char_;
    using qi::lit;
    using qi::eps;
    using namespace qi::labels; // _val

    data %= +(char_ - char_("/:(),")) | '/' >> +(char_ - '/') >> '/' |
            eps[_val = ".*"];
    parameter %= (-data >> ':' >> -data) | lit("...")[_val = Ellipses{}];
    function %= -data >> ':' >> -data >> '(' >> -(parameter % ',') >> ')';
    variable %= -data >> ':' >> -data;

    term %= function | variable;

    // BOOST_SPIRIT_DEBUG_NODES((term)(function)(variable)(name)(parameter));
}

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

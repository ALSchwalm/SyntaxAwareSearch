#define BOOST_SPIRIT_DEBUG

#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>

#include "parser.hpp"

SASParser::SASParser() : SASParser::base_type(term) {
    using ascii::char_;
    using qi::lit;
    using qi::eps;
    using boost::phoenix::construct;
    using namespace qi::labels; // _val

    required_data %= +(char_ - char_("#/:(),")) | '/' >> +(char_ - '/') >> '/';
    data %= required_data | eps[_val = ".*"];

    parameter %= (data >> ':' >> data) | lit("...")[_val = Ellipses{}];
    qualifier = (required_data[_val = construct<Namespace>(_1)]) |
                ("#" >> required_data[_val = construct<Class>(_1)]);

    qualifiers %= qualifier >> *("::" >> qualifier >> !(":" >> data)) >> "::" |
                  eps[_val = std::vector<Qualifier>{}];

    function %=
        qualifiers >> data >> ':' >> data >> '(' >> -(parameter % ',') >> ')';

    variable %= qualifiers >> data >> ':' >> data;

    term %= function | variable;

    BOOST_SPIRIT_DEBUG_NODES(
        (term)(function)(variable)(data)(parameter)(qualifiers));
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

std::ostream& operator<<(std::ostream& stream, const Namespace& ns) {
    stream << ns.name;
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const Class& cls) {
    stream << cls.name;
    return stream;
}

#ifndef SAS_SEARCH
#define SAS_SEARCH

#include <clang-c/Platform.h>
#include <clang-c/Index.h>
#include <vector>
#include <regex>

#include "parser.hpp"

class TermSearchVisitor : public boost::static_visitor<> {
public:
    TermSearchVisitor(CXTranslationUnit TU) : m_TU{TU} {}
    void operator()(Function&) const;

    void operator()(Variable&) const;

private:
    CXTranslationUnit m_TU;
};
#endif

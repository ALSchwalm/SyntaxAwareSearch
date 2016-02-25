#ifndef SAS_MATCHERS
#define SAS_MATCHERS

#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

#include "search.hpp"

namespace clang {

namespace ast_matchers {

AST_MATCHER_P(NamedDecl, matchesUnqualifiedName, std::string, RegExp) {
    assert(!RegExp.empty());
    std::string FullNameString = Node.getNameAsString();
    llvm::Regex RE(RegExp);
    return RE.match(FullNameString);
}

AST_MATCHER_P(QualType, matchesType, std::string, RegExp) {
    assert(!RegExp.empty());
    llvm::Regex RE(RegExp);
    return RE.match(Node.getAsString());
}

AST_MATCHER_P(ParmVarDecl, matchesParameter, ExplicitParameter, param) {
    auto matcher = parmVarDecl(allOf(matchesUnqualifiedName(param.name),
                                     hasType(matchesType(param.type)),
                                     unless(isImplicit())));
    return matcher.matches(Node, Finder, Builder);
}

AST_MATCHER_P(FunctionDecl, matchesParameters, std::vector<FunctionParameter>,
              parameters) {
    bool ellipses_active = false;
    auto iter = Node.param_begin();
    for (const auto& p : parameters) {
        if (p.which() == 1) {
            ellipses_active = true;
            continue;
        }
        if (iter == Node.param_end()) {
            return false;
        }

        auto matcher = matchesParameter(boost::get<ExplicitParameter>(p));
        if (!matcher.matches(**iter, Finder, Builder)) {
            if (!ellipses_active) {
                return false;
            } else {
                ++iter;
            }
        } else {
            ++iter;
            ellipses_active = false;
        }
    }
    if (iter != Node.param_end() && !ellipses_active) {
        return false;
    }
    return true;
}

AST_MATCHER_P(NamespaceDecl, matchesNamespace, Namespace, ns) {
    llvm::Regex RE(ns.name);
    return RE.match(Node.getNameAsString());
}

AST_MATCHER_P(RecordDecl, matchesClass, Class, cls) {
    llvm::Regex RE(cls.name);
    return RE.match(Node.getNameAsString());
}

AST_MATCHER_P(NamedDecl, matchesQualifiers, std::vector<Qualifier>,
              qualifiers) {
    auto context = Node.getDeclContext();

    std::vector<const DeclContext*> contexts;

    while (context && isa<NamedDecl>(context)) {
        contexts.push_back(context);
        context = context->getParent();
    }

    if (qualifiers.size() > contexts.size()) {
        return false;
    }

    for (std::size_t i = 0; i < qualifiers.size(); ++i) {
        const auto& qual = qualifiers[qualifiers.size() - 1 - i];
        if (qual.which() == 0) {
            if (const auto* ND = dyn_cast<NamespaceDecl>(contexts[i])) {
                auto ns = boost::get<Namespace>(qual);
                auto matcher = matchesNamespace(ns);
                if (!matcher.matches(*ND, Finder, Builder)) {
                    return false;
                }
            } else {
                return false;
            }
        } else if (qual.which() == 1) {
            if (const auto* RD = dyn_cast<RecordDecl>(contexts[i])) {
                auto cls = boost::get<Class>(qual);
                auto matcher = matchesClass(cls);
                if (!matcher.matches(*RD, Finder, Builder)) {
                    return false;
                }
            } else {
                return false;
            }
        }
    }
    return true;
}
}
}

#endif

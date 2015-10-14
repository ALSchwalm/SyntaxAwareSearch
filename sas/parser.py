from rply import ParserGenerator
from .lexer import lexer
from .wrappers import *

pg = ParserGenerator(
    [rule.name for rule in lexer.rules],
    cache_id="cache",

    # NOTE: This is pretty arbitrary at the moment
    precedence=[])


@pg.production("term : qualified_variable")
@pg.production("term : variable")
@pg.production("term : qualified_function")
@pg.production("term : function")
@pg.production("term : class")
@pg.production("term : qualified_class")
@pg.production("term : qualified_search")
@pg.production("term : search")
@pg.production("term : term scope")
def term(p):
    if len(p) > 1:
        p[0].contents = p[1]
    return p[0]


@pg.production("class : POUND DATA")
def class_decl(p):
    return Class(p[1].value)


@pg.production("scope : L_CURLY term R_CURLY")
@pg.production("scope : L_CURLY R_CURLY")
def scope(p):
    if len(p) == 2:
        return []
    p[1].qualifiers = None
    return [p[1]]


@pg.production("qualified_variable : qualifier variable")
@pg.production("qualified_variable : qualifier qualified_variable")
@pg.production("qualified_function : qualifier function")
@pg.production("qualified_function : qualifier qualified_function")
@pg.production("qualified_class : qualifier class")
@pg.production("qualified_class : qualifier qualified_class")
@pg.production("qualified_search : qualifier search")
@pg.production("qualified_search : qualifier qualified_search")
def qualified(p):
    if p[0] is None:
        p[1].qualifiers = None
    else:
        p[1].qualifiers.insert(0, p[0])
    return p[1]


@pg.production("qualifier : data DOUBLE_COLON")
@pg.production("qualifier : DOUBLE_COLON")
def qualifier(p):
    if len(p) < 2:
        return None
    return p[0].value


@pg.production("function : data COLON data parameter_list")
@pg.production("function : data COLON parameter_list")
@pg.production("function : COLON data parameter_list")
@pg.production("function : COLON parameter_list")
def function(p):
    name = None
    return_type = None
    for t in p[:-1]:
        if t.name == "COLON" and name is None:
            name = ".*"
        elif t.name == "DATA":
            if name is None:
                name = t.value
            else:
                return_type = t.value
    if return_type is None:
        return_type = ".*"
    parameter_list = p[-1]
    return Function(name, return_type, parameter_list)


@pg.production("parameter_list : L_PAREN list_contents R_PAREN")
@pg.production("parameter_list : L_PAREN R_PAREN")
def parameter_list(p):
    if len(p) == 2:
        return []
    else:
        return p[1]


@pg.production("list_contents : ELLIPSES")
@pg.production("list_contents : variable COMMA list_contents")
@pg.production("list_contents : ELLIPSES COMMA list_contents")
@pg.production("list_contents : variable")
def list_contents(p):
    if len(p) == 1:
        return p
    else:
        p[2].insert(0, p[0])
        return p[2]


@pg.production("variable : data COLON data")
@pg.production("variable : COLON data")
@pg.production("variable : data COLON")
def variable(p):
    type = ".*"
    if p[0].name == "COLON":
        return Variable(type=p[1].value)
    else:
        name = p[0].value
        if len(p) == 3:
            type = p[2].value
        return Variable(name=name, type=type)


@pg.production("data : DATA")
def data(p):
    p[0].value = p[0].value.strip("/")
    return p[0]


@pg.production("search : data")
def search(p):
    return Search(p[0].value)

parser = pg.build()

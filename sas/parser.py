from rply import ParserGenerator
from .lexer import lexer
from .wrappers import *

pg = ParserGenerator(
    [rule.name for rule in lexer.rules],
    cache_id="cache",

    # NOTE: This is pretty arbitrary at the moment
    precedence=[])

@pg.production("main : declaration")
def main(p):
    return p[0]


@pg.production("declaration : qualified_variable")
@pg.production("declaration : variable")
@pg.production("declaration : qualified_function")
@pg.production("declaration : function")
@pg.production("declaration : class")
@pg.production("declaration : qualified_class")
@pg.production("declaration : qualified_search")
@pg.production("declaration : search")
def declaration(p):
    return p[0]

@pg.production("class : POUND DATA")
def class_decl(p):
    return Class(p[1].value)

@pg.production("qualified_variable : qualifier variable")
@pg.production("qualified_variable : qualifier qualified_variable")
@pg.production("qualified_function : qualifier function")
@pg.production("qualified_function : qualifier qualified_function")
@pg.production("qualified_class : qualifier class")
@pg.production("qualified_class : qualifier qualified_class")
@pg.production("qualified_search : qualifier search")
@pg.production("qualified_search : qualifier qualified_search")
def qualified(p):
    p[1].qualifiers.insert(0, p[0])
    return p[1]

@pg.production("qualifier : data DOUBLE_COLON")
def qualifier(p):
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

from rply import ParserGenerator
from .lexer import lexer
from .kinds import CharacterMapping

pg = ParserGenerator(
    [rule.name for rule in lexer.rules],
    cache_id="cache",

    # NOTE: This is pretty arbitrary at the moment
    precedence=[
        ('left', ['PARENT', 'CHILD', 'OR']),
        ('left', ['AND', 'CONSTRAINT']),
        ('left', ['L_PAREN', 'R_PAREN', "EQUAL"]),
    ])


@pg.production("expr : L_PAREN expr R_PAREN")
def expr(p):
    return p[1]

@pg.production("expr : expr CONSTRAINT expr")
@pg.production("expr : expr AND expr")
@pg.production("expr : expr OR expr")
@pg.production("expr : expr CHILD expr")
@pg.production("expr : expr PARENT expr")
def binary_operation(p):
    return [p[1], p[0], p[2]]


@pg.production("expr : expr EQUAL DATA CONSTRAINT expr")
def constraing_data(p):
    # In `\F=foobar~\P` the constraint is on '\F'
    p[2].value = p[2].value.strip("/")
    return p


@pg.production("expr : expr EQUAL DATA")
def equal(p):
    p[2].value = p[2].value.strip("/")
    return p

for kind in CharacterMapping.keys():
    @pg.production("expr : " + kind)
    def kind(p):
        return CharacterMapping[p[0].name]

parser = pg.build()

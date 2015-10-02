from rply import ParserGenerator


def parser_from_lexer(lexer, mapping):
    pg = ParserGenerator(
        [rule.name for rule in lexer.rules],
        cache_id="cache",

        # NOTE: This is pretty arbitrary at the moment
        precedence=[
            ('right', ['NOT']),
            ('left', ['PARENT', 'CHILD', 'OR']),
            ('left', ['AND']),
            ('left', ['L_PAREN', 'R_PAREN', 'TYPE', 'EQUAL']),
        ])

    @pg.production("expr : L_PAREN expr R_PAREN")
    def expr(p):
        return p[1]

    @pg.production("expr : expr AND expr")
    @pg.production("expr : expr OR expr")
    @pg.production("expr : expr CHILD expr")
    @pg.production("expr : expr PARENT expr")
    def binary_operation(p):
        return [p[1], p[0], p[2]]

    @pg.production("expr : expr EQUAL DATA")
    @pg.production("expr : expr TYPE DATA")
    def equal(p):
        p[2].value = p[2].value.strip("/")
        if not isinstance(p[0], list):
            return p
        else:
            # Note that this doesn't work for (\F|\V)=foobar
            p[0].append(p[1])
            p[0].append(p[2])
            return p[0]

    @pg.production("expr : expr NOT CHILD expr")
    @pg.production("expr : expr NOT PARENT expr")
    def not_expr(p):
        # This is a hack
        op = p[2]
        op.name = "NOT_" + op.name
        op.value = "!" + op.value
        return [op, p[0], p[3]]

    for kind in mapping.keys():
        @pg.production("expr : " + kind)
        def kind(p):
            return mapping[p[0].name]

    parser = pg.build()
    return parser

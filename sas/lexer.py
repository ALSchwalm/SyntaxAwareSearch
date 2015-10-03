from rply import LexerGenerator


def lexer_from_mapping(mapping):
    lg = LexerGenerator()

    # Escape data with forward slashes
    lg.add("DATA", r'/.+?/')

    lg.add("L_PAREN", r'\(')
    lg.add("R_PAREN", r'\)')

    lg.add("DOUBLE_COLON", r'::')
    lg.add("COLON", r':')

    lg.add("COMMA", r',')
    lg.add("ELLIPSE", r'\.\.\.')

    lg.add("DATA", r'[^():,{}]+')

    lg.ignore(r'\s+')
    lexer = lg.build()
    return lexer

from rply import LexerGenerator


def lexer_from_mapping(mapping):
    lg = LexerGenerator()

    # Escape data with forward slashes
    lg.add("DATA", r'/.+?/')

    # Add the special characters
    for char in mapping.keys():
        lg.add(char, r"\\" + char)

    # Normal tokens
    lg.add("TYPE", r'~')
    lg.add("AND", r'\&')
    lg.add("OR", r'\|')
    lg.add("L_PAREN", r'\(')
    lg.add("R_PAREN", r'\)')
    lg.add("EQUAL", r'=')
    lg.add("CHILD", r'>')
    lg.add("PARENT", r'<')

    # Everything else is data
    excluded_chars = r'^<>=&|()~'
    for char in mapping.keys():
        excluded_chars += r"\\" + char
        lg.add("DATA", "[{excluded}]+".format(excluded=excluded_chars))

    lg.ignore(r'\s+')
    lexer = lg.build()
    return lexer

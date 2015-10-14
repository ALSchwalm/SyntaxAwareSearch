from rply import LexerGenerator

lg = LexerGenerator()

# Escape data with forward slashes
lg.add("DATA", r'/.+?/')

lg.add("L_PAREN", r'\(')
lg.add("R_PAREN", r'\)')

lg.add("L_CURLY", r'\{')
lg.add("R_CURLY", r'\}')

lg.add("DOUBLE_COLON", r'::')
lg.add("COLON", r':')

lg.add("POUND", r'#')

lg.add("COMMA", r',')
lg.add("ELLIPSES", r'\.\.\.')

lg.add("DATA", r'[^():,{}#]+')

lg.ignore(r'\s+')
lexer = lg.build()

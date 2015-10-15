"""Synatx Aware Search

Usage:
  sas <pattern> <file>... [-l <lang>] [--verbose] [-d] [-e] [-f]
  sas (-h | --help)
  sas --version

Options:
  -h --help     Show this screen.
  --version     Show version.
  -l=<lang>     Set the search lang
  -e            Match expressions
  -d            Match declarations
  -f            Show full matches (i.e., entire function rather than 1st line)
  --verbose     Show more info in output (e.g., AST)
"""

from .lexer import lexer
from .parser import parser
from .config import Config
from docopt import docopt
import linecache


def grep_print(filename, line, full):
    """ Print a cursor using a grep-like syntax
    """
    if not full:
        print("{location}:{line}:{match}".format(
            location=filename,
            line=line,
            match=linecache.getline(filename, line).strip()))
    else:
        for multiline in range(line[0], line[1]+1):
            print("{location}:{line}:{match}".format(
                location=filename,
                line=multiline,
                match=linecache.getline(filename, multiline).strip("\n")))


def matches_from_pattern(config, full):
    find_candidates = None

    if not config.language or config.language == "cpp":
        from .backends.cpp import find_candidates
        find_candidates = find_candidates

    lexed = lexer.lex(config.raw_pattern)
    ast = parser.parse(lexed)

    if config.verbose:
        import pprint
        pprint.pprint(ast)
    for match in find_candidates(config, ast):
        if full:
            yield (match[0][0], match[1][0])
        else:
            yield match[0][0]


def main():
    arguments = docopt(__doc__, version='sas v0.2')
    mode = Config.MATCH_MODE.DECLARATION | Config.MATCH_MODE.EXPRESSION
    if arguments["-d"] or arguments["-e"]:
        mode = 0
        if arguments["-d"]:
            mode |= Config.MATCH_MODE.DECLARATION
        if arguments["-e"]:
            mode |= Config.MATCH_MODE.EXPRESSION

    config = Config(
        raw_pattern=arguments["<pattern>"],
        language=arguments["-l"],
        mode=mode,
        verbose=arguments["--verbose"],
        full=arguments["-f"]
    )

    for filename in arguments["<file>"]:
        config.filename = filename
        for match in matches_from_pattern(config, config.full):
            grep_print(filename, match, config.full)


if __name__ == "__main__":
    main()

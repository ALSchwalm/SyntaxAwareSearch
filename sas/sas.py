"""Synatx Aware Search

Usage:
  sas <pattern> <file>... [-l <lang>] [--verbose]
  sas (-h | --help)
  sas --version

Options:
  -h --help     Show this screen.
  --version     Show version.
  -l=<lang>     Set the search lang
  --verbose     Show more info in output (e.g., AST)
"""

from .lexer import lexer
from .parser import parser
from docopt import docopt
import linecache


def grep_print(file_name, line):
    """ Print a cursor using a grep-like syntax
    """
    print("{location}:{line}:{match}".format(
        location=file_name,
        line=line,
        match=linecache.getline(file_name, line).strip()))


def matches_from_pattern(path, pattern, language=None, verbose=False):
    find_candidates = None

    if not language or language == "cpp":
        from .backends.cpp import find_candidates
        find_candidates = find_candidates

    lexed = lexer.lex(pattern)
    ast = parser.parse(lexed)

    if verbose:
        import pprint
        pprint.pprint(ast)
    for line in find_candidates(path, ast):
        yield line


def main():
    arguments = docopt(__doc__, version='sas v0.1')
    for file_name in arguments["<file>"]:
        try:
            for match in matches_from_pattern(file_name,
                                              arguments["<pattern>"],
                                              arguments["-l"],
                                              arguments["--verbose"]):
                grep_print(file_name, match)
        except:
            print("An error was encountered while parsing `{}`".format(
                file_name
            ))


if __name__ == "__main__":
    main()

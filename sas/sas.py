"""Synatx Aware Search

Usage:
  sas [-l <lang>] [--verbose] [--color] [-Rrdem] <pattern> <path>...
  sas (-h | --help)
  sas --version

Options:
  -h --help     Show this screen.
  --version     Show version.
  -r            Search 'path' recursively (not following symbolic links)
  -R            Search 'path' recursively (following symbolic links)
  -l=<lang>     Set the search lang
  -e            Match expressions
  -d            Match declarations
  -m            Show full matches (i.e., entire function rather than 1st line)
  --color       Highlight the matched text
  --verbose     Show more info in output (e.g., AST)
"""

from __future__ import print_function
from .lexer import lexer
from .parser import parser
from .config import Config
from docopt import docopt
from termcolor import colored
from sys import exit
import os
import linecache


def grep_print(filename, extent, full, color, context=(0, 0)):
    """ Print a cursor using a grep-like syntax
    """
    if not full:
        if not color:
            print("{location}:{line}:{match}".format(
                location=filename,
                line=extent[0][0],
                match=linecache.getline(filename, extent[0][0]).strip("\n")))
        else:
            line = linecache.getline(filename, extent[0][0]).strip("\n")
            pre_match = line[:extent[0][1]-1]
            if extent[0][0] == extent[1][0]:
                match = line[extent[0][1]-1:extent[1][1]-1]
                post_match = line[extent[1][1]-1:]
            else:
                match = line[extent[0][1]-1:]
                post_match = ""
            print("{location}:{line}:{pre_match}".format(
                location=filename,
                line=extent[0][0],
                pre_match=pre_match), end="")
            print(colored(match, 'red', attrs=['bold']), end="")
            print(post_match)
    else:
        for multiline in range(extent[0][0], extent[1][0]+1):
            print("{location}:{line}:{match}".format(
                location=filename,
                line=multiline,
                match=linecache.getline(filename, multiline).strip("\n")))


def matches_from_pattern(config):
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
        yield match


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
        full=arguments["-m"],
        color=arguments["--color"]
    )

    def search_file(filename):
        config.filename = filename
        for match in matches_from_pattern(config):
            grep_print(filename, match, config.full, config.color)

    for filename in arguments["<path>"]:
        if arguments["-R"] or arguments["-r"]:
            if os.path.isfile(filename):
                search_file(filename)
            else:
                followlinks = True if arguments["-R"] else False
                for dirpath, subdirs, files in os.walk(filename,
                                                       followlinks=followlinks):
                    for file in files:
                        search_file(os.path.join(dirpath, file))
        else:
            if os.path.isdir(filename):
                exit("sas: {}: Is a directory".format(filename))
            else:
                search_file(filename)


if __name__ == "__main__":
    main()

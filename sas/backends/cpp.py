from clang.cindex import TranslationUnit
from clang.cindex import CursorKind
from .utils import get_cursors
from ..wrappers import *
from re import match
from rply import Token

def find_candidates(filename, ast):
    """ Find patterns in 'filename' matching 'ast'
    """

    tu = TranslationUnit.from_source(filename, ["-std=c++11"])
    for cursor in resolve_ast(tu, ast):
        yield cursor.location.line


def resolve_ast(tu, ast):
    """ Yields cursors matching the pattern in 'ast'
    """
    if isinstance(ast, Function):
        for cursor in resolve_function(tu, ast):
            yield cursor
    elif isinstance(ast, Variable):
        for cursor in resolve_variable(tu, ast):
            yield cursor


def recursive_children(cursor):
    for child in cursor.get_children():
        for grandchild in recursive_children(child):
            yield grandchild
        yield child

def matches_by_kinds(cursor, variable, kinds):
    return cursor.kind in kinds and \
        match(variable.type, cursor.type.spelling) and \
        match(variable.name, cursor.spelling)

def matches_function_parameters(cursor, function):
    #TODO: Allow ellipses anywhere in the signature
    ellipses_present = False
    parameters = function.parameters
    if len(parameters) > 0 and isinstance(parameters[-1], Token):
        if function.parameters[-1].name == "ELLIPSES":
            ellipses_present = True
            parameters = function.parameters[:-1]
    if not ellipses_present and \
       len(list(cursor.get_arguments())) != len(parameters):
        return False
    elif ellipses_present and \
         len(list(cursor.get_arguments())) < len(parameters):
        return False
    for i, parameter in enumerate(cursor.get_arguments()):
        if len(parameters) <= i and ellipses_present:
            return True
        if not matches_by_kinds(parameter, parameters[i],
                                (CursorKind.PARM_DECL,)):
            return False
    return True

def resolve_function(tu, function):
    for cursor in get_cursors(tu, function.name):
        if cursor.kind in (CursorKind.FUNCTION_DECL, CursorKind.CXX_METHOD) and \
           match(function.return_type, cursor.result_type.spelling) and \
           matches_function_parameters(cursor, function):
            yield cursor

def resolve_variable(tu, variable):
    for cursor in get_cursors(tu, variable.name):
        if matches_by_kinds(cursor, variable, (CursorKind.VAR_DECL,)):
            yield cursor

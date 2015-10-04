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
    elif isinstance(ast, Class):
        for cursor in resolve_class(tu, ast):
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
    cursor_parameters = list(cursor.get_arguments())
    ellipses_active = False
    for parameter in function.parameters:
        if isinstance(parameter, Token):
            ellipses_active = True
            continue
        if len(cursor_parameters) == 0:
            return False

        if not matches_by_kinds(cursor_parameters[0],
                                parameter,
                                (CursorKind.PARM_DECL,)):
            if not ellipses_active:
                return False
            else:
                cursor_parameters = cursor_parameters[1:]
        else:
            cursor_parameters = cursor_parameters[1:]
            ellipses_active = False
    if len(cursor_parameters) and not ellipses_active:
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

def resolve_class(tu, class_t):
    for cursor in get_cursors(tu, class_t.name):
        if cursor.kind in (CursorKind.CLASS_DECL, CursorKind.STRUCT_DECL):
            yield cursor

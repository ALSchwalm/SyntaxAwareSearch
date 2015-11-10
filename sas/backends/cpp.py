from clang.cindex import CursorKind, AccessSpecifier, StorageClass
from clang.cindex import conf
from clang.cindex import Index, TranslationUnit
from .utils import get_cursors, get_root_cursors
from ..wrappers import *
from ..config import Config
from re import match
from rply import Token

SAS_Index = Index.create(excludeDecls=True)


def find_candidates(config, ast):
    """ Find patterns in 'filename' matching 'ast'
    """

    flags = ["-std=c++11"]
    if not config.strict:
        flags.append("-nostdinc++")
    tu = SAS_Index.parse(config.filename, flags,
                         options=TranslationUnit.PARSE_PRECOMPILED_PREAMBLE)
    for cursor in resolve_ast(tu, ast, config):
        start, end = cursor.extent.start, cursor.extent.end
        yield ((start.line, start.column),
               (end.line, end.column))


def has_required_children(tu, cursor, children, config):
    if not children:
        return True
    for child in children:
        try:
            next(resolve_ast(tu, child, config, cursor))
        except StopIteration:
            break
    else:
        return True
    return False


def resolve_ast(tu, ast, config, root=None):
    """ Yields cursors matching the pattern in 'ast'
    """
    def inner_resolve():
        if isinstance(ast, Function):
            for cursor in resolve_function(tu, ast, config, root):
                yield cursor
        elif isinstance(ast, Variable):
            for cursor in resolve_variable(tu, ast, config, root):
                yield cursor
        elif isinstance(ast, Class):
            for cursor in resolve_class(tu, ast, config, root):
                yield cursor
        elif isinstance(ast, Search):
            for cursor in resolve_basic_search(tu, ast, config, root):
                yield cursor
    for result in inner_resolve():
        if config.mode & Config.MATCH_MODE.DEFINITION and not result.is_definition():
            continue
        elif has_required_children(tu, result, ast.contents, config):
            yield result


def matches_by_kinds(cursor, variable, kinds, config):
    return cursor.kind in kinds and \
        match(variable.type, cursor.type.spelling) and \
        match(variable.name, cursor.spelling)


def matches_attributes(cursor, attributes):
    attributes = [attr.lower() for attr in attributes]
    if "virtual" in attributes and not conf.lib.clang_CXXMethod_isVirtual(cursor):
        return False
    if "static" in attributes and (not cursor.is_static_method() and
                                   not cursor.storage_class == StorageClass.STATIC):
        return False
    if "const" in attributes and not cursor.is_const_method():
        return False

    access_map = {
        "public": AccessSpecifier.PUBLIC,
        "protected": AccessSpecifier.PROTECTED,
        "private": AccessSpecifier.PRIVATE,
    }
    access_specifier = cursor.access_specifier
    for attr in attributes:
        if attr in access_map and access_specifier != access_map[attr]:
            return False
    return True


def matches_parameters(cursor, parameters, config, template=False):
    if not template:
        cursor_parameters = list(cursor.get_arguments())
        allowed_kinds = (CursorKind.PARM_DECL,)
    else:
        if parameters is None:
            return True
        allowed_kinds = (CursorKind.TEMPLATE_TYPE_PARAMETER,
                         CursorKind.TEMPLATE_NON_TYPE_PARAMETER,
                         CursorKind.TEMPLATE_TEMPLATE_PARAMETER)
        cursor_parameters = [child for child in cursor.get_children()
                             if child.kind in allowed_kinds]
    print([(parameter.spelling, parameter.type.spelling) for parameter in cursor_parameters])
    ellipses_active = False
    i = 0
    while i < len(parameters):
        parameter = parameters[i]
        if isinstance(parameter, Token):
            ellipses_active = True
            i += 1
            continue
        if len(cursor_parameters) == 0:
            return False

        if not matches_by_kinds(cursor_parameters[0],
                                parameter,
                                allowed_kinds,
                                config):
            if not ellipses_active:
                return False
            else:
                cursor_parameters = cursor_parameters[1:]
                continue
        else:
            cursor_parameters = cursor_parameters[1:]
            ellipses_active = False
        i += 1
    if len(cursor_parameters) and not ellipses_active:
        return False
    return True


def resolve_qualifiers(tu, qualifiers, config, root):
    if qualifiers is None:
        for cursor in get_root_cursors(tu, filename=config.filename, root=root):
            yield cursor
    elif len(qualifiers) == 0:
        for cursor in get_cursors(tu, filename=config.filename, root=root):
            yield cursor
    else:
        def valid_qualifier_cursor(cursor):
            return cursor.kind in (CursorKind.CLASS_DECL,
                                   CursorKind.STRUCT_DECL,
                                   CursorKind.NAMESPACE)

        def recurse_qualifiers(cursor, qualifiers):
            if len(qualifiers) == 0:
                for child in cursor.get_children():
                    yield child
            else:
                for child in cursor.get_children():
                    if not match(qualifiers[0], child.spelling) or \
                       not valid_qualifier_cursor(child):
                        continue
                    for inner in recurse_qualifiers(child, qualifiers[1:]):
                        yield inner
        for cursor in get_root_cursors(tu, qualifiers[0], config.filename, root):
            if not valid_qualifier_cursor(cursor):
                continue
            for inner in recurse_qualifiers(cursor, qualifiers[1:]):
                yield inner


def resolve_function(tu, function, config, root):
    allowed_kinds = []
    if config.mode & Config.MATCH_MODE.DECLARATION:
        allowed_kinds += [CursorKind.FUNCTION_DECL,
                          CursorKind.CXX_METHOD,
                          CursorKind.FUNCTION_TEMPLATE,
                          CursorKind.CONSTRUCTOR,
                          CursorKind.DESTRUCTOR]
    if config.mode & Config.MATCH_MODE.EXPRESSION:
        allowed_kinds += [CursorKind.CALL_EXPR]

    for cursor in resolve_qualifiers(tu, function.qualifiers, config, root):
        if cursor.kind in allowed_kinds and \
           match(function.name, cursor.spelling) and \
           match(function.return_type, cursor.result_type.spelling) and \
           matches_parameters(cursor, function.parameters, config) and \
           matches_parameters(cursor, function.template_parameters,
                              config,
                              template=True) and \
           matches_attributes(cursor, function.attributes):
            yield cursor


def resolve_variable(tu, variable, config, root):
    allowed_kinds = []
    if config.mode & Config.MATCH_MODE.DECLARATION:
        allowed_kinds += [CursorKind.VAR_DECL]
    if config.mode & Config.MATCH_MODE.EXPRESSION:
        allowed_kinds += [CursorKind.DECL_REF_EXPR,
                          CursorKind.MEMBER_REF_EXPR]
    for cursor in resolve_qualifiers(tu, variable.qualifiers, config, root):
        if match(variable.name, cursor.spelling) and \
           matches_by_kinds(cursor, variable, allowed_kinds, config) and \
           matches_attributes(cursor, variable.attributes):
            yield cursor


def resolve_class(tu, class_t, config, root):
    allowed_kinds = []
    if config.mode & Config.MATCH_MODE.DECLARATION:
        allowed_kinds += [CursorKind.CLASS_DECL,
                          CursorKind.STRUCT_DECL,
                          CursorKind.CLASS_TEMPLATE,
                          CursorKind.CLASS_TEMPLATE_PARTIAL_SPECIALIZATION]
    if config.mode & Config.MATCH_MODE.EXPRESSION:
        allowed_kinds += []
    for cursor in resolve_qualifiers(tu, class_t.qualifiers, config, root):
        if match(class_t.name, cursor.spelling) and \
           matches_parameters(cursor, class_t.template_parameters, template=True) and \
           cursor.kind in allowed_kinds:
            yield cursor


def resolve_basic_search(tu, search, config, root):
    for cursor in resolve_qualifiers(tu, search.qualifiers, config, root):
        if match(search.search, cursor.spelling):
            yield cursor

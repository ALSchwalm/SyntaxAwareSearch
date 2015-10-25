# This file provides common utility functions
# It is based on the original LLVM file

from clang.cindex import Cursor
from clang.cindex import conf
from clang.cindex import callbacks
import re


def get_cursors(source, regex=None, filename=None, root=None):
    """Obtain all cursors from a source object with a specific spelling.

    This provides a convenient search mechanism to find all cursors with
    specific spelling within a source. The first argument can be either a
    TranslationUnit or Cursor instance.
    """
    if root is None:
        root_cursor = source if isinstance(source, Cursor) else source.cursor
    else:
        root_cursor = root

    def is_valid(cursor):
        if regex is None or re.match(regex, cursor.spelling):
            return True
        return False

    def visitor(child, parent, children):
        # Create reference to TU so it isn't GC'd before Cursor.
        child._tu = root_cursor.translation_unit
        if filename is not None and filename != str(child.location.file):
            return 1  # continue
        elif is_valid(child):
            children.append(child)
        return 2  # recurse
    children = []
    conf.lib.clang_visitChildren(root_cursor, callbacks['cursor_visit'](visitor),
                                 children)
    for cursor in children:
        yield cursor


def get_root_cursors(source, regex=None, filename=None, root=None):
    """ A generator yielding each 'top level' cursor
    """
    if root is None:
        root_cursor = source if isinstance(source, Cursor) else source.cursor
    else:
        root_cursor = root
    for cursor in root_cursor.get_children():
        if filename is not None and filename != str(cursor.location.file):
            continue
        elif regex is None or re.match(regex, cursor.spelling):
            yield cursor


__all__ = [
    'get_cursors',
    'get_root_cursors'
]

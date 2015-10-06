# This file provides common utility functions
# It is based on the original LLVM file

from clang.cindex import Cursor
import re


def get_cursor(source, regex=None):
    """Obtain a cursor from a source object.

    This provides a convenient search mechanism to find a cursor with specific
    spelling within a source. The first argument can be either a
    TranslationUnit or Cursor instance.

    If the cursor is not found, None is returned.
    """
    # Convenience for calling on a TU.
    root_cursor = source if isinstance(source, Cursor) else source.cursor

    for cursor in root_cursor.walk_preorder():
        if regex is None or re.match(regex, cursor.spelling):
            return cursor
    return None


def get_cursors(source, regex=None):
    """Obtain all cursors from a source object with a specific spelling.

    This provides a convenient search mechanism to find all cursors with
    specific spelling within a source. The first argument can be either a
    TranslationUnit or Cursor instance.
    """
    # Convenience for calling on a TU.
    root_cursor = source if isinstance(source, Cursor) else source.cursor

    for cursor in root_cursor.walk_preorder():
        if regex is None or re.match(regex, cursor.spelling):
            yield cursor


def get_root_cursors(source, regex=None):
    """ A generator yielding each 'top level' cursor
    """
    root_cursor = source if isinstance(source, Cursor) else source.cursor
    for cursor in root_cursor.get_children():
        if regex is None or re.match(regex, cursor.spelling):
            yield cursor


__all__ = [
    'get_cursor',
    'get_cursors',
    'get_root_cursors'
]

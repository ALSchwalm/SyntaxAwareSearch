# This file provides common utility functions
# It is based on the original LLVM file

from clang.cindex import Cursor
import re

def get_cursors(source, regex=None, filename=None):
    """Obtain all cursors from a source object with a specific spelling.

    This provides a convenient search mechanism to find all cursors with
    specific spelling within a source. The first argument can be either a
    TranslationUnit or Cursor instance.
    """
    # Convenience for calling on a TU.
    root_cursor = source if isinstance(source, Cursor) else source.cursor

    def is_valid(cursor):
        if regex is None or re.match(regex, cursor.spelling):
            return True
        return False

    def recursive_children(cursor):
        for child in cursor.get_children():
            if filename is not None and filename != str(child.location.file):
                continue
            for grandchild in recursive_children(child):
                if is_valid(grandchild):
                    yield grandchild
            if is_valid(child):
                yield child
    for cursor in recursive_children(root_cursor):
        yield cursor


def get_root_cursors(source, regex=None, filename=None):
    """ A generator yielding each 'top level' cursor
    """
    root_cursor = source if isinstance(source, Cursor) else source.cursor
    for cursor in root_cursor.get_children():
        if filename is not None and filename != str(cursor.location.file):
            continue
        elif regex is None or re.match(regex, cursor.spelling):
            yield cursor


__all__ = [
    'get_cursor',
    'get_cursors',
    'get_root_cursors'
]

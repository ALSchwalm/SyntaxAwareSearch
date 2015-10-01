def enum(*sequential, **named):
    # http://stackoverflow.com/a/1695250
    enums = dict(zip(sequential, range(len(sequential))), **named)
    return type('Enum', (), enums)

Kinds = enum(
    "FUNCTION",
    "TYPE",
    "VARIABLE",
    "PARAMETER"
)

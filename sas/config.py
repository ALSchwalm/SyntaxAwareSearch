

# https://stackoverflow.com/questions/36932/how-can-i-represent-an-enum-in-python
def enum(**enums):
    return type('Enum', (), enums)


class Config:
    MATCH_MODE = enum(
        DECLARATION=1 << 0,
        EXPRESSION=1 << 1
    )

    EXTENSION_MAP = {
        "cpp": ["cxx", "cpp", "h", "hpp"],
        "c": ["c", "h"]
    }

    def __init__(self, raw_pattern, language,
                 search_all=False,
                 mode=MATCH_MODE.DECLARATION | MATCH_MODE.EXPRESSION,
                 full=False, color=False, filename=None, verbose=False):
        self.filename = filename
        self.raw_pattern = raw_pattern
        self.language = language
        self.verbose = verbose
        self.mode = mode
        self.full = full
        self.color = color,
        self.search_all = search_all

    @property
    def allowed_extensions(self):
        return Config.EXTENSION_MAP[self.language]

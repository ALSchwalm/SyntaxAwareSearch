class SASType(object):
    def __init__(self, qualifiers=[], expression=False):
        self.qualifiers = qualifiers
        self.expression = expression

class Function(SASType):
    def __init__(self, name=".*", return_type=".*",
                 parameters=[], qualifiers=[]):
        SASType.__init__(self, qualifiers)
        self.name = name
        self.return_type = return_type
        self.parameters = parameters

    def __repr__(self):
        return "Function(\"{}\", \"{}\", {}, {})".format(
            self.name,
            self.return_type,
            self.parameters,
            self.qualifiers
        )

class Variable(SASType):
    def __init__(self, name=".*", type=".*", qualifiers=[]):
        SASType.__init__(self, qualifiers)
        self.name = name
        self.type = type

    def __repr__(self):
        return "Variable(\"{}\", \"{}\", {})".format(
            self.name, self.type, self.qualifiers)

class Class(SASType):
    def __init__(self, name=".*", qualifiers=[]):
        SASType.__init__(self, qualifiers)
        self.name = name

    def __repr__(self):
        return "Class(\"{}\", {})".format(self.name,
                                          self.qualifiers)

class Search(SASType):
    def __init__(self, search=".*", qualifiers=[]):
        SASType.__init__(self, qualifiers)
        self.search = search

    def __repr__(self):
        return "Search(\"{}\", {})".format(self.search,
                                           self.qualifiers)

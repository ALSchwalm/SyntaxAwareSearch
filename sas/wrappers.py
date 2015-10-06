class Function(object):
    def __init__(self, name=".*", return_type=".*",
                 parameters=[], qualifiers=[]):
        self.name = name
        self.return_type = return_type
        self.parameters = parameters
        self.qualifiers = qualifiers

    def __repr__(self):
        return "Function(\"{}\", \"{}\", {}, {})".format(
            self.name,
            self.return_type,
            self.parameters,
            self.qualifiers
        )

class Variable(object):
    def __init__(self, name=".*", type=".*", qualifiers=[]):
        self.name = name
        self.type = type
        self.qualifiers = qualifiers

    def __repr__(self):
        return "Variable(\"{}\", \"{}\", {})".format(
            self.name, self.type, self.qualifiers)

class Class(object):
    def __init__(self, name=".*", qualifiers=[]):
        self.name = name
        self.qualifiers = qualifiers

    def __repr__(self):
        return "Class(\"{}\", {})".format(self.name,
                                          self.qualifiers)

class Search(object):
    def __init__(self, search=".*", qualifiers=[]):
        self.search = search
        self.qualifiers = qualifiers

    def __repr__(self):
        return "Search(\"{}\", {})".format(self.search,
                                           self.qualifiers)

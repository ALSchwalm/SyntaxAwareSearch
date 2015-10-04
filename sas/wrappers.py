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

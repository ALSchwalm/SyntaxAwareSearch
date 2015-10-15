class SASType(object):
    def __init__(self, qualifiers=[], contents=[]):
        self.qualifiers = qualifiers
        self.contents = []


class Function(SASType):
    def __init__(self, name=".*", return_type=".*",
                 parameters=[], qualifiers=[], template_parameters=[]):
        SASType.__init__(self, qualifiers)
        self.name = name
        self.return_type = return_type
        self.parameters = parameters
        self.template_parameters = template_parameters

    def __repr__(self):
        return "Function(name=\"{}\", return_type=\"{}\", " \
               "template_parameters={}, parameters={}, qualifiers={}, " \
               "contents={})".format(
                   self.name,
                   self.return_type,
                   self.template_parameters,
                   self.parameters,
                   self.qualifiers,
                   self.contents
               )


class Variable(SASType):
    def __init__(self, name=".*", type=".*", qualifiers=[]):
        SASType.__init__(self, qualifiers)
        self.name = name
        self.type = type

    def __repr__(self):
        return "Variable(\"{}\", \"{}\", {}, {})".format(
            self.name, self.type, self.qualifiers,
            self.contents)


class Class(SASType):
    def __init__(self, name=".*", qualifiers=[]):
        SASType.__init__(self, qualifiers)
        self.name = name

    def __repr__(self):
        return "Class(\"{}\", {}, {})".format(self.name,
                                              self.qualifiers,
                                              self.contents)


class Search(SASType):
    def __init__(self, search=".*", qualifiers=[]):
        SASType.__init__(self, qualifiers)
        self.search = search

    def __repr__(self):
        return "Search(\"{}\", {}, {})".format(self.search,
                                               self.qualifiers,
                                               self.contents)

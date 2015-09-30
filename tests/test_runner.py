import os
from collections import namedtuple
from lagrep.lagrep import matches_from_pattern

TestCase = namedtuple("TestCase", ["lines", "passes", "pattern"])


def parse_case(line):
    info, expression = line.split(":>")
    if "+" not in info and "-" not in info:
        assert("MISSING PLUS OR MINUS IN TEST CASE" and False)
    return TestCase([int(l) for l in info[:-1].split(",")],
                    "+" in info,
                    expression.strip())


def test_all():
    path = os.path.join(os.path.dirname(__file__), 'test_cases.txt')
    test_cpp_1 = os.path.join(os.path.dirname(__file__), 'file1.cpp')
    print(test_cpp_1)
    with open(path) as tests:
        for line in tests.readlines():
            if not line.strip() or line.startswith("#"):
                continue
            case = parse_case(line)
            for match in matches_from_pattern(test_cpp_1,
                                              case.pattern,
                                              language="cpp"):
                if case.passes:
                    assert(match in case.lines)
                else:
                    assert(match not in case.lines)

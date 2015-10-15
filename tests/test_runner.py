import os
from collections import namedtuple
from sas.sas import matches_from_pattern
from sas.config import Config

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
    with open(path) as tests:
        for line in tests.readlines():
            if not line.strip() or line.startswith("#"):
                continue
            case = parse_case(line)
            matches = list(matches_from_pattern(
                Config(case.pattern, "cpp",
                       filename=test_cpp_1), full=False))
            for line in case.lines:
                if case.passes:
                    if line not in matches:
                        print(case.pattern, line)
                    assert(line in matches)
                else:
                    if line in matches:
                        print(case.pattern, line)
                    assert(line not in matches)

#!/usr/bin/env python
import os
from setuptools import setup, find_packages


long_description = open(
    os.path.join(
        os.path.dirname(__file__),
        'README.md'
    )
).read()


setup(
    name='sas',
    author='Adam Schwalm',
    version='0.1',
    url='https://github.com/ALSchwalm/SyntaxAwareSearch',
    download_url = 'https://github.com/ALSchwalm/SyntaxAwareSearch/tarball/0.1',
    description='A grep-like tool with syntax awareness',
    long_description=long_description,
    packages=find_packages('.', exclude=["*.tests", "*.tests.*", "tests.*", "tests"]),
    install_requires=["clang", "rply", "docopt"],
        entry_points={
        'console_scripts': [
            'sas = sas.sas:main',
        ]
    },
    keywords = ["grep", "syntax", "search"],
)

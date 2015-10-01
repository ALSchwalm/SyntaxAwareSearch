SyntaxAwareSearch
=================

SyntaxAwareSearch (sas) is a grep-like tool with an understanding of the
syntax of languages. `sas` uses a domain specific language to allow very
concise but powerful queries of code.

The search Language
-------------------

The `sas` DSL has a few components:

- `term`: A special character used to match, e.g. a function or variable declaration
- `operator`: Expresses a relationship between terms
- `data`: Additional information constraing the type or name of a term

Each languages supported by `sas` may have additional language-specific features,
however, the following `terms` and `operators` are supported:

### Terms
- `\F`: Matches a function declaration
- `\V`: Matches a variable declaration
- `\P`: Matches a function parameter declaration
- `\T`: Matches a type declaration

Terms may also be constrained with `=` which requires that the matching item has a
'name' matching the given regular expression (the 'name' is dependent on the language
and term). The type of the match can also be constrained with `~`.

### Operators
- `>`: The 'child' operator. The expression on the right occurs lexically within the
one on the left
- `<`: The `parent` operator. The expression on the left occurs lexically within the
one on the right
- `|`: The `or` operator. Matches the expression on the left or right

Examples
--------

Given the following C++ program "demo.cpp":

    class DemoClass {
    public:
        void member_func() {}
    };

    int x;

    float top_level(){
        float y;
        return 0.0;
    }

The following expressions would give the provided results.

| Expression   | Result                                                           |
|--------------|------------------------------------------------------------------|
| `\F`         | demo.cpp:3:void member_func() {}<br>demo.cpp:8:float top_level(){|
| `\F=.*level` | demo.cpp:8:float top_level(){                                    |
| `\V`         | demo.cpp:6:int x;<br>demo.cpp:9:float y;                         |
| `\V~int`     | demo.cpp:6:int x;                                                |
| `\F>\V~float`| demo.cpp:8:float top_level(){                                    |
| `\T`         | demo.cpp:1:class DemoClass {                                     |
| `\F>\V|\T`   | demo.cpp:8:float top_level(){<br>demo.cpp:1:class DemoClass {    |
| `\F>(\V|\T)` | demo.cpp:8:float top_level(){                                    |

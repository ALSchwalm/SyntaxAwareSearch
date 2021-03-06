SyntaxAwareSearch
=================

SyntaxAwareSearch (sas) is a grep-like tool with an understanding of the
syntax of C/C++. `sas` uses a domain specific language to allow very
concise but powerful queries of code.

The search Language
-------------------

`sas` currently supports the following kinds of queries. In this document,
`<regex>` represents some ECMAscript regulary expression pattern.

- Variable: `<regex>:<regex>`-  The first regular expression is used to match
against the type of the variable. The second matches against the name. If one
of the expressions is ommited, it will default to ".*" (i.e., it will match
anything).

- Function: `<regex>:<regex>(<parameter list>)`-  The first regular expression is
used to match against the return type of the function, the second matches against
the name of the function. A parameter match takes the form `<regex>:<regex>`,
where the first expression matches against the type of the parameter and the
second matches against the name. An ellipses `...` can also be provided to match
any number of parameters of arbitrary name and type.

- Class: `#<regex>` - The regex is used to match against the name of the class.

- Qualifiers: `<regex>::` - A qualifier may be attached to any of the above form
to restric the class or namespace searched for matches. For example, `boost::int:x`
would match a variable which has a name matching `x` and a type matching `int` in
the namespace/class `boost`.

In some cases, a regex may need to include characters which are reserved for the
search language (e.g., colon or parenthesis). In these cases, the regular expression
may be written between two forward slashes. For example: `.*: void (*) ()` will result
in a parsing error, but `.*:/void (*) ()/` will match variables (in C/C++) which are
function pointers with the appropriate signatures.

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
| `float:top()`| demo.cpp:8:float top_level(){                                    |
| `.*:.*()`    | demo.cpp:3:void member_func() {}<br>demo.cpp:8:float top_level(){|
| `.*:.*`      | demo.cpp:6:int x;<br>demo.cpp:9:float y;                         |
| `int:.*`     | demo.cpp:6:int x;                                                |
| `.*:x`       | demo.cpp:6:int x;                                                |
| `Demo::.*:.*func()` | demo.cpp:3:void member_func() {}                             |

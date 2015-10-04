class test_class {
public:
    void public_member_function();

protected:
    void protected_member_function();

private:
    void private_member_function() { int foo_bar_class; }
};

class test_bar;

int test_foo(int);

int bar() {
    int x;
    int y;
    test_foo(10);
    return 20;
}

int test_foo(int) {}

void has_parameters(float x, int) {}

namespace foo {
namespace bar {
namespace baz {
int x;
}
}
}

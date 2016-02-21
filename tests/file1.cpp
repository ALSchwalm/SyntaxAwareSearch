class test_class {
public:
    void public_member_function();

protected:
    void protected_member_function();

private:
    void private_member_function() { int foo_bar_class; }
};
test_class c;

class test_bar;

int test_foo(int);

int bar() {

    int x;
    int y;
    test_foo(10);
    return 20;
}

int test_foo(int) {
    for (int i = 0; i < 100; ++i) {
    }
}

void has_parameters(float x, int) {}

namespace foo {
namespace bar {
namespace baz {
int x;
}
}
}

int x = test_foo(10);

template <typename T>
void template_func() {
    auto x = []() {};
    has_parameters(1.0, 2);
    int inner_var;
}

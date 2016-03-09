/// Test basic function matching

void func1() {}

void func2(int, int) {}

int func3() {
    void func4();
    return 0;
}

// void:(...)
// 3 5 8
//
// :func3()
// 7
//
// int:
// 5 5

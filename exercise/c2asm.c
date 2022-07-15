int foo(int a, int b) {
    int c;
    asm volatile (
        "add %[sum], %[a], %[b]"
        :[sum]"=r"(c)
        :[a]"r"(a), [b]"r"(b)
    );

    // asm volatile (
    //     "add %0, %1, %2"
    //     :"=r"(c)
    //     :"r"(a), "r"(b)
    // );
    return c;
}
#include "xinos/syscall.h"

/**
 * 产生int 0x80中断的函数
 */
static _inline u32 _syscall0(u32 num) {
    /**
     * 内联汇编（扩展内联汇编）
     * asm ( assembler template
     *  : output operands                // optional, 要加=
     *  : input operands                 // optional
     *  : list of clobbered registers    // optional
     * );
     * 寄存器名称前有两个 %，这有助于 GCC 区分操作数和寄存器。操作数有一个 % 作为前缀
     * 输出操作数有一个约束修饰符 =，这个修饰符表示它是输出操作数并且是只写的。
     * a表示eax, b表示ebx,...可以去网上找
     */
    u32 ret;
    asm volatile(
        "int $0x80\n"
        : "=a"(ret)
        : "a"(num)
    );
    return ret;
}

/**
 * 产生int 0x80中断的函数
 */
static _inline u32 _syscall1(u32 nr, u32 arg) {
    u32 ret;
    asm volatile(
        "int $0x80\n"
        : "=a"(ret)
        : "a"(nr), "b"(arg)
    );
    return ret;
}

u32 test() {
    return _syscall0(SYS_NR_TEST);
}

/**
 * 调度函数, 主动调用这个函数产生int0x80中断
 */
void yield() {
    _syscall0(SYS_NR_YIELD);
}

// 睡眠函数
void sleep(u32 ms) {
    _syscall1(SYS_NR_SLEEP, ms);
}
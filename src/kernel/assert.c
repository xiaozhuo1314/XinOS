#include "xinos/assert.h"
#include "xinos/types.h"
#include "xinos/printk.h"
#include "xinos/stdarg.h"
#include "xinos/stdio.h"

// 用于存储错误语句的位置
static u8 buf[1024];

// 自旋阻塞
static void spin(char *name) {
    printk("spinning in %s\n", name);
    while (true)
        ;
    
}

// 断言失败函数
void assertion_failure(char *exp, char *file, char *base, int line) {
    // 打印错误提示
    printk("\n---------------------------------------------------------\n"
        "assert(%s) failure\n"
        "file: %s\n"
        "base: %s\n"
        "line: %d\n"
        "---------------------------------------------------------\n",
        exp, file, base, line
    );
    // 自旋阻塞
    spin("assertion_failure");

    // 自旋之后应该是不能走到这里的, 如果走到了, 说明有严重问题
    // ud2表示cpu出错
    asm volatile("ud2");
}

// 系统恐慌
void panic(const char *fmt, ...) {
    // 打印错误提示信息
    va_list args;
    va_start(args, fmt);
    int num = vsprintf(buf, fmt, args);
    va_end(args);
    printk("System is panic!!!\n%s\n", buf);

    // 自旋
    spin("panic");
    // 自旋之后应该是不能走到这里的, 如果走到了, 说明有严重问题
    // ud2表示cpu出错
    asm volatile("ud2");
}
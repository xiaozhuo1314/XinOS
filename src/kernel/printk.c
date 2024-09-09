#include "xinos/printk.h"
#include "xinos/stdarg.h"
#include "xinos/stdio.h"
#include "xinos/console.h"

static char buf[1024];

int printk(const char *fmt, ...) {
    // 设置参数指针
    va_list args;
    u32 num = 0;
    // 格式化到buf中
    va_start(args, fmt);
    num = vsprintf(buf, fmt, args);
    va_end(args);
    // 输出到屏幕
    console_write(buf, num);

    return num;
}
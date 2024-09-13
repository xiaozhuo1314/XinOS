#include "xinos/debug.h"
#include "xinos/printk.h"
#include "xinos/stdio.h"
#include "xinos/stdarg.h"

static char buf[1024];

void debugk(char *file ,int line, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    printk("[file: %s] [line: %d] %s", file, line, buf);
}
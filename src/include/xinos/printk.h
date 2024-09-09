#ifndef PRINTK_H
#define PRINTK_H

// printk是在内核中使用的, 而printf是在用户程序使用的
int printk(const char *fmt, ...);

#endif
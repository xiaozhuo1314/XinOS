#ifndef DEBUG_H
#define DEBUG_H

void debugk(char *file ,int line, const char *fmt, ...);

// bochs的魔数断点
#define BMB asm volatile("xchgw %bx, %bx")

// debug语句
#define DEBUGK(fmt, args...) debugk(__BASE_FILE__, __LINE__, fmt, ##args)

#endif
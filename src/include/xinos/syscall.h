#ifndef SYSCALL_H
#define SYSCALL_H

#include "types.h"

typedef enum syscall_t {
    SYS_NR_TEST,                  // 测试
    SYS_NR_YIELD,                 // 调度
}syscall_t;

// SYS_NR_TEST对应的函数调用
u32 test();
// 调度, 通过这个函数生成int0x80中断进行调度
void yield();

#endif
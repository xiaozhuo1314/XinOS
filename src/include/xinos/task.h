#ifndef TASK_H
#define TASK_H

#include "types.h"

typedef u32 target_t();  // 定义了函数, 是u32类型的, 用于cpu去执行

/**
 * 定义任务结构体
 * 1. stack指针, 目前指向的是, 任务所在内存页的栈顶位置
 *    也就是stack变量存储的是一个地址, 这个地址是一个u32数值的地址, 通过*stack可以解引用拿到这个u32数值
 *    而这个u32数值也是一个内存地址
 * 
 */
typedef struct task_t {
    u32 *stack; // 内核栈
} task_t;

/**
 * 进入任务的时候需要保存一些寄存器的值
 */
typedef struct task_frame_t {
    u32 edi;
    u32 esi;
    u32 ebx;
    u32 ebp;
    void (*eip)(void);         // ip寄存器的值, 由于这里我们规定下一条指令是某个函数的入口地址, 因此这里使用函数指针
} task_frame_t;

void task_init();
#endif
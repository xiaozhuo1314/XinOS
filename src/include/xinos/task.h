#ifndef TASK_H
#define TASK_H

#include "types.h"
#include "bitmap.h"

// 定义任务的user id
#define KERNEL_USER 0
#define NORMAL_USER 1
// 任务名称最长名字
#define TASK_NAME_LEN 16

typedef u32 target_t();  // 定义了函数, 是u32类型的, 用于cpu去执行

/**
 * 任务的状态, 进程的状态
 */
typedef enum task_state_t {
    TASK_INIT,     // 初始化
    TASK_RUNNING,  // 执行
    TASK_READY,    // 就绪
    TASK_BLOCKED,  // 阻塞
    TASK_SLEEPING, // 睡眠
    TASK_WAITING,  // 等待
    TASK_DIED,     // 死亡
} task_state_t;

/**
 * 定义任务结构体, 可以认为是PCB的原型
 * 1. stack指针, 目前指向的是, 任务所在内存页的frame的地址
 *    也就是stack变量存储的是一个地址, 这个地址是一个u32数值的地址, 通过*stack可以解引用拿到这个u32数值
 *    而这个u32数值也是一个内存地址, 这个地址是frame的地址
 * 2. 
 * 
 */
typedef struct task_t {
    u32 *stack;                           // 内核栈
    task_state_t state;                   // 任务状态
    u32 priority;                         // 任务优先级
    u32 ticks;                            // 所剩的时间片
    u32 jiffies;                          // 上次执行时的全局时间片计数器jiffies的值
    char name[TASK_NAME_LEN];             // 任务名称
    u32 uid;                              // 用户id, 是内核还是用户
    u32 pde;                              // 任务自己的页目录的虚拟地址
    struct bitmap_t *vmap;                // 任务虚拟内存的位图
    u32 magic;                            // 内核魔数, 由于栈在增长的时候, 由于过度增长, 可能会污染到pcb块, 所以需要一个魔数来当栈增长到这里的时候停止
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

// 任务初始化
void task_init();
// 返回当前的任务
task_t *running_task();
// 调度
void schedule();
// 任务调度
void task_yield();

#endif
/**
 * 系统调用是用户进程与内核沟通的方式。可以将 CPU 从用户态转向内核态
 * 用中断门来实现系统调用，与 linux 兼容，使用 `0x80` 号中断函数
 * - eax 存储系统调用号
 * - ebx 存储第一个参数
 * - ecx 存储第二个参数
 * - edx 存储第三个参数
 * - esi 存储第四个参数
 * - edi 存储第五个参数
 * - ebp 存储第六个参数
 * - eax 存储返回值
 */

#include "xinos/assert.h"
#include "xinos/interrupt.h"
#include "xinos/debug.h"
#include "xinos/syscall.h"
#include "xinos/task.h"

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

// 定义系统调用的个数
#define SYSCALL_SIZE 64

// 创建系统调用表, 本质上系统调用也是一种中断0x80, 所以可以使用handler_t来表示系统调用的函数
handler_t syscall_table[SYSCALL_SIZE];

/**
 * 检查调用号
 * num: 调用号
 */
void syscall_check(u32 num) {
    assert(num < SYSCALL_SIZE);
}

/**
 * 默认的系统调用函数
 */
static void sys_default() {
    panic("syscall not implemented!!!");
}

// 测试的任务
task_t *task = NULL;

// 测试, 在运行过程时, 某一时刻定时器触发时, 一个任务处于block状态(下面的task_block阻塞的), 另一个处于运行态
// 由于此时定时器触发, 要去调度下一个ready状态的且非当前任务的任务, 但是此时一个任务阻塞, 另一个是当前任务
// 找不到下一个任务, 此时就会assert(next != NULL);
static u32 sys_test() {
    if(!task) {
        task = running_task();
        task_block(task, NULL, TASK_BLOCKED);
    } else {
        task_unblock(task);
        task = NULL;
    }
    return 255;
}

/**
 * 初始化系统调用, 这样就可以通过数组和调用号找到对应的调用函数
 */
void syscall_init() {
    for(size_t i = 0; i < SYSCALL_SIZE; ++i) {
        syscall_table[i] = sys_default;
    }
    syscall_table[SYS_NR_TEST] = sys_test;
    syscall_table[SYS_NR_SLEEP] = task_sleep;
    syscall_table[SYS_NR_YIELD] = task_yield;
}
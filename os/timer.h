#ifndef __TIMER_H__
#define __TIMER_H__

#include "type.h"

/* 定时器超时函数类型 */
typedef void (*timer_func)(void *args);

/* 软件定时器结构体 */
struct timer
{
    timer_func func;
    void *args;
    struct timer *next;
#ifdef RV32
    uint32_t timeout; // 以tick计数
#else
    uint64_t timeout; // 以tick计数
#endif
    /* 
     * 下面是为了task_delay加上的
     * 当任务处于sleep状态,说明是主动调用task_delay,所以不去执行func
     * 当任务处于其他状态,需要去执行func
    */
    struct taskInfo *task;
};

#endif
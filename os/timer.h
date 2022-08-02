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
    uint32_t timeout; // 以tick计数
};

#endif
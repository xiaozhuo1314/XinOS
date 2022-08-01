#ifndef __LOCK_H__
#define __LOCK_H__

#include "type.h"

/* 锁的结构体 */
typedef struct lock 
{
    volatile int locked;
} lock_t;

extern int atomic_swap(lock_t *lock);

#endif
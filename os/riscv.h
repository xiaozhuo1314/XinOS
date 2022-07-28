#ifndef __RISCV_H__
#define __RISCV_H__

#include "type.h"

static inline void w_mtvec(reg_t val)
{
    asm volatile ("csrw mtvec, %0" : : "r"(val));
}

#endif
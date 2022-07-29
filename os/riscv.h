#ifndef __RISCV_H__
#define __RISCV_H__

#include "type.h"

/* mie寄存器中断位 */
#define MIE_MEIE (1 << 11) //machine模式外部中断
#define MIE_MTIE (1 << 7) //machine模式定时器中断
#define MIE_MSIE (1 << 3) //machine模式软中断

/* mstatus寄存器全局中断控制位 */
#define MSTATUS_MIE (1 << 3) //machine模式全局中断开关
#define MSTATUS_SIE (1 << 1) //supervisor模式全局中断开关
#define MSTATUS_UIE (1 << 0) //user模式全局中断开关

/* 将trap处理程序地址写入mtvec寄存器 */
static inline void w_mtvec(reg_t val)
{
    asm volatile ("csrw mtvec, %0" : : "r"(val));
}

/* 读取tp寄存器的值,由于在start.S中已经将hart id存到了tp寄存器,那么此函数就是读取当前hart id */
static inline reg_t r_tp()
{
    reg_t val;
    asm volatile ("mv %0, tp" : "=r"(val));
    return val;
}

/* 读取mie寄存器的值 */
static inline reg_t r_mie()
{
    reg_t val;
    asm volatile ("csrr %0, mie" : "=r"(val));
    return val;
}

/* 设置mie寄存器的值 */
static inline void w_mie(reg_t val)
{
    asm volatile ("csrw mie, %0" : : "r"(val));
}

/* 读取mstatus寄存器的值 */
static inline reg_t r_mstatus()
{
    reg_t val;
    asm volatile ("csrr %0, mstatus" : "=r"(val));
    return val;
}

/* 设置mstatus寄存器的值 */
static inline void w_mstatus(reg_t val)
{
    asm volatile ("csrw mstatus, %0" : : "r"(val));
}

#endif
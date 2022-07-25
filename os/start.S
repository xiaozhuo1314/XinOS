#include "platform.h"

    # 设置每个hart的栈大小为1024
    .equ STACK_SIZE, 1024

    .global _start

_start:
    # 获取hart id
    csrr t0, mhartid # 每个hard都有自己的寄存器,而且每个hart都会执行本文件程序
    mv tp, t0 # 将mhartid保存到tp, tp为用于本地线程数据的线程指针寄存器
    bnez t0, park  # 只有hart为0的能继续执行,否则就空转,因为本系统目前只有一个核

    # 设置bss段的所有字节为0,当然这部分也可以用c语言来写
    # 引用数字标签时需要加后缀,b表示引用的标签在该语句的前面，f则表示引用的标签在该语句之后。
    la a0, _bss_start
    la a1, _bss_end
    bgeu a0, a1, 2f

1:
    sw zero, 0(a0)
    addi a0, a0, 4
    bltu a0, a1, 1b

2:
    # 因为要start_kernel要调用初始化函数,必须要有栈,会自动用sp指向的位置作为栈,去执行start_kernel中的一些初始化函数
    # 如果后面多个hart,那么每个hart都是用STACK_SIZE字节作为自己的栈
    # 所以每个hart的栈的起始位置为stacks + (t0 + 1) * STACK_SIZE
    # 加1是因为栈从高地址往低地址生长,所以初始位置在每个hart栈的高地址处,需要再加一个STACK_SIZE
    # 也就是stacks + STACK_SIZE + (t0 << 10)
    slli t0, t0, 10 # t0 = t0 << 10
    la sp, stacks + STACK_SIZE # sp = stacks + STACK_SIZE
    add sp, sp, t0 # 设置每个hart的sp到达自己的起始位置, stacks + STACK_SIZE + (t0 << 10)
    j start_kernel

park:
    wfi # 休眠 Wait For Interrupt
    j park

stacks:
    .skip STACK_SIZE * MAXNUM_CPU

    .end

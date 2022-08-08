#include "platform.h"

    # 设置每个hart的栈大小为1024或2048
#ifdef RV32
    .equ STACK_SIZE, 1024
#else
	.equ STACK_SIZE, 4096
#endif

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
#ifdef RV32
    slli t0, t0, 10 # t0 = t0 << 10
#else
	slli t0, t0, 11
#endif
    la sp, stacks + STACK_SIZE # sp = stacks + STACK_SIZE
    add sp, sp, t0 # 设置每个hart的sp到达自己的起始位置, stacks + STACK_SIZE + (t0 << 10)

#ifdef CONFIG_SYSCALL
    # 对于qemu 6.0及之后的版本,在系统调用的时候如果不设置pmp的话就会产生异常
    # 所以设置pmp entry的话会允许所有32位物理内存地址是R/W/X
    # 但是要注意的是,这仅仅只是一个临时解决办法,这是qemu的bug,不是程序设计问题
    # 下面就是设置pmp,未来修复bug后就会删掉该处代码
    li t0, 0xffffffff
    csrw pmpaddr0, t0
    li t0, 0xf
    csrw pmpcfg0, t0
#endif

#ifdef CONFIG_SYSCALL
    # 设置了sys_call的话就是user模式
    li t0, 1 << 7
#else
    # 由于在kernel函数中最开始切换任务时,mstatus的值为0,所以此时为user模式,这样直接调用schedule函数会由于操作
    # mscratch等只有在machine下才能操作的寄存器而出现2号异常,所以需要在刚开始执行的时候就设置mpp为machine模式
    # 这样在back_os的switch_to函数的mret后,就能一直是machine模式,同时也需要把全局中断打开
    # mstatue寄存器11和12号mpp位表示的是前一个特权模式,在mret的时候会恢复,mstatus寄存器的7位表示的是全局中断
    li t0, 3 << 11 | 1 << 7
#endif
    csrr a0, mstatus
    or t0, t0, a1
    csrw mstatus, t0
    
    j start_kernel

park:
    wfi # 休眠 Wait For Interrupt
    j park

stacks:
    .skip STACK_SIZE * MAXNUM_CPU

    .end

#include "xinos/interrupt.h"
#include "xinos/global.h"
#include "xinos/printk.h"

// 异常一共有32个
#define ENTRY_SIZE 0x20

// 中断描述符表
gate_t idt[IDT_SIZE];

// 中断描述符表的指针, 存放到寄存器的, 跟全局描述符的结构一样
pointer_t idt_ptr;

// 处理函数表, 也就是最终处理的逻辑会在这里
handler_t handler_table[IDT_SIZE];

// 处理函数入口表
extern handler_t handler_entry_table[ENTRY_SIZE];

// 异常需要展示的信息
static char *messages[] = {
    "#DE Divide Error\0",
    "#DB RESERVED\0",
    "--  NMI Interrupt\0",
    "#BP Breakpoint\0",
    "#OF Overflow\0",
    "#BR BOUND Range Exceeded\0",
    "#UD Invalid Opcode (Undefined Opcode)\0",
    "#NM Device Not Available (No Math Coprocessor)\0",
    "#DF Double Fault\0",
    "    Coprocessor Segment Overrun (reserved)\0",
    "#TS Invalid TSS\0",
    "#NP Segment Not Present\0",
    "#SS Stack-Segment Fault\0",
    "#GP General Protection\0",
    "#PF Page Fault\0",
    "--  (Intel reserved. Do not use.)\0",
    "#MF x87 FPU Floating-Point Error (Math Fault)\0",
    "#AC Alignment Check\0",
    "#MC Machine Check\0",
    "#XF SIMD Floating-Point Exception\0",
    "#VE Virtualization Exception\0",
    "#CP Control Protection Exception\0",
};

/**
 * 异常处理函数
 * vector为异常向量, 也就是handler.asm中的%1
 */
void exception_handler(int vector) {
    char *msg = NULL;

    if(vector < 22) msg = messages[vector];
    else msg = messages[15];  // 默认的

    // 输出异常信息
    printk("Exception : [0x%02X] %s \n", vector, msg);
    // 阻塞
    while(true) ;
    // 阻塞如果完成就是出错了
    asm volatile("ud2");
}

// 中断处理初始化
void interrupt_init() {
    /**
     * 当我们去调用int 0x80的时候, cpu回去找中断描述符中下标为16x8+0=128的那个描述符
     * 然后通过这个描述符找到对应的中断处理函数
     * 证据就是, 如果下面的循环中我们把i != 128的描述符不设置, 只设置i=128的描述符
     * 当我们调用int 0x80的时候, 就会去调用这个中断处理函数
     * 而调用int 0x10就会报错找不到中断处理函数
     */
    for(int i = 0; i < ENTRY_SIZE; ++i) {
        gate_t *gate = &idt[i];
        handler_t handler = handler_entry_table[i];
        gate->offset0 = ((u32)handler) & 0xffff;
        gate->selector = (1 << 3);                                                         // 代码段
        gate->reserved = 0;                                                                   // 保留
        gate->type = 0b1110;                                                                // 中断门
        gate->segment = 0;                                                                   // 系统段
        gate->DPL = 0;                                                                        // 至少要是内核态
        gate->present = 1;                                                                    // 有效
        gate->offset1 = (((u32)handler) >> 16) & 0xffff;
    }
    // 设置handler_table
    for(int i = 0; i < ENTRY_SIZE; ++i) {
        handler_table[i] = exception_handler;
    }
    // 设置idt指针
    idt_ptr.base = (u32)&idt;
    idt_ptr.limit = sizeof(idt) - 1;
    // 加载idt_ptr
    asm volatile("lidt idt_ptr\n");
}
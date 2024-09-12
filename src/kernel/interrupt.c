#include "xinos/interrupt.h"
#include "xinos/global.h"

// 中断描述符表
gate_t idt[IDT_SIZE];

// 中断描述符表的指针, 存放到寄存器的, 跟全局描述符的结构一样
pointer_t idt_ptr;

// 中断处理函数
extern void interrupt_handler();

// 中断处理初始化
void interrupt_init() {
    /**
     * 当我们去调用int 0x80的时候, cpu回去找中断描述符中下标为16x8+0=128的那个描述符
     * 然后通过这个描述符找到对应的中断处理函数
     * 证据就是, 如果下面的循环中我们把i != 128的描述符不设置, 只设置i=128的描述符
     * 当我们调用int 0x80的时候, 就会去调用这个中断处理函数
     * 而调用int 0x10就会报错找不到中断处理函数
     */
    for(int i =0; i < IDT_SIZE; ++i) {
        gate_t *gate = &idt[i];
        gate->offset0 = ((u32)interrupt_handler) & 0xffff;
        gate->selector = (1 << 3);                                                         // 代码段
        gate->reserved = 0;                                                                   // 保留
        gate->type = 0b1110;                                                                // 中断门
        gate->segment = 0;                                                                   // 系统段
        gate->DPL = 0;                                                                        // 至少要是内核态
        gate->present = 1;                                                                    // 有效
        gate->offset1 = (((u32)interrupt_handler) >> 16) & 0xffff;
    }
    // 设置idt指针
    idt_ptr.base = (u32)&idt;
    idt_ptr.limit = sizeof(idt) - 1;
    // 加载idt_ptr
    asm volatile("lidt idt_ptr\n");
}
#include "xinos/interrupt.h"
#include "xinos/global.h"
#include "xinos/printk.h"
#include "xinos/debug.h"
#include "xinos/io.h"
#include "xinos/stdlib.h"

// 异常一共有32个, 再加上了16个中断
#define ENTRY_SIZE 0x30
#define EXCEPTION_ENTRY_SIZE 0x20

// 设置日志
#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

// 级联了两个中断控制器, 一个主片, 一个从片
#define PIC_M_CTRL 0x20 // 主片的控制端口
#define PIC_M_DATA 0x21 // 主片的数据端口
#define PIC_S_CTRL 0xa0 // 从片的控制端口
#define PIC_S_DATA 0xa1 // 从片的数据端口
#define PIC_EOI 0x20    // 通知中断控制器中断结束

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
 * 通知中断控制器中断处理结束了
 * vector: 外部中断向量
*/
void send_eoi(int vector) {
    if(vector >= 0x20 && vector < 0x28) {  // 说明只有一个主片
        outb(PIC_M_CTRL, PIC_EOI);
    } else if(vector >= 0x28 && vector < 0x30) {  // 既有主片也有从片
        outb(PIC_M_CTRL, PIC_EOI);
        outb(PIC_S_CTRL, PIC_EOI);
    }
}

u32 counter = 0;
/**
 * 默认中断处理函数
*/
void default_handler(int vector) {
    send_eoi(vector);
    LOGK("[%d] default interrupt called %d...\n", vector, counter++);
}

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
    hang();
    // 阻塞如果完成就是出错了
    asm volatile("ud2");
}

/**
 * 初始化中断控制器
*/
void pic_init() {
    outb(PIC_M_CTRL, 0b00010001); // ICW1: 边沿触发, 级联 8259, 需要ICW4.
    outb(PIC_M_DATA, 0x20);       // ICW2: 主片起始中断向量号 0x20, 也就是send_eoi中需要根据这个来判断有几个中断控制器
    outb(PIC_M_DATA, 0b00000100); // ICW3: IR2接从片.
    outb(PIC_M_DATA, 0b00000001); // ICW4: 8086模式, 正常EOI

    outb(PIC_S_CTRL, 0b00010001); // ICW1: 边沿触发, 级联 8259, 需要ICW4.
    outb(PIC_S_DATA, 0x28);       // ICW2: 从片起始中断向量号 0x28, 也就是send_eoi中需要根据这个来判断有几个中断控制器
    outb(PIC_S_DATA, 2);          // ICW3: 设置从片连接到主片的 IR2 引脚
    outb(PIC_S_DATA, 0b00000001); // ICW4: 8086模式, 正常EOI

    outb(PIC_M_DATA, 0b11111110); // 关闭主片所有中断
    outb(PIC_S_DATA, 0b11111111); // 关闭从片所有中断
}

/**
 * 初始化中断描述符和中断处理函数表
*/
void idt_init() {
    /**
     * 当我们去调用int 0x80的时候, cpu回去找中断描述符中下标为16x8+0=128的那个描述符
     * 然后通过这个描述符找到对应的中断处理函数
     * 证据就是, 如果下面的循环中我们把i != 128的描述符不设置, 只设置i=128的描述符
     * 当我们调用int 0x80的时候, 就会去调用这个中断处理函数
     * 而调用int 0x10就会报错找不到中断处理函数
     */
    for(size_t i = 0; i < ENTRY_SIZE; ++i) {
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
    for(size_t i = 0; i < ENTRY_SIZE; ++i) {
        if(i < EXCEPTION_ENTRY_SIZE)
            handler_table[i] = exception_handler;
        else
            handler_table[i] = default_handler;
    }
    // 设置idt指针
    idt_ptr.base = (u32)&idt;
    idt_ptr.limit = sizeof(idt) - 1;
    // 加载idt_ptr
    asm volatile("lidt idt_ptr\n");
}

// 中断处理初始化
void interrupt_init() {
    pic_init();
    idt_init();
}
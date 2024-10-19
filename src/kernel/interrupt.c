#include "xinos/interrupt.h"
#include "xinos/global.h"
#include "xinos/printk.h"
#include "xinos/debug.h"
#include "xinos/io.h"
#include "xinos/stdlib.h"
#include "xinos/assert.h"

// 异常一共有32个, 再加上了16个中断
#define ENTRY_SIZE 0x30
#define EXCEPTION_ENTRY_SIZE 0x20

// 设置日志
#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

/**
 * 级联了两个中断控制器, 一个主片, 一个从片
 */
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

extern void schedule();

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

/**
 * 设置特殊的中断处理器
 */
void set_interrupt_handler(u32 irq, handler_t handler) {
    assert(irq >= 0 && irq < 16);
    // 设置特殊的处理函数
    handler_table[IRQ_MASTER_NR + irq] = handler;
}

/**
 * 设置是否开启特殊的中断处理
 */
void set_interrupt_mask(u32 irq, bool enable) {
    assert(irq >= 0 && irq < 16);
    u16 port;
    if (irq < 8) {  // 主片
        port = PIC_M_DATA;
    } else {  // 从片
        port = PIC_S_DATA;
        irq -= 8; // 从片的向量号也是从0开始的
    }
    if (enable) {  // 如果允许开启的话, 就将对应位置处的位置为0
        outb(port, inb(port) & ~(1 << irq));
    } else {  // 如果不允许开启的话, 就将对应位置处的位置为1
        outb(port, inb(port) | (1 << irq));
    }
}

/**
 * 默认中断处理函数
*/
void default_handler(int vector) {
    /**
     * 这里要先通知中断处理器中断处理完成了
     * 这样中断处理器就可以重新生成中断了
     * 但是生成了不代表就能被cpu响应
     * 因为此时eflags中的if位还是位0, 此时只能响应不可屏蔽的中断, 例如定时器等硬件中断
     * 很多软中断就被屏蔽了
     * 
     * 如果这里先schedule的话, 因为task中是死循环, 就无法执行到send_eoi
     * 即使task中设置了if位为1, 由于中断处理器中还是屏蔽状态, 就无法响应了
     * 所以要先send_eoi, 然后schedule
     * 
     * 因此刚上来程序执行taska, 然后来了中断执行taskb
     * 但是由于是先执行schedule, 并没有通知中断控制器要响应中断
     * 因此此时就无法再通过中断调用到taska了
    */
    send_eoi(vector);
    DEBUGK("[0x%x] default interrupt called...\n", vector);
}

/**
 * 异常处理函数
 * vector为异常向量, 也就是handler.asm中的%1
 * 后面都是我们pusha和push gs等压入的寄存器信息
 */
void exception_handler(
    int vector,
    u32 edi, u32 esi, u32 ebp, u32 esp,
    u32 ebx, u32 edx, u32 ecx, u32 eax,
    u32 gs, u32 fs, u32 es, u32 ds,
    u32 vector0, u32 error, u32 eip, u32 cs, u32 eflags) 
{
    char *msg = NULL;

    if(vector < 22) msg = messages[vector];
    else msg = messages[15];  // 默认的

    // 输出异常信息
    printk("\nEXCEPTION : %s \n", messages[vector]);
    printk("   VECTOR : 0x%02X\n", vector);
    printk("    ERROR : 0x%08X\n", error);
    printk("   EFLAGS : 0x%08X\n", eflags);
    printk("       CS : 0x%02X\n", cs);
    printk("      EIP : 0x%08X\n", eip);
    printk("      ESP : 0x%08X\n", esp);
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

    outb(PIC_M_DATA, 0b11111111); // 关闭主片所有中断, 用set_interrupt_mask来开启
    outb(PIC_S_DATA, 0b11111111); // 关闭从片所有中断, 用set_interrupt_mask来开启
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

// 清除 IF 位，返回设置之前的值
bool interrupt_disable() {
    asm volatile(
        "pushfl\n"         // 将当前 eflags 压入栈中
        "cli\n"            // 关闭中断, 也就是IF位置为0
        "popl %eax\n"      // 将刚才压入的 eflags 弹出到 eax
        "shrl $9, %eax\n"  // 将 eax 右移 9 位, 为了下面得到 IF 位
        "andl $1, %eax\n"  // 只留下 IF 位
    );
}

// 获得 IF 位
bool get_interrupt_state() {
    asm volatile(
        "pushfl\n"         // 将当前 eflags 压入栈中
        "popl %eax\n"      // 将刚才压入的 eflags 弹出到 eax
        "shrl $9, %eax\n"  // 将 eax 右移 9 位, 为了下面得到 IF 位
        "andl $1, %eax\n"  // 只留下 IF 位
    );
}

// 设置 IF 位
void set_interrupt_state(bool state) {
    if(state) asm volatile("sti\n");
    else asm volatile("cli\n");
}
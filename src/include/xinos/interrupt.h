#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "types.h"

// 设置中断描述符表元素的个数
#define IDT_SIZE 256

/**
 * 每一个详细中断
 */
#define IRQ_CLOCK 0      // 时钟
#define IRQ_KEYBOARD 1   // 键盘
#define IRQ_CASCADE 2    // 8259 从片控制器
#define IRQ_SERIAL_2 3   // 串口 2
#define IRQ_SERIAL_1 4   // 串口 1
#define IRQ_PARALLEL_2 5 // 并口 2
#define IRQ_FLOPPY 6     // 软盘控制器
#define IRQ_PARALLEL_1 7 // 并口 1
#define IRQ_RTC 8        // 实时时钟
#define IRQ_REDIRECT 9   // 重定向 IRQ2
#define IRQ_MOUSE 12     // 鼠标
#define IRQ_MATH 13      // 协处理器 x87
#define IRQ_HARDDISK 14  // ATA 硬盘第一通道
#define IRQ_HARDDISK2 15 // ATA 硬盘第二通道

#define IRQ_MASTER_NR 0x20 // 主片起始中断向量号
#define IRQ_SLAVE_NR 0x28  // 从片起始中断向量号

/**
 * 门的描述符内容
 */
typedef struct gate_t
{
    u16 offset0;                   // 当前描述符所指向的中断处理函数, 在段内偏移地址的0-15位
    u16 selector;                  // 选择子, 表示中断处理函数是在哪个段, 一般为代码段
    u8 reserved;                  // 保留
    u8 type : 4;                    // 任务门(0101)、中断门(1110)、陷阱门(1111), 一般只用中断门
    u8 segment : 1;              // segment=0表示系统段, 表示代码段或数据段
    u8 DPL : 2;                   // 使用int指令访问的最低权限
    u8 present : 1;               // 是否有效
    u16 offset1;                  // 当前描述符所指向的中断处理函数, 在段内偏移地址的16-31位
} _packed gate_t;

// 中断处理函数类型
typedef void *handler_t;

// 通知中断控制器中断处理结束了
void send_eoi(int vector);

/**
 * 设置中断处理
 * set_interrupt_handler: 设置中断处理函数
 * set_interrupt_mask: 设置中断处理是否屏蔽
 */
void set_interrupt_handler(u32 irq, handler_t handler);
void set_interrupt_mask(u32 irq, bool enable);

#endif
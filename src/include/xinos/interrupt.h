#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "types.h"

// 设置中断描述符表元素的个数
#define IDT_SIZE 256

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

// 中断处理函数
typedef void *handler_t;

void interrupt_init();

#endif
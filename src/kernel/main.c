#include "xinos/types.h"
#include "xinos/io.h"

#define CRT_ADDR_REG 0x3D4
#define CRT_DATA_REG 0x3D5
#define CRT_CURSOR_H 0xE
#define CRT_CURSOR_L 0xF

void kernel_init() {
    // 告诉CRT它的地址寄存器设置为0xE, 也就是系统现在想要获取光标的高8位
    outb(CRT_ADDR_REG, CRT_CURSOR_H);
    // 获取光标高8位
    u16 pos = inb(CRT_DATA_REG) << 8;
    // 告诉CRT它的地址寄存器设置为0xF, 也就是系统现在想要获取光标的低8位
    outb(CRT_ADDR_REG, CRT_CURSOR_L);
    pos |= inb(CRT_DATA_REG);
    // 将光标位置修改
    outb(CRT_ADDR_REG, CRT_CURSOR_H);
    outb(CRT_DATA_REG, 0);
    outb(CRT_ADDR_REG, CRT_CURSOR_L);
    outb(CRT_DATA_REG, 150);
    return;
}
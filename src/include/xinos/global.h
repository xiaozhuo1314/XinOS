#ifndef GLOBAL_H
#define GLOBAL_H

#include "types.h"

// 设置全局描述符表为128个, 在汇编中我们设置的是8092个, 也就是2的13次方
#define GDT_SIZE 128

/**
 * 全局描述符表中的段描述符的结构体, 共占8个字节
 * 也就是这8个字节就是表中一个元素的结构
 */
typedef struct descriptor_t {
    unsigned short limit_low;          // 段界限 0-15位
    unsigned int base_low : 24;       // 基地址的低24位, 0-23位
    unsigned char type : 4;              // 段类型
    unsigned char segment : 1;        // 1表示代码段或数据段, 0表示系统段
    unsigned char DPL : 2;             // 描述符特权等级 0-3级
    unsigned char present : 1;         // 存在位, 1表示在内存中, 0表示在磁盘上
    unsigned char limit_high : 4;    // 段界限 16-19位
    unsigned char available : 1;      // 操作系统自定义作用
    unsigned char long_mode : 1;   // 64位扩展标志
    unsigned char big : 1;               // 32位还是16位
    unsigned char granularity : 1;   // 粒度是4K还是1B
    unsigned char base_high;         // 基地址的高8位
} _packed descriptor_t;


/**
 * 段选择子
 */
typedef struct selector_t {
    u8 RPL : 2;                          // 请求特权级, 即用什么权限来请求
    u8 TI : 1;                             // 0表示全局描述符表, 1表示局部描述符表
    u16 index : 13;                    // 索引值, 该值*8(描述符占8字节)+(全局或者局部)描述符表的基地址, 就是要加载的段描述符
} _packed selector_t;

/**
 * 全局描述符表指针, 表示全局描述符的界限和位置
 * 通过这个指针和段选择子, 就可以拿到某个descriptor_t段描述符
 * 
 * 这里有个问题, 就是结构体必须要用_packed压缩
 * 因为如果不这样的话, 会默认4字节对齐, 那么pointer_t就不是
 * 总共6个字节, 而是8个字节了, 因为limit的高位2字节就成了对齐所填充的
 * 事实上我们应该让16位的limit和32位的base紧凑起来
 */
typedef struct pointer_t {
    u16 limit;                         // 描述符的界限
    u32 base;                          // 描述符的基地址
} _packed pointer_t;

void gdt_init();
#endif
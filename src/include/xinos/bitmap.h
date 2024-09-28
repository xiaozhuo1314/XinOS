#ifndef BITMAP_H
#define BITMAP_H

#include "types.h"

/**
 * 位图结构体
 */
typedef struct bitmap_t {
    u8 *bits;     // 位图缓冲区的位置
    u32 length;   // 位图的长度, 字节为单位
    u32 offset;   // 位图开始的偏移, 比如位图代表内存页的使用情况, 那么这个offset就是从哪个页开始(页索引)表示的
} bitmap_t;

// 初始化位图
void bitmap_init(bitmap_t *map, char *bits, u32 length, u32 offset);
// 构造位图
void bitmap_make(bitmap_t *map, char *bits, u32 length, u32 offset);
// 测试位图的某一位是否为 1, index是比特位
bool bitmap_test(bitmap_t *map, u32 index);
// 设置位图某位的值, index是比特位
void bitmap_set(bitmap_t *map, u32 index, bool value);
// 从位图中得到连续的 count个为0的位
int bitmap_scan(bitmap_t *map, u32 count);

#endif
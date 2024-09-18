#ifndef STDLIB_H
#define STDLIB_H

#include "types.h"

// 简易的延时函数
void delay(u32 time_count);
// 系统宕机函数
void hang();

// bcd码转换成整数
u8 bcd_to_bin(u8 value);
// 整数转换成bcd码
u8 bin_to_bcd(u8 value);

#endif
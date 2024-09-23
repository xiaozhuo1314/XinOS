#ifndef RTC_H
#define RTC_H

/**
 * 实时时钟
 * 利用CMOS来实现
 * 
 * clock.c中的时钟是每隔多少时间去触发
 * 而这里的rtc可以设置一个过期时间, 在这个时间到达的时候触发, 就是闹钟功能
 */

#include "types.h"

// 读取地址中的数据
u8 cmos_read(u8 addr);
// 写入数据
void cmos_write(u8 addr, u8 value);
// 设置闹钟, secs是间隔
void set_alarm(u32 secs);

#endif
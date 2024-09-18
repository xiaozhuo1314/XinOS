#ifndef TYPES_H
#define TYPES_H

#define EOF -1                                                // 结尾
#define NULL ((void*)0)                                // 空指针

#define EOS '\0'                                              // 字符串结尾

#define bool _Bool                                          // 布尔类型
#define true 1                                                  // true为1
#define false 0                                                 // false为1

#define _packed __attribute__((packed))         // 用于定义特殊的结构体

// 用于省略函数的栈帧, 也就是调用函数时, 没有了push ebp     mov ebp, esp的操作
#define _ofp __attribute__((optimize("omit-frame-pointer")))

typedef unsigned int size_t;
typedef char int8;
typedef short int16;
typedef int int32;
typedef long long int64;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

// 时间戳
typedef u32 time_t;

#endif
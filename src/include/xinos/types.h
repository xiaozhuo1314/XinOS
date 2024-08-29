#ifndef TYPES_H
#define TYPES_H

#define EOF -1                                                // 结尾
#define NULL 0                                              // 空指针

#define bool _Bool                                          // 布尔类型
#define true 1                                                  // true为1
#define false 0                                                 // false为1

#define _packed __attribute__((packed))         // 用于定义特殊的结构体

typedef unsigned int size_t;
typedef char int8;
typedef short int16;
typedef int int32;
typedef long long int64;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

#endif
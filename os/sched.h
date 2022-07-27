#ifndef __SCHED_H__
#define __SCHED_H__

#include "type.h"

/* 上下文切换的结构体,用于保存各个寄存器 */
struct context {
    /* ignore x0 */
	reg_t ra;
	reg_t sp;
	reg_t gp;
	reg_t tp;
	reg_t t0;
	reg_t t1;
	reg_t t2;
	reg_t s0;
	reg_t s1;
	reg_t a0;
	reg_t a1;
	reg_t a2;
	reg_t a3;
	reg_t a4;
	reg_t a5;
	reg_t a6;
	reg_t a7;
	reg_t s2;
	reg_t s3;
	reg_t s4;
	reg_t s5;
	reg_t s6;
	reg_t s7;
	reg_t s8;
	reg_t s9;
	reg_t s10;
	reg_t s11;
	reg_t t3;
	reg_t t4;
	reg_t t5;
	reg_t t6;
};

/* 任务的结构体 */
struct taskInfo {
    int task_id; // 任务id
    int priority; // 任务优先级
    struct taskInfo *next; // 后一个任务的指针
    struct context ctx; // 任务的上下文结构体的指针
};

/* 任务的类型 */
typedef void (*task_func)(void *param);

/* 定义任务最大个数 */
#define MAX_TASK_NUM 10

/* entry.S中定义的函数 */
extern void switch_to(struct context *next);

/* 任务栈大小 */
#define STACK_SIZE 1024
/* 内核栈 */
uint8_t os_stack[STACK_SIZE];
/* 内核taskInfo */
struct taskInfo os_task;
/* 任务栈,由于任务个数最多10个,所以就申请10个栈 */
uint8_t task_stack[MAX_TASK_NUM][STACK_SIZE];
/* 任务要保存的寄存器的结构体变量 */
// struct context task_ctx[MAX_TASK_NUM];

#endif
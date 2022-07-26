#ifndef __OS_H__
#define __OS_H__

#include "type.h"
#include "platform.h"
#include <stddef.h>
#include <stdarg.h>

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
/* 任务的类型 */
typedef void (*task_func)(void);

/* uart.h */
extern void uart_init(void);
extern int uart_putc(char c);
extern void uart_puts(char *p);
extern void uart_gets(void);

/* printf.c */
extern int printf(const char *s, ...);
extern void panic(char *s);

/* page.c */
extern void page_init(void);
extern void *page_alloc(int npages);
extern void page_free(void *p);
extern void *malloc(size_t size);
extern void free(void *p);
// extern void page_test(void);
// extern void malloc_test(void);

/* sched.c */
extern void sched_init(void);
extern void schedule(void);
extern void task_delay(volatile int cnt);
extern void task_yield(void);
extern int task_create(task_func task);

/* user.c */
extern void os_main(void);

#endif
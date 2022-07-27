#ifndef __OS_H__
#define __OS_H__

#include "type.h"
#include "platform.h"
#include "sched.h"
#include <stddef.h>
#include <stdarg.h>

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
extern int task_create(task_func task, void *param, int priority);
extern void task_exit(void);

/* user.c */
extern void os_main(void);

#endif
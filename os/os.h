#ifndef __OS_H__
#define __OS_H__

#include "riscv.h"
#include "type.h"
#include "platform.h"
#include "sched.h"
#include "lock.h"
#include "timer.h"
#include <stddef.h>
#include <stdarg.h>

/* uart.h */
extern void uart_init(void);
extern int uart_putc(char c);
extern void uart_puts(char *p);
extern void uart_gets(void);
extern void uart_ier(void);

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
extern void task_delay(uint32_t tick);
extern void task_yield(void);
extern int task_create(task_func task, void *param, int priority, uint32_t timeslice);
extern void task_exit(void);
extern void back_os(void);

/* user.c */
extern void user_init(void);

/* kernel.c */
extern void kernel(void);

/* trap.c */
extern void trap_init(void);
// extern void trap_test(void);

/* plic.c */
extern void plic_init(void);
extern int plic_claim(void);
extern void plic_complete(int irq);

/* timer.c */
extern void timer_load(int interval);
extern void timer_init(void);
extern void timer_handler(void); 
extern void timer_init(void);
extern struct timer *timer_create(timer_func func, void *args, uint32_t timeout);
extern void timer_delete(struct timer *t);

/* lock.h */
extern void lock_acquire(lock_t *lock);
extern void lock_free(lock_t *lock);

#endif
#ifndef __OS_H__
#define __OS_H__

#include "type.h"
#include "platform.h"
#include <stddef.h>
#include <stdarg.h>

/* uart.h */
extern void uart_init(void);
extern int uart_putc(char c);
extern void uart_puts(char *p);
extern void uart_gets(void);



#endif
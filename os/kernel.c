#include "os.h"

void start_kernel(void) {
    uart_init();
    uart_puts("hello world\n");
    uart_gets();
    while (1){}
}
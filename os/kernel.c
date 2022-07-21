#include "os.h"

void start_kernel(void) {
    uart_init();
    printf("Hello XinOS\n");
    // uart_gets();
    page_init();
    // page_test();
    while (1){}
}
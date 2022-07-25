#include "os.h"

void start_kernel(void) {
    uart_init();
    printf("Hello XinOS\n");

    page_init();

    while (1){}
}
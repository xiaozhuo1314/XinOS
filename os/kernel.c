#include "os.h"

void start_kernel(void) {
    uart_init();
    printf("Hello XinOS\n");

    page_init();

    sched_init();

    schedule();
    printf("Never Be Here\n");
    while (1){}
}
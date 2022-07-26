#include "os.h"

void start_kernel(void) {
    // 打印串口初始化
    uart_init();
    printf("Hello XinOS\n");
    // 内存管理初始化
    page_init();
    // 任务调度初始化
    sched_init();
    // 创建任务
    os_main();
    // 任务调度开始
    schedule();
    printf("Never Be Here\n");
    while (1){}
}
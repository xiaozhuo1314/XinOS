#include "os.h"

void start_kernel(void) {
    // 打印串口初始化
    uart_init();
    printf("Hello XinOS\n");
    // 内存管理初始化
    page_init();
    // trap初始化
    trap_init();
    // plic初始化
    plic_init();
    // 硬件定时器初始化
    timer_init();
    // 任务调度初始化
    sched_init();
    // 返回到内核执行
    // 这里如果不执行该语句,直接执行kernel函数的话,当走到内核去调起第一个用户任务时
    // 由于mscratch寄存器此时为0,不会保存当前运行的内核任务寄存器
    // 那么当用户任务要切换内核任务时,保存用户任务,并将内核上下文加载到寄存器
    // 由于前面没有保存内核任务执行的位置,且初始化的时候将内核上下文的ra指向的是kernel的第一的语句
    // 导致内核上下文的ra寄存器此时仍然指向的kernel函数第一条语句
    // 那就会又执行kernel函数,导致后续出问题
    back_os();

    printf("Never Be Here!!!");
}

/* 内核任务函数 */
void kernel()
{
    // 创建用户任务
    user_init();

    // 内核任务开始
    while (1)
    {
        printf("======== OS Starts Next User's Task =====\n");
        // 开启其中一个用户任务,这样hart就会去执行用户任务,而不会去执行下面的内核语句了
        task_yield();
        printf("===== BACK 2 OS =====\n");
        task_delay(10000);
    }
}
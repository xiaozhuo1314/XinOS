#include "os.h"

/* entry.S中定义的函数 */
extern void switch_to(struct context *next);

/* 栈大小 */
#define STACK_SIZE 1024
/* 任务栈 */
uint8_t task_stack[STACK_SIZE];
/* 任务要保存的寄存器的结构体变量 */
struct context task_ctx;

/*  设置mscratch寄存器的值 */
static void w_mscratch(reg_t val)
{
    asm volatile("csrw mscratch, %0" : : "r"(val));
}

/* 延时函数,很低级的实现,后续可能会改 */
void task_delay(volatile int cnt)
{
    cnt *= 50000;
    while(cnt--);
}

/* 用户任务0 */
void user_task0()
{
    printf("Task 0: Created & Started!\n");
    while(1)
    {
        printf("Task 0: Running...\n");
        task_delay(1000);
    }
}

/* 调度初始化 */
void sched_init()
{
    // 首先设置mscratch寄存器的值为0
    w_mscratch(0);

    // 第一个任务执行只需要ra指向下一条指令的地址,sp指向对应的栈即可
    // 必须要有栈是因为任务不可能很简单,以后会需要调用函数之类的,需要栈来实现
    task_ctx.ra = (reg_t)user_task0;
    task_ctx.sp = (reg_t)(&(task_stack[STACK_SIZE - 1]));
}

/* 调度函数 */
void schedule()
{
    struct context *next = &task_ctx;
    switch_to(next);
}
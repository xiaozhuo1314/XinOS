#include "os.h"

/* 定义任务最大个数 */
#define MAX_TASK_NUM 10

/* entry.S中定义的函数 */
extern void switch_to(struct context *next);

/* 任务栈大小 */
#define STACK_SIZE 1024
/* 任务栈,由于任务个数最多10个,所以就申请10个栈 */
uint8_t task_stack[MAX_TASK_NUM][STACK_SIZE];
/* 任务要保存的寄存器的结构体变量 */
struct context task_ctx[MAX_TASK_NUM];

/* _tasks_num表示当前执行的任务数量,包括正在执行和被切换下去的任务 */
static int _tasks_num = 0;
/* _task_idx表示当前正在hart上执行的task的索引,由于本系统只有一个hart能够执行任务,所以_task_idx每次只有一个确定的值 */
static int _task_idx = -1;

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

/* 调度初始化 */
void sched_init()
{
    // 首先设置mscratch寄存器的值为0,任务调度在schedule执行
    w_mscratch(0);
}

/* 调度函数 */
void schedule()
{
    if(_tasks_num <= 0)
    {
        panic("Num of task should be greater than zero!");
        return;
    }
    // 获取要调度的task Id,下一个执行任务都是当前执行任务的下一个id,不执行的任务都被切换到了内存中
    // 所以这里不会乱,会按照任务的索引依次循环执行
    _task_idx = (_task_idx + 1) % _tasks_num;
    struct context *next = &(task_ctx[_task_idx]);
    switch_to(next);
}

/* 函数等待并切换下一个任务 */
void task_yield()
{
    schedule();
}

/* 
 * 任务创建函数
 * 参数为待执行任务的第一条指令的地址 
 */
int task_create(task_func task)
{
    if(_tasks_num >= MAX_TASK_NUM)
        return -1;
    // 将任务的信息填写到结构体中
    // 第任务执行只需要ra指向下一条指令的地址,sp指向对应的栈即可
    // 必须要有栈是因为任务不可能很简单,以后会需要调用函数之类的,需要栈来实现
    task_ctx[_tasks_num].sp = (reg_t)(&(task_stack[_tasks_num][STACK_SIZE - 1])); //这里用(reg_t)(task_stack[_tasks_num] + STACK_SIZE - 1)也可以,后者是用的内存地址直接相加获得栈顶部的地址
    task_ctx[_tasks_num].ra = (reg_t)task;
    ++_tasks_num;
    return 0;
}
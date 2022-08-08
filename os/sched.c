#include "os.h"

/* _tasks_num表示当前执行的任务数量,包括正在执行和被切换下去的任务 */
static int _tasks_num = 0;
/* 每次生成新任务时的task id */
static int _task_id = 1;
/* cur_task表示当前task */
struct taskInfo *cur_task = NULL;
/* first_task表示第一个task,也就是优先级最高的task */
struct taskInfo *first_task = NULL;

/*  设置mscratch寄存器的值 */
static void w_mscratch(reg_t val)
{
    asm volatile("csrw mscratch, %0" : : "r"(val));
}

/* 调度初始化 */
void sched_init()
{
    // 首先设置mscratch寄存器的值为0,任务调度在schedule执行
    w_mscratch(0);
    // 初始化内核任务
    os_task.task_id = 0;
    os_task.priority = 0;
    os_task.state = RUNNING; // os的任务一直执行,所以一直是RUNNING
    os_task.timeslice = 0xffffffff;
    os_task.next = NULL;
    os_task.ctx.sp = (reg_t)(&(os_stack[STACK_SIZE - 1]));
    os_task.ctx.pc = (reg_t)kernel; // 由于switch_to函数不用ret而是用mret,所以这里得需要改成pc
    // 设置mie寄存器中软件定时器开启
    w_mie(r_mie() | MIE_MSIE);
}

/* 插入新任务到链表 */
int insert_task(struct taskInfo * new_task)
{
    // 现在还没有任务
    if(first_task == NULL && _tasks_num == 0)
    {
        first_task = new_task;
        ++_tasks_num;
        return 0;
    }
    // 现在有了任务,但是first_task为空
    if(first_task == NULL) return -1;
    // 从前往后找进行插入,优先级相同的话按照先进先出的原则
    // 先判断是否是开头
    if(new_task->priority < first_task->priority)
    {
        new_task->next = first_task;
        first_task = new_task;
    }
    else
    {
        struct taskInfo *it = first_task;
        struct taskInfo *prev = first_task;
        while(it && new_task->priority >= it->priority)
        {
            prev = it;
            it = it->next;
        }

        prev->next = new_task;
        new_task->next = it;
    }
    ++_tasks_num;
    return 0;
}

/* 从任务链表中取得一个任务 */
struct taskInfo *pop_task()
{
    if(first_task == NULL)
        return NULL;
    struct taskInfo *task = first_task;
    struct taskInfo *prev = first_task;
    while(task && task->state == SLEEPING)
    {
        prev = task;
        task = task->next;
    }
    if(task == NULL)
        return NULL;
    
    if(task->priority < 256)
        task->priority++; //减小优先级,否则就只能一直高优先级执行了
    
    // 将任务拿出来,降低优先级后再放到任务链表中
    if(task->task_id == first_task->task_id)
        first_task = first_task->next;
    else
        prev->next = task->next;

    //减小数量,然后重新插入
    --_tasks_num;
    insert_task(task);
    return task;
}

/* 用户任务调度函数 */
void schedule()
{
    if(_tasks_num <= 0)
    {
        back_os();
        // panic("Num of task should be greater than zero!");
        return;
    }
    // 获取要调度的task Id,下一个执行任务都是当前执行任务的下一个id,不执行的任务都被切换到了内存中
    // 所以这里不会乱,会按照任务的索引依次循环执行
    if((cur_task = pop_task()) == NULL)
    {
        back_os();
        panic("After back_os: never be here");
        return;
    }
    struct context *next = &(cur_task->ctx);
    switch_to(next);
}

/* 切换下一个用户任务 */
void task_yield()
{
    // 抢占式系统中,任务要想主动放弃hart,需要生成软中断
    reg_t hart_id = r_tp();
    *((uint32_t*)CLIENT_MSIP(hart_id)) = 1;
}

/* 延时函数,很低级的实现,后续可能会改 */
#ifdef RV32
void task_delay(uint32_t tick)
{
    cur_task->state = SLEEPING;
    timer_create(NULL, NULL, tick);
    task_yield();
}
#else
void task_delay(uint64_t tick)
{
    cur_task->state = SLEEPING;
    timer_create(NULL, NULL, tick);
    task_yield();
}
#endif

/* 退出任务 */
void task_exit()
{
    if(cur_task == NULL)
        return;
    if(cur_task->task_id == first_task->task_id) 
    {
        first_task = first_task->next;
    }
    else
    {
        // 查找cur_task的前后,是的前后的节点越过cur_task直接相连
        struct taskInfo *it = first_task;
        struct taskInfo *prev = first_task;
        while(it && it->task_id != cur_task->task_id)
        {
            prev = it;
            it = it->next;
        }
        prev->next = cur_task->next;
    }
    free((void *)cur_task);
    --_tasks_num;
    // 这里按照之前非抢占式情况会将当前正在执行的要被退出的任务保存上下文
    // 但是抢占式之后switch_to函数没有保存指令了,所以就不会保存了,不需要设置mscratch为0了
    task_yield(); // 不知道为啥直接调用schedule函数会出现异常
}

/* 返回内核任务 */
void back_os()
{
    // 写入mstatus的mpp位为machine模式,是的内核代码运行在machine模式
    w_mstatus(r_mstatus() | 3 << 11);
    switch_to(&(os_task.ctx));
}

/* 
 * 任务创建函数
 * 参数为待执行任务的第一条指令的地址,所带的参数,任务优先级
 */
#ifdef RV32
int task_create(task_func task, void *param, int priority, uint32_t timeslice)
{
    if(_tasks_num >= MAX_TASK_NUM)
        return -1;
    /* 
     * 将任务的信息填写到结构体中
     * 第任务执行只需要ra指向下一条指令的地址,sp指向对应的栈即可
     * 必须要有栈是因为任务不可能很简单,以后会需要调用函数之类的,需要栈来实现
    */
    // 开辟任务的结构体
    struct taskInfo *new_task = (struct taskInfo *)malloc(sizeof(struct taskInfo));
    new_task->task_id = _task_id++;
    new_task->priority = priority;
    new_task->timeslice = timeslice;
    new_task->next = NULL;
    new_task->ctx.sp = (reg_t)(&(task_stack[_tasks_num][STACK_SIZE - 1]));
    new_task->ctx.pc = (reg_t)task; // 由于switch_to函数不用ret而是用mret,所以这里得需要改成pc
    if(param != NULL)
        new_task->ctx.a0 = (reg_t)param;
    // 插入新任务到任务链表中
    if(insert_task(new_task) < 0)
    {
        printf("插入任务失败\n");
        free((void *)new_task);
        return -1;
    }
    return 0;
}
#else
int task_create(task_func task, void *param, int priority, uint64_t timeslice)
{
    if(_tasks_num >= MAX_TASK_NUM)
        return -1;
    /* 
     * 将任务的信息填写到结构体中
     * 第任务执行只需要ra指向下一条指令的地址,sp指向对应的栈即可
     * 必须要有栈是因为任务不可能很简单,以后会需要调用函数之类的,需要栈来实现
    */
    // 开辟任务的结构体
    struct taskInfo *new_task = (struct taskInfo *)malloc(sizeof(struct taskInfo));
    new_task->task_id = _task_id++;
    new_task->priority = priority;
    new_task->timeslice = timeslice;
    new_task->next = NULL;
    new_task->ctx.sp = (reg_t)(&(task_stack[_tasks_num][STACK_SIZE - 1]));
    new_task->ctx.pc = (reg_t)task; // 由于switch_to函数不用ret而是用mret,所以这里得需要改成pc
    if(param != NULL)
        new_task->ctx.a0 = (reg_t)param;
    // 插入新任务到任务链表中
    if(insert_task(new_task) < 0)
    {
        printf("插入任务失败\n");
        free((void *)new_task);
        return -1;
    }
    return 0;
}
#endif
#include "xinos/task.h"
#include "xinos/debug.h"
#include "xinos/memory.h"
#include "xinos/assert.h"
#include "xinos/interrupt.h"
#include "xinos/string.h"
#include "xinos/bitmap.h"
#include "xinos/syscall.h"

// 页的大小为4K
#define PAGE_SIZE 0x1000

// 外部的虚拟内存位图
extern bitmap_t kernel_map;
// 外部的任务切换函数
extern void task_switch(task_t *next);

// 任务数量
#define NR_TASKS 64
// 任务数组
static task_t* task_table[NR_TASKS];

/**
 * 在task_table生成一个空闲任务
 */
static task_t* get_free_task() {
    for(size_t i = 0; i < NR_TASKS; ++i) {
        if(task_table[i] == NULL) {
            return task_table[i] = (task_t*)alloc_kpage(1); // todo后续还得free内存
        }
    }
    panic("No more tasks");
}

/**
 * 从任务数组中查找某种状态的数组, 自己除外
 */
static task_t* task_search(task_state_t state) {
    task_t *task = NULL;
    // 当前任务
    task_t *cur = running_task();
    // 开始查找
    for(size_t i = 0; i < NR_TASKS; ++i) {
        task_t *ptr = task_table[i];
        // 任务为空或者是当前的任务或者状态不满足
        if(ptr == NULL || ptr == cur || ptr->state != state) continue;
        // 如果候选task为空, 或者候选task的需要的时间比遍历到的需要的时间小, 或者候选task的上一次执行的时间片比遍历到的时间片大
        // 也就是尽量让长任务、最久未执行的任务优先去执行 todo 多级时间片调度
        if(task == NULL || task->ticks < ptr->ticks || task->jiffies > ptr->jiffies)
            task = ptr;
    }
    return task;
}

/**
 * 由于任务的栈顶是0x1000或者0x2000
 * 所以可以根据栈顶来看当前是哪个任务
 */
task_t *running_task() {
    /**
     * asm的内联汇编跟直接编写汇编不太一样
     * 内联汇编中的指令是 指令 src, dest
     * 而直接编写汇编的指令是 指令 dest, src
     * 所以这里将esp的值给eax的话需要用mov esp, eax
     */
    asm volatile(
        "movl %esp, %eax\n"
        "andl $0xfffff000, %eax\n"
    );
}

/**
 * 调度任务
 */
void schedule() {
    // 当前必须要处于屏蔽中断的状态, 否则会有中断来了打断这个函数, 因为目前的更换任务就是因为中断产生的
    assert(!get_interrupt_state());
    // 当前任务
    task_t *cur = running_task();
    // 获取就绪任务
    task_t *next = task_search(TASK_READY);

    assert(next != NULL);
    assert(next->magic == KERNEL_MAGIC);

    // 更改当前任务的属性
    if(cur->state == TASK_RUNNING) cur->state = TASK_READY;
    // 如果当前任务没有了时间片, 就让时间片等于优先级
    if(cur->ticks == 0) cur->ticks = cur->priority;
    // 更改下一个任务的属性
    next->state = TASK_RUNNING;
    // 切换任务, 由于这个切换是通过中断的多层函数调用到这里, 因此栈顶的位置不是任务创建时的stack位置
    // 当任务切换回当前任务后, 按照函数调用的原则, 就会从下一行的printk继续执行
    // 把多层函数调用返回, 也就是把那个时候的中断处理执行完, 又由于中断的时候会保存当时任务执行的下一条语句
    // 假设是thread_x的printk函数, 那么中断函数处理完后才会去执行printk函数打印, 栈顶才会回到当时的位置
    task_switch(next);

    printk("back to task %s\n", cur->name);
}

u32 thread_a() {
    // 由于中断之后if位为0, 而此时中断并未返回也就是没有执行iret弹回原来的eflags值
    // 所以这里需要人为置为1, 这样就可以接收中断切换任务了
    set_interrupt_state(true);
    while(true) {
        printk("AAA\n");
        yield(); // 调度任务
    }
}

u32 thread_b() {
    // 由于中断之后if位为0, 而此时中断并未返回也就是没有执行iret弹回原来的eflags值
    // 所以这里需要人为置为1, 这样就可以接收中断切换任务了
    set_interrupt_state(true);
    while(true) {
        printk("BBB\n");
        yield();  // 调度任务
    }
}

u32 thread_c() {
    // 由于中断之后if位为0, 而此时中断并未返回也就是没有执行iret弹回原来的eflags值
    // 所以这里需要人为置为1, 这样就可以接收中断切换任务了
    set_interrupt_state(true);
    while(true) {
        printk("CCC\n");
        yield(); // 调度任务
    }
}

/**
 * 创建任务
 * target: 要执行的函数
 * name: 任务名称
 * priority: 优先级
 * uid: 调用任务的的用户id
 */
static task_t* task_create(target_t target, const char *name, u32 priority, u32 uid) {
    // 获取一个任务结构体
    task_t *task = get_free_task();
    memset((void*)task, 0, PAGE_SIZE);
    /**
     * 找到栈底
     */
    // 1. 找到内存页的顶部
    u32 stack = (u32)task + PAGE_SIZE;
    // 2. 顶部向下的一段内存保存frame, 因此栈是从frame之后开始
    stack -= sizeof(task_frame_t);
    // 获取栈的结构体
    task_frame_t *frame = (task_frame_t*)stack;
    // 填充结构体内容
    frame->ebx = 0x11111111;         // 随便填
    frame->esi = 0x22222222;         // 随便填
    frame->edi = 0x33333333;        // 随便填
    frame->ebp = 0x44444444;       // 随便填
    // eip指针要指向target, 这样才能往任务执行
    frame->eip = (void*)target;
    // 任务名
    strcpy(task->name, name);
    /**
     * 让task的栈位置指向stack
     * 
     * 这里设计的很巧妙, 假设任务a所在的页为0x2000, 也就是task结构体在0x2000
     * 该页内存的分布从底向上则是task结构体(结构体顶部有个魔数), 然后是栈, 然后是frame信息, 栈是从frame的下部往下生长的
     * 
     * 以a任务为例, a任务的页高地址为0x2000, 页低地址为0x1000, a变量就是指向了0x1000
     * 我们将frame信息存到了[0x2000 - sizeof(task_frame_t), 0x2000)
     * 由于对象内存是从低地址往高地址存储, 且变量的顺序与声明的顺序一致
     * 因此从0x2000 - sizeof(task_frame_t)开始, 往高地址方向, 依次是edi esi ebx ebp eip
     * 而且a->stack = stack, 意思就是让a指向的0x1000处内存中存储了stack的地址, 也就是0x2000 - sizeof(task_frame_t)
     * 那么我们就可以通过0x1000内存处的数据, 得到frame的地址
     * 然后在task_switch中依次将frame的edi esi ebx ebp弹出栈, 然后ret语句会将此时esp指向的返回地址弹出给eip
     * 而frame中ebp变量的下一个就是eip变量, 也就是target代表的函数地址, 这样就能够将target弹出给eip了
     * 那么就能调用target代表的函数了
     */
    task->stack = (u32*)stack;
    task->priority = priority;
    task->ticks = priority;
    task->jiffies = 0; // 初始化为0
    task->state = TASK_READY;
    task->uid = uid;
    task->vmap = &kernel_map;  // 目前是内核任务
    task->pde = KERNEL_PAGE_DIR; // 页表目录位置
    task->magic = KERNEL_MAGIC;

    return task;
}

/**
 * 任务调度
 */
void task_yield() {
    schedule();
}

/**
 * 设置任务
 */
static void task_setup() {
    // 用当前任务作为引子, 引出abc三个任务, 后续就是这三个任务交替执行了
    // 事实上当前任务并没有显式创建, 而是用正在进行的任务当作了
    task_t *cur = running_task();
    cur->magic = KERNEL_MAGIC;
    cur->ticks = 1;

    memset((void*)task_table, 0, sizeof(task_table));
}

/**
 * 初始化任务
 */
void task_init() {
    task_setup();
    task_create(thread_a, "a", 5, KERNEL_USER);
    task_create(thread_b, "b", 5, KERNEL_USER);
    task_create(thread_c, "c", 5, KERNEL_USER);
}
#include "xinos/task.h"
#include "xinos/printk.h"

// 页的大小为4K
#define PAGE_SIZE 0x1000

/**
 * 定义任务
 * 
 * 这里突然有点误区了
 * task_t *a = (task_t*)0x1000
 * 表示的是定义了一个a变量, 是一个指针, 指向了一个task_t对象
 * 这个变量中存放的值是0x1000, 也就是这个对象正好在0x1000位置
 * 要想获得a变量的地址, 就得用&a
 * 而*a表示的是对a进行解引用, 也就是获取内存0x1000位置处的内容
 * 而且int *p=1这种写法在用户程序中是错误的
 */
task_t *a = (task_t*)0x1000;
task_t *b = (task_t*)0x2000;

extern void task_switch(task_t *next);

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
    asm volatile("movl %esp, %eax\n"
                        "andl $0xfffff000, %eax\n"
    );
}

/**
 * 调度任务
 */
void schedule() {
    task_t *cur = running_task();
    task_t *next = cur == a ? b : a;
    // 切换任务
    task_switch(next);
    return;
}

u32 thread_a() {
    while(true) {
        printk("thread_a\n");
        schedule();
    }
}

u32 thread_b() {
    while(true) {
        printk("thread_b\n");
        schedule();
    }
}

static void task_create(task_t *task, target_t target) {
    // 找到栈底
    u32 stack = (u32)task + PAGE_SIZE;
    // 栈底及向下的一段内存保存frame
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
    /**
     * 让task的栈位置指向stack
     * 
     * 这里设计的很巧妙
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
}

void task_init() {
    printk("a变量的内容: %p\n", a);
    printk("a变量内容所代表的内存位置处的内容: %p\n", *a);
    printk("a变量的地址: %p\n", &a);
    task_create(a, thread_a);
    task_create(b, thread_b);
    schedule();
}
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

task_t *running_task() {
    asm volatile("movl %esp, %eax\n"
                 ""
    );
}

void task_init() {
    // printk("a变量的内容: %p\n", a);
    // printk("a变量内容所代表的内存位置处的内容: %p\n", *a);
    // printk("a变量的地址: %p\n", &a);


}
#include "xinos/interrupt.h"
#include "xinos/syscall.h"
#include "xinos/debug.h"

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

/**
 * idle任务
 */
void idle_thread() {
    // 由于中断的时候找不到任务就会默认执行idle_task, 而中断的时候if位设置为0了, 所以这里需要设置回去
    set_interrupt_state(true);

    while(true) {
        // 我们需要在idle中执行hlt来关闭cpu
        asm volatile("hlt\n");
        // 当被唤醒后, 找下一个任务调度
        yield();
    }
}

/**
 * init任务
 */
void init_thread() {
    // 设置允许中断, 这样从其他任务调度过来的时候, 可以接收中断
    set_interrupt_state(true);

    u32 counter = 0;
    while (true) {
        LOGK("init task %d....\n", ++counter);
        sleep(500);
    }
}

/**
 * test任务
 */
void test_thread() {
    set_interrupt_state(true);
    u32 counter = 0;

    while (true) {
        LOGK("test task %d....\n", ++counter);
        sleep(709);
    }
}
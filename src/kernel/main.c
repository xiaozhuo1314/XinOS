#include "xinos/debug.h"
#include "xinos/interrupt.h"

extern void console_init();
extern void gdt_init();
extern void interrupt_init();
extern void clock_init();
extern void hang();
extern void time_init();
extern void rtc_init();
extern void memory_map_init(); // 物理内存数组设置
extern void mapping_init(); // 内存映射
extern void task_init();

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

void intr_test()
{
    bool intr = interrupt_disable();
    // do something
    set_interrupt_state(intr);
}

extern void memory_test();

void kernel_init() {
    memory_map_init();
    mapping_init();
    interrupt_init();
    clock_init();
    // time_init();
    // rtc_init();
    task_init();
    set_interrupt_state(true);  // 开启中断
}
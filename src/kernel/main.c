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
    // clock_init();
    // time_init();
    // rtc_init();

    bool intr = interrupt_disable();
    set_interrupt_state(true);
    LOGK("%d\n", intr);
    LOGK("%d\n", get_interrupt_state());
    BMB;
    intr = interrupt_disable();
    BMB;
    set_interrupt_state(true);
    LOGK("%d\n", intr);
    LOGK("%d\n", get_interrupt_state());

    // 开启中断, 时钟中断是不可屏蔽的, 因此不需要多次sti
    // asm volatile("sti");
    hang();
}
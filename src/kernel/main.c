extern void console_init();
extern void gdt_init();
extern void interrupt_init();
extern void clock_init();
extern void hang();
extern void time_init();
extern void rtc_init();
extern void memory_map_init(); // 物理内存数组设置
extern void mapping_init(); // 内存映射

extern void memory_test();

void kernel_init() {
    memory_map_init();
    mapping_init();
    interrupt_init();
    // clock_init();
    // time_init();
    // rtc_init();

    // 测试页目录和页表
    memory_test();

    // 开启中断, 时钟中断是不可屏蔽的, 因此不需要多次sti
    // asm volatile("sti");
    hang();
}
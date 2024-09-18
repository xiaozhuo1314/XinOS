extern void console_init();
extern void gdt_init();
extern void interrupt_init();
extern void clock_init();
extern void hang();
extern void time_init();

void kernel_init() {
    console_init();
    gdt_init();
    interrupt_init();
    // task_init();
    clock_init();
    time_init();
    // 开启中断, 时钟中断是不可屏蔽的, 因此不需要多次sti
    asm volatile("sti");
    hang();
}
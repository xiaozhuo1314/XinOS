#include "xinos/types.h"
#include "xinos/io.h"
#include "xinos/string.h"
#include "xinos/console.h"
#include "xinos/assert.h"
#include "xinos/debug.h"
#include "xinos/global.h"
#include "xinos/task.h"
#include "xinos/interrupt.h"
#include "xinos/stdlib.h"

char msg[] = "hello xinos\n";

void kernel_init() {
    console_init();
    gdt_init();
    // task_init();
    interrupt_init();

    // 由于interrupt_init中把所有主从片的中断都关闭了, 所以需要打开
    // 开启后会出现时钟中断
    asm volatile("sti");

    u32 counter = 0;
    while(true) {
        DEBUGK("looping in kernel init %d...\n", counter++);
        delay(100000000);
    }
}
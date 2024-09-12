#include "xinos/types.h"
#include "xinos/io.h"
#include "xinos/string.h"
#include "xinos/console.h"
#include "xinos/assert.h"
#include "xinos/debug.h"
#include "xinos/global.h"
#include "xinos/task.h"
#include "xinos/interrupt.h"

char msg[] = "hello xinos\n";

void kernel_init() {
    console_init();
    gdt_init();
    // task_init();
    interrupt_init();
}
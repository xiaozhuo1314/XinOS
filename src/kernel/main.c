#include "xinos/types.h"
#include "xinos/io.h"
#include "xinos/string.h"
#include "xinos/console.h"

char msg[] = "hello xinos\n";

void kernel_init() {
    console_init();
    for(size_t i = 0; i < 300; ++i) {
        console_write(msg, sizeof(msg));
    }
    char end[] = "end!!!";
    console_write(end, sizeof(end));
    return;
}
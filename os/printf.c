#include "os.h"

static char out_buf[1024];

/* 参考 https://github.com/cccriscv/mini-riscv-os/blob/master/05-Preemptive/lib.c */
static int _vsnprintf() {
    int format = 0, longarg = 0;
    size_t pos = 0;
    
}
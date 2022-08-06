#include "syscall.h"

.global sleep
sleep:
    li a7, SYS_sleep
    ecall
    ret
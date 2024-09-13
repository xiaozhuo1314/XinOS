#include "xinos/stdlib.h"

void delay(u32 time_count) {
    while(time_count--) ;
}

void hang() {
    while(true) ;
}
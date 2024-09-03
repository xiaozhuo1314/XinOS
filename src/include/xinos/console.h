#ifndef CONSOLE_H
#define CONSOLE_H

#include "types.h"

void console_init();
void console_clear();
void console_write(char *buf, u32 count);

#endif
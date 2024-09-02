#include "xinos/types.h"
#include "xinos/io.h"
#include "xinos/string.h"

#define CRT_ADDR_REG 0x3D4
#define CRT_DATA_REG 0x3D5
#define CRT_CURSOR_H 0xE
#define CRT_CURSOR_L 0xF

void kernel_init() {
    char message[] = "hello xinos\n";
    char buf[1024] = {0};

    int res;
    res = strcmp(buf, message);  // -1
    strcpy(buf, message);
    res = strcmp(buf, message);  // 0

    strcat(buf, message);
    res = strcmp(buf, message);  // 1

    res = strlen(message);  // 比sizeof小1, 因为\0
    res = sizeof(message);

    char *ptr = strchr(message, 'o');
    ptr = strrchr(message, 'o');

    memset(buf, 0, sizeof(buf));
    res = memcmp(buf, message, sizeof(message));  // -1
    memcpy(buf, message, sizeof(message));
    res = memcmp(buf, message, sizeof(message));  // 0
    ptr = memchr(buf, 'o', sizeof(message));
    return;
}
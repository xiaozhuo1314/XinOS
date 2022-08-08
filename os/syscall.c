#include "os.h"

/*
 * a7存放的是系统调用号
 * a0-a5存放的是需要调用的系统调用函数的参数
 * 将系统调用函数的返回值写入到a0中
 * 按照unix约定,系统调用返回值为0代表成功,为负数代表失败
 */
void do_syscall(struct context *ctx)
{
    uint32_t call_num = ctx->a7;
    switch (call_num)
    {
    case SYS_sleep:
        task_delay(ctx->a0);
        ctx->a0 = 0;
        break;
    default:
        printf("Unknown syscall no: %d\n", call_num);
        ctx->a0 = -1;
        break;
    }
}
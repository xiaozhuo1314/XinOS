#include "os.h"

/* 由于CLINT时钟 每秒10000000 ticks,那么经过这些ticks就过了一秒钟 */
#define TIMER_INTERVAL CLINT_TIMEBASE_FREQ

static uint32_t _ticks = 0;

/* 
 * mtime寄存器是实时计数器,上电后硬件复位为0并开始记录tick,表示系统运行了多少个tick,即多少时间,这个寄存器仅此一个,所有hart共享
 * mtimecmp寄存器每个hart一个,不会被硬件复位为0,需要软件设置值 
 * 当mtime >= mtimecmp,client会产生一个timer中断,如果要想该中断正常产生,
 *   1.必须mstatus的全局总中断开启(这里采用machine模式)
 *   2.还需要mie中的MIE_MTIE(machine模式的定时器中断)开启
 * 当timer中断发生时,hart会设置mip.mtip(即当前machine模式下的timer中断正在发生),程序可以在mtimecmp中写入新值以清除mip.mtip
 */

/* 使mtimecmp寄存器加载ticks */
void timer_load(int interval)
{
    reg_t hart_id = r_tp();
    // MTIMECMP应该是当前时间+interval间隔
    *((uint64_t*)CLIENT_MTIMECMP(hart_id)) = *((uint64_t*)CLIENT_MTIME) + interval;
}

/* 硬件定时器初始化函数 */
void timer_init()
{
    // mtimecmp寄存器加载ticks,使得1s后触发中断
    timer_load(TIMER_INTERVAL);

    // 设置全局中断打开,在plic_init中已经开启了,这里不需要再次开启
    // w_mstatus(r_mstatus() | MSTATUS_MIE);

    // 设置mie寄存器中硬件定时器开启
    w_mie(r_mie() | MIE_MTIE);
}

/* 显示系统从开机到现在运行时间 */
void elapsed_time()
{
    uint32_t seconds = _ticks % 60;
    uint32_t tmp = _ticks / 60;
    uint32_t minutes = tmp % 60;
    uint32_t hours = tmp / 60;
    char times[24] = {0};
    int idx = 0;
    // 小时
    if(hours > 9)
    {
        times[idx++] = '0' + hours / 10;
        times[idx++] = '0' + hours % 10;
    }
    else
    {
        times[idx++] = '0';
        times[idx++] = '0' + hours;
    }
    times[idx++] = ':';
    // 分钟
    if(minutes > 9)
    {
        times[idx++] = '0' + minutes / 10;
        times[idx++] = '0' + minutes % 10;
    }
    else
    {
        times[idx++] = '0';
        times[idx++] = '0' + minutes;
    }
    times[idx++] = ':';
    // 秒
    if(seconds > 9)
    {
        times[idx++] = '0' + seconds / 10;
        times[idx++] = '0' + seconds % 10;
    }
    else
    {
        times[idx++] = '0';
        times[idx++] = '0' + seconds;
    }
    times[idx] = 0;
    printf("%s\n", times);
}

/* 硬件定时器中断处理函数 */
void timer_handler()
{
    ++_ticks;
    elapsed_time();
    // 重新设置mtimecmp寄存器清除mip.mtip,并且等待下一个硬件定时器中断
    timer_load(TIMER_INTERVAL);
    // 由于back_os之后会选择一个用户任务执行,所以调用back_os应该在最后,这样前面的timer_load等函数才能重新设置定时器
    back_os();
}
#include "os.h"

/* 初始化plic,os运行在machine模式,所以下面的设置都是针对machine模式 */
void plic_init()
{
    // 读取hart id,用于开启中断、设置优先级等
    reg_t hart_id = r_tp();

    // 设置uart代表的中断源等级为1,等级范围为1-7,数字越大等级越高,0代表该中断源被关闭
    *((uint32_t*)(PLIC_PRIORITY(UART0_IRQ))) = 1;

    // 设置uart代表的中断源开启,每个中断源对应的plic寄存器中的某一二进制位,而plic在hart上是地址映射
    uint64_t plic_menable = *((uint64_t*)(PLIC_MENABLE(hart_id)));
    *((uint64_t*)(PLIC_MENABLE(hart_id))) = plic_menable | (1 << UART0_IRQ);

    // 设置阈值
    *((uint32_t*)(PLIC_MTHRESHOLD(hart_id))) = 0;
    
    // 设置mie寄存器(不是mstatus的mie位)允许machine模式外部中断
    w_mie(r_mie() | MIE_MEIE);

    // 设置mstatus中的machine模式总中断开启
    w_mstatus(r_mstatus() | MSTATUS_MIE);
}

/* 返回plic中当前优先级最高的中断源id */
int plic_claim()
{
    reg_t hart_id = r_tp();
    int irq = *((uint32_t*)(PLIC_MCLAIM(hart_id)));
    return irq;
}

/* 告知plic某中断源的中断已经处理完成 */
void plic_complete(int irq)
{
    reg_t hart_id = r_tp();
    *((uint32_t*)(PLIC_MCOMPLETE(hart_id))) = irq;
}
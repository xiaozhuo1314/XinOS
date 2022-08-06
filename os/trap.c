#include "os.h"

extern void trap_vector(void);

/* trap初始化指的是设置trap处理基址,在这里就是trap处理函数的地址,即设置mtvec寄存器 */
void trap_init()
{
    w_mtvec((reg_t)trap_vector);
}

/* 外部中断处理函数 */
void external_interrupt_handler()
{
    // 获取plic中当前优先级最高的中断源id
    int irq = plic_claim();
    if(irq == UART0_IRQ) //去读取键盘输入
    {
        uart_ier();
    }
    else if(irq)
    {
        printf("unexpected interrupt irq = %d\n", irq);
    }

    // 完成后要发送complete
    if(irq)
        plic_complete(irq);
}

void software_interrupt_handler()
{
    // 关闭当前的软中断
    reg_t hart_id = r_tp();
    *((uint32_t*)CLIENT_MSIP(hart_id)) = 0;
    // 任务切换
    schedule();
}

reg_t trap_handler(reg_t epc, reg_t cause, struct context *ctx)
{
    // 这里传递过来的epc为指令地址,如果是异常那就是产生异常的语句,如果是中断那就是产生中断的语句的下一条语句
    reg_t return_epc = epc;
    reg_t cause_code = cause & 0xfff;
    if(cause & 0x80000000) //如果是中断,因为mcause寄存器最高位为1代表中断,为0表示异常
    {
        switch (cause_code)
        {
        case 3:
            // printf("software interruption!\n");
            software_interrupt_handler();
            break;
        case 7:
            // printf("timer interruption!\n");
            timer_handler();
            break;
        case 11:
            // printf("external interruption!\n");
            external_interrupt_handler();
            break;
        default:
            printf("unknown async exception!\n");
            break;
        }
    }
    else //异常
    {
        printf("Sync exceptions!, code = %d\n", cause_code);
        switch (cause_code)
        {
        case 8: //本质上系统调用是一个异常
            uart_puts("System call from user mode\n");
            do_syscall(ctx);
            return_epc += 4;
            break;
        
        default:
            panic("OOPS! What can I do!");
            break;
        }
        // 如果不想要panic阻塞住,且想执行产生异常的下一条指令,就可以注释掉上一条panic语句
        // 然后将return_epc加上一个地址跳过产生异常的指令,由于是32位,加4即可
        // return_epc += 4;
    }
    return return_epc;
}

void trap_test()
{
	/*
	 * Synchronous exception code = 7
	 * Store/AMO access fault
	 */
	*(int *)0x00000000 = 100;

	/*
	 * Synchronous exception code = 5
	 * Load access fault
	 */
	//int a = *(int *)0x00000000;

	printf("Yeah! I'm return back from trap!\n");
}
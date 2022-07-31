#ifndef __PLATFORM_H__
#define __PLATFORM_H__

/* cpu个数 */
#define MAXNUM_CPU 8

/* uart0的物理地址 */
#define UART0 0x10000000L

/*
 * https://github.com/qemu/qemu/blob/master/include/hw/riscv/virt.h
 * uart中断号id
 */
#define UART0_IRQ 10

/*
 * This machine puts platform-level interrupt controller (PLIC) here.
 * Here only list PLIC registers in Machine mode.
 * see https://github.com/qemu/qemu/blob/master/include/hw/riscv/virt.h
 * #define VIRT_PLIC_HART_CONFIG "MS"
 * #define VIRT_PLIC_NUM_SOURCES 127
 * #define VIRT_PLIC_NUM_PRIORITIES 7
 * #define VIRT_PLIC_PRIORITY_BASE 0x04
 * #define VIRT_PLIC_PENDING_BASE 0x1000
 * #define VIRT_PLIC_ENABLE_BASE 0x2000
 * #define VIRT_PLIC_ENABLE_STRIDE 0x80
 * #define VIRT_PLIC_CONTEXT_BASE 0x200000
 * #define VIRT_PLIC_CONTEXT_STRIDE 0x1000
 * #define VIRT_PLIC_SIZE(__num_context) \
 *     (VIRT_PLIC_CONTEXT_BASE + (__num_context) * VIRT_PLIC_CONTEXT_STRIDE)
 */
/* 设置plic_base地址,以下宏都是Machine模式下的 */
#define PLIC_BASE 0x0c000000L
#define PLIC_PRIORITY(interrupt_id) (PLIC_BASE + (interrupt_id) * 4)
#define PLIC_PENDING(interrupt_id) (PLIC_BASE + 0x1000 + ((interrupt_id) / 32) * 4)
#define PLIC_MENABLE(hart_id) (PLIC_BASE + 0x2000 + (hart_id) * 0x80)
#define PLIC_MTHRESHOLD(hart_id) (PLIC_BASE + 0x200000 + (hart_id) * 0x100)
#define PLIC_MCLAIM(hart_id) (PLIC_BASE + 0x200004 + (hart_id) * 0x1000)
#define PLIC_MCOMPLETE(hart_id) (PLIC_BASE + 0x200004 + (hart_id) * 0x1000)

/*
 * The Core Local INTerruptor (CLINT) block holds memory-mapped control and
 * status registers associated with software and timer interrupts.
 * QEMU-virt reuses sifive configuration for CLINT.
 * see https://gitee.com/qemu/qemu/blob/master/include/hw/riscv/sifive_clint.h
 * enum {
 * 	SIFIVE_SIP_BASE     = 0x0,
 * 	SIFIVE_TIMECMP_BASE = 0x4000,
 * 	SIFIVE_TIME_BASE    = 0xBFF8
 * };
 *
 * enum {
 * 	SIFIVE_CLINT_TIMEBASE_FREQ = 10000000
 * };
 *
 * Notice:
 * The machine-level MSIP bit of mip register are written by accesses to
 * memory-mapped control registers, which are used by remote harts to provide
 * machine-mode interprocessor interrupts.
 * For QEMU-virt machine, Each msip register is a 32-bit wide WARL register
 * where the upper 31 bits are tied to 0. The least significant bit is
 * reflected in the MSIP bit of the mip CSR. We can write msip to generate
 * machine-mode software interrupts. A pending machine-level software
 * interrupt can be cleared by writing 0 to the MSIP bit in mip.
 * On reset, each msip register is cleared to zero.
 */
#define CLIENT_BASE 0x2000000L
#define CLIENT_MSIP(hart_id) (CLIENT_BASE + (hart_id) * 4) //32位,但是高31位不可用,最后一位映射到mip.msip,表示machine模式的软中断在pending,当时用完后需要将该寄存器设置为0,使pending结束
#define CLIENT_MTIME (CLIENT_BASE + 0xBFF8) // 64位,硬件会自动设置
#define CLIENT_MTIMECMP(hart_id) (CLIENT_BASE + 0x4000 + (hart_id) * 8) // 64位

/* CLINT时钟 每秒10000000 ticks */
#define CLINT_TIMEBASE_FREQ 10000000

#endif
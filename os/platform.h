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

#endif
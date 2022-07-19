#include "platform.h"
#include "type.h"

/* 
 * 返回uart某个寄存器的地址 
 * uart的寄存器是8位的,所以用一个uint8_t *p指针来指向
 * 这样,指针是32位或64位的,但是指向的地址是一个8位的寄存器
*/
#define UART_REG(reg) ((volatile uint8_t *)(UART0 + reg))

/* uart的寄存器 */
// 注: DLL和DLM合起来用于配置分频器,以得到合适的波特率
#define RHR 0	// Receive Holding Register (read mode)
#define THR 0	// Transmit Holding Register (write mode)
#define DLL 0	// LSB of Divisor Latch (write mode)
#define IER 1	// Interrupt Enable Register (write mode)
#define DLM 1	// MSB of Divisor Latch (write mode)
#define FCR 2	// FIFO Control Register (write mode)
#define ISR 2	// Interrupt Status Register (read mode)
#define LCR 3	// Line Control Register
#define MCR 4	// Modem Control Register
#define LSR 5	// Line Status Register
#define MSR 6	// Modem Status Register
#define SPR 7	// ScratchPad Register

/*
 * POWER UP DEFAULTS
 * IER = 0: TX/RX holding register interrupts are both disabled
 * ISR = 1: no interrupt penting
 * LCR = 0
 * MCR = 0
 * LSR = 60 HEX
 * MSR = BITS 0-3 = 0, BITS 4-7 = inputs
 * FCR = 0
 * TX = High
 * OP1 = High
 * OP2 = High
 * RTS = High
 * DTR = High
 * RXRDY = High
 * TXRDY = Low
 * INT = Low
 */

/*
 * LINE STATUS REGISTER (LSR)
 * LSR BIT 0:
 * 0 = no data in receive holding register or FIFO.
 * 1 = data has been receive and saved in the receive holding register or FIFO.
 * ......
 * LSR BIT 5:
 * 0 = transmit holding register is full. 16550 will not accept any data for transmission.
 * 1 = transmitter hold register (or FIFO) is empty. CPU can load the next character.
 * ......
 */

#define LSR_TX_IDLE (1 << 5)
#define LSR_RX_READY (1 << 0)

#define uart_read_reg(reg) (*(UART_REG(reg)))
#define uart_write_reg(reg, v) (*(UART_REG(reg)) = (v))

/* 初始化uart */
void uart_init() {
    // 关闭所有中断
    // IER寄存器0-3每一个二进制位都表示一个中断的开和关,4-7位为0
    uart_write_reg(IER, 0x00);

    // 设置波特率
    // LCR的第7位(最高位)表示是否开启分频器以设置波特率
    uint8_t lcr = uart_read_reg(LCR);
    uart_write_reg(LCR, lcr | (1 << 7));
    uart_write_reg(DLL, 0x03);
    uart_write_reg(DLM, 0x00);

    // 设置数据交换格式
    // word length 8 bits
    // stop bits length 1 bit
    // no parity
    // no break control
    // disabled baud latch
    lcr = 0x03;
    uart_write_reg(LCR, lcr);
}

int uart_putc(char c) {
    // LSR第5位表示写寄存器是否是满的,0是满的,1是空的
    while((uart_read_reg(LSR) & LSR_TX_IDLE) == 0); //如果是满的,就一直循环等待,但是这样是比较低效的
    // 写数据
    return uart_write_reg(THR, c);
}

void uart_puts(char *p) {
    while(*p) {
        uart_putc(*(p++));
    }
}

/* 输入字符 */
void uart_gets() {
    char c;
    while(1) {
        // LSR第0位表示是否有数据到来,0代表没有
        while((uart_read_reg(LSR) & LSR_RX_READY) == 0);
        c = (char)uart_read_reg(RHR);
        if(c == '\r' || c == '\n') {
            uart_puts("\r\n");
        } else if(c == 127 || c == 8) {
            // 删除字符就得要这样写
            uart_puts("\b \b");
        } else if(c == 27) {
            break;
        } else {
            uart_putc(c);
        }
    }
    uart_puts("Goodbye\n");
}

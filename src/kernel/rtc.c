#include "xinos/rtc.h"
#include "xinos/debug.h"
#include "xinos/io.h"
#include "xinos/assert.h"
#include "xinos/interrupt.h"
#include "xinos/time.h"

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

/**
 * 为了读取指定偏移位置的字节
 * 首先需要使用out向端口0x70发送需要读取的内存位置的偏移值
 * 然后使用in指令从0x71端口读取指定的字节信息
 * 不过在选择字节（寄存器）时最好屏蔽NMI中断
 * 屏蔽NMI中断就是向0x70端口写入0x80
 */
#define CMOS_ADDR 0x70 // CMOS 地址寄存器
#define CMOS_DATA 0x71 // CMOS 数据寄存器

// CMOS信息的寄存器索引(写入)
#define CMOS_SECOND 0x01
#define CMOS_MINUTE 0x03
#define CMOS_HOUR 0x05

// 四个寄存器
#define CMOS_A 0x0a
#define CMOS_B 0x0b
#define CMOS_C 0x0c
#define CMOS_D 0x0d

// nmi中断
#define CMOS_NMI 0x80

/**
 * 按照字节读取cmos寄存器数据
 */
u8 cmos_read(u8 addr) {
    /**
     * 需要使用out向端口0x70发送需要读取的内存位置的偏移值
     * 需要注意的是: 读取数据的时候需要关闭nmi不可屏蔽中断
     */
    outb(CMOS_ADDR, CMOS_NMI | addr);
    // 然后使用in指令从0x71端口读取指定的字节信息
    return inb(CMOS_DATA);
}

/**
 * 写寄存器的值
 */
void cmos_write(u8 addr, u8 value) {
    /**
     * 需要先告诉0x70端口要操作的偏移值
     * 然后才能去写
     */
    outb(CMOS_ADDR, CMOS_NMI | addr);
    outb(CMOS_DATA, value);
}

// 计数
static u32 volatile counter = 0;

/**
 * 实时时钟中断处理函数
 */
void rtc_handler(int vector) {
    // 我们在设置中断set_mask时, 是采用从片的第一个向量号代表实时时钟中断, 因此向量号是0x28
    assert(vector == 0x28);

    // 发送中断可以继续的产生的信号
    send_eoi(vector);

    // 读取CMOS寄存器C, 允许CMOS继续产生中断
    cmos_read(CMOS_C);

    // 设置闹钟, 这里设置10秒后继续中断
    set_alarm(10);

    LOGK("rtc handler %d...\n", counter++);
}

/**
 * 设置闹钟以产生实时中断
 */
void set_alarm(u32 secs) {
    // 获取当前时间
    tm time;
    time_read(&time);

    // 获取闹钟响的时间(这里假定是在一天内的)
    // todo 一天之外的
    u8 sec = secs % 60;
    secs /= 60;
    u8 min = secs % 60;
    secs /= 60;
    u32 hour = secs;
    time.tm_sec += sec;
    if (time.tm_sec >= 60) {
        time.tm_sec %= 60;
        time.tm_min += 1;
    }
    time.tm_min += min;
    if (time.tm_min >= 60) {
        time.tm_min %= 60;
        time.tm_hour += 1;
    }
    time.tm_hour += hour;
    if (time.tm_hour >= 24) {
        time.tm_hour %= 24;
    }

    // 写入闹钟的时间
    cmos_write(CMOS_HOUR, bin_to_bcd(time.tm_hour));
    cmos_write(CMOS_MINUTE, bin_to_bcd(time.tm_min));
    cmos_write(CMOS_SECOND, bin_to_bcd(time.tm_sec));
}

/**
 * rtc初始化
 */
void rtc_init() {
    cmos_write(CMOS_B, 0b00100010); // 打开闹钟中断
    cmos_read(CMOS_C); // 读 C 寄存器，以允许 CMOS 中断
    set_alarm(20);
    // 设置中断频率
    outb(CMOS_A, (inb(CMOS_A) & 0xf) | 0b1110);  // 250ms, 但是由于我们并未开启周期中断, 所以这个没啥用
    
    set_interrupt_handler(IRQ_RTC, rtc_handler);
    set_interrupt_mask(IRQ_RTC, true);
    set_interrupt_mask(IRQ_CASCADE, true);
}
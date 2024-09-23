#include "xinos/io.h"
#include "xinos/interrupt.h"
#include "xinos/assert.h"
#include "xinos/debug.h"

/**
 * 三个计数器
 * 计数器 0，端口号 0x40，用于产生时钟信号，它采用工作模式2
 * 计数器 1，端口号 0x41，用于 DRAM(内存) 的定时刷新控制
 * 计数器 2，端口号 0x42，用于内部扬声器发出不同音调的声音，原理是给扬声器输送某频率的方波
 * 
 * 计数器 0 用于产生时钟中断，就是连接在 IRQ0 引脚上的时钟
 * 也就是控制计数器 0 可以控制时钟发生的频率，以改变时间片的间隔
 * 
 * 控制字寄存器，端口号 0x43，是 8 位寄存器，控制字寄存器也称为模式控制寄存器
 * 用于指定计数器的 工作方式、读写格式 及 数制
 * 
 * - 模式 0：计数结束时中断
 * - 模式 1：硬件可重触发单稳方式
 * - 模式 2：比率发生器，用于分频
 * - 模式 3：方波发生器
 * - 模式 4：软件触发选通
 * - 模式 5：硬件触发选通
 * 
 * 这三个计数器中0x40是连接到中断控制器的IRQ0引脚的
 * 也就是芯片是连接到中断控制器(0x20和0xa0)的, 然后通过中断控制器给cpu发送中断信号
 */
// 现在还不使用0x41
#define PIT_CHAN0_REG 0X40
#define PIT_CHAN2_REG 0X42
#define PIT_CTRL_REG 0X43

/**
 * HZ表示时钟中断发生的频率, 这里是每秒发生100次, 也就是10ms发生一次
 * 
 * OSCILLATOR表示振荡器的频率, 也就是振荡器一秒震动OSCILLATOR次
 * 那么我们设置CLOCK_COUNTER为OSCILLATOR / HZ, 也就是跳动CLOCK_COUNTER次触发一次时钟中断
 * 也就是每秒发生100次
 * 
 * JIFFY表示每两次中断的间隔, 这里为10ms
 */
#define HZ 100
#define OSCILLATOR 1193182
#define CLOCK_COUNTER (OSCILLATOR / HZ)
#define JIFFY (1000 / HZ)

/**
 * 蜂鸣器相关
 * 
 * 扬声器有两种状态，输入和输出，状态可以通过键盘控制器中的端口号 `0x61` 设置
 * 该寄存器结构如下
   | 位  | 描述            |
   | --- | --------------- |
   | 0   | 计数器 2 门有效 |
   | 1   | 扬声器数据有效  |
   | 2   | 通道校验有效    |
   | 3   | 奇偶校验有效    |
   | 4   | 保留            |
   | 5   | 保留            |
   | 6   | 通道错误        |
   | 7   | 奇偶错误        |

   需要将 0 ~ 1 位置为 1，然后计数器 2 设置成 3 方波模式，就可以播放方波的声音。
 */
#define SPEAKER_REG 0x61
#define BEEP_HZ 440  // 一般设置为440HZ
#define BEEP_COUNTER (OSCILLATOR / BEEP_HZ)

// 时间片计数器
u32 volatile jiffies = 0;
u32 jiffy = JIFFY;

// 蜂鸣器相关
u32 volatile beeping = 0;

/**
 * 设置beeping
 */
void start_beep() {
    if (!beeping) { // 如果beeping为0, 说明还没开始蜂鸣, 需要设置0 ~ 1位为 1
        outb(SPEAKER_REG, inb(SPEAKER_REG) | 0b11);
    }
    // 加5是为了每次让蜂鸣器响5个时钟中断
    beeping = jiffies + 5;
}

/**
 * 停止beeping
 */
void stop_beep() {
    if (beeping && jiffies >= beeping) {
        outb(SPEAKER_REG, inb(SPEAKER_REG) & 0xfc);  // 设置0 ~ 1位为 0
        beeping = 0;
    }
}

/**
 * 时钟中断处理函数
 */
void clock_handler(int vector) {
    // 时钟中断向量为0x20
    assert(vector == 0x20);
    // 告诉中断控制器已经中断处理完成,否则一直在屏蔽中
    send_eoi(vector);
    // 蜂鸣器
    if(jiffies % 200 == 0) {  // 每200个时钟中断就让蜂鸣器响5个周期
        start_beep();
    }
    // 累加计数器
    ++jiffies;
    DEBUGK("clock jiffies %d ...\n", jiffies);
    // 调用beeping停止, 尝试去停止
    stop_beep();
}

/**
 * 设置控制字寄存器和计数器
 */
void pit_init() {
    /**
     * 控制字结构：
        | 7   | 6   | 5   | 4   | 3   | 2   | 1   | 0   |
        | --- | --- | --- | --- | --- | --- | --- | --- |
        | SC1 | SC0 | RL1 | RL0 | M2  | M1  | M0  | BCD |

        - SC(Select Counter) 0 ~ 1：计数器选择位
            - 00 计数器 0
            - 01 计数器 1
            - 10 计数器 2
            - 11 无效
        - RL(Read Load) 0 ~ 1：读写操作位
            - 00 锁存数据，供 CPU 读
            - 01 只读写低字节
            - 10 只读写高字节
            - 11 先读写低字节，后读写高字节
        - M (Mode) 0 ~ 2：模式选择
            - 000：模式 0
            - 001：模式 1
            - 010：模式 2
            - 011：模式 3
            - 100：模式 4
            - 101：模式 5
        - BCD：(Binary Coded Decimal) 码
            - 0 表示二进制计数器
            - 1 二进制编码的十进制计数器
     */
    outb(PIT_CTRL_REG, 0b00110100);  // 010表示是时钟中断, 而不是其他的模式
    // 下面设置时钟中断的间隔, 也就是振荡器震荡多少次触发中断
    outb(PIT_CHAN0_REG, CLOCK_COUNTER & 0xff);
    outb(PIT_CHAN0_REG, (CLOCK_COUNTER >> 8) & 0xff);

    // 配置计数器 2 蜂鸣器
    outb(PIT_CTRL_REG, 0b10110110);
    outb(PIT_CHAN2_REG, (u8)BEEP_COUNTER);
    outb(PIT_CHAN2_REG, (u8)(BEEP_COUNTER >> 8));
}

void clock_init() {
    pit_init();
    // 设置时钟中断函数
    set_interrupt_handler(IRQ_CLOCK, clock_handler);
    // 打开时钟中断
    set_interrupt_mask(IRQ_CLOCK, true);
}
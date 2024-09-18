#include "xinos/time.h"
#include "xinos/debug.h"
#include "xinos/stdlib.h"
#include "xinos/io.h"

// 定义日志打印函数
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

// CMOS信息的寄存器索引
#define CMOS_SECOND 0x00  // (0 ~ 59) 秒
#define CMOS_MINUTE 0x02  // (0 ~ 59) 分钟
#define CMOS_HOUR 0x04    // (0 ~ 23) 小时
#define CMOS_WEEKDAY 0x06 // (1 ~ 7) 星期天 = 1，星期六 = 7
#define CMOS_DAY 0x07     // (1 ~ 31) 日期
#define CMOS_MONTH 0x08   // (1 ~ 12) 月份
#define CMOS_YEAR 0x09    // (0 ~ 99) 年份
#define CMOS_CENTURY 0x32 // 世纪(可能不存在)
#define CMOS_NMI 0x80  // 读取时最好屏蔽到NMI中断, 否则时间可能不准确

#define MINUTE 60          // 每分钟的秒数
#define HOUR (60 * MINUTE) // 每小时的秒数
#define DAY (24 * HOUR)    // 每天的秒数
#define YEAR (365 * DAY)   // 每年的秒数，以365天算

/**
 * 每个月开始时的已经过去天数, 比方说5月, 那么month[5]就是前面4个月的天数
 * 这里第一个0是占位符, 表示第0月的, 无实际意义
 * 
 * 29表示的是闰月, 这里就先加上, 后面在进行获取时间时不是闰年的会减去
 */
static int month[13] = {
    0, // 这里占位，没有 0 月，从 1 月开始
    0,
    (31),
    (31 + 29),
    (31 + 29 + 31),
    (31 + 29 + 31 + 30),
    (31 + 29 + 31 + 30 + 31),
    (31 + 29 + 31 + 30 + 31 + 30),
    (31 + 29 + 31 + 30 + 31 + 30 + 31),
    (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31),
    (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30),
    (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31),
    (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30)
};

// 系统开机的时间戳
time_t startup_time;
// 世纪
int century;

/**
 * 生成时间戳
 * 
 * time: 时间结构体
 */
time_t mktime(tm *time) {
    time_t ans;
    // 年份从1970年开始算, 而time->tm_year是[0, 99], 也就是可能会有2024中的24这种情况, 因此需要加100后减去70
    // 而想2078年这样的, year就是108
    int year = ((time->tm_year >= 70) ? (time->tm_year - 70) : (time->tm_year - 70 + 100));
    ans = YEAR * year;

    // 已经过去的闰年，每个加 1 天
    ans += DAY * ((year + 1) / 4);

    // 已经过完的月份, 比如说是5月份, 那么就是
    ans += month[time->tm_mon] * DAY;

    // 如果 2 月已经过了, 并且当前不是闰年, 由于我们前面是按照闰年计算的month数组, 因此需要减去一天
    if(time->tm_mon > 2 && ((year + 2) % 4)) 
        ans -= DAY;
    
    // 这个月已经过去的天
    ans += (time->tm_mday - 1) * DAY;

    // 今天过去多少小时了
    ans += time->tm_hour * HOUR;

    // 当前小时过去多少分了
    ans += time->tm_min * MINUTE;

    // 当前分过去多少秒了
    ans += time->tm_sec;

    return ans;
}

/**
 * 获取当前天是一年中的哪一天
 */
time_t get_yday(tm *time) {
    time_t ans;
    // 已经过去多少天了
    ans = month[time->tm_mon];
    // 这个月第几天
    ans += time->tm_mday;

    // 获取当前从1970开始的年份
    int year = ((time->tm_year >= 70) ? (time->tm_year - 70) : (time->tm_year - 70 + 100));

    // 如果不是闰年且已经过了2月份, 就需要减去一天
    if(time->tm_mon > 2 && ((year + 2) % 4)) ans -= 1;

    return ans;
}

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
 * 以bcd码的形式读取时间
 */
void time_read_bcd(tm *time) {
    /**
     * CMOS的访问速度很慢, 为了减小时间误差, 在读取了下面循环中所有数值后
     * 若此时CMOS中秒值发生了变化, 那么就重新读取所有值
     * 这样内核就能把与CMOS的时间误差控制在1秒之内
     */
    do
    {
        time->tm_sec = cmos_read(CMOS_SECOND);
        time->tm_min = cmos_read(CMOS_MINUTE);
        time->tm_hour = cmos_read(CMOS_HOUR);
        time->tm_wday = cmos_read(CMOS_WEEKDAY);
        time->tm_mday = cmos_read(CMOS_DAY);
        time->tm_mon = cmos_read(CMOS_MONTH);
        time->tm_year = cmos_read(CMOS_YEAR);
        century = cmos_read(CMOS_CENTURY);
    } while (time->tm_sec != cmos_read(CMOS_SECOND));
}

/**
 * 以整数形式读取时间
 */
void time_read(tm *time) {
    // 首先以bcd码读取数据
    time_read_bcd(time);
    // 转成整数
    time->tm_sec = bcd_to_bin(time->tm_sec);
    time->tm_min = bcd_to_bin(time->tm_min);
    time->tm_hour = bcd_to_bin(time->tm_hour);
    time->tm_wday = bcd_to_bin(time->tm_wday);
    time->tm_mday = bcd_to_bin(time->tm_mday);
    time->tm_mon = bcd_to_bin(time->tm_mon);
    time->tm_year = bcd_to_bin(time->tm_year);
    // 得到当前天在一年中的天数
    time->tm_yday = get_yday(time);
    // 夏令时暂时不用
    time->tm_isdst = -1;
    century = bcd_to_bin(century);
}

/**
 * 时间初始化
 */
void time_init()
{
    tm time;
    time_read(&time);
    startup_time = mktime(&time);
    LOGK("startup time: %d%d-%02d-%02d %02d:%02d:%02d\n",
         century,
         time.tm_year,
         time.tm_mon,
         time.tm_mday,
         time.tm_hour,
         time.tm_min,
         time.tm_sec);
    LOGK("startup timestamp is: %d\n", startup_time);
}

// 将 bcd 码转成整数
u8 bcd_to_bin(u8 value) {
    return (value & 0xf) + (value >> 4) * 10;
}

// 将整数转成 bcd 码
u8 bin_to_bcd(u8 value) {
    return (value / 10) * 0x10 + (value % 10);
}
#include "xinos/console.h"
#include "xinos/io.h"

/**
 * 一些宏
 */
#define CRT_ADDR_REG 0x3D4                                             // CRT(6845)索引寄存器
#define CRT_DATA_REG 0x3D5                                              // CRT(6845)数据寄存器
#define CRT_CURSOR_H 0xE                                                  // 光标位置 - 高位
#define CRT_CURSOR_L 0xF                                                  // 光标位置 - 低位
#define CRT_START_ADDR_H 0xC                                        // 屏幕左上角的字符的内存位置的高位, 具体解释见screen变量
#define CRT_START_ADDR_L 0xD                                        // 屏幕左上角的字符的内存位置的低位, 具体解释见screen变量
#define MEM_BASE 0xB8000                                                  // 显示内存起始位置
#define MEM_SIZE 0x4000                                                      // 显示内存大小
#define MEM_END (MEM_BASE + MEM_SIZE)                   // 显示内存结束位置
#define WIDTH 80                                                                    // 显卡文本模式的列数
#define HEIGHT 25                                                                   // 显卡文本模式的行数
#define ROW_SIZE (WIDTH * 2)                                             // 显卡文本模式一行的字节数, 由于每个字符需要两个字节, 一个字节是字符本身, 一个是字符的样式
#define SCR_SIZE (ROW_SIZE * HEIGHT)                            // 显卡文本模式字节总数
// 下面是一些ascii码
#define ASCII_NUL 0x00
#define ASCII_ENQ 0x05
#define ASCII_BEL 0x07  // \a
#define ASCII_BS 0x08    // \b
#define ASCII_HT 0x09    // \t
#define ASCII_LF 0x0A    // \n
#define ASCII_VT 0x0B    // \v
#define ASCII_FF 0x0C    // \f
#define ASCII_CR 0x0D    // \r
#define ASCII_DEL 0x7F

/**
 * 这里需要着重解释一下screen变量
 * screen表示的是屏幕左上角第一个字符在内存中的位置
 * 这个位置是不固定的, 因为比方说我们屏幕中有若干个字符
 * 可能好几行, 然后我们把这些字符翻滚到屏幕外
 * 此时屏幕的左上角就是下一个待写入的字符了
 * 这个待写入字符的内存位置就是screen
 * 然后我们在屏幕中又写了很多字, 但是没有翻滚
 * 那么此时screen还是左上角字符内存中的位置
 * 而此时的下一个待写入字符的内存位置就是pos
 */
static u32 screen;
static u32 pos;
static x, y;                             // 当前光标的坐标, 不是内存中的位置, 而是屏幕上的坐标
static u8 attr = 7;                  // 字符样式
static u16 erase = 0x0720;    // 带有样式的空格, 因为前一个字节是字符空格0x20, 后一个字节是样式0x07, 而从第一个字符开始的内存地址是增长的, 所以0x07是在高位的

// 获得当前屏幕在内存中的位置
static void get_screen() {
    outb(CRT_ADDR_REG, CRT_START_ADDR_H);
    screen = inb(CRT_DATA_REG) << 8;
    outb(CRT_ADDR_REG, CRT_START_ADDR_L);
    screen |= inb(CRT_DATA_REG);

    // 每个字符需要两个字节, 一个字符本身一个样式, 所以需要乘以2
    screen <<= 1;
    // 加上显卡文本模式映射区域的起始地址
    screen += MEM_BASE;
}

// 设置当前屏幕在内存中的位置
static void set_screen() {
    outb(CRT_ADDR_REG, CRT_START_ADDR_H);
    // 由于是每两个字节一个字符, 所以需要先>>1
    // 然后由于我们现在需要获取高8位, 因此再>>8
    // 所以一共>>9
    outb(CRT_DATA_REG, ((screen - MEM_BASE) >> 9) & 0xff);
    outb(CRT_ADDR_REG, CRT_START_ADDR_L);
    // 于是每两个字节一个字符, 所以需要>>1
    outb(CRT_DATA_REG, ((screen - MEM_BASE) >> 1) & 0xff);
}

// 获得当前光标在内存中的位置
static void get_cursor() {
    outb(CRT_ADDR_REG, CRT_CURSOR_H);
    pos = inb(CRT_DATA_REG) << 8;
    outb(CRT_ADDR_REG, CRT_CURSOR_L);
    pos |= inb(CRT_DATA_REG);
    // 由于一个字符需要两个字节
    pos <<= 1;
    pos += MEM_BASE;

    // 下面去获取x和y
    get_screen();  // 获得screen
    u32 delta = (pos - screen) >> 1;
    x = delta % WIDTH;
    y = delta / WIDTH;
}

static void set_cursor() {
    outb(CRT_ADDR_REG, CRT_CURSOR_H);
    outb(CRT_DATA_REG, ((pos - MEM_BASE) >> 9) & 0xff);
    outb(CRT_ADDR_REG, CRT_CURSOR_L);
    outb(CRT_DATA_REG, ((pos - MEM_BASE) >> 1) & 0xff);
}

void console_clear() {

}

void console_init() {
    get_screen();
    // 这里我们让screen在当前基础上往后移动了一行, 因为一行80字符, 每个字符需要两个字节
    // 也就是此时到达了下一行的开始位置, 因为screen代表的是屏幕左上角字符的内存位置
    // 也就是下一行的开始位置是要翻滚到屏幕左上角
    // 也就是原来的第一行被翻滚了, 原来的第二行要成为第一行
    screen += 160;
    set_screen();

    pos = 10 * 2 + screen;
    set_cursor();
}
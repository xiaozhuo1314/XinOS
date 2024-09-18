#include "xinos/console.h"
#include "xinos/io.h"
#include "xinos/string.h"

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
#define MEM_SIZE 0x4000                                                     // 显示内存大小
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
 * 这个待写入字符的内存位置就是screen, 也就是screen是当前屏幕左上角在内存中的位置
 * 然后我们在屏幕中又写了很多字, 但是没有翻滚
 * 那么此时screen还是左上角字符内存中的位置
 * 而此时的下一个待写入字符的内存位置就是pos
 */
static u32 screen;
static u32 pos;
static int x, y;                             // 当前光标的坐标, 不是内存中的位置, 而是当前屏幕上的坐标
static u8 attr = 7;                  // 字符样式
static u16 erase = 0x0720;    // 带有样式的空格, 因为前一个字节是字符空格0x20, 后一个字节是样式0x07, 而从第一个字符开始的内存地址是增长的, 所以0x07是在高位的

// 使用按键进行蜂鸣
extern void start_beep();

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
    screen = pos = MEM_BASE;
    x = y = 0;
    set_screen();
    set_cursor();

    // 由于mem_base到mem_end之间可能刚才写了一些字符, 此时需要设置为空格
    u16 *ptr = (u16*)MEM_BASE;
    while(ptr < (u16*)MEM_END) {
        *ptr++ = erase;
    }
}

// 退格, 也就是删除一个字符
static void command_bs() {
    if(x) {  // 如果当前行的字符数字大于0
        x--;
    } else {  // 等于0
        if(y == 0 || pos == screen) {
            return;
        }
        x = WIDTH - 1;
        y--;
    }
    // 设置被删除字符的位置为空格
    pos -= 2;
    *(u16*)pos = erase;
}

// del键
static void command_del() {
    *(u16*)pos = erase;
}

// \r, 就是回到开头的位置
static void command_cr() {
    pos -= (x << 1);
    x = 0;
}

// 屏幕滚动一行
static void scroll_up() {
    if(screen + SCR_SIZE + ROW_SIZE >= MEM_END) {
        // 由于此时没有内存了, 就需要将当前屏幕上的内容拷贝到内存开始位置
        memcpy((void*)MEM_BASE, (void*)screen, SCR_SIZE);
        pos -= (screen - MEM_BASE);
        screen = MEM_BASE;
        // 重新获取此时的x和y
        u32 delta = (pos - screen) >> 1;
        x = delta % WIDTH;
        y = delta / WIDTH;
        set_cursor();
    }
    if(y + 1 >= HEIGHT) {
        // 把下面的一行清空
        u16 *ptr = (u16*)(screen + SCR_SIZE);
        for(size_t i = 0; i < WIDTH; ++i) {
            *ptr++ = erase;
        }
        screen += ROW_SIZE;
    }
    // 由于向上翻滚了一行, 所以需要y减一
    y--;
    set_screen();
}

// \n, 就是当前光标位置的下一行的对应位置
static void command_lf() {
    if(y + 1 >= HEIGHT) {
        scroll_up();
    }
    y++;
    pos += ROW_SIZE;
}

// 默认字符
static void command_default(char c) {
    char *ptr = (char*)pos;
    // 写入字符和样式
    *ptr++ = c;
    *ptr++ = attr;
    // 更新光标位置, 此时只是更新了数值
    pos += 2;
    // 更新x和y
    ++x;
    if(x >= WIDTH) {
        x -= WIDTH;
        if(y + 1 >= HEIGHT) {
            scroll_up();
        }
        y++;
    }
}

// 往屏幕写, 其实用汇编写更好, 但是这里用C语言来写
void console_write(char *buf, u32 count) {
    char c;
    while(count--) {
        c = *buf++;
        if(c == EOS) {
            break;
        }
        switch (c)
        {
            case ASCII_NUL:
                break;
            case ASCII_ENQ:
                break;
            case ASCII_BEL:
                start_beep();
                break;
            case ASCII_BS:
                command_bs();
                break;
            case ASCII_HT:
                break;
            case ASCII_LF:
                command_lf();
                command_cr();
                break;
            case ASCII_VT:
                break;
            case ASCII_FF:
                command_lf();
                break;
            case ASCII_CR:
                command_cr();
                break;
            case ASCII_DEL:
                command_del();
                break;
            default:
                command_default(c);
                break;
        }
    }
    // 光标移到后面
    set_cursor();
}

void console_init() {
    console_clear();
}
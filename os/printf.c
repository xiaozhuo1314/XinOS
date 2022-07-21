#include "os.h"

static char out_buf[1024];

/* 参考 https://github.com/cccriscv/mini-riscv-os/blob/master/05-Preemptive/lib.c */
static int _vsnprintf(char * out, size_t n, const char* s, va_list vl) {
    int format = 0, longarg = 0;
    size_t pos = 0;
    for (; *s; s++)
    {
        if(format) 
        {
            switch (*s)
            {
            case 'd': // %d
            {
                long num = (longarg ? va_arg(vl, long) : va_arg(vl, int));
                // 负数的时候,就转换为正数,然后在缓冲区加一个负号即可
                if(num < 0)
                {
                    num = -num;
                    if(out && pos < n)
                    {
                        out[pos] = '-';
                    }
                    ++pos;
                }
                int digits = 0; // num是几位数,如三位数、四位数等
                for (long i = num; i != 0; i /= 10, ++digits);
                for (int i = digits - 1; i >= 0; --i)
                {
                    if(out && pos + i < n)
                    {
                        out[pos + i] = '0' + (num % 10);
                    }
                    num /= 10;
                }
                pos += digits;
                longarg = 0;
                format = 0;
                break;
            }
            case 'l': // %ld
            {
                longarg = 1; // 因为后面还有个d,所以放到d那里去处理了
                break;
            }
            case 'p': // %p
            {
                longarg = 1; // p也是一个long,但是前面得加上0x
                if(out && pos < n) 
                {
                    out[pos] = '0';
                }
                ++pos;
                if(out && pos < n)
                {
                    out[pos] = 'x';
                }
                ++pos;
                // 不加break,是因为需要让%p向下执行%x十六进制的程序,这样就能打印出十六进制的地址
            }
            case 'x': // %x十六进制
            {
                long num = (longarg ? va_arg(vl, long) : va_arg(vl, int));
                // 每四个bit为一个十六进制位,所以一个字节就是2个十六进制位,例如int就是8个十六进制位
                // 所以int的十六进制位占据pos + 0-7这8个位置
                int hex_digits = 2 * (longarg ? sizeof(long) : sizeof(int));
                for (int i = hex_digits - 1; i >= 0; --i)
                {
                    int d = ((num >> (4 * i)) & 0xF);
                    if(out && pos < n)
                    {
                        out[pos] = (d < 10 ? '0' + d : 'a' + d - 10);
                    }
                    ++pos;
                }
                longarg = 0;
                format = 0;
                break;
            }
            case 's':  // %s
            {
                const char *str = va_arg(vl, const char*);
                while(*str)
                {
                    if(out && pos < n)
                    {
                        out[pos] = *str;
                    }
                    ++pos;
                    ++str;
                }
                longarg = 0;
                format = 0;
                break;
            }
            case 'c': // %c
            {
                if(out && pos < n)
                {
                    out[pos] = *s;
                }
                ++pos;
                longarg = 0;
                format = 0;
                break;
            }
            default: //浮点型先不搞
                break;
            }
        } 
        else if(*s == '%') // 开始格式化输出
        {
            format = 1;
        }
        else
        {
            if(out && pos < n)
            {
                out[pos] = *s;
            }
            ++pos;
        }
    }
    if(out && pos < n)
    {
        out[pos] = 0;
    }
    else if(out && n)
    {
        out[n - 1] = 0;
    }
    return pos;
}

static int _vprintf(const char *s, va_list vl)
{
    // 获取格式化之后的打印字符串的总长度(不包括最后的0)
    // 格式化之后的打印字符串的总长度是指将%d等格式已经转换为具体数字或字符之后的总长度(不包括最后的0)
    int res = _vsnprintf(NULL, -1, s, vl);
    if(res + 1 >= sizeof(out_buf)) //需要将最后的0也放入到out_buf中,所以res要+1
    {
        uart_puts("error: output string size overflow\n");
        while(1);
    }
    _vsnprintf(out_buf, res + 1, s, vl);
    uart_puts(out_buf);
    return res;
}

/* s代表的是还未格式化的字符串,也就是printf的第一个参数 */
int printf(const char *s, ...)
{
    int res = 0;
    va_list vl;
    va_start(vl, s);
    res = _vprintf(s, vl);
    va_end(vl);
    return res;
}

void panic(char *s)
{
    printf("panic: ");
    printf(s);
    printf("\n");
    while(1);
}
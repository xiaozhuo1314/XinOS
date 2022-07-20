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
            case 'l': // %ld
                longarg = 1; // 因为后面还有个d,所以放到d那里去处理了
                break;
            case 'p': // %p
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
                break;
            case 'x': // %x十六进制
                break;
            case 's':  // %s
                break;
            case 'c': // %c
                break;
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
    
}
#ifndef ASSERT_H
#define ASSERT_H

/**
 * exp表示表达式
 * file表示出现问题的是在哪个文件
 * base表示主文件
 * line表示哪一行
 */
void assertion_failure(char *exp, char *file, char *base, int line);

/**
 * 断言的宏
 * 可以采用assert(3 < 5)这种方式
 */
#define assert(exp) \
    if(exp)         \
        ;           \
    else            \
        assertion_failure(#exp, __FILE__, __BASE_FILE__, __LINE__)

/**
 * 用于停止cpu运行
 */
void panic(const char *fmt, ...);
#endif
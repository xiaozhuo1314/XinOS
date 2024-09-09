#ifndef STDARG_H
#define STDARG_H

typedef int acpi_native_int;

// 用于遍历参数
typedef char* va_list;

// 向上取整的数
#define _AUPBND (sizeof(acpi_native_int) - 1)
// 向下取整的数, 这里_AUPBND和_ADNBND是相等的
#define _ADNBND (sizeof(acpi_native_int) - 1)

// 向下取整, 即向低字节取整
#define _bnd(X, bnd) (((sizeof(X)) + (bnd)) & (~(bnd)))

/**
 * ap表示va_list类型的变量, 因为是start, 所以ap现在并无指向
 * A表示第一个参数, 例如printf(format, ...)中的format
 * A表示的第一个参数, 由于栈是从高地址往低地址增长
 * 因此第二个参数的内存位置比A的内存位置高
 * 所以是加上A的大小取整
 */
#define va_start(ap, A) (void) ((ap) = (((char*)&(A)) + (_bnd(A, _AUPBND))))

/**
 * 遍历的arg
 * ap表示va_list类型的变量, 已经有了指向
 * T表示类型
 */
#define va_arg(ap, T) (*(T*) (((ap) += (_bnd(T, _AUPBND))) - (_bnd(T, _ADNBND))))

/**
 * 结束
 */
#define va_end(ap) ((ap) = (va_list)0)

#endif
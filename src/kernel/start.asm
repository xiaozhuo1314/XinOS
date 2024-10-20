[bits 32]

extern kernel_init
extern console_init
extern memory_init
extern gdt_init

; 链接器默认的起始程序名是_start, 所以在ENTRYPOINT位置处的就是_start
global _start
_start:
    push ebx  ; ards结构体数量的内存地址放到栈中, 当作函数的参数
    push eax  ; 魔数
    call console_init  ; 调用控制台初始化函数
    call gdt_init  ; 调用gdt初始化函数
    ; 这里要注意的是, 魔数和ards结构体数量是在memory_init中使用的
    ; 虽然我们在上一行调用了console_init, 但是console_init函数执行完后两个参数还在栈中
    ; 因此在执行memory_init的时候是可以当作参数使用的
    call memory_init
    call kernel_init
    jmp $
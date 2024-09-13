[bits 32]

extern kernel_init

; 链接器默认的起始程序名是_start, 所以在ENTRYPOINT位置处的就是_start
global _start
_start:
    call kernel_init
    ; 调用中断处理函数
    int 0x80
    ; mov bx, 0
    ; div bx
    jmp $
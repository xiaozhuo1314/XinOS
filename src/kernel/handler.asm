[bits 32]

; 引入外部处理函数表
extern handler_table
; 标明是代码
section .text

; 使用宏, 表示宏为INTERRUPT_HANDLER, 2表示参数个数,需要两个参数%1和%2
%macro INTERRUPT_HANDLER 2
interrupt_handler_%1:
; 发生异常的时候, 某些异常cpu会帮我们压入一个错误码, 但是有些不会
; 就是当第二个参数为0的时候不会, 因此当第二个参数为0的时候, 我们手动压入一个
; 使得处理函数的逻辑一致
%ifn %2
    push 0x20180719
%endif
    ; 错误码是cpu帮我们压入, 0x20180719是我们自己压入, 这两个都是第二个参数, 那么此时要去压入第一个参数
    push %1
    ; 跳转到处理入口
    jmp interrupt_entry
%endmacro

interrupt_entry:
    ; 我们起始并不需要错误码或者0x20180719, 而是需要中断号就可以, 中断号此时就在栈顶
    mov eax, [esp]
    ; 调用终端处理函数, handler_table中每个函数地址都占用4个字节
    call [handler_table + eax * 4]
    ; 调用完后回复栈, 就是去掉两个参数
    add esp, 8
    ; 返回, 要用iret
    iret

; 宏展开, 就得到了每一个interrupt_handler_%1函数
INTERRUPT_HANDLER 0x00, 0; divide by zero
INTERRUPT_HANDLER 0x01, 0; debug
INTERRUPT_HANDLER 0x02, 0; non maskable interrupt
INTERRUPT_HANDLER 0x03, 0; breakpoint
INTERRUPT_HANDLER 0x04, 0; overflow
INTERRUPT_HANDLER 0x05, 0; bound range exceeded
INTERRUPT_HANDLER 0x06, 0; invalid opcode
INTERRUPT_HANDLER 0x07, 0; device not avilable
INTERRUPT_HANDLER 0x08, 1; double fault
INTERRUPT_HANDLER 0x09, 0; coprocessor segment overrun
INTERRUPT_HANDLER 0x0a, 1; invalid TSS
INTERRUPT_HANDLER 0x0b, 1; segment not present
INTERRUPT_HANDLER 0x0c, 1; stack segment fault
INTERRUPT_HANDLER 0x0d, 1; general protection fault
INTERRUPT_HANDLER 0x0e, 1; page fault
INTERRUPT_HANDLER 0x0f, 0; reserved
INTERRUPT_HANDLER 0x10, 0; x87 floating point exception
INTERRUPT_HANDLER 0x11, 1; alignment check
INTERRUPT_HANDLER 0x12, 0; machine check
INTERRUPT_HANDLER 0x13, 0; SIMD Floating - Point Exception
INTERRUPT_HANDLER 0x14, 0; Virtualization Exception
INTERRUPT_HANDLER 0x15, 1; Control Protection Exception
INTERRUPT_HANDLER 0x16, 0; reserved
INTERRUPT_HANDLER 0x17, 0; reserved
INTERRUPT_HANDLER 0x18, 0; reserved
INTERRUPT_HANDLER 0x19, 0; reserved
INTERRUPT_HANDLER 0x1a, 0; reserved
INTERRUPT_HANDLER 0x1b, 0; reserved
INTERRUPT_HANDLER 0x1c, 0; reserved
INTERRUPT_HANDLER 0x1d, 0; reserved
INTERRUPT_HANDLER 0x1e, 0; reserved
INTERRUPT_HANDLER 0x1f, 0; reserved

; 现在我们有了每一个interrupt_handler_%1函数, 但是需要一个表来统一管理
; 表中的每一个interrupt_handler_%1指针都是4字节
section .data
global handler_entry_table
handler_entry_table:
    dd interrupt_handler_0x00
    dd interrupt_handler_0x01
    dd interrupt_handler_0x02
    dd interrupt_handler_0x03
    dd interrupt_handler_0x04
    dd interrupt_handler_0x05
    dd interrupt_handler_0x06
    dd interrupt_handler_0x07
    dd interrupt_handler_0x08
    dd interrupt_handler_0x09
    dd interrupt_handler_0x0a
    dd interrupt_handler_0x0b
    dd interrupt_handler_0x0c
    dd interrupt_handler_0x0d
    dd interrupt_handler_0x0e
    dd interrupt_handler_0x0f
    dd interrupt_handler_0x10
    dd interrupt_handler_0x11
    dd interrupt_handler_0x12
    dd interrupt_handler_0x13
    dd interrupt_handler_0x14
    dd interrupt_handler_0x15
    dd interrupt_handler_0x16
    dd interrupt_handler_0x17
    dd interrupt_handler_0x18
    dd interrupt_handler_0x19
    dd interrupt_handler_0x1a
    dd interrupt_handler_0x1b
    dd interrupt_handler_0x1c
    dd interrupt_handler_0x1d
    dd interrupt_handler_0x1e
    dd interrupt_handler_0x1f
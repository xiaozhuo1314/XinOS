[bits 32]

; 标明是代码
section .text
; 获取printk
extern printk

; 声明interrupt_handler
global interrupt_handler

interrupt_handler:
    ; 将参数压栈
    push message
    ; 调用printk, 由于前一步压栈了, 因此printk知道了参数, 且call指令还会把返回地址压栈, ret指令会将返回地址出栈
    call printk
    ; 将参数弹出
    add esp, 4
    ; 由于是中断处理函数, 因此需要使用iret
    iret

; 声明数据
section .data
message:
    db "default interrupt", 10, 0
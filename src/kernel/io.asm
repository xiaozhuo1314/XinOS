[bits 32]

section .text ; 代码段

; global的意思是将函数导出, 供io.h中的extern u8 inb(u16 port)链接
; 实现io.h中的extern u8 inb(u16 port)
global inb
inb:
    ; 将栈基址压栈
    push ebp
    ; 将esp地址赋给ebp, 这样就形成了新栈桢
    mov ebp, esp
    ; 将eax清空
    xor eax, eax
    ; 获取参数, 参数是在当前栈的高地址处, 因为栈是高地址往低地址生长
    ; 因为先是参数, 然后是返回地址, 然后是原ebp, 此时现ebp指向原ebp的低地址
    ; 所以参数port的低地址是在ebp + 8的位置, 寄存器取数据是往高地址取, 因此此时就可以了
    mov edx, [ebp + 8]
    ; 将dx代表的端口号中的8bit输入到ax寄存器, al是ax的低8位, ax是16位的
    ; 也就是从dx代表的端口号读取8bit
    in al, dx
    ; 一点点延迟
    jmp $+2
    jmp $+2
    jmp $+2
    ; 恢复原来的栈桢
    leave
    ; 返回函数
    ret

global inw
inw:
    ; 将栈基址压栈
    push ebp
    ; 将esp地址赋给ebp, 这样就形成了新栈桢
    mov ebp, esp
    ; 将eax清空
    xor eax, eax
    ; 获取参数
    mov edx, [ebp + 8]
    ; 读取数据
    in ax, dx
    ; 一点点延迟
    jmp $+2
    jmp $+2
    jmp $+2
    ; 恢复原来的栈桢
    leave
    ; 返回函数
    ret

global outb
outb:
    ; 将栈基址压栈
    push ebp
    ; 将esp地址赋给ebp, 这样就形成了新栈桢
    mov ebp, esp
    ; 获取port
    mov edx, [ebp + 8]
    ; 获取value
    mov eax, [ebp + 12]
    ; 输出数据
    out dx, al
    ; 一点点延迟
    jmp $+2
    jmp $+2
    jmp $+2
    ; 恢复原来的栈桢
    leave
    ; 返回函数
    ret

global outw
outw:
    ; 将栈基址压栈
    push ebp
    ; 将esp地址赋给ebp, 这样就形成了新栈桢
    mov ebp, esp
    ; 获取port
    mov edx, [ebp + 8]
    ; 获取value
    mov eax, [ebp + 12]
    ; 输出数据
    out dx, ax
    ; 一点点延迟
    jmp $+2
    jmp $+2
    jmp $+2
    ; 恢复原来的栈桢
    leave
    ; 返回函数
    ret


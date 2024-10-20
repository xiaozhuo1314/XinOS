[bits 32]
section .text

global task_switch

; task_switch最重要的就是将ip指针指向其他任务的要执行的指令的地址
task_switch:
    ; 函数需要原有的栈底存起来, 用于后面下一次到达当前任务的时候恢复这些寄存器
    push ebp
    ; 将esp的值赋给ebp
    mov ebp, esp
    ; 保存其他寄存器
    push ebx
    push esi
    push edi
    ; 将esp的值赋给eax
    mov eax, esp
    ; 当前栈顶4k对齐, 这样就可以获得栈顶所在页的低地址
    and eax, 0xfffff000
    ; 将当前栈顶的位置写入到eax指向的内存区域, 也就是task的stack位置
    ; 此时stack处就是当前任务进行到的栈顶的位置, 不是任务创建时候的那个stack
    ; 这样后面恢复的时候, 就可以通过下面的语句进行恢复
    mov [eax], esp
    ; 获取参数, 也就是下一个任务的地址, ebp + 8表示参数在内存中的地址, [ebp + 8]表示取参数值
    mov eax, [ebp + 8]
    ; 将栈顶指向下一个任务的上一次进行到的栈顶的位置
    mov esp, [eax]
    ; 这个时候esp指向了任务的上一次栈顶处, 因此就把它当时的edi-ebp这四个弹出去了, 然后下一个内存位置就是eip的值
    ; 因为我们创建任务时把eip存储的是task函数的地址, 因此通过ret把当前esp指向的内存的值(就是frame结构体中的eip)
    ; 弹出到eip寄存器就可以切换任务了
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret
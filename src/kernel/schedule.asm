global task_switch

; task_switch最重要的就是将ip指针指向其他任务的要执行的指令的地址
task_switch:
    ; 函数需要原有的栈底存起来
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
    ; 将当前栈顶的位置写入到eax指向的内存区域
    mov [eax], esp
    ; 获取参数, 也就是下一个任务的地址, ebp + 8表示参数在内存中的地址, [ebp + 8]表示取参数值
    mov eax, [ebp + 8]
    ; 将栈顶指向下一个任务的地址
    mov esp, [eax]
    ; 弹出之前保存的
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret
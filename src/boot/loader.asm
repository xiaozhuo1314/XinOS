[org 0x1000]

; 用于判断错误, 就是内核加载器加载到0x1000位置,
; 本文件就是内核加载器的程序
; 这里我们人为设置了内核加载器起始位置的字节是0x55aa
; 如果后续判断的时候该位置处不是0x55aa , 那么就是错误的
dw 0x55aa

mov si, loading
call print

; 内存检测
detect_memory:
    xor ebx, ebx ; ebx置为0
    ; es:di是结构体的缓存位置
    mov ax, 0
    mov es, ax  ; 特殊寄存器不能直接置0
    mov edi, ards_buffer ; 这里其实就设置了di
    mov edx, 0x534d4150  ; 固定签名
.next:
    ; 子功能号
    mov eax, 0xe820
    ; ards结构的大小(字节)
    mov ecx, 20
    ; 调用中断
    int 0x15
    ; 如果cf置位为1, 就跳转到error
    jc error
    ; 否则就将缓存的指针指向下一个, 其实就是edi往后移动20字节
    ; di是edi的低16位, cx是ecx的低16位
    add di, cx
    ; 将结构体的数量加一
    inc word [ards_count]
    ; 如果ebx为0的话, 说明检测结束, 否则应该继续.next
    cmp ebx, 0
    jnz .next

    ; 检测结束
    mov si, detecting
    call print

    ; 下面去查看结构体内容, 也就是看一下内存有没有问题
    ; 结构体数量
    mov cx, [ards_count]
    ; 结构体指针
    mov si, 0
.show:
    ; 将基地址写到eax
    mov eax, [ards_buffer + si] ; 结构体中的低8字节为内存基地址
    mov ebx, [ards_buffer + si + 8] ; 结构体的[8, 16)为内存长度
    mov edx, [ards_buffer + si + 16] ; 结构体的[16, 20)为内存类型, 由于此时ecx已经存储了ards_count, 所以不能用
    add si, 20
    xchg bx, bx ; bochs断点
    ; loop在循环的时候, 每经过一次循环就会将ecx中的值减一, 直到ecx中的值变为0
    loop .show

; 阻塞
jmp $

print:
    mov ah, 0x0e
.next:
    mov al, [si]
    cmp al, 0
    jz .done
    int 0x10
    inc si
    jmp .next
.done:
    ret

loading:
    db "Loading XinOS...", 10, 13, 0; \n\r

detecting:
    db "Detecting Memory Success...", 10, 13, 0

error:
    mov si, .msg
    call print
    hlt ; 让cpu停止
    jmp $
    .msg db "loading Error!!!", 10, 13, 0; \n\r

ards_count:
    dw 0

ards_buffer:
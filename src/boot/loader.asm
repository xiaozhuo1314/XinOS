[org 0x1000]

; 用于判断错误, 就是内核加载器加载到0x1000位置,
; 本文件就是内核加载器的程序
; 这里我们人为设置了内核加载器起始位置的字节是0x55aa
; 如果后续判断的时候该位置处不是0x55aa , 那么就是错误的
dd 0x55aa

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
    ; 检测结束后跳转到准备保护模式代码
    jmp prepare_protected_mode

; 全局描述符表最多有8192个元素(每个元素8字节), 也就是下标从0 - 8191
; 但是第0个元素必须设置为0
; 然后我们这里其实只用两个就可以了, 一个描述符描述代码段, 另一个描述符描述数据段
; 也就是下面的gdt_code和gdt_data
prepare_protected_mode:
    xchg bx, bx
    ; 关闭中断
    cli
    ; 打开A20线
    in al, 0x92
    or al, 0b10
    out 0x92, al
    ; 加载gdt
    lgdt [gdt_ptr]  ; gdt_ptr是后面写的
    ; 启动保护模式
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    ; 用跳转来刷新缓存,  虽然前面改了cr0启动了, 但是缓存还没刷新, 所以用jmp去刷新缓存,启用保护模式
    jmp dword code_selector:protect_mode  ; code_selector是后面写的

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
    .msg db "Loading Error!!!", 10, 13, 0; \n\r

[bits 32]  ; 已经到了32位
protect_mode:
    xchg bx, bx
    ; 初始化段寄存器为数据段选择子
    mov ax, data_selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    xchg bx, bx
    ; 修改栈顶, 0x10000是要在可用区域, 也就是操作系统加载的位置
    mov esp, 0x10000
    ; 保护模式后可以修改1M以外的内存
    mov byte [0xb8000], 'P'
    mov byte [0x200000], 'P'
jmp $

; 代码段和数据段选择子
code_selector equ (1 << 3)  ; 下标为1的是代码段选择子, 前三位不是下标值
data_selector equ (2 << 3)  ; 下标为2的是数据段选择子, 前三位不是下标值

; 内存开始的位置
memory_base equ 0
; 内存长度/界限, 以4K为粒度, 也就是每4K作为一个数组元素, 下标最多是4GB / 4KB - 1
memory_limit equ ((1024 * 1024 * 1024 * 4) / (1024 * 4)) - 1 ; 4GB / 4KB - 1

; 描述符指针
gdt_ptr:
    dw (gdt_end - gdt_base) - 1 ; size - 1
    dd gdt_base

gdt_base:
    ; 一个描述符占8个字节
    dd 0, 0 ; 第一个描述符的8个字节都为0, dd是4字节
gdt_code:
    ; 段界限0~15位
    dw memory_limit & 0xffff
    ; 基地址0~15位
    dw memory_base & 0xffff
    ; 基地址16~23位
    db (memory_base >> 16) & 0xff
    ; 一些数据, 由于db、dw这些是从高地址排下来的, 因此我们应该先看高地址的含义
    ; 存在 - dpl 0 - 代码段或数据段 - 代码段 - 非依从 - 可读 - 未被访问过
    db 0b_1_00_1_1_0_1_0
    ; 粒度4K - 32位 - 非64位 - 0 - 段界限16~19
    db 0b1_1_0_0_0000 | ((memory_limit >> 16) & 0xf)
    ; 基地址的24~31
    db (memory_base >> 24) & 0xff
gdt_data:
    ; 段界限0~15位
    dw memory_limit & 0xffff
    ; 基地址0~15位
    dw memory_base & 0xffff
    ; 基地址16~23位
    db (memory_base >> 16) & 0xff
    ; 一些数据, 由于db、dw这些是从高地址排下来的, 因此我们应该先看高地址的含义
    ; 存在 - dpl 0 - 代码段或数据段 - 数据段 - 向上 - 可写 - 未被访问过
    db 0b_1_00_1_0_0_1_0
    ; 粒度4K - 32位 - 非64位 - 0 - 段界限16~19
    db 0b1_1_0_0_0000 | ((memory_limit >> 16) & 0xf)
    ; 基地址的24~31
    db (memory_base >> 24) & 0xff
gdt_end:

ards_count:
    dw 0

ards_buffer:
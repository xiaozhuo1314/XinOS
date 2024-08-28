[org 0x7c00] ;本程序在内存中的位置, bios会跳转到这个位置

mov ax, 3 ;设置屏幕为文本模式
int 0x10 ;bios中断, 会根据ax的值选择不同的调用程序, 当ax=3时, 会输出80*25分辨率的文字, 颜色为16

; 初始化段寄存器
mov ax, 0
mov ds, ax
mov es, ax
mov ss, ax
mov sp, 0x7c00 ;将栈指针指向0x7c00

;0xB8000 - 0xBFFFF共32KB, 是文本显示器的内存区域, 而实模式下寻址是ds<<4 + 16位偏移地址
; 因此ds应该设置成0xb800, 偏移地址为0
;mov ax, 0xb800
;mov ds, ax  ;设置段寄存器为0xb800
;mov byte ds:[0], 'H'  ; 偏移地址设置为了0, 就是[0]中的0

mov si, booting
call print

mov edi, 0x1000; 读取的目标内存
mov ecx, 2; 内核加载器的起始扇区, 要跟dd命令对应
mov bl, 4; 内核加载器的扇区数量, 要跟dd命令对应

call read_disk

; 比较0x1000是否是0x55aa, ds是数据段寄存器
cmp word ds:[0x1000], 0x55aa
jnz error
; 否则就跳转到代码段的0x1002的位置
; 也就是跳过loader的前两个魔数
jmp 0:0x1002

; 阻塞
jmp $

read_disk:
    ; 设置读写扇区的数量
    mov dx, 0x1f2
    mov al, bl
    out dx, al

    inc dx; 0x1f3
    mov al, cl; 起始扇区的前八位
    out dx, al

    inc dx; 0x1f4
    shr ecx, 8
    mov al, cl; 起始扇区的中八位
    out dx, al

    inc dx; 0x1f5
    shr ecx, 8
    mov al, cl; 起始扇区的高八位
    out dx, al

    inc dx; 0x1f6
    shr ecx, 8
    and cl, 0b1111; 将高四位置为 0

    mov al, 0b1110_0000;
    or al, cl
    out dx, al; 主盘 - LBA 模式

    inc dx; 0x1f7
    mov al, 0x20; 读硬盘
    out dx, al

    xor ecx, ecx; 将 ecx 清空
    mov cl, bl; 得到读写扇区的数量

    .read:
        push cx; 保存 cx
        call .waits; 等待数据准备完毕
        call .reads; 读取一个扇区
        pop cx; 恢复 cx
        loop .read

    ret

    .waits:
        mov dx, 0x1f7
        .check:
            in al, dx
            jmp $+2; nop 直接跳转到下一行
            jmp $+2; 一点点延迟
            jmp $+2
            and al, 0b1000_1000
            cmp al, 0b0000_1000
            jnz .check
        ret

    .reads:
        mov dx, 0x1f0
        mov cx, 256; 一个扇区 256 字
        .readw:
            in ax, dx
            jmp $+2; 一点点延迟
            jmp $+2
            jmp $+2
            mov [edi], ax
            add edi, 2
            loop .readw
        ret

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

booting:
    db "Booting XinOS...", 10, 13, 0; \n\r

error:
    mov si, .msg
    call print
    hlt ; 让cpu停止
    jmp $
    .msg db "Booting Error!!!", 10, 13, 0; \n\r

;由于主引导程序得有512字节, 最后的两个字节是两个魔数, 因此就要求到这一行语句的时候得有510个字节,
;所以这里就使用0填充
; 512字节分为代码446B, 硬盘分区表有64B=4*16B, 也就是最多有4个主分区, 最后两个是魔数
; $表示当前位置, $$表示开头位置, $ - $$表示从开头到这里有多少字节的数据
times 510 - ($ - $$) db 0

;bios要求主引导程序的512字节的最后两个字节是0x55aa
db 0x55, 0xaa
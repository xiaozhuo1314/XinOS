[bits 32]

; 链接器默认的起始程序名是_start, 所以在ENTRYPOINT位置处的就是_start
global _start
_start:
    mov byte [0xb8000], 'K'
    jmp $
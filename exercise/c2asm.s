# C all ASM

    .text
    .global _start
    .global foo

_start:
    la sp, stack_end
    li a0, 1
    li a1, 2
    call foo

stop:
    j stop

    nop

stack_start:
    .rept 12
    .word 0
    .endr
stack_end:

    .end

# asm code for c
# int square(int num) {
#     return num * num;
# }
# void _start() {
#     square(3);
# }

    .text
    .global _start

_start:
    la sp, stack_end
    li a0, 3
    call square

stop:
    j stop

square:
    # prologue
    addi sp, sp, -8
    sw s0, 0(sp)
    sw s1, 4(sp)

    # code block
    mv s0, a0
    mul s1, s0, s0
    mv a0, s1

    # epilogue
    lw s0,  0(sp)
    lw s1, 4(sp)
    addi sp, sp, 8
    ret

    nop # add nop here just for demo in gdb

stack_start:
    .rept 12
    .word 0
    .endr
stack_end:

    .end

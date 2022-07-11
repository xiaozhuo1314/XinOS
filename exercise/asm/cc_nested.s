# asm code for c
# int square(int num) {
#     return num * num;
# }
#
# int aa_bb(int n1, int n2) {
#     return square(n1) + square(n2);
# }
#
# void _start() {
#     aa_bb(3, 4);
# }

    .text
    .global _start

_start:
    la sp, stack_end
    li a0, 3
    li a1, 4
    call aa_bb
    
stop:
    j stop

aa_bb:
    # prologue
    addi sp, sp, -16
    sw s0, 0(sp)
    sw s1, 4(sp)
    sw s2, 8(sp)
    sw ra, 12(sp)

    # code block
    mv s0, a0
    mv s1, a1
    li s2, 0

    mv a0, s0
    jal ra, square
    add s2, s2, a0

    mv a0, s1
    jal ra, square
    add s2, s2, a0

    mv a0, s2

    # epilogue
    lw s0, 0(sp)
    lw s1, 4(sp)
    lw s2, 8(sp)
    lw ra, 12(sp)
    addi sp, sp, 16

    ret

    nop # add nop here just for demo in gdb

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
    lw s0, 0(sp)
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

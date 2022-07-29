# 本文件用于保存/重载入所有通用寄存器,用于上下文切换
# call switch_to函数的时候,硬件会使ra寄存器存储调用switch_to函数后下一条指令的地址

# 保存寄存器值到内存中,base为所要保存到的内存中的目的地址
.macro reg_save base
    # 这里还有个x0寄存器没有保存,由x0寄存器的值一直是0,所以不需要保存
    sw ra, 0(\base)
	sw sp, 4(\base)
	sw gp, 8(\base)
	sw tp, 12(\base)
	sw t0, 16(\base)
	sw t1, 20(\base)
	sw t2, 24(\base)
	sw s0, 28(\base)
	sw s1, 32(\base)
	sw a0, 36(\base)
	sw a1, 40(\base)
	sw a2, 44(\base)
	sw a3, 48(\base)
	sw a4, 52(\base)
	sw a5, 56(\base)
	sw a6, 60(\base)
	sw a7, 64(\base)
	sw s2, 68(\base)
	sw s3, 72(\base)
	sw s4, 76(\base)
	sw s5, 80(\base)
	sw s6, 84(\base)
	sw s7, 88(\base)
	sw s8, 92(\base)
	sw s9, 96(\base)
	sw s10, 100(\base)
	sw s11, 104(\base)
	sw t3, 108(\base)
	sw t4, 112(\base)
	sw t5, 116(\base)
    # 其实还有一个寄存器t6,但是这里没有保存,因为在reg_save函数中base就是t6寄存器
    # 所以t6寄存器的值需要在外部的switch_to函数中保存到mscratch寄存器中
    # 然后在switch_to函数中将t6保存到t5的后面
.endm

.macro reg_restore base
    # 这里还有个a0寄存器没有载入,由于a0寄存器的值一直是0,所以不需要载入
    lw ra, 0(\base)
	lw sp, 4(\base)
	lw gp, 8(\base)
	lw tp, 12(\base)
	lw t0, 16(\base)
	lw t1, 20(\base)
	lw t2, 24(\base)
	lw s0, 28(\base)
	lw s1, 32(\base)
	lw a0, 36(\base)
	lw a1, 40(\base)
	lw a2, 44(\base)
	lw a3, 48(\base)
	lw a4, 52(\base)
	lw a5, 56(\base)
	lw a6, 60(\base)
	lw a7, 64(\base)
	lw s2, 68(\base)
	lw s3, 72(\base)
	lw s4, 76(\base)
	lw s5, 80(\base)
	lw s6, 84(\base)
	lw s7, 88(\base)
	lw s8, 92(\base)
	lw s9, 96(\base)
	lw s10, 100(\base)
	lw s11, 104(\base)
	lw t3, 108(\base)
	lw t4, 112(\base)
	lw t5, 116(\base)
	lw t6, 120(\base)
.endm

    .text

# trap的处理过程分为四大步: trap初始化 -> trap的top half -> trap的bottom half -> 从trap返回,这四步能够被软件控制的为第一步和第三步
# trap初始化指的是设置trap处理基址,在这里就是trap处理函数的地址,即设置mtvec寄存器
# trap的top half是硬件完成的,包含以下四步：
#   1. 把mstatus寄存器的mie值复制到mstatus寄存器的mpie备份,待将来恢复,并设置mstatus寄存器的mie为0,
#      mstatus寄存器的mie是控制machine模式下的中断总开关,那么此时就关中断了,即不去处理在执行trap函数时发生的中断
#   2. 设置mepc备份pc寄存器,同时pc寄存器设置为mtvec寄存器的值,需要注意的是,
#      异常发生时mepc备份完pc后里面保存的是导致异常指令的地址,即处理完后还是去执行发生异常的语句,
#      中断发生时mepc备份完pc后里面保存的是中断指令的下一条语句,即处理完后就执行中断的下一条语句了,不会再执行中断指令
#   3. 根据trap的种类设置mcause,并根据需要设置mtval附加信息
#   4. 将trap发生前的权限模式保存到mstatus的mpp中,再把hart权限模式改为machine,也就是说无论在何权限模式下触发trap,hart首先切换到machine模式
# trap的bottom half就是自定义的trap处理函数,主要分为五步:
#   1. 保存当前控制流的上下文
#   2. 调用c语言写的trap handler
#   3. 从trap handler函数返回,由于该函数可能返回下一指令的地址,所以mepc寄存器的值可能需要调整
#   4. 恢复上下文信息
#   5. 执行mret指令返回到trap之前的状态(也就是下一步从trap返回)
# 从trap返回与权限级别有关,在不同权限级别下退出trap有各自的返回指令,对于软件而言就是mret、sret、uret,以mret为例,它执行以下三步:
#   1. 当前hart的权限模式从mstatus的mpp中恢复,然后设置mstatus的mpp为user模式
#   2. mstatus.mie从mstatus.mpie恢复,即mstatus.mie=mstatus.mpie,然后设置mstatus.mpie=1
#   3. 恢复pc寄存器为mpec的值
# 中断处理函数入口
	.global trap_vector
	.align 4
trap_vector:
	# 1.保存当前控制流的上下文
	csrrw t6, mscratch, t6 # 将当前上下文的地址保存到t6寄存器,用于保存上下文
	reg_save t6 # 保存时还有个原来的t6寄存器值没有保存,此时需要去保存
	mv t5, t6 # 保存上下文内存地址,用于后面保存t6原有的值
	csrr t6, mscratch # 恢复t6的值
	sw t6, 120(t5) # 保存t6的值
	csrw mscratch, t5 # 将上下文地址恢复到mscratch
	# 2.传递参数并执行c语言写的trap handler
	csrr a0, mepc # mepc为第一个参数,即中断或异常处理函数完后的第一条指令
	csrr a1, mcause # 造成中断或异常的原因
	call trap_handler # 执行c语言函数
	# 3.从trap handler函数返回,并调整mepc
	csrw mepc, a0
	# 4.恢复上下文信息
	csrr t6, mscratch
	reg_restore t6
	# 5.执行mret指令返回到trap之前的状态
	mret

# 切换上下文函数
    # mscratch用于保存内存中存储寄存器值的地址,也就是结构体context在内存中的地址
    # 这里用t6当作reg_restore和reg_save函数的参数原因是,t6是最后一个存储的
    # 可以在switch_to函数中进行设置,当然也可以让t5之类的最后存储
    .global switch_to
    .align 4
switch_to:
    csrrw t6, mscratch, t6 # 互换mscratch和t6的值,此时t6指向了当前结构体context在内存中的地址,mscratch备份了t6的值
    beqz t6, 1f #如果t6不为0,说明前一句汇编执行前mscratch不为0,说明此时是有任务在执行的,否则的话说明没有任务在执行,也就不需要去备份寄存器了
    reg_save t6 # 调用宏,将寄存器的值存储到t6指向的内存中
    # 下面要将t6原来的值存储到内存中,由于t6原来的值备份到了mscratch中,所以需要取出来放到内存中
    # 将mscratch中原t6的值还给t6后,t6就不再指向内存中结构体context的地址,那么现在就没有寄存器指向内存中结构体context地址了
    # 所以需要使用一个寄存器先把t6指向的内存地址保存起来,再去把mscratch中原t6的值还给t6,这里采用t5保存t6指向的内存地址
    # 因为t5的值已经被reg_save宏备份到了内存中了
    mv t5, t6
    csrr t6, mscratch
    sw t6, 120(t5)
1:
    # 1用于将传递过来的新结构体context的地址放到mscratch寄存器中,并将新结构体载入到各个寄存器中
    csrw mscratch, a0
    # t6要作为reg_restore的参数,也就是t6保存的是新结构体context的地址,也就是a0中的值
    mv t6, a0
    reg_restore t6
    # 函数返回,此时切换上下文成功
    ret

.end

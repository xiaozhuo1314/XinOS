/*
 * rvos.ld
 * Linker script for outputting to RVOS
 */

/*
 * https://sourceware.org/binutils/docs/ld/Miscellaneous-Commands.html
 * OUTPUT_ARCH command specifies a particular output machine architecture.
 * "riscv" is the name of the architecture for both 64-bit and 32-bit
 * RISC-V target. We will further refine this by using -march=rv32ima
 * and -mabi=ilp32 when calling gcc.
 */
/* 设置体系架构 */
OUTPUT_ARCH("riscv")

 /*
 * https://sourceware.org/binutils/docs/ld/Entry-Point.html
 * ENTRY command is used to set the "entry point", which is the first instruction
 * to execute in a program.
 * The argument of ENTRY command is a symbol name, here is "_start" which is
 * defined in start.S.
 */
/* 设置程序入口点 */
ENTRY(_start)

 /*
 * https://sourceware.org/binutils/docs/ld/MEMORY.html
 * The MEMORY command describes the location and size of blocks of memory in
 * the target.
 * The syntax for MEMORY is:
 * MEMORY
 * {
 *     name [(attr)] : ORIGIN = origin, LENGTH = len
 *     ......
 * }
 * Each line defines a memory region.
 * Each memory region must have a distinct name within the MEMORY command. Here
 * we only define one region named as "ram".
 * The "attr" string is an optional list of attributes that specify whether to
 * use a particular memory region for an input section which is not explicitly
 * mapped in the linker script. Here we assign 'w' (writeable), 'x' (executable),
 * and 'a' (allocatable). We use '!' to invert 'r' (read-only) and
 * 'i' (initialized).
 * The "ORIGIN" is used to set the start address of the memory region. Here we
 * place it right at the beginning of 0x8000_0000 because this is where the
 * QEMU-virt machine will start executing.
 * Finally LENGTH = 128M tells the linker that we have 128 megabyte of RAM.
 * The linker will double check this to make sure everything can fit.
 */
/* 设置可使用的内存区域 */
MEMORY
{
    ram(wxa!ri) : ORIGIN = 0x80000000, LENGTH = 128M
}

 /*
 * https://sourceware.org/binutils/docs/ld/SECTIONS.html
 * The SECTIONS command tells the linker how to map input sections into output
 * sections, and how to place the output sections in memory.
 * The format of the SECTIONS command is:
 * SECTIONS
 * {
 *     sections-command
 *     sections-command
 *     ......
 * }
 *
 * Each sections-command may of be one of the following:
 * (1) an ENTRY command
 * (2) a symbol assignment
 * (3) an output section description
 * (4) an overlay description
 * We here only demo (2) & (3).
 *
 * We use PROVIDE command to define symbols.
 * https://sourceware.org/binutils/docs/ld/PROVIDE.html
 * The PROVIDE keyword may be used to define a symbol.
 * The syntax is PROVIDE(symbol = expression).
 * Such symbols as "_text_start", "_text_end" ... will be used in mem.S.
 * Notice the period '.' tells the linker to set symbol(e.g. _text_start) to
 * the CURRENT location ('.' = current memory location). This current memory
 * location moves as we add things.
 */
/* 将MEMORY人为分成几节,由于是从低地址往高地址增长,所以要从text开始写,按照text->rodata->data->bss->heap的顺序 */
SECTIONS
{
    /*
    * We are going to layout all text sections in .text output section,
    * starting with .text. The asterisk("*") in front of the
    * parentheses means to match the .text section of ANY object file.
    */
    .text : {
        PROVIDE(_text_start = .); /* 代码段起始位置 */
        *(.text .text.*) /* 输入文件中的.text和.text.*组合到输出文件的.text中 */
        PROVIDE(_text_end = .); /* 代码段终止位置 */
    } > ram

    .rodata : {
        PROVIDE(_rodata_start = .); /* 只读段起始位置 */
        *(.rodata .rodata.*) /* 输入文件中的.rodata和.rodata.*组合到输出文件的.rodata中 */
        PROVIDE(_rodata_end = .); /* 只读段终止位置 */
    } > ram

    .data : {
        /*
        * . = ALIGN(4096) tells the linker to align the current memory
        * location to 4096 bytes. This will insert padding bytes until
        * current location becomes aligned on 4096-byte boundary.
        * This is because our paging system's resolution is 4,096 bytes.
        */
        . = ALIGN(4096); /* 让数据段4K对齐 */
        PROVIDE(_data_start = .); /* 数据段起始位置 */
        /*
		 * sdata and data are essentially the same thing. We do not need
		 * to distinguish sdata from data.
		 */
        *(.sdata .sdata.*) /* 输入文件中的.sdata和.sdata.*组合到输出文件的.data中 */
        *(.data .data.*) /* 输入文件中的.data和.data.*组合到输出文件的.data中 */
        PROVIDE(_data_end = .); /* 数据段终止位置 */
    } > ram

    .bss : {
        /*
		 * https://sourceware.org/binutils/docs/ld/Input-Section-Common.html
		 * In most cases, common symbols in input files will be placed
		 * in the ‘.bss’ section in the output file.
		 */
        PROVIDE(_bss_start = .); /* bss段起始位置 */
        *(.sbss .sbss.*) /* 输入文件中的.sbss和.sbss.*组合到输出文件的.bss中 */
        *(.bss .bss.*) /* 输入文件中的.bss和.bss.*组合到输出文件的.bss中 */
        *(COMMON) /* 在大多数情况下,输入文件(源文件)中的common会被替代为.bss写入到输出文件中 */
        PROVIDE(_bss_end = .); /* bss段终止位置 */
    } > ram

    /* 设置内存起始位置 */
    PROVIDE(_memory_start = ORIGIN(ram));
    PROVIDE(_memory_end = ORIGIN(ram) + LENGTH(ram));

    /* 设置heap起止位置 */
    PROVIDE(_heap_start = _bss_end);
    PROVIDE(_heap_size = _memory_end - _heap_start);
    PROVIDE(_heap_end = _memory_end);
}
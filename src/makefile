# 构建目录
BUILD:=../build
# 源码目录
SRC:=.
# 内核开始执行的地址, 链接的地址
ENTRYPOINT:=0x10000

# 32位程序
CFLAGS:= -m32
# 不需要gcc内置函数, 例如memcpy
CFLAGS+= -fno-builtin
# 不需要标准头文件
CFLAGS+= -nostdinc
# 不需要位置无关的代码, 位置无关的意思是代码可以加载到任何位置
CFLAGS+= -fno-pic
# 不需要位置无关的可执行程序
CFLAGS+= -fno-pie
# 不需要标准库
CFLAGS+= -nostdlib
# 不需要栈保护
CFLAGS+= -fno-stack-protector
# 将flags后面的空格\t之类的去掉
CFLAGS:=$(strip ${CFLAGS})

DEBUG:= -g
INCLUDE:=-I$(SRC)/include

$(BUILD)/boot/%.bin: $(SRC)/boot/%.asm
	$(shell mkdir -p $(dir $@))
	nasm -f bin $< -o $@

$(BUILD)/%.o: $(SRC)/%.asm
	$(shell mkdir -p $(dir $@))
	nasm -f elf32 $(DEBUG) $< -o $@

$(BUILD)/%.o: $(SRC)/%.c
	$(shell mkdir -p $(dir $@))
	gcc $(CFLAGS) $(DEBUG) $(INCLUDE) -c $< -o $@

$(BUILD)/kernel.bin: $(BUILD)/kernel/start.o \
	$(BUILD)/kernel/main.o \
	$(BUILD)/kernel/io.o \
	$(BUILD)/kernel/schedule.o \
	$(BUILD)/kernel/console.o \
	$(BUILD)/kernel/assert.o \
	$(BUILD)/kernel/debug.o \
	$(BUILD)/kernel/printk.o \
	$(BUILD)/kernel/global.o \
	$(BUILD)/kernel/task.o \
	$(BUILD)/lib/vsprintf.o \
	$(BUILD)/lib/string.o

	$(shell mkdir -p $(dir $@))
	ld -m elf_i386 -static $^ -o $@ -Ttext $(ENTRYPOINT)

$(BUILD)/system.bin: $(BUILD)/kernel.bin
	objcopy -O binary $< $@

$(BUILD)/system.map: $(BUILD)/kernel.bin
	nm $< | sort > $@

$(BUILD)/master.img: $(BUILD)/boot/boot.bin \
	$(BUILD)/boot/loader.bin \
	$(BUILD)/system.bin \
	$(BUILD)/system.map

	yes | bximage -q -hd=16 -func=create -sectsize=512 -imgmode=flat $@
	dd if=$(BUILD)/boot/boot.bin of=$@ bs=512 count=1 conv=notrunc
	dd if=$(BUILD)/boot/loader.bin of=$@ bs=512 count=4 seek=2 conv=notrunc
	dd if=$(BUILD)/system.bin of=$@ bs=512 count=200 seek=10 conv=notrunc

test: $(BUILD)/master.img

.PHONY: clean
clean: 
	rm -rf $(BUILD)

.PHONY: bochs
bochs: $(BUILD)/master.img
	/home/silence/Desktop/XinOS/bochs/bochs/bin/bochs-gdb -q -f /home/silence/Desktop/XinOS/bochs/bochs/bin/bochsrc.gdb
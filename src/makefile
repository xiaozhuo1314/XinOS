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

$(BUILD)/kernel.bin: \
	$(BUILD)/kernel/start.o \
	$(BUILD)/kernel/main.o \
	$(BUILD)/kernel/io.o \
	$(BUILD)/kernel/console.o \
	$(BUILD)/kernel/printk.o \
	$(BUILD)/kernel/assert.o \
	$(BUILD)/kernel/debug.o \
	$(BUILD)/kernel/global.o \
	$(BUILD)/kernel/task.o \
	$(BUILD)/kernel/schedule.o \
	$(BUILD)/kernel/interrupt.o \
	$(BUILD)/kernel/handler.o \
	$(BUILD)/kernel/clock.o \
	$(BUILD)/kernel/rtc.o \
	$(BUILD)/kernel/time.o \
	$(BUILD)/kernel/memory.o \
	$(BUILD)/kernel/bitmap.o \
	$(BUILD)/kernel/gate.o \
	$(BUILD)/lib/string.o \
	$(BUILD)/lib/vsprintf.o \
	$(BUILD)/lib/stdlib.o \

	$(shell mkdir -p $(dir $@))
	ld -m elf_i386 -static $^ -o $@ -Ttext $(ENTRYPOINT)

$(BUILD)/system.bin: $(BUILD)/kernel.bin
	objcopy -O binary $< $@

$(BUILD)/system.map: $(BUILD)/kernel.bin
	nm $< | sort > $@

$(BUILD)/master.img: $(BUILD)/boot/boot.bin \
	$(BUILD)/boot/loader.bin \
	$(BUILD)/system.bin \
	$(BUILD)/system.map \

	# 创建一个 16M 的硬盘镜像
	yes | bximage -q -hd=16 -func=create -sectsize=512 -imgmode=flat $@
	# 将 boot.bin 写入主引导扇区
	dd if=$(BUILD)/boot/boot.bin of=$@ bs=512 count=1 conv=notrunc
	# 将 loader.bin 写入硬盘
	dd if=$(BUILD)/boot/loader.bin of=$@ bs=512 count=4 seek=2 conv=notrunc
	# 测试 system.bin 小于 100k, 否则需要修改下面的 count
	test -n "$$(find $(BUILD)/system.bin -size -100k)"
	# 将 system.bin 写入硬盘
	dd if=$(BUILD)/system.bin of=$@ bs=512 count=200 seek=10 conv=notrunc

image: $(BUILD)/master.img

.PHONY: clean
clean: 
	rm -rf $(BUILD)

.PHONY: bochs-gdb
bochs-gdb: $(BUILD)/master.img
	/home/silence/bochs/bochs-gdb/bin/bochs -q -f bochsrc.gdb -unlock

.PHONY: bochs
bochs: $(BUILD)/master.img
	bochs -q -f bochsrc -unlock

QEMU:= qemu-system-i386 \
	-m 32M \
	-boot c \
	-drive file=$(BUILD)/master.img,if=ide,index=0,media=disk,format=raw \
	-rtc base=localtime \
	-audiodev pa,id=hda,server=$(shell echo $(HOST_IP)):4713 \
	-machine pcspk-audiodev=hda \

.PHONY: qemu
qemu: $(BUILD)/master.img
	$(QEMU)

.PHONY: qemug
qemu-gdb: $(BUILD)/master.img
	$(QEMU) -s -S
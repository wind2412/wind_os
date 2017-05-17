GCC := i386-elf-gcc
CFLAGS := -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -fno-omit-frame-pointer -fno-stack-protector -fno-pic -O -nostdinc -I include
# -fno-pic: 不生成pic代码。即不生成位置无关代码。position independent code. 详细请参阅CSAPP。必须固定生成在0x100000这种。。必须位置相关。
# -static: 优先使用静态共享库。 ***.a
# -fno-strict-aliasing 强制通知gcc：函数形参中的指针指向的可能是同一个位置。也就是去掉-O2参数的不安全优化。
# -fno-omit-frame-pointer： -fomit-frame-pointer参数是强制让所有的函数调用入栈的push ebp，push esp全部省略，因此无法回溯堆栈状况。所以-fno-参数就是取消这种强制性，使堆栈可以回溯。
# -MD 每次编译.c文件时，都会生成.d文件保存类似于Makefile的生成依赖关系。
# -fno-stack-protector：强制去掉堆栈保护。
SFLAGS := -fno-pic -m32 -nostdinc -fno-builtin
LDFLAGS := -T ldscript.ld -m elf_i386 -nostdlib
AS := i386-elf-as
LD := i386-elf-ld
OBJCOPY := i386-elf-objdump
OBJCOPY := i386-elf-objcopy
EXCEPT := ./sign.c 
BOOT_EXCEPT := ./boot/bootasm.o ./boot/bootmain.o  #mdzz! echo的输出有陷阱。输出时是不带“./***”路径的！而是直接显示“***”！！我是按照@echo的输出做得改动。。所以其实这里字符串如果不加./ ，就匹配不上。。mdzz。。。
C_SOURCE := $(filter-out $(EXCEPT), $(shell find . -name "*.c"))
S_SOURCE := $(shell find . -name "*.S")
C_OBJ := $(patsubst %.c, %.o, $(C_SOURCE))
S_OBJ := $(patsubst %.S, %.o, $(S_SOURCE))
DIR_BIN := ./bin/

all : sign $(C_OBJ) $(S_OBJ) bin/bootloader bin/wind_os.img clean_obj
	@echo clean trunk objs.

bin/wind_os.img : bin/bootloader bin/wind_os_kern
	@dd if=/dev/zero of=$@ count=1000  # 1000 blocks each 512B as a page and filled wind_os.img with bit '0'.
	@dd if=$(DIR_BIN)bootloader of=$@ conv=notrunc  # if don't use 'conv=notrunc' the .img file will shrink to the size of bootloader of 512B!! 
	@dd if=$(DIR_BIN)wind_os_kern of=$@ seek=1 conv=notrunc  # don't overlap the bootloader of the 1st block. with seek=1 it will write in .img at the 2nd block of 512B.


sign : sign.c
	@gcc -o $@ $<

bin/bootloader : boot/bootasm.o boot/bootmain.o #include/x86.o bootmain.o在编译时没有问题。但是两个链接时有问题。因为x86.o定义了重要函数，也需要被链接。不像编译，链接还需要自己手动指定。而，xv6直接定义在x86.h中，没有x86.c，因而链接可以很省事。
	@$(LD) -m elf_i386 -nostdlib -N -e start -Ttext 0x7c00 -o boot/bootloader.o $^   # ld and makes start at 0x7c00 position
	@$(OBJCOPY) -S -O binary -j .text boot/bootloader.o $@           # objcopy .text to bootblock. bootblock.o is so big that larger than 512B.
	@./sign $@

boot/bootasm.o : boot/bootasm.S
	@$(GCC) $(CFLAGS) -I./boot -c $< -o $@   # bootloader过大，超过512B。不得不开O2优化。就是调试可能会出错，因为反汇编代码变得不同了。。

boot/bootmain.o : boot/bootmain.c
	@$(GCC) $(CFLAGS) -c $< -o $@

.c.o:
	$(GCC) $(CFLAGS) -c $< -o $@ #raw relocatable object file
	
.s.o:
	$(AS) $< -o $@              #raw relocatable object file (shan’t use ld.)

.S.o:
	$(AS) $< -o $@              #raw relocatable object file (shan’t use ld.) 提示链接的时候缺少crt0.o，是因为用i386-elf-gcc编译造成。改用i386-elf-as编译就好了。不清楚为什么。

bin/wind_os_kern: $(filter-out $(BOOT_EXCEPT), $(C_OBJ) $(S_OBJ))
	$(LD) $(LDFLAGS) -o $@  $^

	
qemu:
	@qemu $(DIR_BIN)wind_os.img -parallel stdio  # boot from the hard disk with '-boot c' or '-hda'. if it is a floppy.img we'd use '-boot a' or 'fda'.
	
debug:
	@qemu -S -s -hda $(DIR_BIN)wind_os.img & # mdzz!!!这里调了好长时间。。必须加上&表示后台运行才行！！究竟是什么原理。。。。不加，gdb会卡住。。。
	@cgdb -d i386-elf-gdb -q -x $(DIR_BIN)gdbinit  # if -nx is not read any .gdbinit file, the -x will be read the .gdbinit file.

.PHONY: clean

clean_obj:
	@rm -rf $(C_OBJ) $(S_OBJ) sign boot/bootloader.o 
	@find . -name "*.d" -type f -exec rm -rf {} \;
	
clean:  clean_obj
	@rm -rf bin/wind_os_kern bin/wind_os.img
	@echo clean all.
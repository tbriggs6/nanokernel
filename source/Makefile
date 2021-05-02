TARGET=i686-elf
AS:=$(TARGET)-as
CC:=$(TARGET)-gcc
LD:=$(TARGET)-ld

PWD:=`pwd`
CFLAGS := -ffreestanding -O0 -Wall -Wextra -nostdlib -nostartfiles -nodefaultlibs  -Wl,--build-id=none  -ggdb -I$(PWD) -I$(PWD)/lib
CPPFLAGS:=$(CFLAGS)
LIBS:=-L/opt/cross/lib/gcc/i686-elf/6.4.0  -lgcc

OBJS:=\
asm/boot.o \
asm/intr.o \
lib/list.o \
lib/kmalloc.o \
lib/kstdlib.o \
lib/fifo.o \
lib/bitmap.o \
lib/elf.o \
kernel.o \
console.o \
handler.o \
pic.o \
chrdev.o \
ps2.o  \
keyboard.o  \
memory.o 


all: myos.bin idle.bin

.PHONEY: all clean iso run-qemu

myos.bin: $(OBJS) linker.ld
	$(LD) -T linker.ld -o $@ $(LDFLAGS) $(OBJS) $(LIBS)

idle.bin: idle.o linker.ld
	$(CC) $(CFLAGS) -o idle.bin idle.o

%.o: %.c
	$(CC) -c $< -o $@ -std=gnu99 $(CFLAGS) $(CPPFLAGS)

%.o: %.s
	$(CC) $(ASFLAGS) $< -o $@

clean:
	rm -rf isodir
	rm -f myos.bin myos.iso $(OBJS)

iso: myos.iso

isodir isodir/boot isodir/boot/grub:
	mkdir -p $@

isodir/boot/myos.bin: myos.bin isodir/boot
	cp $< $@

isodir/boot/grub/grub.cfg: grub.cfg isodir/boot/grub
	cp $< $@

myos.iso: isodir/boot/myos.bin isodir/boot/grub/grub.cfg
	grub-mkrescue -o $@ isodir

run-qemu: myos.iso
	qemu-system-i386 -d int,cpu_reset -s -cdrom myos.iso -monitor stdio 

boot.S: gdt.h
boot.o: boot.S gdt.h

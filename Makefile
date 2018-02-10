TARGET=i686-elf
AS:=$(TARGET)-as
CC:=$(TARGET)-gcc
LD:=$(TARGET)-ld

CFLAGS := -ffreestanding -O2 -Wall -Wextra -nostdlib -nostartfiles -nodefaultlibs  -Wl,--build-id=none  -ggdb
CPPFLAGS:=$(CFLAGS)
LIBS:=-L/opt/cross/lib/gcc/i686-elf/6.4.0  -lgcc

OBJS:=\
boot.o \
kernel.o \
console.o \
intr.o \
handler.o \
pic.o \
kmalloc.o \
kstdlib.o \
list.o \
kbd.o  \
chrdev.o \
fifo.o \

all: myos.bin

.PHONEY: all clean iso run-qemu

myos.bin: $(OBJS) linker.ld
	$(LD) -T linker.ld -o $@ $(LDFLAGS) $(OBJS) $(LIBS)

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
	qemu-system-i386 -d int,cpu_reset -s -cdrom myos.iso

boot.S: gdt.h
boot.o: boot.S gdt.h

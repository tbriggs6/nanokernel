NOOSCFLAGS := -ffreestanding -O0 -Wall -Wextra -nostdlib -nostartfiles -nodefaultlibs  -Wl,--build-id=none  -ggdb 
NOOSCPPFLAGS:=$(CFLAGS)

CFLAGS := -O0 -ggdb 
CPPFLAGS:=$(CFLAGS)

LIBS:=-L/opt/cross/lib/gcc/i686-elf/6.4.0  -lgcc

all: 	test_elf simple

# host applications

test_elf: elf.o test_elf.o
	gcc $(CPPFLAGS) -o test_elf elf.o test_elf.o

.c.o:
	gcc $(CFLAGS) -c $<

# test application
simple: simple.c
	i686-elf-gcc $(NOOSCFLAGS) -o simple simple.c

clean: 
	rm -f *.o test_elf simple

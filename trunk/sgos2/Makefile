#Makefile for SGOS2

all:
	make -C kernel
	make -C services

run:

windgb:
	start qemu -no-kqemu -m 32 -s -S -L ./qemu sgos2.img &
	gdb --command scripts/gdb kernel/kernel

debug:
	qemu -no-kqemu -m 32 -s -S -L ./qemu sgos2.img &
	gdb --command scripts/gdb kernel/kernel

clean:
	make -C kernel clean
	make -C services clean


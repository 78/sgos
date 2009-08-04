#Makefile for SGOS2

all:
	make -C kernel
	make -C services
#	make -C tools/wf
	tools/wf/wf sgos2.img -src kernel/kernel.bin -dest sgos2/kernel.bin 
run:

bochsdbg:
	bochsdbg -q -f scripts/bochsrc.bxrc

windbg:
	start qemu -no-kqemu -m 32 -s -S -L ./qemu sgos2.img &
	gdb --command scripts/gdb kernel/kernel

debug:
	qemu -no-kqemu -m 32 -s -S -L ./qemu sgos2.img &
	gdb --command scripts/gdb kernel/kernel

clean:
	make -C kernel clean
	make -C services clean


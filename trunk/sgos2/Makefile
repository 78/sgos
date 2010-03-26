#Makefile for SGOS2

# OS type.
ifndef OSTYPE
export OSTYPE := $(shell uname)
endif

# Tool set
ifeq ($(OSTYPE), Linux)
export RM 	  	  := rm -f
export EXE_SUFFIX := 
else
export RM	  	  := rm -f
export EXE_SUFFIX := .exe
endif

# Check the build tool
define wf-if-not-exist
	$(if $(wildcard $1),,@make -C tools/wf)
endef

.PHONY : all check run bochsdbg bochs windbg debug clean
# Target
all: check grub
	make -C crt
	make -C kernel 
#make -C apps
	@echo OK!

grub: scripts/menu.lst
	tools/wf/wf sgos2.img -src scripts/menu.lst -dest boot/grub/menu.lst
	tools/wf/wf sgos2.img -src scripts/startup.txt -dest sgos2/startup.txt
	tools/wf/wf sgos2.img -src tools/vm86/vm86.com -dest sgos2/vm86.com

check:
	$(call wf-if-not-exist, tools/wf/wf$(EXE_SUFFIX))

run:
	qemu -m 64 -L ./qemu sgos2.img -soundhw pcspk

bochsdbg:
	bochsdbg -q -f scripts/bochsrc.bxrc
	
bochs:
	bochs -q -f scripts/bochsrc.bxrc

windbg:
	start insight kernel/kernel &
	start qemu -no-kqemu -m 64 -s -S -L ./qemu sgos2.img &

startup:
	start insight apps/hello/hello.exe &
	start qemu -no-kqemu -m 64 -s -S -L ./qemu sgos2.img &

debug:
	qemu -no-kqemu -m 32 -s -S -L ./qemu sgos2.img &
	gdb --command scripts/gdb kernel/kernel

clean:
	make -C crt clean
	make -C kernel clean
	make -C apps clean


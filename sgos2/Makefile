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
all: check
	make -C kernel
	make -C services
	tools/wf/wf sgos2.img -src kernel/kernel.bin -dest sgos2/kernel.bin
	tools/wf/wf sgos2.img -src scripts/menu.lst -dest boot/grub/menu.lst
	@echo OK!

check:
	$(call wf-if-not-exist, tools/wf/wf$(EXE_SUFFIX))

run:
	qemu -no-kqemu -m 512 -L ./qemu sgos2.img

bochsdbg:
	bochsdbg -q -f scripts/bochsrc.bxrc
	
bochs:
	bochs -q -f scripts/bochsrc.bxrc

windbg:
	start qemu -no-kqemu -m 512 -s -S -L ./qemu sgos2.img &
	gdb --command scripts/gdb kernel/kernel

debug:
	qemu -no-kqemu -m 32 -s -S -L ./qemu sgos2.img &
	gdb --command scripts/gdb kernel/kernel

clean:
	make -C kernel clean
	make -C services clean


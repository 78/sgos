#Makefile for SGOS2

# Export var for all subdir.
ifndef OSTYPE
	export OSTYPE := $(shell uname)
endif

# Tool set
ifeq ($(OSTYPE), Linux)
	export DEL := rm -f
	export EXT :=
else
	export DEL := del
	export EXT := .exe
endif

# Check the build tool
define wf-if-not-exist
	$(if $(wildcard $1), @echo $1 is not exist!,@make -C tools/wf)
endef

.PHONY : all run bochsdbg bochs windbg debug clean
# Target
all: check
	make -C kernel
	make -C services
#	make -C tools/wf
	tools/wf/wf sgos2.img -src kernel/kernel.bin -dest sgos2/kernel.bin
	@echo OK!

check:
	$(call wf-if-not-exist, tools/wf/wf$(EXT))

run:
	@echo just a joke, nothing to do!

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


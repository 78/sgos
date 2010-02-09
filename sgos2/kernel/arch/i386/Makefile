# I386 architecture

CCFLAGS += -Iinclude/arch_i386
ASFLAGS += -Iinclude/arch_i386


LDSCRIPT = arch/i386/kernel.ld

# Kernel Objects
OBJECTS += arch/i386/init/multiboot.o \
	arch/i386/init/init.o \
	arch/i386/io/port.o \
	arch/i386/io/terminal.o \
	arch/i386/mmu/gdt.o \
	arch/i386/mmu/page.o \
	arch/i386/mmu/dir.o \
	arch/i386/mmu/map.o \
	arch/i386/mmu/fault.o \
	arch/i386/clock/rtc.o \
	arch/i386/debug/dbgx86.o \
	arch/i386/cpu/threading.o \
	arch/i386/cpu/sysenter.o \
	arch/i386/cpu/switch.o \
	arch/i386/cpu/lock.o \
	arch/i386/cpu/fastcall.o \
	arch/i386/cpu/vm86.o \
	arch/i386/math/fpu.o \
	arch/i386/irq/irq.o \
	arch/i386/irq/isr.o \
	arch/i386/irq/interrupt.o \
	arch/i386/misc/mmop.o




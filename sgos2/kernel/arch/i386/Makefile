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
	arch/i386/irq/irq.o \
	arch/i386/irq/isr.o \
	arch/i386/irq/interrupt.o \

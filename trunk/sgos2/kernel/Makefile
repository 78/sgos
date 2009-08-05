# Makefile for SGOS2:kernel

# Target machine information
ARCH = i386

ASFLAGS = -g
CCFLAGS = -g -Werror -Iinclude -fno-builtin -ffreestanding -fleading-underscore
LDFLAGS = 

ifndef OSTYPE
OSTYPE := $(shell uname)
endif

ifeq ($(OSTYPE), Linux)
# linux env
CCFLAGS += -fno-stack-protector
LDFLAGS += -T$(LDSCRIPT) 
else
# windows env
LDFLAGS += -Ttext=0xC0100000 -e_start
endif

AS = as $(ASFLAGS)
CC = gcc $(CCFLAGS)
LD = ld $(LDFLAGS)
OC = objcopy $(OCFLAGS)
OD = objdump $(ODFLAGS)
LD2 = ld2

# Include Target Platform Makefile
include arch/$(ARCH)/Makefile

# Kernel objects
OBJECTS += lib/ctype.o \
	lib/string.o \
	lib/vsprintf.o \
	debug/debug.o \
	debug/format.o \
	init/kinit.o

# Target

all: kernel

kernel: $(OBJECTS)
	$(LD) $(OBJECTS) -o$@
	$(OD) -S $@ > $@.dmp
	$(OC) -Obinary $@ $@.bin 

clean:
	rm -f $(OBJECTS) \
		kernel.dmp \
		kernel \
		kernel.bin

# Standard Procedures
.s.o:
	$(AS) -o $@ $<

.c.o:
	$(CC) -c -o $@ $<

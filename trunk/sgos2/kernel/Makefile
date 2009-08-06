# Makefile for SGOS2:kernel

# Target machine information
ARCH = i386

ASFLAGS = -g
CCFLAGS = -g -Werror -Iinclude -fno-builtin -ffreestanding -fleading-underscore
LDFLAGS = 

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

# Kernel dir
DIR_ARCH	= arch
DIR_IPC		= ipc
DIR_LOADER	= loader
DIR_MM		= mm
DIR_DEBUG	= debug
DIR_INIT	= init
DIR_LIB		= lib
DIR_PROC	= proc

# Sub objs
OBJ_ARCH	= $(patsubst %.c,%.o,  $(wildcard $(DIR_ARCH)/*.c))
OBJ_IPC		= $(patsubst %.c,%.o,  $(wildcard $(DIR_IPC)/*.c))
OBJ_LOADER	= $(patsubst %.c,%.o,  $(wildcard $(DIR_LOADER)/*.c))
OBJ_MM		= $(patsubst %.c,%.o,  $(wildcard $(DIR_MM)/*.c))
OBJ_DEBUG	= $(patsubst %.c,%.o,  $(wildcard $(DIR_DEBUG)/*.c))
OBJ_INIT	= $(patsubst %.c,%.o,  $(wildcard $(DIR_INIT)/*.c))
OBJ_LIB		= $(patsubst %.c,%.o,  $(wildcard $(DIR_LIB)/*.c))
OBJ_PROC	= $(patsubst %.c,%.o,  $(wildcard $(DIR_PROC)/*.c))

# All objects, here can be set the first link obj for your kernel
OBJECTS     =

# Platform objects
include arch/$(ARCH)/Makefile

# Kernel objects
OBJECTS	   += $(OBJ_ARCH) \
	      $(OBJ_IPC) \
	      $(OBJ_LOADER) \
	      $(OBJ_MM) \
	      $(OBJ_DEBUG) \
	      $(OBJ_INIT) \
	      $(OBJ_LIB) \
	      $(OBJ_PROC)

# Target
all: kernel

kernel: $(OBJECTS)
	$(LD) $(OBJECTS) -o$@
	$(OD) -S $@ > $@.dmp
	$(OC) -Obinary $@ $@.bin 

clean:
	$(RM) $(OBJECTS) \
		kernel.dmp \
		kernel \
		kernel.bin

# Standard Procedures
.s.o:
	$(AS) -o $@ $<

.c.o:
	$(CC) -c -o $@ $<

#sgos api

CCFLAGS = -g -Werror -I../include -I../../include -fno-builtin -ffreestanding -fleading-underscore
LDFLAGS = -shared --add-stdcall-alias
ODFLAGS = -S

#here defines the program information
PROGRAM = libc
LDFLAGS += --image-base 0x7FE00000 ../api/api.dll

CC = gcc $(CCFLAGS)
LD = ld $(LDFLAGS)
OD = objdump $(ODFLAGS)
LD2 = ld2
WF = ../../tools/wf/wf
IMAGE = ../../sgos2.img

OBJECTS = $(patsubst %.c,%.o,  $(wildcard *.c))

all: $(PROGRAM)

$(PROGRAM): $(OBJECTS)
	$(LD) $(OBJECTS) -o$@.dll
	$(OD) $@.dll > $@.dmp
	$(LD2) $@.dll $@.bxm
	$(WF) $(IMAGE) -src $@.bxm -dest sgos2/$@.bxm

# Standard Procedures
.c.o:
	$(CC) -c -o $@ $<

clean:
	$(RM) $(OBJECTS)
	$(RM) $(PROGRAM).bxm
	$(RM) $(PROGRAM)
	$(RM) $(PROGRAM).dmp
	$(RM) $(PROGRAM).dll
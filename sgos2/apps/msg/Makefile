#sgos api

CCFLAGS = -g -Werror -I../include -I../../include -fno-builtin -ffreestanding -fleading-underscore
LDFLAGS = -shared
ODFLAGS = -S

#here defines the program information
PROGRAM = msg
LDFLAGS += --image-base 0x7FC00000 ../libc/libc.dll ../api/api.dll

CC = gcc $(CCFLAGS)
LD = ld $(LDFLAGS)
OD = objdump $(ODFLAGS)
LD2 = ../../tools/ld2/ld2
WF = ../../tools/wf/wf
IMAGE = ../../sgos2.img

OBJECTS = $(patsubst %.c,%.o,  $(wildcard *.c))

all: $(PROGRAM).bxm

$(PROGRAM).bxm: $(OBJECTS)
	$(LD) $(OBJECTS) -o$(PROGRAM).dll
	$(OD) $(PROGRAM).dll > $(PROGRAM).dmp
	$(LD2) $(PROGRAM).dll $@
	$(WF) $(IMAGE) -src $@ -dest sgos2/$@

# Standard Procedures
.c.o:
	$(CC) -c -o $@ $<

clean:
	$(RM) $(OBJECTS)
	$(RM) $(PROGRAM).bxm
	$(RM) $(PROGRAM).dmp
	$(RM) $(PROGRAM).dll
#sgos api

CCFLAGS = -g -Werror -I../include -I../../include -fno-builtin -ffreestanding -fleading-underscore
LDFLAGS = -r
ODFLAGS = -S

#here defines the program information
PROGRAM = libc

CC = gcc $(CCFLAGS)
LD = ld $(LDFLAGS)
OD = objdump $(ODFLAGS)
LD2 = ../../tools/ld2/ld2
WF = ../../tools/wf/wf
IMAGE = ../../sgos2.img

OBJECTS = $(patsubst %.c,%.o,  $(wildcard *.c))

all: $(PROGRAM).a

$(PROGRAM).a: $(OBJECTS)
	$(LD) $(OBJECTS) -o$(PROGRAM).a
	$(OD) $(PROGRAM).a > $(PROGRAM).dmp

# Standard Procedures
.c.o:
	$(CC) -c -o $@ $<

clean:
	$(RM) $(OBJECTS)
	$(RM) $(PROGRAM).a
	$(RM) $(PROGRAM).dmp
	$(RM) $(PROGRAM).dll
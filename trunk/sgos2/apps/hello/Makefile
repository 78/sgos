#sgos api

CCFLAGS = -g -Werror -I../include -I../../include -fno-builtin -ffreestanding -fleading-underscore
LDFLAGS = 
ODFLAGS = -S

#here defines the program information
PROGRAM = hello
LDFLAGS += ../libc/libc.dll -e_entry

CC = gcc $(CCFLAGS)
LD = ld $(LDFLAGS)
OD = objdump $(ODFLAGS)
LD2 = ld2
WF = ../../tools/wf/wf
IMAGE = ../../sgos2.img

OBJECTS = $(patsubst %.c,%.o,  $(wildcard *.c))

all: $(PROGRAM)

$(PROGRAM): $(OBJECTS)
	$(LD) $(OBJECTS) -o$@.exe
	$(OD) $@.exe > $@.dmp
	$(LD2) $@.exe $@.run
	$(WF) $(IMAGE) -src $@.run -dest sgos2/$@.run

# Standard Procedures
.c.o:
	$(CC) -c -o $@ $<

clean:
	$(RM) $(OBJECTS)
	$(RM) $(PROGRAM).run
	$(RM) $(PROGRAM)
	$(RM) $(PROGRAM).dmp
	$(RM) $(PROGRAM).exe
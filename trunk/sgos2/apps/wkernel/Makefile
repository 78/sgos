#sgos api

CCFLAGS = -g -Werror -I../include -I../../include -fno-builtin -ffreestanding -fleading-underscore
LDFLAGS =
ODFLAGS = -S

#here defines the program information
PROGRAM = wkernel

CC = gcc $(CCFLAGS)
LDLIB = ld  -r $(LDFLAGS)
LD = ld --image-base 0x7FF00000 -shared $(LDFLAGS) ../../api/api.dll
OD = objdump $(ODFLAGS)
WF = ../../tools/wf/wf
IMAGE = ../../sgos2.img

OBJECTS = $(patsubst %.c,%.o,  $(wildcard *.c))

all: $(PROGRAM).dll

$(PROGRAM).dll: $(OBJECTS)
	$(LD) $(OBJECTS) -o$(PROGRAM).dll
	$(OD) $(PROGRAM).dll > $(PROGRAM).dmp
	$(WF) $(IMAGE) -src $@ -dest sgos2/$@

# Standard Procedures
.c.o:
	$(CC) -c -o $@ $<

clean:
	$(RM) $(OBJECTS)
	$(RM) $(PROGRAM).a
	$(RM) $(PROGRAM).dmp
	$(RM) $(PROGRAM).dll
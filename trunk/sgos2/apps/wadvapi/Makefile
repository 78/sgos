#sgos api

CCFLAGS = -g -Werror -I../include -I../../include -fno-builtin -ffreestanding -fleading-underscore
LDFLAGS = ../../api/api.dll --add-stdcall-alias
ODFLAGS = -S

#here defines the program information
PROGRAM = advapi32

CC = gcc $(CCFLAGS)
LDLIB = ld  -r $(LDFLAGS)
LD = ld --image-base 0x7FA00000 -shared $(LDFLAGS)
OD = objdump $(ODFLAGS)
WF = ../../tools/wf/wf
IMAGE = ../../sgos2.img

OBJECTS = $(patsubst %.c,%.o,  $(wildcard *.c))

all: $(PROGRAM).dll

$(PROGRAM).dll: $(OBJECTS)
	$(LD) $(OBJECTS) -o$(PROGRAM).dll
	$(OD) $(PROGRAM).dll > $(PROGRAM).dmp
	$(WF) $(IMAGE) -src $@ -dest sgos/$@

# Standard Procedures
.c.o:
	$(CC) -c -o $@ $<

clean:
	$(RM) $(OBJECTS)
	$(RM) $(PROGRAM).a
	$(RM) $(PROGRAM).dmp
	$(RM) $(PROGRAM).dll
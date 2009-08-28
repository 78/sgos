#sgos api

CCFLAGS = -g -fno-builtin -nostdlib -fleading-underscore
ODFLAGS = -S
LDFLAGS = -r

#here defines the program information
PROGRAM = crt

CC = gcc $(CCFLAGS)
LD = ld $(LDFLAGS)

OBJECTS = $(patsubst %.cpp,%.o,  $(wildcard *.cpp)) \
	$(patsubst %.S,%.o,  $(wildcard *.S))

all: $(PROGRAM).a

$(PROGRAM).a: $(OBJECTS)
	$(LD) -o $(PROGRAM).a $(OBJECTS) 

# Standard Procedures
.c.o:
	$(CC) -c -o $@ $<

clean:
	$(RM) $(OBJECTS)
	$(RM) $(PROGRAM).a

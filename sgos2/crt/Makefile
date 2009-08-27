#sgos api

CCFLAGS = -g -fno-builtin -nostdlib -fleading-underscore
ODFLAGS = -S

#here defines the program information
PROGRAM = crt

CC = gcc $(CCFLAGS)
AR = ar

OBJECTS = $(patsubst %.cpp,%.o,  $(wildcard *.cpp)) \
	$(patsubst %.S,%.o,  $(wildcard *.S))

all: $(PROGRAM).a

$(PROGRAM).a: $(OBJECTS)
	$(AR) r $(PROGRAM).a $(OBJECTS) 

# Standard Procedures
.c.o:
	$(CC) -c -o $@ $<

clean:
	$(RM) $(OBJECTS)
	$(RM) $(PROGRAM).a

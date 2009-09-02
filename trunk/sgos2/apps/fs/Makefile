#sgos module

CXXFLAGS = -g -Werror -I../../include/c++ -I../../include -nostdlib -fleading-underscore --no-exceptions
CCFLAGS = -g -Werror -I../../include -nostdlib -fno-builtin -fleading-underscore
LDFLAGS = -shared 
ODFLAGS = -S

#here defines the program information
PROGRAM = vfs
LDFLAGS += --image-base 0x00400000 ../../crt/crt.a \
	../libc/libc.dll ../system/system.dll ../vfs/vfs.dll

CXX = g++ $(CXXFLAGS)
CC = gcc $(CCFLAGS)
LD = ld $(LDFLAGS)
OD = objdump $(ODFLAGS)
LD2 = ../../tools/ld2/ld2
WF = ../../tools/wf/wf
IMAGE = ../../sgos2.img

OBJECTS = $(patsubst %.cpp,%.o,  $(wildcard *.cpp)) \
	$(patsubst %.c,%.o,  $(wildcard *.c))

all: $(PROGRAM).run

$(PROGRAM).run: $(OBJECTS)
	$(LD) $(OBJECTS) -o$(PROGRAM).exe
	$(OD) $(PROGRAM).exe > $(PROGRAM).dmp
	$(LD2) $(PROGRAM).exe $@
	$(WF) $(IMAGE) -src $@ -dest sgos2/$@

# Standard Procedures
.cpp.o:
	$(CXX) -c -o $@ $<
.c.o:
	$(CC) -c -o $@ $<

clean:
	$(RM) $(OBJECTS)
	$(RM) $(PROGRAM).run
	$(RM) $(PROGRAM).dmp
	$(RM) $(PROGRAM).exe

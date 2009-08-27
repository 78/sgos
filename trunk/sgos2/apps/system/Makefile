#sgos messenger

CXXFLAGS = -g -Werror -I../../include/c++ -I../../include -nostdlib -fleading-underscore --no-exceptions
LDFLAGS = -shared
ODFLAGS = -S

#here defines the program information
PROGRAM = system
LDFLAGS += --image-base 0x7FD00000 ../libc/libc.dll ../msg/msg.dll ../api/api.dll

CXX = g++ $(CXXFLAGS)
LD = ld $(LDFLAGS)
OD = objdump $(ODFLAGS)
LD2 = ../../tools/ld2/ld2
WF = ../../tools/wf/wf
IMAGE = ../../sgos2.img

OBJECTS = $(patsubst %.cpp,%.o,  $(wildcard *.cpp)) \
	$(patsubst %.cpp,%.o,  $(wildcard base/*.cpp)) \
	$(patsubst %.cpp,%.o,  $(wildcard message/*.cpp)) \
	$(patsubst %.cpp,%.o,  $(wildcard threading/*.cpp)) \
	$(patsubst %.cpp,%.o,  $(wildcard service/*.cpp))

all: $(PROGRAM).bxm

$(PROGRAM).bxm: $(OBJECTS)
	$(LD) $(OBJECTS) -o$(PROGRAM).dll
	$(OD) $(PROGRAM).dll > $(PROGRAM).dmp
	$(LD2) $(PROGRAM).dll $@
	$(WF) $(IMAGE) -src $@ -dest sgos2/$@

# Standard Procedures
.cpp.o:
	$(CXX) -c -o $@ $<

clean:
	$(RM) $(OBJECTS)
	$(RM) $(PROGRAM).bxm
	$(RM) $(PROGRAM).dmp
	$(RM) $(PROGRAM).dll

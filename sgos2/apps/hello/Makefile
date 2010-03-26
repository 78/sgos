#sgos apps

CXXFLAGS = -g -Werror -I../../include/c++ -I../../include -nostdlib -fleading-underscore --no-exceptions
LDFLAGS = ../../api/api.dll
ODFLAGS = -S

#here defines the program information
PROGRAM = hello

CXX = g++ $(CXXFLAGS)
LD = ld $(LDFLAGS)
OD = objdump $(ODFLAGS)
LD2 = ../../tools/ld2/ld2
WF = ../../tools/wf/wf
IMAGE = ../../sgos2.img

OBJECTS = $(patsubst %.cpp,%.o,  $(wildcard *.cpp)) \
	 ../../crt/crt.a

all: $(PROGRAM).exe

$(PROGRAM).exe: $(OBJECTS)
	$(LD) $(OBJECTS) -o$(PROGRAM).exe
	$(OD) $(PROGRAM).exe > $(PROGRAM).dmp
	$(WF) $(IMAGE) -src $@ -dest sgos2/$@

# Standard Procedures
.c.o:
	$(CXX) -c -o $@ $<

clean:
	$(RM) $(OBJECTS)
	$(RM) $(PROGRAM).dmp
	$(RM) $(PROGRAM).exe

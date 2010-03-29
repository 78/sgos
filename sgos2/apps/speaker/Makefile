#sgos apps

CXXFLAGS = -g -Werror -I../../include/c++ -I../../include -nostdlib -fleading-underscore --no-exceptions
LDFLAGS = 
ODFLAGS = -S

#here defines the program information
PROGRAM = speaker
LDFLAGS += ../../crt/crt.a
LDFLAGS += ../../api/api.dll

CXX = g++ $(CXXFLAGS)
LD = ld $(LDFLAGS)
OD = objdump $(ODFLAGS)
WF = ../../tools/wf/wf
IMAGE = ../../sgos2.img

OBJECTS = $(patsubst %.cpp,%.o,  $(wildcard *.cpp))

all: $(PROGRAM).exe

$(PROGRAM).exe: $(OBJECTS)
	$(LD) $(OBJECTS) -o$(PROGRAM).exe
	$(OD) $(PROGRAM).exe > $(PROGRAM).dmp
	$(WF) $(IMAGE) -src $@ -dest sgos/$@

# Standard Procedures
.cpp.o:
	$(CXX) -c -o $@ $<

clean:
	$(RM) $(OBJECTS)
	$(RM) $(PROGRAM).dmp
	$(RM) $(PROGRAM).exe

# Target file name (without extension).
TARGET = sim

#Handy Path defines
ROOT_PATH = ..
PRJ_PATH = $(ROOT_PATH)/src

PROJECT = sim

OBJECTS= $(CSRCS:.c=.o) $(ASRCS:.S=.o) $(CPPSRCS:.cpp=.o)

CSRCS = 
# CSRCS += $(UTIL_PATH)/multi-steppers/multi-steppers.c

CPPSRCS = 
CPPSRCS += $(PRJ_PATH)/sim.cpp

CPPSRCS += $(PRJ_PATH)/RobotCtrl.cpp
CPPSRCS += $(PRJ_PATH)/RobotSequence.cpp
CPPSRCS += $(PRJ_PATH)/RobotOperations.cpp

CPPSRCS += $(PRJ_PATH)/DummyOperation.cpp
 
# the Extra Include Directories which may contain the various header files are listed here
INCDIRS = $(ROOT_PATH)

INCDIRS += $(SRC_PATH)/
INCDIRS += $(PRJ_PATH)

#Set The Compiler Optimization Level Here
OPTIMIZATION = 0

#  Compiler Options
GCFLAGS = -O$(OPTIMIZATION) --std=c++14
GCFLAGS += $(patsubst %,-I%,$(INCDIRS)) -I.
GCFLAGS += -Wa,-adhlns=$(subst $(suffix $<),.lst,$<) 

GPFLAGS = -O$(OPTIMIZATION)
GPFLAGS += $(patsubst %,-I%,$(INCDIRS)) -I.

LDFLAGS = -O$(OPTIMIZATION) -Wl,-Map=$(PROJECT).map,--cref,--gc-sections -lm -lgcc -lstdc++ 


#  Compiler/Assembler/Linker Paths
GCC = gcc
GPP = g++
AS = gcc
LD = g++
OBJCOPY = objcopy
REMOVE = rm -f
SIZE = size

COBJ      = $(CSRCS:.c=.o) 
CPPOBJ    = $(CPPSRCS:.cpp=.o) 

OBJECTS= $(COBJ) $(CPPOBJ)

all:: $(TARGET).exe

clean:
	$(REMOVE) $(OBJECTS)

$(TARGET).exe : $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) -o $(PROJECT).exe
	
#########################################################################
#  Default rules to compile .c and .cpp file to .o
#  and assemble .s files to .o

$(COBJ) : %.o : %.c 
	$(GCC) $(GCFLAGS) -c $< -o $@

$(CPPOBJ) : %.o : %.cpp 
	$(GPP) $(GPFLAGS) -c $< -o $@ 




# Target file name (without extension).
TARGET = sim

#Handy Path defines
ROOT_PATH = ..
COMMON_PATH = $(ROOT_PATH)/common
BRD_PATH = $(ROOT_PATH)/boards
SRC_PATH = $(COMMON_PATH)/src
INC_PATH = $(COMMON_PATH)/inc
CORE_PATH = $(COMMON_PATH)/Core/CM3
UTIL_PATH = $(ROOT_PATH)/src/utils
DRIVER_PATH = $(ROOT_PATH)/drivers
TEST_PATH = $(ROOT_PATH)/tests
PRJ_PATH = $(ROOT_PATH)/src/VirtualMachine
MENU_PATH = $(PRJ_PATH)/menus

PROJECT = sim

OBJECTS= $(CSRCS:.c=.o) $(ASRCS:.S=.o) $(CPPSRCS:.cpp=.o)

CSRCS = 
CSRCS += $(UTIL_PATH)/plc_emu/parser.c
CSRCS += $(UTIL_PATH)/plc_emu/plclib.c
# CSRCS += $(UTIL_PATH)/plc_emu/plcemu.c


CPPSRCS = $(PRJ_PATH)/sim.cpp
# CPPSRCS += $(UTIL_PATH)/vm/vm.cpp
# CPPSRCS += $(UTIL_PATH)/ring_buffer/stack.cpp
# CPPSRCS += $(UTIL_PATH)/queue/queue.cpp

# CPPSRCS += $(UTIL_PATH)/Parameters/parameters.cpp
# CPPSRCS += $(UTIL_PATH)/Robot/robot.cpp
# CPPSRCS += $(UTIL_PATH)/modbus/SimpleModbusSlave.cpp
# CPPSRCS += $(UTIL_PATH)/Robot/RobotCtrl.cpp
# CPPSRCS += $(UTIL_PATH)/Robot/MoldInfo.cpp
# CPPSRCS += $(UTIL_PATH)/Robot/RobotSequence.cpp
# CPPSRCS += $(UTIL_PATH)/state_machine/state_machine.cpp
# CPPSRCS += $(UTIL_PATH)/Robot/RobotCtrlDisplay.cpp
# CPPSRCS += $(UTIL_PATH)/Operations/SingleActingOperation.cpp
# CPPSRCS += $(UTIL_PATH)/Operations/DoubleActingOperation.cpp
# CPPSRCS += $(UTIL_PATH)/Operations/DummyOperation.cpp
# CPPSRCS += $(UTIL_PATH)/Operations/RobotOperations.cpp
# CPPSRCS += $(UTIL_PATH)/Operations/GripperCloseOperation.cpp
# CPPSRCS += $(UTIL_PATH)/Operations/GripperReleaseOperation.cpp
# CPPSRCS += $(UTIL_PATH)/Operations/ArmDownOperation.cpp
# CPPSRCS += $(UTIL_PATH)/Operations/ArmUpOperation.cpp
# CPPSRCS += $(UTIL_PATH)/Operations/ArmForwardOperation.cpp
# CPPSRCS += $(UTIL_PATH)/Operations/ArmBackwardOperation.cpp
# CPPSRCS += $(UTIL_PATH)/Operations/SwingInOperation.cpp
# CPPSRCS += $(UTIL_PATH)/Operations/SwingOutOperation.cpp
 
# the Extra Include Directories which may contain the various header files are listed here
INCDIRS = $(ROOT_PATH)

INCDIRS += $(SRC_PATH)/
INCDIRS += $(PRJ_PATH)
INCDIRS += $(UTIL_PATH)
INCDIRS += $(UTIL_PATH)/ring_buffer
INCDIRS += $(UTIL_PATH)/queue
INCDIRS += $(UTIL_PATH)/parameters
INCDIRS += $(UTIL_PATH)/robot
INCDIRS += $(UTIL_PATH)/state_machine
INCDIRS += $(UTIL_PATH)/stringFuncs/
INCDIRS += $(UTIL_PATH)/delay_timer
INCDIRS += $(UTIL_PATH)/modbus
INCDIRS += $(UTIL_PATH)/vm
INCDIRS += $(UTIL_PATH)/plc_emu

#Set The Compiler Optimization Level Here
OPTIMIZATION = 0

#  Compiler Options
GCFLAGS = -O$(OPTIMIZATION) --std=gnu99
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
REMOVE = cs-rm -f
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




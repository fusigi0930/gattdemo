# don't forget add -Wl-rpath='$$ORIGIN'
PROJ_DIR = .
BUILD_DIR = $(PROJ_DIR)/build
OUT_DIR = $(BUILD_DIR)/out

MODULE = ble_gatt

CC  = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
AR  = $(CROSS_COMPILE)ar
AS  = $(CROSS_COMPILE)as
LD  = $(CROSS_COMPILE)ld
NM  = $(CROSS_COMPILE)nm
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
STRIP   = $(CROSS_COMPILE)strip
SIZE    = $(CROSS_COMPILE)size
MAKE    = make

CUR_ARCH = $(shell $(CC) -dumpmachine)

INC :=
INC += -I$(PROJ_DIR)/lib/blzlib/include

DEFINES :=
DEFINES += -DDEBUG

CXXFLAGS :=
CXXFLAGS += -Wno-unused-variable -MMD -MP
CXXFLAGS += -Wall -Wextra
CXXFLAGS += -Wno-unused-parameter
CXXFLAGS += -Wno-unused-function
CXXFLAGS += -Wno-missing-field-initializers
CXXFLAGS += -std=c++11
CXXFLAGS += -fno-inline
CXXFLAGS += -fPIE -O0 -g
CXXFLAGS += $(INC) $(DEFINES) $(OPTIM)

LIBDIR := -L$(PROJ_DIR)/lib/blzlib/lib/$(CUR_ARCH)

LIBS := $(LIBDIR) -lblzlib -lsystemd

LDFLAGS :=
LDFLAGS += -pthread
LDFLAGS += -fdata-sections -ffunction-sections
LDFLAGS += -Wl,--gc-sections,-Map=$(OUT_DIR)/mapfile
LDFLAGS += -Wl,-rpath='$$ORIGIN'
LDFLAGS += -fPIE

SRCS :=
SRCS += cgatt.cpp
SRCS += main.cpp

:all: $(MODULE)

.SUFFIXES =
.SUFFIXES = .c .o .cpp .cpp.o .h

CXXOBJS :=  $(patsubst %.cpp,$(BUILD_DIR)/%.cpp.o,$(SRCS))

$(BUILD_DIR)/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@


$(MODULE): $(CXXOBJS)
	mkdir -p $(OUT_DIR)
	$(file >$(BUILD_DIR)/build.release, $(shell find $(BUILD_DIR) -name "*.o"))
	$(CXX) -o $(OUT_DIR)/$@ "@$(BUILD_DIR)/build.release" $(LIBS) $(LDFLAGS)
	@echo done

clean:
	rm -rf $(BUILD_DIR)
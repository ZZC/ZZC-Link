# If RACK_DIR is not defined when calling the Makefile, default to two directories above
RACK_DIR ?= ../..

include $(RACK_DIR)/arch.mk

ifeq ($(ARCH), lin)
    CXXFLAGS += -DLINK_PLATFORM_LINUX=1
endif

ifeq ($(ARCH), mac)
    CXXFLAGS += -DLINK_PLATFORM_MACOSX=1
endif

ifeq ($(ARCH), win)
    CXXFLAGS += -DLINK_PLATFORM_WINDOWS=1
    LDFLAGS += -lwsock32 -lws2_32 -liphlpapi
endif

# FLAGS will be passed to both the C and C++ compiler
FLAGS += -isystemlink/include
FLAGS += -isystemlink/include/ableton/link
FLAGS += -isystemlink/modules/asio-standalone/asio/include
ifeq ($(ARCH), win)
    FLAGS += -fpermissive
endif
CFLAGS +=
CXXFLAGS +=

# Add .cpp and .c files to the build
SOURCES += $(wildcard src/*.cpp)

# Add files to the ZIP package when running `make dist`
# The compiled plugin is automatically added.
DISTRIBUTABLES += $(wildcard LICENSE*) res

# Include the VCV Rack plugin Makefile framework
include $(RACK_DIR)/plugin.mk

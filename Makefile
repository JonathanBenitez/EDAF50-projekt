# Define the compiler and the linker. The linker must be defined since
# the implicit rule for linking uses CC as the linker. g++ can be
# changed to clang++.
CXX = g++
CC  = $(CXX)

# Generate dependencies in *.d files
DEPFLAGS = -MT $@ -MMD -MP -MF $*.d

# Define preprocessor, compiler, and linker flags. Uncomment the # lines
# if you use clang++ and wish to use libc++ instead of GNU's libstdc++.
# -g is for debugging.
CPPFLAGS =  -std=c++11 -I.
CXXFLAGS =  -O2 -Wall -Wextra -pedantic-errors -Wold-style-cast
CXXFLAGS += -std=c++11
CXXFLAGS += -g
CXXFLAGS += $(DEPFLAGS)
LDFLAGS =   -g
#CPPFLAGS += -stdlib=libc++
#CXXFLAGS += -stdlib=libc++
#LDFLAGS +=  -stdlib=libc++

# Targets


BUILD_DIR = bin
COPY_FILES = $(BUILD_DIR)/testServer $(BUILD_DIR)/client $(BUILD_DIR)/myclient $(BUILD_DIR)/myServer

all:
	make -C src

install: $(COPY_FILES)

$(BUILD_DIR)/testServer: src/test/testServer
$(BUILD_DIR)/client: src/test/client
$(BUILD_DIR)/myServer: src/test/myserver
$(BUILD_DIR)/myclient: src/test/myclient
$(BUILD_DIR)/%:
	cp -f $< $@
# Create the library; ranlib is for Darwin (OS X) and maybe other systems.
# Doesn't seem to do any damage on other systems.

# Phony targets
.PHONY: all clean distclean

SRC = $(wildcard *.cc)

# Standard clean
# Include the *.d files
-include $(SRC:.cc=.d)


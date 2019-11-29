PROJECT_DIR := ../examples
NAME := fire
BUILD_DIR := build
# Build testbed object files into here
TESTBED_BUILD_DIR = $(BUILD_DIR)/testbed
# Build project object files and binaries into here
PROJECT_BUILD_DIR = $(BUILD_DIR)/project

# Location of the TestBed project
# Does not need to be compiled, VideoCapture.cpp and Main.cpp are used directly by this tool
TESTBED := .

# Location of the 32blit project
# Will be compiled automatically if not available
# Warning: currently does not disambiguate between architecture, so "make clean" this project if it's been used to build an MCU target
# TODO: Allow lib32blit.a to be built into our local $(BUILD_DIR)
32BLIT := ../32blit

# Location of the ffmpeg libraries
# recommended to grab a snapshot and ./configure --prefix=$(pwd)/build && make && make install
FFMPEG := ../ffmpeg-4.1.4/build

# FFMPEG is disabled by default because it requires a glut of new libs
ENABLE_FFMPEG := false


define BOILERPLATE_CPP
#include "$(NAME).hpp"

using namespace blit;

void init() {

}

void render(uint32_t time) {

}

void update(uint32_t time) {

}
endef
export BOILERPLATE_CPP

define BOILERPLATE_HPP
#include "32blit.hpp"
endef
export BOILERPLATE_HPP

PROJECTS := $(wildcard $(PROJECT_DIR)/*)
PROJECT := $(PROJECT_DIR)/$(NAME)
LIB32BLIT := $(32BLIT)/build.32blit-sdl
INC32BLIT := $(32BLIT)

# Use the project folder name as the output binary name
# This is also the name that will prefix video recordings if ENABLE_FFMPEG=true
OUTPUT_NAME = $(shell basename $(PROJECT))

SDL_LIBS = `sdl2-config --libs`

# Glom all .cpp files from the testbed
TESTBED_SRC_FILES := $(TESTBED)/Main.cpp

# Glom all .cpp files from our desired project
PROJECT_SRC_FILES := $(wildcard $(PROJECT)/*.cpp)

LDFLAGS += -l32blit -L$(LIB32BLIT) -L$(TESTBED)
CPPFLAGS += -g -Wall `sdl2-config --cflags` -D__STDC_CONSTANT_MACROS -std=c++11 -I$(INC32BLIT)
CXXFLAGS +=

# Handle requirements for FFMPEG if it's enabled
ifeq ($(ENABLE_FFMPEG), true)
FFMPEG_LIBS = -Wl,-Bstatic -lavformat -lavcodec -lavdevice -lavfilter -lavutil -lswresample -lswscale -Wl,-Bdynamic -llzma -lz
LDFLAGS +=  -L$(FFMPEG)/lib $(FFMPEG_LIBS)
CPPFLAGS +=  -I$(FFMPEG)/include
TESTBED_SRC_FILES += $(TESTBED)/VideoCapture.cpp
else
FFMPEG_LIBS = 
CPPFLAGS += -DNO_FFMPEG_CAPTURE
endif

LDFLAGS += $(SDL_LIBS) -lpthread

# Convert our lists of .cpp files into lists of target .o files
TESTBED_OBJ_FILES = $(patsubst $(TESTBED)/%.cpp,$(TESTBED_BUILD_DIR)/%.o,$(TESTBED_SRC_FILES))
PROJECT_OBJ_FILES = $(patsubst $(PROJECT)/%.cpp,$(PROJECT_BUILD_DIR)/%.o,$(PROJECT_SRC_FILES))
ALL_OBJ_FILES = $(PROJECT_OBJ_FILES) $(TESTBED_OBJ_FILES)



list:
	@echo "\nUsage:"
	@echo "make $(PROJECT_DIR)/<project> (ENABLE_FFMPEG=true)   - Build an existing project"
	@echo "make new NAME=<project>                           - Create a new blank project"
	@echo "\nAvailable projects:"
	@ls $(PROJECT_DIR)

all: $(PROJECTS)

.PHONY: force

$(PROJECTS): force
	$(MAKE) single_project PROJECT=$@ ENABLE_FFMPEG=$(ENABLE_FFMPEG)

lib32blit-sdl:
	make -C ../32blit 32blit-sdl

clean:
	rm -r build

single_project: lib32blit-sdl Makefile $(LIB32BLIT)/lib32blit.a $(PROJECT_BUILD_DIR) $(TESTBED_BUILD_DIR) $(PROJECT_BUILD_DIR)/$(OUTPUT_NAME)

debug:
	echo $(ALL_OBJ_FILES)

new:
	mkdir -p $(PROJECT_DIR)/$(NAME)
	echo "$$BOILERPLATE_CPP" > $(PROJECT_DIR)/$(NAME)/$(NAME).cpp
	echo "$$BOILERPLATE_HPP" > $(PROJECT_DIR)/$(NAME)/$(NAME).hpp

$(LIB32BLIT)/lib32blit.a:
	$(MAKE) -C $(32BLIT) PREFIX= MCU= CXXFLAGS="-fPIC -std=c++11" BUILDDIR=build.testbed

$(PROJECT_BUILD_DIR)/$(OUTPUT_NAME): $(ALL_OBJ_FILES)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(PROJECT_BUILD_DIR)/%.o: $(PROJECT)/%.cpp $(PROJECT)/%.hpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -fPIC -c -o $@ $<

$(TESTBED_BUILD_DIR)/%.o: $(TESTBED)/%.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -fPIC -c -o $@ $<

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(PROJECT_BUILD_DIR): $(BUILD_DIR)
	mkdir -p $(PROJECT_BUILD_DIR)

$(TESTBED_BUILD_DIR): $(BUILD_DIR)
	mkdir -p $(TESTBED_BUILD_DIR)

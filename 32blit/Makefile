
PREFIX := arm-none-eabi-

BUILDDIR := build.$(PREFIX)32blit-stm32
TARGET = $(BUILDDIR)/lib32blit.a
SOURCES = $(wildcard *.cpp, */*.cpp)
OBJECTS = $(addprefix $(BUILDDIR)/, $(notdir $(SOURCES:.cpp=.o)) )

INCLUDES = -I32blit -Itypes -Igraphics -Iengine #-I../lua

CXX = $(PREFIX)g++
AR = $(PREFIX)ar
MCU = -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -fpermissive
LIBS = -lm -lnosys #-llua
LIBDIR = -L../lua

CXXFLAGS = $(MCU) $(INCLUDES) -std=c++11 -fdata-sections -ffunction-sections -Wall -Os
LDFLAGS = -Wl,-E --specs=nano.specs -fPIC -static $(LIBS) $(LIBDIR)

all: dirs $(OBJECTS) $(TARGET)

$(TARGET): $(OBJECTS)
	$(AR) -rcs $@ $^
	arm-none-eabi-ranlib $@

$(BUILDDIR)/%.o : */%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

dirs:
	mkdir -p $(BUILDDIR)

32blit-sdl:
	make MCU= PREFIX=$(PREFIX) BUILDDIR=build.$(PREFIX)32blit-sdl

clean:
	rm $(OBJECTS) $(TARGET)
	rmdir $(BUILDDIR)

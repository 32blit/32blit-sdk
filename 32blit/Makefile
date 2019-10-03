BUILDDIR = build
TARGET = $(BUILDDIR)/lib32blit.a
SOURCES = $(wildcard *.cpp, */*.cpp)
OBJECTS = $(addprefix $(BUILDDIR)/, $(notdir $(SOURCES:.cpp=.o)) )

PREFIX := arm-none-eabi-

INCLUDES = -I32blit -Itypes -Igraphics -Iengine -I../lua

CXX = $(PREFIX)g++
AR = $(PREFIX)ar
MCU = -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -fpermissive
LIBS = -lm -lnosys -llua
LIBDIR = -L../lua

CXXFLAGS = $(MCU) $(INCLUDES) -std=c++11 -fdata-sections -ffunction-sections -Wall -Os
LDFLAGS = -Wl,-E --specs=nano.specs -fPIC -static $(LIBS) $(LIBDIR)

all: dirs $(OBJECTS) $(TARGET)

$(TARGET): $(OBJECTS)
	$(AR) -rcs $@ $^
	ranlib $@

$(BUILDDIR)/%.o : */%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

dirs:
	mkdir -p $(BUILDDIR)

clean:
	rm $(OBJECTS) $(TARGET)
	rmdir $(BUILDDIR)

# Simple Makefile for ICL 1501 emulator

UNAME_S := $(shell uname -s)


CXX = g++
CXXFLAGS = -std=c++20 -O2 -Wall -g
TARGET = icl1501
IMGUI_DIR = ./imgui
IMGUI_SRCS = \
	imgui/imgui.cpp \
	imgui/imgui_draw.cpp \
	imgui/imgui_tables.cpp \
	imgui/imgui_widgets.cpp \
	imgui/imgui_demo.cpp \
	imgui/backends/imgui_impl_sdl2.cpp \
	imgui/backends/imgui_impl_opengl3.cpp

IMGUI_INCLUDES = -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
SDL2_INCLUDES = $(shell sdl2-config --cflags)
SDL2_LIBS = $(shell sdl2-config --libs)

ifeq ($(UNAME_S), Linux)
	GL_LIBS = -lGL
endif
ifeq ($(UNAME_S), Darwin)
	GL_LIBS = -framework OpenGL
endif

#GL_LIBS = -lGL

# Add all '*.cpp' files from the 'src' directory to SRCS
SRCS = $(wildcard src/*.cpp) $(IMGUI_SRCS)
# Place object files in obj/ directory, preserving subdirectory for backends
OBJS = $(patsubst src/%.cpp,obj/%.o,$(filter src/%.cpp,$(SRCS))) \
       $(patsubst imgui/%.cpp,obj/%.o,$(filter imgui/%.cpp,$(SRCS))) \
       $(patsubst imgui/backends/%.cpp,obj/backends/%.o,$(filter imgui/backends/%.cpp,$(SRCS)))

# Dependency files
DEP = $(OBJS:.o=.d)
DEP := $(patsubst obj/%,deps/%,$(DEP))

# Ensure obj and deps directories (with subdirs) exist before compiling
$(shell mkdir -p obj)
$(shell mkdir -p obj/backends)
$(shell mkdir -p deps)
$(shell mkdir -p deps/backends)

# Tell make where to find .cpp files
vpath %.cpp src imgui imgui/backends

# Pattern rule for object files in obj/ from sources in src/ or imgui/
obj/%.o: src/%.cpp $(HDR)
	$(CXX) $(CXXFLAGS) -MMD -MP -MF deps/$(notdir $(@:.o=.d)) -c $< -o $@
obj/%.o: imgui/%.cpp $(HDR)
	$(CXX) $(CXXFLAGS) -MMD -MP -MF deps/$(notdir $(@:.o=.d)) -c $< -o $@
obj/backends/%.o: imgui/backends/%.cpp $(HDR)
	$(CXX) $(CXXFLAGS) -MMD -MP -MF deps/backends/$(notdir $(@:.o=.d)) -c $< -o $@

CXXFLAGS += $(IMGUI_INCLUDES) $(SDL2_INCLUDES)
LDFLAGS = $(SDL2_LIBS) $(GL_LIBS)

MAKEFLAGS += -j

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

emulator.o: tape_reader.hpp

clean:
	rm -f $(TARGET) $(OBJS) $(DEP)
	rm -rf obj
	rm -rf deps

run: $(TARGET)
	./$(TARGET)

# Include dependency files
-include $(DEP)

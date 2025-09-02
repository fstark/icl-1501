# Simple Makefile for ICL 1501 emulator

UNAME_S := $(shell uname -s)


CXX = g++
CXXFLAGS = -std=c++20 -O2 -Wall -g
TARGET = icl1501
IMGUI_DIR = ./imgui
IMGUI_SRCS = \
	$(IMGUI_DIR)/imgui.cpp \
	$(IMGUI_DIR)/imgui_draw.cpp \
	$(IMGUI_DIR)/imgui_tables.cpp \
	$(IMGUI_DIR)/imgui_widgets.cpp \
	$(IMGUI_DIR)/imgui_demo.cpp \
	$(IMGUI_DIR)/backends/imgui_impl_sdl2.cpp \
	$(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp

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

SRCS = addrs.cpp cpu.cpp crt.cpp disassembler.cpp emulator.cpp io.cpp iw.cpp memory.cpp tape.cpp tape_reader.cpp utils.cpp $(IMGUI_SRCS)
OBJS = $(SRCS:.cpp=.o)

# Generate dependency files list from source files
DEP = $(SRCS:.cpp=.d)


CXXFLAGS += $(IMGUI_INCLUDES) $(SDL2_INCLUDES)
LDFLAGS = $(SDL2_LIBS) $(GL_LIBS)

MAKEFLAGS += -j

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp $(HDR)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

emulator.o: tape_reader.hpp

clean:
	rm -f $(TARGET) $(OBJS) $(DEP)

run: $(TARGET)
	./$(TARGET)

# Include dependency files
-include $(DEP)

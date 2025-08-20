# Simple Makefile for ICL 1501 emulator

CXX = g++
CXXFLAGS = -std=c++20 -O2 -Wall
TARGET = icl1501
SRC = addrs.cpp disassembler.cpp emulator.cpp io.cpp iw.cpp memory.cpp tape.cpp tape_reader.cpp utils.cpp 
HDR = $(SRC:.cpp=.hpp)
OBJ = $(SRC:.cpp=.o)

MAKEFLAGS += -j

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJ)

%.o: %.cpp $(HDR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJ)

run: $(TARGET)
	./$(TARGET)

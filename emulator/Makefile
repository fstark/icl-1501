# Simple Makefile for ICL 1501 emulator

CXX = g++
CXXFLAGS = -std=c++20 -O2 -Wall
TARGET = icl1501
SRC = emulator.cpp utils.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

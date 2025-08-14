CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2
TARGET = icl1501_disassembler
SOURCES = icl1501_disassembler.cpp icl1501_disassembler_tests.cpp icl1501_formatter.cpp icl1501_symbols.cpp

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)

# Run the built-in test suite
test: $(TARGET)
	./$(TARGET) --test

.PHONY: all clean test

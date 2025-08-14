#ifndef ICL1501_DISASSEMBLER_H
#define ICL1501_DISASSEMBLER_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <sstream>
#include <cstdint>
#include <algorithm>
#include <cctype>
#include <map>
#include "icl1501_formatter.h"
#include "icl1501_symbols.h"

class SimpleICL1501Disassembler {
private:
    std::vector<uint8_t> program_data;
    ICL1501SymbolTable symbol_table;
    bool use_labels;
    int start_address;  // Linear starting address for the program
    
    void printHeader();
    bool isBRU(int offset);
    bool decodeBRU(int addr, int offset, ICL1501Formatter& formatter);
    void printBRU(int addr, uint8_t byte1, uint8_t byte2, uint8_t page, uint8_t location, ICL1501Formatter& formatter);
    void printUnknown(int addr, int offset, ICL1501Formatter& formatter);
    
public:
    SimpleICL1501Disassembler(bool enable_labels = false);
    
    // Load from hex string (interpreting manual's octal format)
    bool loadFromHexString(const std::string& hex_str);
    
    // Load octal pairs from manual format like "105-042"
    bool loadFromOctalPairs(const std::string& octal_pairs);
    
    void disassemble();
    
    // Test harness (implementation in separate file)
    bool runTests();
};

#endif // ICL1501_DISASSEMBLER_H

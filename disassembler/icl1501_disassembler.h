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
#include <ranges>
#include <string_view>
#include <span>
#include "icl1501_formatter.h"
#include "icl1501_symbols.h"

class SimpleICL1501Disassembler
{
private:
    std::vector<uint8_t> program_data;
    ICL1501SymbolTable symbol_table;
    bool use_labels;
    int start_address; // Linear starting address for the program

    void printHeader();

    // Helper function to format addresses in section-relative format
    std::string formatSectionRelativeAddress(int target_address, int context_address);

    // Helper function to reduce duplication in branch instruction detection
    bool isBranchInstruction(int offset, uint8_t byte1_pattern, uint8_t byte2_lsb);

    // Helper function to reduce duplication in branch instruction decoding
    bool decodeBranchInstruction(int addr, int offset, ICL1501Formatter &formatter,
                                 const std::string &mnemonic, const std::string &description);

    bool isBRU(int offset);
    bool isBRE(int offset);
    bool isBRH(int offset);
    bool isBRL(int offset);
    bool isTLJ(int offset);
    bool isTMJ(int offset);
    bool isTLX(int offset);
    bool isTMX(int offset);
    bool decodeBRU(int addr, int offset, ICL1501Formatter &formatter);
    bool decodeBRE(int addr, int offset, ICL1501Formatter &formatter);
    bool decodeBRH(int addr, int offset, ICL1501Formatter &formatter);
    bool decodeBRL(int addr, int offset, ICL1501Formatter &formatter);
    bool decodeTLJ(int addr, int offset, ICL1501Formatter &formatter);
    bool decodeTMJ(int addr, int offset, ICL1501Formatter &formatter);
    bool decodeTLX(int addr, int offset, ICL1501Formatter &formatter);
    bool decodeTMX(int addr, int offset, ICL1501Formatter &formatter);
    void printBranch(int addr, uint8_t byte1, uint8_t byte2, uint8_t page, uint8_t location, const std::string &mnemonic, const std::string &description, ICL1501Formatter &formatter);
    void printJumpInstruction(int addr, uint8_t byte1, uint8_t byte2, const std::string &mnemonic,
                              const std::string &comment_prefix, ICL1501Formatter &formatter);
    void printExitInstruction(int addr, uint8_t byte1, uint8_t byte2, const std::string &mnemonic,
                              const std::string &comment_prefix, ICL1501Formatter &formatter);
    void createJumpLabel(int address, int offset); // Helper for jump label generation
    void printUnknown(int addr, int offset, ICL1501Formatter &formatter);

public:
    SimpleICL1501Disassembler(bool enable_labels = false);

    // Load from hex string (interpreting manual's octal format)
    bool loadFromHexString(std::string_view hex_str);

    // Load octal pairs from manual format like "105-042"
    bool loadFromOctalPairs(std::string_view octal_pairs);

    void disassemble();

    // Pure disassembly function - returns output as vector of strings
    std::vector<std::string> disassemble(std::span<const uint8_t> data, int start_addr = 0, bool enable_labels = false, bool terse = false);
};

// Standalone test function
bool runICL1501DisassemblerTests();

#endif // ICL1501_DISASSEMBLER_H

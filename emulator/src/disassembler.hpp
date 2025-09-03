#pragma once
#include <string>
#include <bitset>
#include "iw.hpp"
#include "binary.hpp"

class disassembler_t {
public:
    const std::string mnemonic(const iw_t& instruction) const;
    const std::string disassemble(const iw_t& instruction) const;
};

// Free function to disassemble a binary_t
std::string disassemble_binary(const disassembler_t& disassembler, const binary_t& binary);

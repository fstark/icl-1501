#pragma once
#include <string>
#include <bitset>
#include "iw.hpp"

class disassembler_t {
public:
    const std::string mnemonic(const iw_t& instruction) const;
    const std::string disassemble(const iw_t& instruction) const;
};

#include "iw.hpp"
#include "disassembler.hpp"

#include <iostream>

void test_iw_t()
{
    assert(iw_t::instr_map()[0] == iw_t::kTLX);
    assert(iw_t::instr_map()[0b10000011'00100010] == iw_t::kLDX);

    iw_t instruction(0203, 042);
    disassembler_t disasm;
    std::cout << "Instruction: " << disasm.mnemonic(instruction) << std::endl;
    instruction = iw_t(0020, 042);
    std::cout << "Instruction: " << disasm.disassemble(instruction) << std::endl;
}

#include "disassembler.hpp"
#include "utils.hpp"

const std::string disassembler_t::mnemonic(const iw_t& instruction) const
{
    auto map = iw_t::instr_map();
    auto instr = map[instruction.as_word()];
    // std::cout << std::bitset<8>(iwl) << " " << std::bitset<8>(iwr) << " -> "
    //           << std::bitset<16>(((int)iwl) << 8 | iwr) << std::endl;
    // std::cout << (int)instr << std::endl;
    // std::cout << (int)iw_t::kLDX << std::endl;
    return iw_t::types()[(int)instr].mnemonic;
}

const std::string disassembler_t::disassemble(const iw_t& instruction) const
{
    std::string result = mnemonic(instruction);
    auto decode = iw_t::types()[(int)iw_t::instr_map()[instruction.as_word()]].decode;

    if (decode & iw_t::kDECODE_SHIFT)
    {
        result += " " + std::to_string(instruction.shift_count());
    }

    if (decode & iw_t::kDECODE_JUMP)
    {
        result += " " + std::to_string(instruction.signed_jump_count());
    }

    if (decode & iw_t::kDECODE_INDEX_REGISTER)
    {
        result += " " + instruction.indexing_register_name();
    }

    if (decode & iw_t::kDECODE_SECTION_LEVEL)
    {
        result += " P" + std::to_string(instruction.section2()) + std::to_string(instruction.level());
    }

    if (decode & iw_t::kDECODE_INDEX_REGISTER_OP)
    {
        result += " " + iw_t::register_name(instruction.indexing_register(), instruction.indexing_mode());
    }

    if (decode & iw_t::kDECODE_PAGE_NUMBER)
    {
        result += " P" + to_octal(instruction.page_number(), 2);
    }

    if (decode & iw_t::kDECODE_ADRS_LEVEL_BYTE)
    {
        auto a = instruction.address();
        result += " P" + to_octal(a.page(), 1) + "-" + to_octal(a.location());
    }

    if (decode & iw_t::kDECODE_IOC_CHANNEL)
    {
        result += " " + instruction.ioc_channel_name();
    }

    if (decode & iw_t::kDECODE_SECTION)
    {
        result += " S#" + std::to_string(instruction.section1());
    }

    if (decode & iw_t::kDECODE_LITERAL)
    {
        result += " " + std::to_string(instruction.literal());
    }

    if (decode & iw_t::kDECODE_OLITERAL)
    {
        result += " " + to_octal(instruction.literal());
    }

    if (decode & iw_t::kDECODE_BLITERAL)
    {
        result += " " + std::bitset<8>(instruction.literal()).to_string();
    }

    if (decode & iw_t::kDECODE_MASK)
    {
        result += " " + std::bitset<8>(instruction.literal()).to_string();
    }

    if (decode & iw_t::kDECODE_IOC)
    {
        result += " ; " + instruction.describe_ioc_channel() + " " + instruction.describe_ioc_function_code();
    }

    if (decode & iw_t::kDECODE_UV)
    {
        result += " " + std::string(instruction.set_u() ? "+U" : "-U") + std::string(instruction.set_v() ? "+V" : "-V");
    }

    if (decode & iw_t::kDECODE_ADRS_BYTE)
    {
        result += " P-" + to_octal(instruction.literal());
    }

    return result;
}

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

    if (decode & iw_t::kDECODE_DBO)
    {
        result += " " + instruction.as_octal();
    }
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

    if (decode & iw_t::kDECODE_INDEX_REGISTER_OP)
    {
        result += " " + iw_t::register_name(instruction.indexing_register(), instruction.indexing_mode());
    }

    if (decode & iw_t::kDECODE_SECTION_LEVEL)
    {
        result += " P" + std::to_string(instruction.section2()) + std::to_string(instruction.level());
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
        result += " " + std::string(instruction.u() ? "+U" : "-U") + std::string(instruction.v() ? "+V" : "-V");
    }

    if (decode & iw_t::kDECODE_ZP_ADRS)
    {
        result += " P0-" + to_octal(instruction.literal());
    }

    return result;
}

std::string disassemble_binary(const disassembler_t& disassembler, const binary_t& binary)
{
    std::string result;
    
    for (const auto& span : binary.spans())
    {
        // Add ORG directive for each span
        result += "\tORG " + span.start_address_.as_string() + "\n\n";
        
        // Check if span size is odd (cannot be disassembled as instruction words)
        if (span.data_.size() % 2 != 0)
        {
            throw std::runtime_error("Cannot disassemble span at " + span.start_address_.as_string() + 
                                    ": odd number of bytes (" + std::to_string(span.data_.size()) + ")");
        }
        
        // Disassemble each instruction word in the span
        addrs_t current_address = span.start_address_;
        for (size_t i = 0; i < span.data_.size(); i += 2)
        {
            // Create instruction word from two bytes
            uint8_t left_byte = span.data_[i];
            uint8_t right_byte = span.data_[i + 1];
            iw_t instruction(left_byte, right_byte);
            
            // result += current_address.as_string() + ": " + 
            //          instruction.as_octal() + "      " + 
            result += "\t" + disassembler.disassemble(instruction) + "\n";
            
            current_address = current_address.next_instruction();
        }
        
        // Add blank line between spans for readability
        result += "\n\n";
    }
    
    return result;
}

#pragma once

#include <cstdint>
#include <string>
#include <cassert>
#include <cstdio>
#include <bitset>

#include "addrs.hpp"
#include "utils.hpp"

// Instruction word
class iw_t
{
    uint8_t iwl; // left word
    uint8_t iwr; // right word

public:
    iw_t(uint8_t left, uint8_t right) : iwl(left), iwr(right) {}

    typedef enum
    {
        kUnchanged = 0b00,
        kIncrement = 0b10,
        kDecrement = 0b11,
    } eIndexingMode;

    uint16_t as_word() const
    {
        return ((uint16_t)iwl << 8) | iwr; // Combine left and right words into a linear address
    }

    eIndexingMode indexing_mode() const
    {
        return static_cast<eIndexingMode>(iwr & 0b11); // bits 0-1 of the right word
    }

    uint8_t indexing_register() const
    {
        return iwl & 0b111; // bits 0-2 of the left word
    }

    std::string indexing_register_name() const
    {
        return register_name(indexing_register());
    }

    uint8_t indexing_page() const
    {
        return (iwr >> 2) & 0b111111; // bits 2-7 of the right word
    }

    typedef enum
    {
        kClass0 = 0b00, // Jump
        kClass1 = 0b01, // Branch + I/O
        kClass2 = 0b10, // Transfer + Arithmetic
        kClass3 = 0b11  // Boolean + Compare
    } eInstructionClass;

    uint8_t jump_count() const
    {
        return iwl & 0b00011110;
    }

    bool direction() const
    {
        return (iwl & 01) != 0;
    }

    int signed_jump_count() const
    {
        int count = jump_count();
        return direction() ? count : -count;
    }

    uint8_t literal() const
    {
        return iwr;
    }

    addrs_t address() const
    {
        //  Section is always 0, level and address from instruction
        return addrs_t(0, iwl & 0b0000111, iwr & 0b01111110);
    }

    uint8_t section1() const
    {
        return (iwr & 0b00111000) >> 3;
    }

    uint8_t section2() const
    {
        return (iwr & 0b11100000) >> 5;
    }

    uint8_t level() const
    {
        return (iwr & 0b00011100) >> 2;
    }

    //  section + level in top of iwr
    uint8_t page_number() const
    {
        return (iwr & 0b11111100);
    }

    uint8_t shift_count() const
    {
        return iwl & 0b00000111;
    }

    bool set_v() const
    {
        return (iwr & 0b01000000) != 0;
    }

    bool set_u() const
    {
        return (iwr & 0b10000000) != 0;
    }

    bool compare(uint8_t value1, uint8_t mask1, u_int8_t value2, u_int8_t mask2) const
    {
        // Compare the values with the masks
        return (iwl & mask1) == (value1 & mask1) &&
               (iwr & mask2) == (value2 & mask2);
    }

    bool compare(uint8_t value1, uint8_t mask1) const
    {
        return compare(value1, mask1, 0, 0);
    }

    static std::string register_name(uint8_t reg, eIndexingMode mode = kUnchanged)
    {
        if (reg >= 8)
            return "R?";
        std::string reg_name;
        switch (mode)
        {
        case kIncrement:
            reg_name = "I";
            break;
        case kDecrement:
            reg_name = "D";
            break;
        case kUnchanged:
            reg_name = "R";
            break;
        }
        return reg_name + "#" + std::to_string(reg);
    }

    typedef enum
    {
        kUnknown = 0, // Default
        kTLJ,
        kTMJ,
        kTLX,
        kTMX,
        kBRU,
        kBRE,
        kBRH,
        kBRL,
        kSBU,
        kSBE,
        kSBH,
        kSBL,
        kEXB,
        kEXU,
        kSMS,
        kSMC,
        kSSC,
        kSAC,
        kLSW,
        kEMP,
        kLPS,
        kDPI,
        kEPI,
        kCPI,
        kTRM,
        kLDA_Imm,
        kLDA_Dir,
        kLDA_Ind,
        kLDX,
        kLIA,
        kSTA_Dir,
        kSTA_Ind,
        kADA_Imm,
        kADA_Dir,
        kADA_Ind,
        kADX,
        kSUA_Imm,
        kSUA_Dir,
        kSUA_Ind,
        kSUX,
        kANA_Imm,
        kANA_Dir,
        kANA_Ind,
        kSAN,
        kERA_Imm,
        kERA_Dir,
        kERA_Ind,
        kSER,
        kIRA_Imm,
        kIRA_Dir,
        kIRA_Ind,
        kSIR,
        kCPA_Imm,
        kCPA_Dir,
        kCPA_Ind,
        kCPX,
        kIOC
    } eInstructionType;

    typedef struct instruction_def
    {
        eInstructionType instr;
        std::string mnemonic;
        uint16_t value;
        uint16_t mask;
        int decode;
    } instruction_def;

    uint8_t ioc_channel() const
    {
        return iwl & 0b00000111;
    }

    uint8_t ioc_function_code() const
    {
        return iwr;
    }

    std::string ioc_channel_name() const
    {
        assert(ioc_channel() < 8);
        return "C#" + std::to_string(ioc_channel());
    }

    std::string ioc_function_code_name()
    {
        return to_octal(ioc_function_code());
    }

    std::string describe_ioc_channel() const
    {
        auto c = ioc_channel();
        switch (c)
        {
        case 0:
            return "current tape";
        case 1:
            return "tape #1";
        case 2:
            return "tape #2";
        case 3:
            return "keyboard";
        case 4:
            return "CRT";
        default:
            return "[TODO]"; // todo
        }
    }

    std::string describe_ioc_tape() const
    {
        switch (ioc_function_code())
        {
        case 0000:
            return "forward, slow, erase";
        case 0001:
            return "forward, slow";
        case 0002:
            return "forward, fast";
        case 0003:
            return "reverse, slow";
        case 0004:
            return "reverse, fast";
        case 0005:
            return "stop";
        case 0007:
            return "tape transfer byte blocking";
        case 0207:
            return "tape transfer byte non-blocking";
        case 0010:
            return "write mode";
        case 0011:
            return "read mode";
        case 0012:
            return "rewind";
        case 0016:
        case 0026:
        case 0036:
        case 0046:
            return "select deck and load status [TODO]";
        default:
            return "???";
        }
    }

    std::string describe_ioc_keyboard() const
    {
        switch (ioc_function_code())
        {
        case 0007:
            return "keyboard transfer byte blocking";
        case 0207:
            return "keyboard transfer byte non-blocking";
        case 0013:
            return "beep";
        case 0016:
            return "load status";
        default:
            return "???";
        }
    }

    std::string describe_ioc_CRT() const
    {
        // int code = ioc_function_code();
        // int ss = code >> 6;
        // int ll = ((code & 0b00100000) >> 4) + ((code & 0b0000010) >> 1);
        // int i = code & 0b00010000;
        // int u = code & 0b00001000;
        // int d = code & 0b00000100;
        // int m = code & 0b00000001;

        return "[TODO]";
    }

    std::string describe_ioc_function_code() const
    {
        auto c = ioc_channel();
        switch (c)
        {
        case 0:
        case 1:
        case 2:
            return describe_ioc_tape();
        case 3:
            return describe_ioc_keyboard();
        case 4:
            return describe_ioc_CRT();
        default:
            return "[TODO]";
        }
    }

    static const int kDECODE_NONE = 0x00;
    static const int kDECODE_JUMP = 0x01;
    static const int kDECODE_LITERAL = 0x02;
    static const int kDECODE_INDEX_REGISTER = 0x04;
    static const int kDECODE_IOC = 0x08;
    static const int kDECODE_IOC_CHANNEL = 0x10;
    static const int kDECODE_OLITERAL = 0x20;
    static const int kDECODE_INDEX_REGISTER_OP = 0x40;
    static const int kDECODE_PAGE_NUMBER = 0x80;
    static const int kDECODE_ADRS_LEVEL_BYTE = 0x100;
    static const int kDECODE_MASK = 0x200;
    static const int kDECODE_SECTION = 0x400;
    static const int kDECODE_UV = 0x800;
    static const int kDECODE_ADRS_BYTE = 0x1000;
    static const int kDECODE_SECTION_LEVEL = 0x2000;
    static const int kDECODE_BLITERAL = 0x4000;
    static const int kDECODE_SHIFT = 0x8000;

    static const std::vector<instruction_def>
    types()
    {
        static const std::vector<instruction_def> types = {
            instruction_def{kUnknown, "???", 0b00000000'00000000, 0b00000000'00000000, kDECODE_NONE},
            instruction_def{kTLJ, "TLJ", 0b00000000'00000000, 0b11100000'00000000, kDECODE_JUMP | kDECODE_LITERAL},
            instruction_def{kTMJ, "TMJ", 0b00100000'00000000, 0b11100000'00000000, kDECODE_JUMP | kDECODE_MASK},
            instruction_def{kTLX, "TLX", 0b00000000'00000000, 0b11111111'00000000, kDECODE_LITERAL},
            instruction_def{kTMX, "TMX", 0b00100000'00000000, 0b11111111'00000000, kDECODE_MASK},
            instruction_def{kBRU, "BRU", 0b01000000'00000000, 0b11111000'00000001, kDECODE_ADRS_LEVEL_BYTE},
            instruction_def{kBRE, "BRE", 0b01000000'00000001, 0b11111000'00000001, kDECODE_ADRS_LEVEL_BYTE},
            instruction_def{kBRH, "BRH", 0b01001000'00000000, 0b11111000'00000001, kDECODE_ADRS_LEVEL_BYTE},
            instruction_def{kBRL, "BRL", 0b01001000'00000001, 0b11111000'00000001, kDECODE_ADRS_LEVEL_BYTE},
            instruction_def{kSBU, "SBU", 0b01010000'00000000, 0b11111000'00000001, kDECODE_ADRS_LEVEL_BYTE},
            instruction_def{kSBE, "SBE", 0b01010000'00000001, 0b11111000'00000001, kDECODE_ADRS_LEVEL_BYTE},
            instruction_def{kSBH, "SBH", 0b01011000'00000000, 0b11111000'00000001, kDECODE_ADRS_LEVEL_BYTE},
            instruction_def{kSBL, "SBL", 0b01011000'00000001, 0b11111000'00000001, kDECODE_ADRS_LEVEL_BYTE},
            instruction_def{kEXB, "EXB", 0b01110000'00000000, 0b11111000'00000001, kDECODE_ADRS_LEVEL_BYTE},
            instruction_def{kEXU, "EXU", 0b01100000'00000000, 0b11111111'11111111, kDECODE_NONE},
            instruction_def{kSMS, "SMS", 0b01101000'00000000, 0b11111111'11000111, kDECODE_SECTION},
            instruction_def{kSMC, "SMC", 0b01101001'00000000, 0b11111111'00111111, kDECODE_UV},
            instruction_def{kSSC, "SSC", 0b01101010'00000000, 0b11111111'00000111, kDECODE_UV | kDECODE_SECTION},
            instruction_def{kSAC, "SAC", 0b01101011'00000000, 0b11111111'11111111, 0},
            instruction_def{kLSW, "LSW", 0b01101100'00000000, 0b11111111'11111111, 0},
            instruction_def{kEMP, "EMP", 0b01101100'00000000, 0b11111111'11111111, 0}, // replaces LSW
            instruction_def{kLPS, "LPS", 0b01101101'00000000, 0b11111111'11111111, 0},
            instruction_def{kDPI, "DPI", 0b01101110'00000000, 0b11111111'11111111, 0},
            instruction_def{kEPI, "EPI", 0b01101110'00000001, 0b11111111'11111111, 0},
            instruction_def{kCPI, "CPI", 0b01101110'00000010, 0b11111111'11111111, 0},
            instruction_def{kTRM, "TRM", 0b01101111'00000000, 0b11111111'11111111, 0},
            instruction_def{kLDA_Imm, "LDA", 0b10000000'00000000, 0b11111111'00000000, kDECODE_LITERAL},
            instruction_def{kLDA_Dir, "LDA", 0b10001000'00000000, 0b11111111'00000000, kDECODE_ADRS_BYTE},
            instruction_def{kLDA_Ind, "LDA", 0b10001000'00000000, 0b11111000'00000000, kDECODE_INDEX_REGISTER_OP | kDECODE_SECTION_LEVEL},
            instruction_def{kLDX, "LDX", 0b10000000'00000000, 0b11111000'00000000, kDECODE_INDEX_REGISTER | kDECODE_LITERAL},
            instruction_def{kLIA, "LIA", 0b10010000'00000000, 0b11111000'00000000, kDECODE_INDEX_REGISTER | kDECODE_LITERAL},
            instruction_def{kSTA_Dir, "STA", 0b10011000'00000000, 0b11111111'00000000, kDECODE_ADRS_BYTE},
            instruction_def{kSTA_Ind, "STA", 0b10011000'00000000, 0b11111000'00000000, kDECODE_INDEX_REGISTER_OP | kDECODE_PAGE_NUMBER},
            instruction_def{kADA_Imm, "ADA", 0b10100000'00000000, 0b11111111'00000000, kDECODE_LITERAL},
            instruction_def{kADA_Dir, "ADA", 0b10101000'00000000, 0b11111111'00000000, kDECODE_ADRS_BYTE},
            instruction_def{kADA_Ind, "ADA", 0b10101000'00000000, 0b11111000'00000000, kDECODE_INDEX_REGISTER_OP | kDECODE_PAGE_NUMBER},
            instruction_def{kADX, "ADX", 0b10100000'00000000, 0b11111000'00000000, kDECODE_INDEX_REGISTER | kDECODE_LITERAL},
            instruction_def{kSUA_Imm, "SUA", 0b10110000'00000000, 0b11111111'00000000, kDECODE_LITERAL},
            instruction_def{kSUA_Dir, "SUA", 0b10111000'00000000, 0b11111111'00000000, kDECODE_ADRS_BYTE},
            instruction_def{kSUA_Ind, "SUA", 0b10111000'00000000, 0b11111000'00000000, kDECODE_INDEX_REGISTER_OP | kDECODE_PAGE_NUMBER},
            instruction_def{kSUX, "SUX", 0b10110000'00000000, 0b11111000'00000000, kDECODE_INDEX_REGISTER | kDECODE_LITERAL},
            instruction_def{kANA_Imm, "ANA", 0b11000000'00000000, 0b11111111'00000000, kDECODE_BLITERAL},
            instruction_def{kANA_Dir, "ANA", 0b11001000'00000000, 0b11111111'00000000, kDECODE_ADRS_BYTE},
            instruction_def{kANA_Ind, "ANA", 0b11001000'00000000, 0b11111000'00000000, kDECODE_INDEX_REGISTER_OP | kDECODE_PAGE_NUMBER},
            instruction_def{kSAN, "SAN", 0b11000000'00000000, 0b11111000'00000000, kDECODE_SHIFT | kDECODE_BLITERAL},
            instruction_def{kERA_Imm, "ERA", 0b11010000'00000000, 0b11111111'00000000, kDECODE_BLITERAL},
            instruction_def{kERA_Dir, "ERA", 0b11011000'00000000, 0b11111111'00000000, kDECODE_ADRS_BYTE},
            instruction_def{kERA_Ind, "ERA", 0b11011000'00000000, 0b11111000'00000000, kDECODE_INDEX_REGISTER_OP | kDECODE_PAGE_NUMBER},
            instruction_def{kSER, "SER", 0b11010000'00000000, 0b11111000'00000000, kDECODE_SHIFT | kDECODE_BLITERAL},
            instruction_def{kIRA_Imm, "IRA", 0b11110000'00000000, 0b11111111'00000000, kDECODE_BLITERAL},
            instruction_def{kIRA_Dir, "IRA", 0b11111000'00000000, 0b11111111'00000000, kDECODE_ADRS_BYTE},
            instruction_def{kIRA_Ind, "IRA", 0b11111000'00000000, 0b11111000'00000000, kDECODE_INDEX_REGISTER_OP | kDECODE_PAGE_NUMBER},
            instruction_def{kSIR, "SIR", 0b11110000'00000000, 0b11111000'00000000, kDECODE_SHIFT | kDECODE_BLITERAL},
            instruction_def{kCPA_Imm, "CPA", 0b11100000'00000000, 0b11111111'00000000, kDECODE_LITERAL},
            instruction_def{kCPA_Dir, "CPA", 0b11101000'00000000, 0b11111111'00000000, kDECODE_ADRS_BYTE},
            instruction_def{kCPA_Ind, "CPA", 0b11101000'00000000, 0b11111000'00000000, kDECODE_INDEX_REGISTER_OP | kDECODE_PAGE_NUMBER},
            instruction_def{kCPX, "CPX", 0b11100000'00000000, 0b11111000'00000000, kDECODE_INDEX_REGISTER | kDECODE_LITERAL},
            instruction_def{kIOC, "IOC", 0b01111000'00000000, 0b11111000'00000000, kDECODE_IOC_CHANNEL | kDECODE_OLITERAL | kDECODE_IOC}};
        return types;
    }

    static const eInstructionType *instr_map()
    {

        static eInstructionType instr_map[65536] = {kUnknown};

        static bool initialized = false;
        if (!initialized)
        { // We fill from least specific match (least bits) to most specific
            for (int bits = 0; bits != 17; bits++)
            {
                for (auto &type : iw_t::types())
                {
                    if (std::popcount(type.mask) != bits)
                        continue;
                    std::cout << type.mnemonic << " ("
                              << std::bitset<16>(type.value) << ","
                              << std::bitset<16>(type.mask) << ") = " << bits << " bits" << std::endl;
                    for (int i = 0; i < 65536; i++)
                    {
                        iw_t instr(i >> 8, i & 0xff);
                        if (instr.compare(type.value >> 8, type.mask >> 8, type.value & 0xff, type.mask & 0xff))
                        {
                            instr_map[i] = type.instr;
                        }
                    }
                }
            }

            // for (int i = 0; i < 65536; i++)
            // {
            //     if (instr_map[i] == kUnknown)
            //     {
            //         std::cout << "Instruction " << std::oct << std::setfill('0') << std::setw(3) << i
            //                   << " is unknown" << std::dec << std::endl;
            //     }
            //     else
            //     {
            //         std::cout << "Instruction " << std::hex << std::setfill('0') << std::setw(3) << i
            //                   << " is of type " << instr_map[i] << std::dec << std::endl;
            //     }
            // }
            initialized = true;
        }

        return instr_map;
    }

    const std::string as_octal() const
    {
        std::string s = "";
        s += to_octal(iwl);
        s += "-";
        s += to_octal(iwr);
        return s;
    }
};

void test_iw_t();

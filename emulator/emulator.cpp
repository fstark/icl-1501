#include "utils.hpp"
#include <cstdint>
#include <string>
#include <cassert>
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <bit>
#include <bitset>
#include <iomanip>

namespace icl1501
{

    class addrs_t
    {
        uint8_t b0;
        uint8_t b1;

    public:
        addrs_t(uint8_t b0, uint8_t b1) : b0(b0 & 0x3F), b1(b1) {}
        addrs_t(uint8_t section, uint8_t level, uint8_t location)
            : b0((section << 3) | (level & 0x03)), b1(location) {}
        addrs_t(uint16_t linear_address) : b0((linear_address >> 8) & 0x3f), b1(linear_address & 0xFF) {}
        addrs_t(const std::string &v)
        {
            if (v.size() != 7 || v[0] != 'P' || v[3] != '-')
            {
                throw std::invalid_argument("Invalid address format");
            }

            try
            {
                int page = std::stoi(std::string(v.substr(1, 2)), nullptr, 8);
                int location = std::stoi(v.substr(4, 3), nullptr, 8);
                b0 = (page & 0x3F);
                b1 = location & 0xFF;
            }
            catch (const std::exception &e)
            {
                throw std::invalid_argument("Invalid address format");
            }
        }

        uint16_t linear() const
        {
            return (b0 << 8) | b1; // Combine b0 and b1 into a linear address
        }

        uint8_t section() const
        {
            return (b0 >> 3) & 0x07; // Extract bits 3-5 from b0
        }

        uint8_t level() const
        {
            return b0 & 0x03; // Extract bits 0-1 from b0
        }

        uint8_t page() const
        {
            return b0 & 077; // Extract bits 0-5 from b0
        }

        uint8_t location() const
        {
            return b1; // b1
        }

        void set_section(uint8_t section)
        {
            b0 = (b0 & 0x07) | ((section & 0x07) << 3); // Set bits 3-5 to section
        }

        void set_level(uint8_t level)
        {
            b0 = (b0 & 0xFC) | (level & 0x03); // Set bits 0-1 to level
        }

        void set_page(uint8_t page)
        {
            b0 = (b0 & 0xC0) | (page & 0x3F); // Set bits 0-5 to page
        }

        void set_location(uint8_t location)
        {
            b1 = location & 0xFF; // Set b1 to location
        }

        bool operator==(const addrs_t &other) const
        {
            return b0 == other.b0 && b1 == other.b1;
        }

        addrs_t operator+(uint16_t offset) const
        {
            uint16_t new_linear = linear() + offset;
            return addrs_t(new_linear);
        }

        addrs_t next_instruction() const
        {
            return (*this) + 2;
        }
    };

    std::string to_string(const addrs_t &addr)
    {
        char buffer[8];
        snprintf(buffer, sizeof(buffer), "P%02o-%03o", addr.page(), addr.location());
        return buffer;
    }

    void test_addrs_t()
    {
        // Section/level/page/location
        addrs_t addr(012, 034);
        assert(addr.section() == 1);
        assert(addr.level() == 2);
        assert(addr.page() == 012);
        assert(addr.location() == 034);

        // Comparison
        assert(addr == addrs_t(1, 2, 034));

        //  Printing
        assert(to_string(addr) == "P12-034");

        //  Increment and wrap-around
        assert(addrs_t(1, 2, 034).next_instruction() == addrs_t(012, 036));
        assert(addrs_t(077, 0376).next_instruction() == addrs_t(000, 000));

        // Modifications
        addr = addrs_t(012, 034);
        addr.set_section(3);
        addr.set_level(1);
        addr.set_location(045);
        assert(addr == addrs_t(3, 1, 045));
        addr.set_page(042);
        assert(addr == addrs_t(4, 2, 045));
    }

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

        uint8_t section() const
        {
            return (iwr & 0b00111000) >> 3;
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

        static std::string to_octal(uint8_t value, int w = 3)
        {
            char buffer[4];
            char format[5];
            snprintf(format, sizeof(format), "%%0%do", w);
            snprintf(buffer, sizeof(buffer), format, value);
            return std::string(buffer);
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
            return "C#"+std::to_string(ioc_channel());
        }

        std::string ioc_function_code_name()
        {
            return to_octal(ioc_function_code());
        }


        std::string describe_ioc_channel() const
        {    auto c = ioc_channel();
            switch (c)
            {
                case 0: return "current tape";
                case 1: return "tape #1";
                case 2: return "tape #2";
                case 3: return "keyboard";
                case 4: return "CRT";
                default : return "[TODO]"; //todo
            }
        }

        std::string describe_ioc_tape() const
        {
            switch (ioc_function_code())
            {
                case 0000: return "forward, slow, erase";
                case 0001: return "forward, slow";
                case 0002: return "forward, fast";
                case 0003: return "reverse, slow";
                case 0004: return "reverse, fast";
                case 0005: return "stop";
                case 0007: return "tape transfer byte blocking";
                case 0207: return "tape transfer byte non-blocking";
                case 0010: return "write mode";
                case 0011: return "read mode";
                case 0012: return "rewind";
                case 0016:
                case 0026:
                case 0036:
                case 0046: return "select deck and load status [TODO]";
                default: return "???";
            }
        }

        std::string describe_ioc_keyboard() const
        {
            switch (ioc_function_code())
            {
                case 0007: return "keyboard transfer byte blocking";
                case 0207: return "keyboard transfer byte non-blocking";
                case 0013: return "beep";
                case 0016: return "load status";
                default: return "???";
            }
        }

        std::string describe_ioc_CRT() const
        {
            int code = ioc_function_code();
            int ss = code >>6;
            int ll = ((code & 0b00100000)>>4)+(code & 0b0000010)>>1;
            int i = code & 0b00010000;
            int u = code & 0b00001000;
            int d = code & 0b00000100;
            int m = code & 0b00000001;

            return "[TODO]";
        }

        std::string describe_ioc_function_code() const
        {
            auto c = ioc_channel();
            switch (c)
            {
                case 0:
                case 1:
                case 2: return describe_ioc_tape();
                case 3: return describe_ioc_keyboard();
                case 4: return describe_ioc_CRT();
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
        static const int kDECODE_PAGE_ADDRESS = 0x100;

        static const std::vector<instruction_def> types()
        {
            static const std::vector<instruction_def> types = {
                instruction_def{kUnknown, "???"    , 0b00000000'00000000, 0b00000000'00000000, kDECODE_NONE},
                instruction_def{kTLJ    , "TLJ"    , 0b00000000'00000000, 0b11100000'00000000, kDECODE_JUMP|kDECODE_LITERAL},
                instruction_def{kTMJ    , "TMJ"    , 0b00100000'00000000, 0b11100000'00000000, 0},
                instruction_def{kTLX    , "TLX"    , 0b00000000'00000000, 0b11111111'00000000, 0},
                instruction_def{kTMX    , "TMX"    , 0b00100000'00000000, 0b11111111'00000000, 0},
                instruction_def{kBRU    , "BRU"    , 0b01000000'00000000, 0b11111000'00000001, kDECODE_PAGE_ADDRESS},
                instruction_def{kBRE    , "BRE"    , 0b01000000'00000001, 0b11111000'00000001, 0},
                instruction_def{kBRH    , "BRH"    , 0b01001000'00000000, 0b11111000'00000000, 0},
                instruction_def{kBRL    , "BRL"    , 0b01001000'00000001, 0b11111000'00000000, kDECODE_PAGE_ADDRESS},
                instruction_def{kSBU    , "SBU"    , 0b01010000'00000000, 0b11111000'00000001, 0},
                instruction_def{kSBE    , "SBE"    , 0b01010000'00000001, 0b11111000'00000001, 0},
                instruction_def{kSBH    , "SBH"    , 0b01011000'00000000, 0b11111000'00000001, 0},
                instruction_def{kSBL    , "SBL"    , 0b01011000'00000001, 0b11111000'00000001, 0},
                instruction_def{kEXB    , "EXB"    , 0b01110000'00000000, 0b11111000'00000001, 0},
                instruction_def{kEXU    , "EXU"    , 0b01100000'00000000, 0b11111111'11111111, 0},
                instruction_def{kSMS    , "SMS"    , 0b01101000'00000000, 0b11111111'11000111, 0},
                instruction_def{kSMC    , "SMC"    , 0b01101001'00000000, 0b11111111'00111111, 0},
                instruction_def{kSSC    , "SSC"    , 0b01101010'00000000, 0b11111111'00000111, 0},
                instruction_def{kSAC    , "SAC"    , 0b01101011'00000000, 0b11111111'11111111, 0},
                instruction_def{kLSW    , "LSW"    , 0b01101100'00000000, 0b11111111'11111111, 0},
                instruction_def{kEMP    , "EMP"    , 0b01101100'00000000, 0b11111111'11111111, 0}, // replaces LSW
                instruction_def{kLPS    , "LPS"    , 0b01101101'00000000, 0b11111111'11111111, 0},
                instruction_def{kDPI    , "DPI"    , 0b01101110'00000000, 0b11111111'11111111, 0},
                instruction_def{kEPI    , "EPI"    , 0b01101110'00000001, 0b11111111'11111111, 0},
                instruction_def{kCPI    , "CPI"    , 0b01101110'00000010, 0b11111111'11111111, 0},
                instruction_def{kTRM    , "TRM"    , 0b01101111'00000000, 0b11111111'11111111, 0},
                instruction_def{kLDA_Imm, "LDA Imm", 0b10000000'00000000, 0b11111111'00000000, 0},
                instruction_def{kLDA_Dir, "LDA Dir", 0b10001000'00000000, 0b11111111'00000000, 0},
                instruction_def{kLDA_Ind, "LDA Ind", 0b10001000'00000000, 0b11111000'00000000, 0},
                instruction_def{kLDX    , "LDX"    , 0b10000000'00000000, 0b11111000'00000000, kDECODE_INDEX_REGISTER|kDECODE_LITERAL},
                instruction_def{kLIA    , "LIA"    , 0b10010000'00000000, 0b11111000'00000000, 0},
                instruction_def{kSTA_Dir, "STA Dir", 0b10011000'00000000, 0b11111111'00000000, 0},
                instruction_def{kSTA_Ind, "STA"    , 0b10011000'00000000, 0b11111000'00000000, kDECODE_INDEX_REGISTER_OP|kDECODE_PAGE_NUMBER},
                instruction_def{kADA_Imm, "ADA Imm", 0b10100000'00000000, 0b11111111'00000000, 0},
                instruction_def{kADA_Dir, "ADA Dir", 0b10101000'00000000, 0b11111111'00000000, 0},
                instruction_def{kADA_Ind, "ADA Ind", 0b10101000'00000000, 0b11111000'00000000, 0},
                instruction_def{kADX    , "ADX"    , 0b10100000'00000000, 0b11111000'00000000, 0},
                instruction_def{kSUA_Imm, "SUA Imm", 0b10110000'00000000, 0b11111111'00000000, 0},
                instruction_def{kSUA_Dir, "SUA Dir", 0b10111000'00000000, 0b11111111'00000000, 0},
                instruction_def{kSUA_Ind, "SUA Ind", 0b10111000'00000000, 0b11111000'00000000, 0},
                instruction_def{kSUX    , "SUX"    , 0b10110000'00000000, 0b11111000'00000000, 0},
                instruction_def{kANA_Imm, "ANA Imm", 0b11000000'00000000, 0b11111111'00000000, 0},
                instruction_def{kANA_Dir, "ANA Dir", 0b11001000'00000000, 0b11111111'00000000, 0},
                instruction_def{kANA_Ind, "ANA Ind", 0b10001000'00000000, 0b11111000'00000000, 0},
                instruction_def{kSAN    , "SAN"    , 0b11000000'00000000, 0b11111000'00000000, 0},
                instruction_def{kERA_Imm, "ERA Imm", 0b11010000'00000000, 0b11111111'00000000, 0},
                instruction_def{kERA_Dir, "ERA Dir", 0b11011000'00000000, 0b11111111'00000000, 0},
                instruction_def{kERA_Ind, "ERA Ind", 0b11011000'00000000, 0b11111000'00000000, 0},
                instruction_def{kSER    , "SER"    , 0b11010000'00000000, 0b11111000'00000000, 0},
                instruction_def{kIRA_Imm, "IRA Imm", 0b11110000'00000000, 0b11111111'00000000, 0},
                instruction_def{kIRA_Dir, "IRA Dir", 0b11111000'00000000, 0b11111111'00000000, 0},
                instruction_def{kIRA_Ind, "IRA Ind", 0b11111000'00000000, 0b11111000'00000000, 0},
                instruction_def{kSIR    , "SIR"    , 0b11110000'00000000, 0b11111000'00000000, 0},
                instruction_def{kCPA_Imm, "CPA Imm", 0b11100000'00000000, 0b11111111'00000000, 0},
                instruction_def{kCPA_Dir, "CPA Dir", 0b11101000'00000000, 0b11111111'00000000, 0},
                instruction_def{kCPA_Ind, "CPA Ind", 0b11101000'00000000, 0b11111000'00000000, 0},
                instruction_def{kCPX    , "CPX"    , 0b11100000'00000000, 0b11111000'00000000, kDECODE_INDEX_REGISTER|kDECODE_LITERAL},
                instruction_def{kIOC    , "IOC"    , 0b01111000'00000000, 0b11111000'00000000, kDECODE_IOC_CHANNEL|kDECODE_OLITERAL|kDECODE_IOC}};
            return types;
        }

        static const eInstructionType *instr_map()
        {

            static eInstructionType instr_map[65536] = {kUnknown};

            static bool initialized = false;
            if (!initialized)
            {    // We fill from least specific match (least bits) to most specific
                for (int bits = 0; bits != 16; bits++)
                {
                    for (auto &type : iw_t::types())
                    {
                        if (std::popcount(type.mask) != bits)
                            continue;
                        std::cout << type.mnemonic << " ("
                                << std::bitset<16>(type.value) << ","
                                << std::bitset<16>(type.mask) << ")" << std::endl;
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

        const std::string
        mnemonic() const
        {
            auto map = iw_t::instr_map();
            auto instr = map[((int)iwl) << 8 | iwr];
            // std::cout << std::bitset<8>(iwl) << " " << std::bitset<8>(iwr) << " -> "
            //           << std::bitset<16>(((int)iwl) << 8 | iwr) << std::endl;
            // std::cout << (int)instr << std::endl;
            // std::cout << (int)iw_t::kLDX << std::endl;
            return iw_t::types()[(int)instr].mnemonic;
        }

        const std::string disassemble() const
        {
            std::string result = mnemonic();
            auto decode = iw_t::types()[(int)iw_t::instr_map()[((int)iwl) << 8 | iwr]].decode;

            if (decode & kDECODE_JUMP)
            {
                result += " " + std::to_string(signed_jump_count());
            }

            if (decode & kDECODE_INDEX_REGISTER)
            {
                result += " " + indexing_register_name();
            }

            if (decode & kDECODE_INDEX_REGISTER_OP)
            {
                result += " " + register_name(indexing_register(), indexing_mode());
            }

            if (decode & kDECODE_PAGE_NUMBER)
            {   
                result += " P" + to_octal(page_number(),2);
            }

            if (decode & kDECODE_PAGE_ADDRESS)
            {
                auto a = address();
                result += " P" + to_octal(a.page(),1) + "-" + to_octal(a.location());
            }

            if (decode & kDECODE_IOC_CHANNEL)
            {
                result += " " + ioc_channel_name();
            }

            if (decode & kDECODE_LITERAL)
            {
                result += " " + std::to_string(literal());
            }

            if (decode & kDECODE_OLITERAL)
            {
                result += " " + to_octal(literal());
            }

            if (decode & kDECODE_IOC)
            {
                result += " ; " + describe_ioc_channel() + " " + describe_ioc_function_code();
            }

            return result;
        }

        const std::string to_octal() const
        {
            std::string s = "";
            s += to_octal(iwl);
            s += "-";
            s += to_octal(iwr);
            return s;
        }
    };

    void test_iw_t()
    {
        assert( iw_t::instr_map()[0] == iw_t::kTLX );
        assert( iw_t::instr_map()[0b10000011'00100010] == iw_t::kLDX );

        iw_t instruction(0203, 042);
        std::cout << "Instruction: " << instruction.mnemonic() << std::endl;
        instruction = iw_t(0020, 042);
        std::cout << "Instruction: " << instruction.disassemble() << std::endl;
    }
    // stack==P00-040

    class memory_t
    {
        uint8_t data[16384];

    public:
        memory_t() { std::fill(std::begin(data), std::end(data), 0); }

        uint8_t &operator[](size_t index)
        {
            assert(index < sizeof(data));
            return data[index];
        }

        const uint8_t &operator[](size_t index) const
        {
            assert(index < sizeof(data));
            return data[index];
        }

        uint8_t &operator[](addrs_t addr)
        {
            return (*this)[addr.linear()];
        }

        const uint8_t &operator[](addrs_t addr) const
        {
            return (*this)[addr.linear()];
        }

        void get(addrs_t adrs, uint8_t &b0, uint8_t &b1) const
        {
            b0 = (*this)[adrs];
            b1 = (*this)[adrs + 1];
        }

        iw_t get_instruction(addrs_t adrs) const
        {
            uint8_t b0, b1;
            get(adrs, b0, b1);
            return iw_t(b0, b1);
        }

        //  Fill memory from adrs with content of bytes
        void copy(addrs_t adrs, const std::vector<uint8_t> &bytes)
        {
            assert(adrs.linear() + bytes.size() <= sizeof(data));
            for (size_t i = 0; i < bytes.size(); ++i)
            {
                (*this)[adrs + i] = bytes[i];
            }
        }
    };

    void test_memory_t()
    {
        memory_t mem;
        mem[addrs_t(012, 034)] = 0xAB;
        assert(mem[addrs_t(012, 034)] == 0xAB);
        mem[addrs_t(012, 034)] = 0xCD;
        assert(mem[addrs_t(012, 034)] == 0xCD);

        mem.copy(0, {0x01, 0x02, 0x03, 0x04});
        assert(mem[addrs_t(0, 0)] == 0x01);
        assert(mem[addrs_t(0, 1)] == 0x02);
        assert(mem[addrs_t(0, 2)] == 0x03);
        assert(mem[addrs_t(0, 3)] == 0x04);

        mem.copy(0, vector_from_hex("04030201"));
        assert(mem[addrs_t(0, 0)] == 0x04);
        assert(mem[addrs_t(0, 1)] == 0x03);
        assert(mem[addrs_t(0, 2)] == 0x02);
        assert(mem[addrs_t(0, 3)] == 0x01);

        mem.copy(0, vector_from_octal_pairs("105-042 123-056"));
        assert(mem[addrs_t(0, 0)] == 0105);
        assert(mem[addrs_t(0, 1)] == 0042);
        assert(mem[addrs_t(0, 2)] == 0123);
        assert(mem[addrs_t(0, 3)] == 0056);
    }

    class io_t
    {
    };

    class clock_t
    {
        uint64_t cycles;
    };

    class cpu_t
    {
        clock_t clock;
        memory_t &memory;
        io_t &io;

    public:
        cpu_t(memory_t &mem, io_t &io_device) : memory(mem), io(io_device) {}
    };



    void load_bootstrap(memory_t &memory)
    {
        // Load the bootstrap code into memory starting at P01-000
        memory.copy(
            addrs_t("P01-000"),
            vector_from_octal_pairs("201-030 170-007 231-002 341-230 111-003 170-016 170-005 100-030"));
    }

    void test_disassemble_memory()
    {
         memory_t memory;
        load_bootstrap( memory );

        std::cout << "Testing disassembly" << std::endl;

        addrs_t adrs("P01-000");
        for (int i=0;i!=8;i++)
        
         {
            iw_t w = memory.get_instruction(adrs);
            std::cout << to_string(adrs) << ": " << w.to_octal() << "      " << w.disassemble() << std::endl;
            adrs = adrs.next_instruction();
        }
   }
}

int main()
{
    icl1501::test_addrs_t();
    icl1501::test_memory_t();
    icl1501::test_iw_t();
    icl1501::test_disassemble_memory();
    // icl1501::iw_t::test();
    return 0;
}

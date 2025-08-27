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

#include "addrs.hpp"
#include "iw.hpp"
#include "disassembler.hpp"
#include "memory.hpp"

#include "io.hpp"

// stack==P00-040

// class clock_t
// {
//     uint64_t cycles;
// };

class cpu_t
{
    // clock_t clock;
    memory_t &memory_;
    io_t &io_;
    uint8_t sp_;

    disassembler_t disassembler;

    uint8_t sp() const { return sp_ & 0x1f; }

    addrs_t sp_base( int stack ) const
    {
        return addrs_t(0, 040 + stack * 2); 
    }

    addrs_t sp_addrs() const
    {
        return sp_base(sp());
    }

    //  Current instruction address
    addrs_t iaw() const { return memory_.get_addrs(sp_addrs()); }
    void set_iaw(const addrs_t addrs)
    {
        memory_.set_addrs(sp_addrs(), addrs);
    }

    addrs_t index_register_addrs( int reg) const
    {
        return addrs_t(0, reg );
    }

    uint8_t &index_register(int reg)
    {
        assert(reg >= 1 && reg <= 8);
        return memory_[index_register_addrs(reg)];
    }

    const uint8_t &index_register(int reg) const
    {
        assert(reg >= 1 && reg <= 8);
        return memory_[index_register_addrs(reg)];
    }

    
public:
    cpu_t(memory_t &mem, io_t &io_device) : memory_(mem), io_(io_device) {}

    void reset()
    {
        sp_ = 0;
        set_iaw(addrs_t(0, 030));
        compare_ = kEqual;
    }

    void step()
    {
        dump();

        // Fetch the instruction at the current instruction address
        addrs_t pc = iaw();
        iw_t iw = memory_.get_instruction(pc);

        if (!execute( iw ))
        {
            pc = pc.next_instruction();
            set_iaw(pc);
        }
    };

    void register_update(uint8_t &reg, iw_t::eIndexingMode mode)
    {
        switch (mode)
        {
            case iw_t::kIncrement:
                reg++;
                break;
            case iw_t::kDecrement:
                reg--;
                break;
            case iw_t::kUnchanged:
                // Do nothing
                break;
        }
    }

    typedef enum
    {
        kLow,
        kEqual,
        kHigh
    } eCompareResult;

    eCompareResult compare_;

    void compare( uint8_t v0, uint8_t v1)
    {
        if (v0 < v1)
        {
            compare_ = kLow;
        }
        else if (v0 == v1)
        {
            compare_ = kEqual;
        }
        else
        {
            compare_ = kHigh;
        }
    }

    uint8_t section() const
    {
        return iaw().section();
    }

    bool execute(const iw_t &iw)
    {
        bool result = false; // We move to next instruction by default

        // Decode the instruction and execute it
        auto instr_type = iw_t::instr_map()[iw.as_word()];

        switch (instr_type)
        {
            case iw_t::kLDX:
                index_register(iw.indexing_register()) = iw.literal();
                break;
            case iw_t::kIOC:
                io_.execute(iw);
                break;
            case iw_t::kSTA_Ind:
            {
                addrs_t addr{ iw.page_number(), index_register(iw.indexing_register()) };
                memory_[addr] = io_.accumulator();
                register_update(index_register(iw.indexing_register()), iw.indexing_mode());
                break;
            }
            case iw_t::kCPX:
                compare( index_register(iw.indexing_register()), iw.literal());
                break;
            case iw_t::kBRL:
            {
                addrs_t target = iw.address();
                target.set_section(section());
                if (compare_==kLow)
                {
                    set_iaw(target);
                    result = true;
                }
                break;
            }
            case iw_t::kUnknown:
                throw std::runtime_error("Unknown instruction: " + iw.as_octal());
            default:
                disassembler_t disassembler;
                throw std::runtime_error("Unimplemented instruction: " + disassembler.disassemble(iw));
        }
        return result;
    }

    void dump() const
    {
        std::cout << "CPU state:" << std::endl;
        auto pc = iaw();
        auto iw = memory_.get_instruction(pc);

        std::cout << "  " << pc.as_string() << ": ";
        std::cout << iw.as_octal() << "     ";
        std::cout << disassembler.disassemble(iw) << std::endl;

        std::cout << "  SP : ";
        for (int i = 0; i < 8; ++i)
        {
            if (i==sp())
                std::cout << "*";
            std::cout << (sp_base(i)).as_string() << " ";
        }
        std::cout << std::endl;

        std::cout << "  IAW: ";
        for (int i = 0; i < 8; ++i)
        {
            if (i==sp())
                std::cout << "*";
            std::cout << memory_.get_addrs(sp_base(i)).as_string() << " ";
        }
        std::cout << std::endl;

        std::cout << "   ACC R#1 R#2 R#3 R#4 R#5 R#6 R#7 R#8 ";
        static const char *compare_str[] = {"L", "E", "H"};
        std::cout << " CMP:" << compare_str[compare_] << std::endl;
        std::cout << "   ";
        std::cout << to_octal(io_.accumulator()) << " ";
        for (int i = 1; i <= 8; ++i)
            std::cout << to_octal(index_register(i)) << " ";
        std::cout << std::endl;

        memory_.dump( {0,030}, 16);
    }
};

void load_bootstrap(memory_t &memory)
{
    // Load the bootstrap code into memory starting at P01-000
    memory.copy(
        addrs_t("P01-000"),
        vector_from_octal_pairs("201-030 170-007 231-002 341-230 111-003 170-016 170-005 100-030"));
}

void test_disassemble_memory(const std::string &adrs_string, const std::string &data_string)
{
    addrs_t adrs(adrs_string);
    auto data = vector_from_octal_pairs(data_string);

    memory_t memory;
    memory.copy(adrs, data);

    std::cout << "Testing disassembly" << std::endl;

    disassembler_t disassembler;

    for (size_t i = 0; i != data.size() / 2; i++)
    {
        iw_t w = memory.get_instruction(adrs);
        std::cout << adrs.as_string() << ": " << w.as_octal() << "      " << disassembler.disassemble(w) << std::endl;
        adrs = adrs.next_instruction();
    }
}

#include "crt.hpp"

void display( const crt_t::screen_buffer_t screen )
{
    // std::cout << "\033[H"; // Move cursor to home position
    std::cout << "SCREEN:" << std::endl;
    for (auto row=0;row!=crt_t::matrix_height_*crt_t::screen_lines_;row++)
    {
        for (auto col=0;col!=crt_t::matrix_width_*crt_t::screen_columns_;col++)
        {
            std::cout << screen[row][col];
        }
        std::cout << std::endl;
    }
    std::cout << "-------" << std::flush;

    std::string s;
    std::cin >> s;
}

void load_font(memory_t &memory)
{
    memory.copy(
        addrs_t("P04-000"),
        vector_from_octal(
            //    -   _   0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F   G   H   I   J   K   L   M   N   O   P   Q   R   S   T   U   V   W   X   Y   Z   ,   #   @   -   %   $   *   .   <   >   /   (   )   ?   c   =   "   !   '   :   ;   -   \   &   |
            "000 010 010 076 000 162 042 030 047 074 141 066 106 174 177 076 177 177 177 076 177 000 060 177 177 177 177 177 177 076 177 042 001 077 007 177 143 007 141 000 024 014 010 143 044 052 000 000 101 040 034 000 000 034 024 000 000 000 000 000 100 002 060 000 "
            "000 010 010 101 102 111 101 024 105 112 021 111 111 022 111 101 101 111 011 101 010 101 100 010 100 002 006 101 011 101 011 105 001 100 030 040 024 010 121 130 167 062 010 023 052 034 140 010 042 020 042 000 002 042 024 007 000 007 000 000 100 002 116 000 "
            "000 010 176 101 177 111 111 022 105 111 011 111 111 021 111 101 101 111 011 101 010 177 100 024 100 014 010 101 011 121 031 111 177 100 140 030 010 170 111 070 000 052 010 010 177 076 150 024 024 010 101 101 001 177 024 000 137 007 066 133 100 002 131 177 "
            "000 010 010 101 100 111 111 177 105 111 005 111 051 022 111 101 101 101 001 111 010 101 100 042 100 002 060 101 011 041 051 121 001 100 030 040 024 010 105 000 167 072 010 144 052 034 000 042 010 004 000 042 131 042 024 007 000 000 066 073 100 002 046 177 "
            "000 010 010 076 000 106 066 020 071 060 003 066 036 174 066 042 076 101 001 171 177 000 077 101 100 177 177 177 006 136 106 042 001 077 007 177 143 007 103 000 024 074 010 143 022 052 000 101 000 002 000 034 006 000 024 000 000 000 000 000 100 016 120 000"
            ));
    memory.copy(
        addrs_t("P01-000"),
        vector_from_ascii(
            // "--- HELLO, WORLD!"
            " -_0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ,#@-%$*.<>/()?c=\"!':;-\\&|"
            " -_0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ,#@-%$*.<>/()?c=\"!':;-\\&|"
            " -_0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ,#@-%$*.<>/()?c=\"!':;-\\&|"
            " -_0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ,#@-%$*.<>/()?c=\"!':;-\\&|",
            true
        ));
    }

int main(int argc, char **argv)
{
    std::string adrs = "P00-030";
    std::string data = "201-030 170-007 231-002 341-230 111-003 170-016 170-005 100-030";

    if (argc == 2)
    {
        data = argv[1];
    }
    if (argc == 3)
    {
        adrs = argv[1];
        data = argv[2];
    }

    test_addrs_t();
    test_memory_t();
    test_iw_t();
    test_disassemble_memory(adrs, data);
    // icl1501::iw_t::test();

    memory_t memory;
    load_bootstrap(memory);

    load_font(memory);

    io_t io( memory );
    io.execute( iw_t{ 0174, 0b00001010 } ); // IOC C#4 020 ; Screen in P01-000, underline

    cpu_t cpu(memory, io);
    cpu.reset();
    std::cout << "Bootstrap loaded into memory." << std::endl;
    while (1)
    {
        crt_t::screen_buffer_t screen = {};
        io.crt().render( screen);
        display( screen );
        cpu.step();
    }
    std::cout << "CPU step executed." << std::endl;

    return 0;
}

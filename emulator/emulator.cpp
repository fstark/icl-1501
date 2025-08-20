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
        set_iaw(addrs_t(1, 0));
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

int main(int argc, char **argv)
{
    std::string adrs = "P01-000";
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

    io_t io;
    cpu_t cpu(memory, io);
    cpu.reset();
    std::cout << "Bootstrap loaded into memory." << std::endl;
    while (1)
        cpu.step();
    std::cout << "CPU step executed." << std::endl;

    return 0;
}

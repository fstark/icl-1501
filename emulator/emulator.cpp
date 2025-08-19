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

// class clock_t
// {
//     uint64_t cycles;
// };

class cpu_t
{
    // clock_t clock;
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

void test_disassemble_memory(const std::string &adrs_string, const std::string &data_string)
{
    addrs_t adrs(adrs_string);
    auto data = vector_from_octal_pairs(data_string);

        memory_t memory;
    memory.copy(adrs, data);

    std::cout << "Testing disassembly" << std::endl;

    disassembler_t disassembler;

        for (int i = 0; i != data.size() / 2; i++)
    {
        iw_t w = memory.get_instruction(adrs);
        std::cout << adrs.as_string() << ": " << w.as_octal() << "      " << disassembler.disassemble(w) << std::endl;
        adrs = adrs.next_instruction();
    }
}

int main( int argc, char **argv )
{
    std::string adrs = "P01-000";
    std::string data = "201-030 170-007 231-002 341-230 111-003 170-016 170-005 100-030";

    if (argc==2)
    {
        data = argv[1];
    }
    if (argc==3)
    {
        adrs = argv[1];
        data = argv[2];
    }

    test_addrs_t();
    test_memory_t();
    test_iw_t();
    test_disassemble_memory( adrs, data );
    // icl1501::iw_t::test();
    return 0;
}

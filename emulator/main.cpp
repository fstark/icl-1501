#include <cstdint>
#include <string>
#include <cassert>
#include <cstdio>
#include <iostream>

class addrs_t
{
public:
    addrs_t(uint8_t b0, uint8_t b1) : b0(b0 & 0x3F), b1(b1) {}
    addrs_t(uint8_t section, uint8_t level, uint8_t location)
        : b0((section << 3) | (level & 0x03)), b1(location) {}
    addrs_t(uint16_t linear_address) : b0((linear_address >> 8) & 0x3f), b1(linear_address & 0xFF) {}

    uint8_t b0;
    uint8_t b1;

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
    snprintf(buffer, sizeof(buffer), "P%02O-%03O", (addr.b0 & 0b00111111), addr.b1);
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
        return (*this)[(addr.b0 << 8) | addr.b1];
    }

    const uint8_t &operator[](addrs_t addr) const
    {
        return (*this)[(addr.b0 << 8) | addr.b1];
    }

    void get(addrs_t adrs, uint8_t &b0, uint8_t &b1) const
    {
        b0 = (*this)[adrs];
        b1 = (*this)[adrs + 1];
    }
};

void test_memory_t()
{
    memory_t mem;
    mem[addrs_t(012, 034)] = 0xAB;
    assert(mem[addrs_t(012, 034)] == 0xAB);
    mem[addrs_t(012, 034)] = 0xCD;
    assert(mem[addrs_t(012, 034)] == 0xCD);
}

class io_t
{
};

class cpu_t
{
    memory_t &memory;
    io_t &io;

public:
    cpu_t(memory_t &mem, io_t &io_device) : memory(mem), io(io_device) {}
};

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
        kIncrement = 0b01,
        kDecrement = 0b10,
    } eIndexingMode;

    eIndexingMode indexing_mode() const
    {
        return static_cast<eIndexingMode>(iwr & 0b11); // bits 0-1 of the right word
    }

    uint8_t indexing_register() const
    {
        return iwl & 0b111; // bits 0-2 of the left word
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

    int jump_count() const
    {
        int jump_count = (iwl & 0b00011110);
        return iwl & 01 ? jump_count : -jump_count; // If bit 0 is set, it's a backward jump
    }

    uint8_t mask() const
    {
        return iwr;
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

    uint8_t page_address() const
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

    bool compare(uint8_t value, uint8_t mask) const
    {
        // Compare the value with the mask
        return (iwl & mask) == (value & mask);
    }

    static std::string to_octal(uint8_t value)
    {
        char buffer[4];
        snprintf(buffer, sizeof(buffer), "%03o", value);
        return std::string(buffer);
    }

    static std::string register_name(uint8_t reg)
    {
        if (reg >= 8)
            return "I?";
        std::string reg_name = "I";
        return reg_name + std::to_string(reg);
    }

    std::string mnemonic() const
    {
        std::string res = "???";
        if (compare(0200, 0270))
        {
            res = "LDX ";
            res += register_name(indexing_register());
            res += ",";
            res += to_octal(literal());
        }
        return res;
    }
};

void test_iw_t()
{
    iw_t instruction(0203, 042);
    std::cout << "Instruction: " << instruction.mnemonic() << std::endl;
}
// stack==P00-040

int main()
{
    test_addrs_t();
    test_memory_t();
    test_iw_t();
    return 0;
}

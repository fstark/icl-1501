#pragma once
#include <cstdint>
#include <string>
#include <cassert>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <bit>
#include <algorithm>

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

    std::string as_string()
    {
        char buffer[8];
        snprintf(buffer, sizeof(buffer), "P%02o-%03o", page(), location());
        return buffer;
    }
};

void test_addrs_t();
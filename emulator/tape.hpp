#pragma once

#include <vector>
#include <cstdint>
#include <cassert>
#include <string>
#include <stdio.h>

#include "utils.hpp"

const int BPI = 1600; // 1600 bits per inch
const double IPS = 10.0; // 10 inches per second

class tape_location_t
{
    double inches_;
public:
    class position{};
    class time{};
    class byte{};

    tape_location_t(double inches = 0.0,position xx={}) : inches_(inches) {}
    tape_location_t(double t,time xx) : inches_(t*IPS) {}
    tape_location_t(size_t index,byte xx) : inches_(index*8.0/BPI) {}

    double inches() const { return inches_; }

    bool operator<(const tape_location_t &other) const
    {
        return inches_ < other.inches_;
    }

    bool operator==(const tape_location_t &other) const
    {
        return inches_ == other.inches_;
    }

    tape_location_t operator+( tape_location_t other) const
    {
        return tape_location_t(inches_ + other.inches_);
    }

    static tape_location_t one_inch() { return tape_location_t(1.0); }
    static tape_location_t one_second() { return tape_location_t(1.0,time{}); }
    static tape_location_t one_byte() { return tape_location_t(1, byte{}); }

    std::string as_string() const
    {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%04.4f", inches_);
        return buffer;
    }
};

class tape_byte_t
{
    tape_location_t location_;
    uint8_t value_;
public:
    tape_byte_t(tape_location_t loc, uint8_t value) : location_(loc), value_(value) {}

    tape_location_t location() const { return location_; }
    uint8_t value() const { return value_; }

    bool operator<(const tape_byte_t &other) const
    {
        return location_ < other.location_;
    }

    bool operator==(const tape_byte_t &other) const
    {
        return location_ == other.location_ && value_ == other.value_;
    }

    std::string as_string() const
    {
        return location_.as_string() + ": " + to_octal(value_);
    }
};

/**
 * This represent a physical tape.
 * It doesn't move, it just contains data.
 */
class tape_t
{
    //  Very simple version 0
    std::vector<tape_byte_t> data_;

public:
    tape_t(const std::vector<uint8_t> &data = {})
    {
        tape_location_t loc = { 3, tape_location_t::time{} };
        for (const auto &byte : data)
        {
            data_.emplace_back(tape_byte_t{loc, byte});
            loc = loc + tape_location_t::one_byte();
        }
    }

    size_t size() const
    {
        return data_.size();
    }

    uint8_t operator[](size_t index) const
    {
        assert(index < data_.size());
        return data_[index].value();
    }

    void dump() const
    {
        for (const auto &byte : data_)
        {
            printf("%s\n", byte.as_string().c_str());
        }
    }
};

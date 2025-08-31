#pragma once

#include <vector>
#include <cstdint>
#include <cassert>
#include <string>
#include <stdio.h>

#include "utils.hpp"

#include "tape_location.hpp"

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
 * #### : it probably needs to contain the length of the tape
 */
class tape_t
{
    //  Very simple version 0
    std::vector<tape_byte_t> data_;

    tape_location_t end_ = {1200};  //  100 feet default

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

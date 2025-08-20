#pragma once

#include <vector>
#include <cstdint>
#include <cassert>

/**
 * This represent a physical tape.
 * It doesn't move, it just contains data.
 */
class tape_t
{
    //  Very simple version 0
    std::vector<uint8_t> data_;

public:
    tape_t(const std::vector<uint8_t> &data = {})
        : data_(data)
    {
    }

    size_t size() const
    {
        return data_.size();
    }

    uint8_t operator[](size_t index) const
    {
        assert(index < data_.size());
        return data_[index];
    }
};

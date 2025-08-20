#pragma once
#include <cstdint>
#include <vector>
#include <cassert>

#include "iw.hpp"
#include "tape_reader.hpp"

class io_t
{
    tape_reader_t tape_readers_[2];
    int tape_index_ = 1;
    uint8_t accumulator_ = 0;

public:
    io_t()
        : tape_readers_{nullptr,new tape_t({1, 2, 3, 4, 5})}
    {
    }

    uint8_t accumulator()
    {
        return accumulator_;
    }

    static const int kTapeTransferByteBlocking = 0007;

    // Instuction is assumed to be an IOC
    void execute(const iw_t &iw)
    {
        int channel = iw.ioc_channel();
        int function_code = iw.ioc_function_code();

        switch (channel)
        {
        case 1:
        case 2:
            tape_index_ = channel - 1; // fallthrough to read from the tape
        case 0:
            // tape_readers_[tape_index_].execute(function_code);
            switch (function_code)
            {
            case kTapeTransferByteBlocking:
                accumulator_ = tape_readers_[tape_index_].next();
                break;
            default:
                throw std::runtime_error("Unimplemented tape function code: " + std::to_string(function_code));
            }
            break;
        default:
            throw std::runtime_error("Unimplemented IOC channel: " + std::to_string(channel));
        }
    }
};

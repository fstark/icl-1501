#pragma once
#include <cstdint>
#include <vector>
#include <cassert>

#include "iw.hpp"
#include "tape_reader.hpp"
#include "crt.hpp"

class io_t
{
    tape_reader_t tape_readers_[2];
    int tape_index_ = 1;
    uint8_t accumulator_ = 0;
    crt_t crt_;

public:
    io_t( memory_t &memory )
        : tape_readers_{nullptr,new tape_t({1, 2, 3, 4, 5})}
        , crt_(memory)
    {
    }

    uint8_t accumulator()
    {
        return accumulator_;
    }

    static const int kTapeForwardWithErase = 0000;
    static const int kTapeTransferByteBlocking = 0007;

    const crt_t &crt() const
    {
        return crt_;
    }

    int tape_count() const
    {
        return sizeof(tape_readers_) / sizeof(tape_readers_[0]);
    }

    const tape_reader_t &tape_reader(int index) const
    {
        assert(index >= 0 && index < 2);
        return tape_readers_[index];
    }

    int tape_index() const
    {
        return tape_index_;
    }

    // Instuction is assumed to be an IOC
    void execute(const iw_t &iw)
    {
        uint8_t channel = iw.ioc_channel();
        uint8_t function_code = iw.ioc_function_code();

        switch (channel)
        {
        case 1:
        case 2:
            tape_index_ = channel - 1; // fallthrough to read from the tape
        case 0:
        {
            auto &tape_reader = tape_readers_[tape_index_];
                // tape_readers_[tape_index_].execute(function_code);
                switch (function_code)
            {
            case kTapeForwardWithErase:
                tape_reader.set_mode( tape_reader_t::kForwardErase );
                break;
            case kTapeTransferByteBlocking:
            // NO IDEA IF THIS IS THE PROPER THING TO DO
            // I DON"T UNDERSTAND YET WHAT STARTS THE BOOT TAPE
                if (tape_reader.mode()==tape_reader_t::kStop)
                    tape_reader.set_mode(tape_reader_t::kForward);
                accumulator_ = tape_reader.next();
                break;
            default:
                throw std::runtime_error("Unimplemented tape function code: " + std::to_string(function_code));
            }
            break;
        }
        case 4:
            crt_.execute(function_code);
            break;
            default:
            throw std::runtime_error("Unimplemented IOC channel: " + std::to_string(channel));
        }
    }
};

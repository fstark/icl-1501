#pragma once

#include <cstdint>
#include <vector>
#include <iostream>

#include "tape.hpp"

/*
    Info:
    tape can move at 10 ips or 40 ips
    takes 30ms to go from 0 to 10 ips
    can move forward or backward
    can read forward and backward
    can read high speed forward and backward?


    64 microsecond square wave

    1600 dpi, phase-modulated

    Runaway detection:
        if tape runs but no flux transitions for ~5 s (slow) or 50 ms (fast), logic halts tape

    Need transfer‑byte every 512 µs
        or underrun error?
*/

/**
 * This represent a physical tape reader/writer
 */
class tape_reader_t
{
    size_t position_;
    tape_t *tape_;

public:
    tape_reader_t(tape_t *tape)
        : position_(0), tape_(tape)
    {
        if (tape)
        {
            std::cout << "Mounted tape:\n";
            tape->dump();
        }
    }

    bool has_next() const
    {
        return position_ < tape_->size();
    }

    uint8_t next()
    {
        if (!has_next())
            return 0xff;
        return (*tape_)[position_++];
    }
};

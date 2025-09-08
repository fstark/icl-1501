#pragma once

#include "memory.hpp"
#include "io.hpp"
#include "cpu.hpp"
#include "crt.hpp"
#include "breakpoint.hpp"

//  The time as seen by the emulator
//  Time passes in clock ticks from the 6MHz clock
class emu_clock_t
{
    uint64_t ticks_ = 0;                  // Number of ticks since start of emu
    const int kClockFrequency = 6000000; // 6MHz clock
public:

    uint64_t ticks() const { return ticks_; }
    double seconds() const { return static_cast<double>(ticks_) / kClockFrequency; }
    double frequency() const { return static_cast<double>(kClockFrequency); }

    bool operator==(const emu_clock_t &other) const { return ticks_ == other.ticks_; }
    bool operator=(const emu_clock_t &other) const { return ticks_ == other.ticks_; }
    bool operator<(const emu_clock_t &other) const { return ticks_ < other.ticks_; }
    emu_clock_t operator+(int offset)
    {
        emu_clock_t result;
        result.ticks_ = ticks_ + offset;
        return result;
    }
    emu_clock_t operator+=(int offset)
    {
        emu_clock_t c;
        std::cout << "c==" << (c+1).ticks_ << "\n";

        *this = *this + offset;
        return *this;
    }

    std::string to_string() const
    {
        int hours = ticks_ / (kClockFrequency * 3600);
        int minutes = (ticks_ / (kClockFrequency * 60)) % 60;
        int seconds = (ticks_ / kClockFrequency) % 60;
        int milliseconds = (ticks_ % kClockFrequency) * 1000 / kClockFrequency;
        int microseconds = (ticks_ % kClockFrequency) * 1000000 / kClockFrequency;
        int nseconds = (ticks_ % kClockFrequency) * 1000000000 / kClockFrequency;
        char buffer[100];
        snprintf(buffer, sizeof(buffer), "%lu (%02d:%02d:%02d.%03d.%03d.%03d)", ticks_, hours, minutes, seconds, milliseconds, microseconds, nseconds);
        return std::string(buffer);
    }
};

class emulator_t : private breakpoint_delegate_t
{
    emu_clock_t clock_;
    memory_t memory_;
    io_t io_;
    cpu_t cpu_;
    crt_t::screen_buffer_t screen_;
    breakpoint_list_t breakpoints_;

    public:
    emulator_t();

    emu_clock_t clock() const { return clock_; }

    void render_crt();

    cpu_t &cpu();
    io_t &io();
    memory_t &memory();
    crt_t::screen_buffer_t &screen();
    breakpoint_list_t &breakpoints() { return breakpoints_; }
    virtual void breakpoints_changed( const breakpoint_list_t &breakpoints) { cpu_.set_breakpoints(breakpoints_); }
    virtual void breakpoint_changed( const breakpoint_t &breakpoint) { cpu_.set_breakpoints(breakpoints_); }

    void step() { cpu_.step(); clock_ += 4; if (memory_.changed()) render_crt(); }
};

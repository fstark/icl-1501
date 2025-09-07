#pragma once

#include "memory.hpp"
#include "io.hpp"
#include "cpu.hpp"
#include "crt.hpp"
#include "breakpoint.hpp"

class emulator_t : private breakpoint_delegate_t
{
    memory_t memory_;
    io_t io_;
    cpu_t cpu_;
    crt_t::screen_buffer_t screen_;
    breakpoint_list_t breakpoints_;


    public:
    emulator_t();

    void render_crt();

    cpu_t &cpu();
    io_t &io();
    memory_t &memory();
    crt_t::screen_buffer_t &screen();
    breakpoint_list_t &breakpoints() { return breakpoints_; }
    virtual void breakpoints_changed( const breakpoint_list_t &breakpoints) { cpu_.set_breakpoints(breakpoints_); }
    virtual void breakpoint_changed( const breakpoint_t &breakpoint) { cpu_.set_breakpoints(breakpoints_); }
};

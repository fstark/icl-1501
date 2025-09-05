#pragma once

#include "memory.hpp"
#include "io.hpp"
#include "cpu.hpp"
#include "crt.hpp"

class emulator_t
{
    memory_t memory_;
    io_t io_;
    cpu_t cpu_;
    crt_t::screen_buffer_t screen_;
public:
    emulator_t();

    void render_crt();

    cpu_t &cpu();
    io_t &io();
    memory_t &memory();
    crt_t::screen_buffer_t &screen();
};

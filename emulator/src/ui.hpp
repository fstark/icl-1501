#pragma once

#include "emulator.hpp"
#include "addrs.hpp"
#include "memory.hpp"
#include "crt.hpp"

// UI state variables
extern bool show_data;
extern bool show_assembler;
extern addrs_t sAddrs;
extern addrs_t sIaw;

// UI rendering functions
void render_addrs(addrs_t addrs);
void render_addrs(const char *label, addrs_t addrs);
void render_active_addrs(addrs_t addrs, int id);
void render_active_addrs(const char *label, addrs_t addrs, int id);
void render_memory(const memory_t &memory, const addrs_t &addrs);
void render_assembler(emulator_t& emu);
void render_screen(const crt_t::screen_buffer_t &screen);
void render_internals(emulator_t &emu);
void render_emulator(emulator_t &emu);

// Main UI entry point
void run_emulator();

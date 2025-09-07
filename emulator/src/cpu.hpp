#pragma once

#include "utils.hpp"
#include <cstdint>
#include <string>
#include <cassert>
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <bit>
#include <bitset>
#include <iomanip>
#include <array>

#include "addrs.hpp"
#include "iw.hpp"
#include "disassembler.hpp"
#include "memory.hpp"
#include "io.hpp"
#include "breakpoint.hpp"

// stack==P00-040

// class clock_t
// {
//     uint64_t cycles;
// };


class cpu_t
{
    // clock_t clock;
    memory_t &memory_;
    io_t &io_;
    uint8_t sp_;

    disassembler_t disassembler;

    std::array<eBreakpointType, 16384> breakpoint_map_ = {NONE};
    bool breakpoints_enabled = false;
    std::array<eBreakpointType, 16384> breakpoint_map_save;


    eBreakpointType breakpoint_type_ = NONE;
    addrs_t breakpoint_addr_{0,0};

public:
    memory_t &memory() { return memory_; }
    io_t &io() { return io_; }

    class breakpoint_exception : public std::runtime_error
    {
        addrs_t addr_;
        eBreakpointType type_;
        public:
        breakpoint_exception(const std::string &msg, const addrs_t &addr, eBreakpointType type)
            : std::runtime_error(msg), addr_(addr), type_(type) {}
        addrs_t addr() const { return addr_; }
        eBreakpointType type() const { return type_; }
    };

    void set_breakpoints( const breakpoint_list_t &breakpoints )
    {
        breakpoint_map_.fill(NONE);
        for (const auto &bp : breakpoints.breakpoints())
        {
            if (bp.enabled())
            {
                breakpoint_map_[bp.addr().linear()] = static_cast<eBreakpointType>(breakpoint_map_[bp.addr().linear()] | static_cast<eBreakpointType>(bp.mask()));
            }
        }
        breakpoint_map_save = breakpoint_map_;
        std::cout << breakpoint_map_[1] << std::endl;
    }

    void get_breakpoint(addrs_t &addr, eBreakpointType &type) const
    {
        addr = breakpoint_addr_;
        type = breakpoint_type_;
    }

    // Memory access trampoline functions with breakpoint support
    uint8_t read_byte(const addrs_t &addr) const
    {
        if (breakpoints_enabled && breakpoint_map_[addr.linear()] & READ)
        {
            throw breakpoint_exception("Breakpoint hit", addr, READ);
        }
        return memory_[addr];
    }
    void write_byte(const addrs_t &addr, uint8_t value)
    {
        if (breakpoints_enabled && breakpoint_map_[addr.linear()] & WRITE)
        {
            throw breakpoint_exception("Breakpoint hit", addr, WRITE);
        }
        memory_.set(addr, value);
    }
    iw_t read_instruction(const addrs_t &addr) const
    {
        if (breakpoints_enabled && breakpoint_map_[addr.linear()] & EXECUTE)
        {
            throw breakpoint_exception("Breakpoint hit", addr, EXECUTE);
        }
        return memory_.get_instruction(addr);
    }
    
    addrs_t read_address(const addrs_t &addr) const { 
        uint8_t high = read_byte(addr);
        uint8_t low = read_byte(addrs_t(addr.high(), addr.low() + 1));
        return addrs_t(high, low);
    }
    void write_address(const addrs_t &addr, const addrs_t &value) { 
        write_byte(addr, value.high());
        write_byte(addrs_t(addr.high(), addr.low() + 1), value.low());
    }

    uint8_t sp() const { return sp_ & 0x1f; }

    addrs_t sp_base( int stack ) const
    {
        return addrs_t(0, 040 + stack * 2); 
    }

    addrs_t sp_addrs() const
    {
        return sp_base(sp());
    }

    //  Current instruction address
    addrs_t iaw() const { return read_address(sp_addrs()); }
    void set_iaw(const addrs_t addrs)
    {
        write_address(sp_addrs(), addrs);
    }

    addrs_t index_register_addrs( int reg) const
    {
        return addrs_t(0, reg );
    }

    uint8_t index_register(int reg) const
    {
        assert(reg >= 1 && reg <= 8);
        return read_byte(index_register_addrs(reg));
    }

    void set_index_register(int reg, uint8_t b)
    {
        assert(reg >= 1 && reg <= 8);
        write_byte(index_register_addrs(reg), b);
    }

    cpu_t(memory_t &mem, io_t &io_device) : memory_(mem), io_(io_device) {}

    void reset()
    {
        std::cout << "CPU Reset" << std::endl;
        sp_ = 0;
        memory_.enable_rom(true);
        set_iaw(addrs_t(1, 0));
        compare_ = kEqual;
    }

    /*
        This is a bit sophisticated due to breakpoints
        We enable breakpoints only during step, so access to memory done outside
        does not trigger exceptions.
        When a breakpoint occurs, the instruction will not be executed.
        This means that if we redid the instruction, we would hit the same breakpoint again.

        To avoid this, if the previous step ended in a breakpoint, we clear it from the map before executing the instruction, avoiding this specific breakpoint.
        There can still be other breakpoints on the same instruction, in which step will stop again, and at the next step, we will clear this breakpoint again.

        After execution, if we have to hack the map and there was no exception, we restore the map, so those breakpoints will be active again.

        [note there is a potential bug: we should also save the whole memory and restore it if we hit a breakpoint, because (among other problems) an instruction can contain two writes. Example instruction is STA I#1 P00, which will modify the memory cell and increment the index register (which is a memory cell too). The bug does not appear because non-indempotent memory modifications are performed last in the instruction execution, but this is fragile and should be fixed.]
        */

    void step()
    {
        dump();

        bool previously_had_breakpoint = (breakpoint_type_ != NONE);
        if (previously_had_breakpoint)
        {
            // Clear the breakpoint temporarily
            breakpoint_map_[breakpoint_addr_.linear()] = static_cast<eBreakpointType>(breakpoint_map_[breakpoint_addr_.linear()] & ~breakpoint_type_);
        }

        breakpoints_enabled = true;
        breakpoint_type_ = NONE;
        try
        {
            // Fetch the instruction at the current instruction address
            addrs_t pc = iaw();
            iw_t iw = read_instruction(pc);

            if (!execute( iw ))
            {
                pc = pc.next_instruction();
                set_iaw(pc);
            }

            // Restore any cleared breakpoint
            if (previously_had_breakpoint)
                breakpoint_map_ = breakpoint_map_save;
        }
        catch (const breakpoint_exception &e)
        {
            std::cout << "Breakpoint hit at " << e.addr().as_string() << " [" << ( (e.type()&READ)?"R":"") << ( (e.type()&WRITE)?"W":"") << "]" << std::endl;
            breakpoint_addr_ = e.addr();
            breakpoint_type_ = e.type();
        }
        breakpoints_enabled = false;
    };

    void register_update(uint8_t reg, iw_t::eIndexingMode mode)
    {
        addrs_t areg = index_register_addrs(reg);
        switch (mode)
        {
            case iw_t::kIncrement:
                write_byte(areg, read_byte(areg) + 1);
                break;
            case iw_t::kDecrement:
                write_byte(areg, read_byte(areg) - 1);
                break;
            case iw_t::kUnchanged:
                // Do nothing
                break;
        }
    }

    typedef enum
    {
        kLow,
        kEqual,
        kHigh
    } eCompareResult;

    eCompareResult compare_;

    void compare( uint8_t v0, uint8_t v1)
    {
        if (v0 < v1)
        {
            compare_ = kLow;
        }
        else if (v0 == v1)
        {
            compare_ = kEqual;
        }
        else
        {
            compare_ = kHigh;
        }
    }

    uint8_t section() const
    {
        return iaw().section();
    }

    bool execute(const iw_t &iw)
    {
        bool result = false; // We move to next instruction by default

        // Decode the instruction and execute it
        auto instr_type = iw_t::types()[iw_t::instr_map()[iw.as_word()]].instr;

        switch (instr_type)
        {
            case iw_t::kLDX:
                set_index_register(iw.indexing_register(), iw.literal());
                break;
            case iw_t::kIOC:
                io_.execute(iw);
                memory_.clear_crt_monitor();
                memory_.monitor( io_.crt().screen(), 256 );
                memory_.monitor( io_.crt().font(), 320 );
                memory_.monitor( io_.crt().alt_font(), 320 );
                break;
            case iw_t::kSTA_Ind:
            {
                addrs_t addr{ iw.page_number(), index_register(iw.indexing_register()) };
                write_byte(addr, io_.accumulator());
                register_update(iw.indexing_register(), iw.indexing_mode());
                break;
            }
            case iw_t::kCPX:
                compare( index_register(iw.indexing_register()), iw.literal());
                break;
            case iw_t::kBRL:
            {
                addrs_t target = iw.address();
                target.set_section(section());
                if (compare_==kLow)
                {
                    set_iaw(target);
                    result = true;
                }
                break;
            }
            //note : SBU and other stack operation must memory_.enable_rom(false);

            case iw_t::kUnknown:
                throw std::runtime_error("Unknown instruction: " + iw.as_octal());
            default:
                disassembler_t disassembler;
                throw std::runtime_error("Unimplemented instruction: " + disassembler.disassemble(iw));
        }
        return result;
    }

    void dump() const
    {
        std::cout << "CPU state:" << std::endl;
        auto pc = iaw();
        auto iw = read_instruction(pc);

        std::cout << "  " << pc.as_string() << ": ";
        std::cout << iw.as_octal() << "     ";
        std::cout << disassembler.disassemble(iw) << std::endl;

        std::cout << "  SP : ";
        for (int i = 0; i < 8; ++i)
        {
            if (i==sp())
                std::cout << "*";
            std::cout << (sp_base(i)).as_string() << " ";
        }
        std::cout << std::endl;

        std::cout << "  IAW: ";
        for (int i = 0; i < 8; ++i)
        {
            if (i==sp())
                std::cout << "*";
            std::cout << read_address(sp_base(i)).as_string() << " ";
        }
        std::cout << std::endl;

        std::cout << "   ACC R#1 R#2 R#3 R#4 R#5 R#6 R#7 R#8 ";
        static const char *compare_str[] = {"L", "E", "H"};
        std::cout << " CMP:" << compare_str[compare_] << std::endl;
        std::cout << "   ";
        std::cout << to_octal(io_.accumulator()) << " ";
        for (int i = 1; i <= 8; ++i)
            std::cout << to_octal(index_register(i)) << " ";
        std::cout << std::endl;

        memory_.dump( {0,030}, 16);
    }
};

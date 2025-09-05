#pragma once

#include "addrs.hpp"
#include <vector>
#include <array>
#include <tuple>

typedef enum
{
    READ = 1,
    WRITE = 2,
    EXECUTE = 4
} eBreakpointType;

class breakpoint_t
{
    addrs_t addr_;
    bool enabled_ = true;
    int mask_ = 0;

    public:
    breakpoint_t(addrs_t addr, eBreakpointType type) : addr_(addr), mask_(type) {}

    addrs_t addr() const { return addr_; }
    bool enabled() const { return enabled_; }
    int mask() const { return mask_; }

    void set_enabled(bool e) { enabled_ = e; }

    void update_mask(eBreakpointType type, bool enable)
    {
        if (enable)
            mask_ |= type;
        else
            mask_ &= ~type;
    }
};

class breakpoint_list_t
{
    std::vector<breakpoint_t> breakpoints_;
    std::array<int, 16384> breakpoint_map_ = {0};

    void sort_breakpoints()
    {
        std::sort(breakpoints_.begin(), breakpoints_.end(),
                  [](const breakpoint_t &a, const breakpoint_t &b) { return a.addr().linear() < b.addr().linear(); });
    }

    void rebuild_map()
    {
        breakpoint_map_.fill(0);
        for (const auto &bp : breakpoints_)
        {
            if (bp.enabled())
            {
                breakpoint_map_[bp.addr().linear()] = bp.mask();
            }
        }
    }
public:
    void add_breakpoint(const addrs_t &addr, eBreakpointType type)
    {
        // incorrect, we should merge with existing breakpoint if any
        remove_breakpoint(addr);
        breakpoints_.emplace_back(addr, type);
        sort_breakpoints();
        rebuild_map();
    }
    void remove_breakpoint(const addrs_t &addr)
    {
        breakpoints_.erase(std::remove_if(breakpoints_.begin(), breakpoints_.end(),
                                          [&addr](const breakpoint_t &bp) { return bp.addr() == addr; }),
                           breakpoints_.end());
        sort_breakpoints();
        rebuild_map();
    }
    const std::vector<breakpoint_t> &breakpoints() const
    {
        return breakpoints_;
    }

    std::vector<breakpoint_t> &breakpoints()
    {
        return breakpoints_;
    }

    bool is_breakpoint(const addrs_t &addr, eBreakpointType type) const
    {
        if (addr.linear() < 0 || addr.linear() >= breakpoint_map_.size())
            return false;
        return !!(breakpoint_map_[addr.linear()] & type);
    }
};

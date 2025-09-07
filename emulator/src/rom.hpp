#pragma once

#include <cstdint>
#include <cstddef>

// A simple ROM (Read-Only Memory) class
// Contains 16 bytes of bootstrap code at P01-000 / 256
class rom_t
{
private:
    uint8_t data_[16];

public:
    // Constructor - loads the bootstrap code
    rom_t();
    
    // Read a byte at the given offset
    // Returns 0 if offset is outside ROM range (>= 16)
    uint8_t get(size_t offset) const;
};

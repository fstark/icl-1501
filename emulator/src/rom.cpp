#include "rom.hpp"
#include "utils.hpp"

rom_t::rom_t()
{
    // Bootstrap code: "201-030 170-007 231-002 341-230 111-003 170-016 170-005 100-030"
    auto bootstrap = vector_from_octal_pairs("201-030 170-007 231-002 341-230 111-003 170-016 170-005 100-030");
    
    // Copy to our fixed-size array
    for (size_t i = 0; i < 16 && i < bootstrap.size(); ++i) {
        data_[i] = bootstrap[i];
    }
    
    // Fill remaining bytes with zeros if needed
    for (size_t i = bootstrap.size(); i < 16; ++i) {
        data_[i] = 0;
    }
}

uint8_t rom_t::get(size_t offset) const
{
    if (offset >= 16)
		return 0;
    return data_[offset];
}

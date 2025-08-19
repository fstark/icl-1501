#include "addrs.hpp"

void test_addrs_t()
{
    // Section/level/page/location
    addrs_t addr(012, 034);
    assert(addr.section() == 1);
    assert(addr.level() == 2);
    assert(addr.page() == 012);
    assert(addr.location() == 034);

    // Comparison
    assert(addr == addrs_t(1, 2, 034));

    //  Printing
    assert(addr.as_string() == "P12-034");

    //  Increment and wrap-around
    assert(addrs_t(1, 2, 034).next_instruction() == addrs_t(012, 036));
    assert(addrs_t(077, 0376).next_instruction() == addrs_t(000, 000));

    // Modifications
    addr = addrs_t(012, 034);
    addr.set_section(3);
    addr.set_level(1);
    addr.set_location(045);
    assert(addr == addrs_t(3, 1, 045));
    addr.set_page(042);
    assert(addr == addrs_t(4, 2, 045));
}

#include "memory.hpp"

void test_memory_t()
{
    memory_t mem;
    mem[addrs_t(012, 034)] = 0xAB;
    assert(mem[addrs_t(012, 034)] == 0xAB);
    mem[addrs_t(012, 034)] = 0xCD;
    assert(mem[addrs_t(012, 034)] == 0xCD);

    mem.copy(0, {0x01, 0x02, 0x03, 0x04});
    assert(mem[addrs_t(0, 0)] == 0x01);
    assert(mem[addrs_t(0, 1)] == 0x02);
    assert(mem[addrs_t(0, 2)] == 0x03);
    assert(mem[addrs_t(0, 3)] == 0x04);

    mem.copy(0, vector_from_hex("04030201"));
    assert(mem[addrs_t(0, 0)] == 0x04);
    assert(mem[addrs_t(0, 1)] == 0x03);
    assert(mem[addrs_t(0, 2)] == 0x02);
    assert(mem[addrs_t(0, 3)] == 0x01);

    mem.copy(0, vector_from_octal_pairs("105-042 123-056"));
    assert(mem[addrs_t(0, 0)] == 0105);
    assert(mem[addrs_t(0, 1)] == 0042);
    assert(mem[addrs_t(0, 2)] == 0123);
    assert(mem[addrs_t(0, 3)] == 0056);
}

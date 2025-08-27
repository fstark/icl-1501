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

#include "addrs.hpp"
#include "iw.hpp"
#include "disassembler.hpp"
#include "memory.hpp"
#include "cpu.hpp"

void load_bootstrap(memory_t &memory)
{
    // Load the bootstrap code into memory starting at P01-000
    memory.copy(
        addrs_t("P01-000"),
        vector_from_octal_pairs("201-030 170-007 231-002 341-230 111-003 170-016 170-005 100-030"));
}

void test_disassemble_memory(const std::string &adrs_string, const std::string &data_string)
{
    addrs_t adrs(adrs_string);
    auto data = vector_from_octal_pairs(data_string);

    memory_t memory;
    memory.copy(adrs, data);

    std::cout << "Testing disassembly" << std::endl;

    disassembler_t disassembler;

    for (size_t i = 0; i != data.size() / 2; i++)
    {
        iw_t w = memory.get_instruction(adrs);
        std::cout << adrs.as_string() << ": " << w.as_octal() << "      " << disassembler.disassemble(w) << std::endl;
        adrs = adrs.next_instruction();
    }
}

#include "crt.hpp"

void display( const crt_t::screen_buffer_t screen )
{
    // std::cout << "\033[H"; // Move cursor to home position
    std::cout << "SCREEN:" << std::endl;
    for (auto row=0;row!=crt_t::matrix_height_*crt_t::screen_lines_;row++)
    {
        for (auto col=0;col!=crt_t::matrix_width_*crt_t::screen_columns_;col++)
        {
            std::cout << screen[row][col];
        }
        std::cout << std::endl;
    }
    std::cout << "-------" << std::flush;

    std::string s;
    std::cin >> s;
}

void load_font(memory_t &memory)
{
    memory.copy(
        addrs_t("P04-000"),
        vector_from_octal(
            //    -   _   0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F   G   H   I   J   K   L   M   N   O   P   Q   R   S   T   U   V   W   X   Y   Z   ,   #   @   -   %   $   *   .   <   >   /   (   )   ?   c   =   "   !   '   :   ;   -   \   &   |
            "000 010 010 076 000 162 042 030 047 074 141 066 106 174 177 076 177 177 177 076 177 000 060 177 177 177 177 177 177 076 177 042 001 077 007 177 143 007 141 000 024 014 010 143 044 052 000 000 101 040 034 000 000 034 024 000 000 000 000 000 100 002 060 000 "
            "000 010 010 101 102 111 101 024 105 112 021 111 111 022 111 101 101 111 011 101 010 101 100 010 100 002 006 101 011 101 011 105 001 100 030 040 024 010 121 130 167 062 010 023 052 034 140 010 042 020 042 000 002 042 024 007 000 007 000 000 100 002 116 000 "
            "000 010 176 101 177 111 111 022 105 111 011 111 111 021 111 101 101 111 011 101 010 177 100 024 100 014 010 101 011 121 031 111 177 100 140 030 010 170 111 070 000 052 010 010 177 076 150 024 024 010 101 101 001 177 024 000 137 007 066 133 100 002 131 177 "
            "000 010 010 101 100 111 111 177 105 111 005 111 051 022 111 101 101 101 001 111 010 101 100 042 100 002 060 101 011 041 051 121 001 100 030 040 024 010 105 000 167 072 010 144 052 034 000 042 010 004 000 042 131 042 024 007 000 000 066 073 100 002 046 177 "
            "000 010 010 076 000 106 066 020 071 060 003 066 036 174 066 042 076 101 001 171 177 000 077 101 100 177 177 177 006 136 106 042 001 077 007 177 143 007 103 000 024 074 010 143 022 052 000 101 000 002 000 034 006 000 024 000 000 000 000 000 100 016 120 000"
        ));
    memory.copy(
        addrs_t("P06-000"),
        vector_from_octal(
            //    -   _   0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F   G   H   I   J   K   L   M   N   O   P   Q   R   S   T   U   V   W   X   Y   Z   ,   #   @   -   %   $   *   .   <   >   /   (   )   ?   c   =   "   !   '   :   ;   -   \   &   |
            "000 010 010 076 000 162 042 030 047 074 141 066 106 174 177 076 177 177 177 076 177 000 060 177 177 177 177 177 177 076 177 042 001 077 007 177 143 007 141 000 024 014 010 143 044 052 000 000 101 040 034 000 000 034 024 000 000 000 000 000 100 002 060 000 "
            "000 010 010 101 102 111 101 024 105 112 021 111 111 022 111 101 101 111 011 101 010 101 100 010 100 002 006 101 011 101 011 105 001 100 030 040 024 010 121 130 167 062 010 023 052 034 140 010 042 020 042 000 002 042 024 007 000 007 000 000 100 002 116 000 "
            "000 010 176 101 177 111 111 022 105 111 011 111 111 021 111 101 101 111 011 101 010 177 100 024 100 014 010 101 011 121 031 111 177 100 140 030 010 170 111 070 000 052 010 010 177 076 150 024 024 010 101 101 001 177 024 000 137 007 066 133 100 002 131 177 "
            "000 010 010 101 100 111 111 177 105 111 005 111 051 022 111 101 101 101 001 111 010 101 100 042 100 002 060 101 011 041 051 121 001 100 030 040 024 010 105 000 167 072 010 144 052 034 000 042 010 004 000 042 131 042 024 007 000 000 066 073 100 002 046 177 "
            "000 010 010 076 000 106 066 020 071 060 003 066 036 174 066 042 076 101 001 171 177 000 077 101 100 177 177 177 006 136 106 042 001 077 007 177 143 007 103 000 024 074 010 143 022 052 000 101 000 002 000 034 006 000 024 000 000 000 000 000 100 016 120 000"
        ));
    for (addrs_t a = addrs_t{"P06-000"}; a!=addrs_t{"P07-177"}; a = a+1)
    {
        memory[a] ^= 0xff; // Inverted font
    }
    memory.copy("P01-000", vector_from_ascii( " -_0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ,#@-%$*.<>/()?c=\"!':;-\\&|"));
    memory.copy("P01-100", vector_from_ascii(" -_0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ,#@-%$*.<>/()?c=\"!':;-\\&|",true));
    memory.copy("P01-200", vector_from_ascii(" -_0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ,#@-%$*.<>/()?c=\"!':;-\\&|"));
    memory.copy("P01-300", vector_from_ascii(" -_0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ,#@-%$*.<>/()?c=\"!':;-\\&|",true));
}

void test_cpu_t()
{
    memory_t memory;
    load_bootstrap(memory);

    load_font(memory);

    io_t io( memory );
    // io.execute(iw_t{0174, 0b00000010}); // IOC C#4 020 ; Screen in P01-000, no underline
    io.execute(iw_t{0174, 0b00001010}); // IOC C#4 020 ; Screen in P01-000, underline

    cpu_t cpu(memory, io);
    cpu.reset();
    std::cout << "Bootstrap loaded into memory." << std::endl;
    while (1)
    {
        crt_t::screen_buffer_t screen = {};
        io.crt().render( screen);
        display( screen );
        cpu.step();
    }
    std::cout << "CPU step executed." << std::endl;
}

void test_imgui()
{
    // IMGUI TEST GOES HERE
}

int main(int argc, char **argv)
{
    std::string adrs = "P00-030";
    std::string data = "201-030 170-007 231-002 341-230 111-003 170-016 170-005 100-030";

    if (argc == 2)
    {
        data = argv[1];
    }
    if (argc == 3)
    {
        adrs = argv[1];
        data = argv[2];
    }

    test_addrs_t();
    test_memory_t();
    test_iw_t();
    test_disassemble_memory(adrs, data);
    // icl1501::iw_t::test();

    test_cpu_t();

    test_imgui();

    return 0;
}

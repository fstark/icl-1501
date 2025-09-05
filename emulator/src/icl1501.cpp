#include "emulator.hpp"
#include "ui.hpp"
#include "utils.hpp"
#include <cstdint>
#include <string>
#include <cassert>
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <fstream>

#include "addrs.hpp"
#include "iw.hpp"
#include "disassembler.hpp"
#include "assembler.hpp"
#include "binary.hpp"

using namespace std::literals;

void cmd_assemble(const std::string &source_file, const std::string &output_file = "")
{
    std::ifstream source(source_file);
    if (!source)
    {
        throw std::runtime_error("Failed to open source file: " + source_file);
    }

    // Read the entire file content
    std::string content((std::istreambuf_iterator<char>(source)),
                       std::istreambuf_iterator<char>());

    std::ostream *out;
    std::ofstream output;
    if (output_file.empty()) {
        out = &std::cout;
    } else {
        output.open(output_file, std::ios::binary);
        if (!output)
            throw std::runtime_error("Failed to open output file: " + output_file);
        out = &output;
    }

    try {
        binary_t binary = assemble(content);
        *out << to_string(binary);
    }
    catch (const std::exception &e) {
        std::cerr << "Assembly failed: " << e.what() << std::endl;
        throw;
    }
}

void cmd_disassemble(const std::string &source_file, const std::string &output_file = "")
{
    std::ifstream source(source_file, std::ios::binary);
    if (!source)
    {
        throw std::runtime_error("Failed to open source file: " + source_file);
    }

    // Read the entire file content
    std::string content((std::istreambuf_iterator<char>(source)),
                       std::istreambuf_iterator<char>());
    
    std::ostream *out;
    std::ofstream output;
    if (output_file.empty()) {
        out = &std::cout;
    } else {
        output.open(output_file);
        if (!output)
            throw std::runtime_error("Failed to open output file: " + output_file);
        out = &output;
    }

    try {
        // Parse the content as a binary_t
        binary_t binary = binary_from_string(content);
        
        // Create disassembler and generate disassembly
        disassembler_t disassembler;
        std::string disassembly = disassemble_binary(disassembler, binary);
        
        // Output the result
        *out << disassembly;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error disassembling file: " << e.what() << std::endl;
        throw;
    }
}

void test_assembler1( iw_t iw )
{
    iw_t iw2{0,0};

    disassembler_t disassembler;
    auto str = " "s+disassembler.disassemble(iw);
    std::cout << iw.as_octal() << " =>" << str << " => " << std::flush;
    assembler_t assembler;
    if (!assembler.assemble(str, iw2))
    {
        std::cout << str << " does not assemble " << std::endl;
            throw std::runtime_error("Assembler round trip failed");
    }
    else
        if (iw!= iw2)
        {
            std::cout << str << " does not assemble to " << iw.as_octal() << " but to " << iw2.as_octal() << std::endl;
            throw std::runtime_error("Assembler round trip failed");
        }
    std::cout << iw2.as_octal() << std::endl;
}

void test_assembler()
{
    iw_t iw{0201,0030};
    for (int i=0;i!=65536;i++)
    {
        iw.set_word(i);
        // std::cout << iw.as_octal() << " " << std::flush;
        test_assembler1(iw);
    }
}

void run_tests()
{
    // std::string adrs = "P00-030";
    // std::string data = "201-030 170-007 231-002 341-230 111-003 170-016 170-005 100-030";

    // test_addrs_t();
    // test_memory_t();
    // test_iw_t();
    // test_disassemble_memory(adrs, data);

    // test_cpu_t();

    test_assembler();
}


int main(int argc, char **argv)
{
    if (argc >= 2) {
        std::string mode = argv[1];
        if (mode == "-d" && argc >= 3) {
            std::string infile = argv[2];
            std::string outfile = (argc >= 4) ? argv[3] : "";
            cmd_disassemble(infile, outfile);
            return 0;
        } else if (mode == "-a" && argc >= 3) {
            std::string infile = argv[2];
            std::string outfile = (argc >= 4) ? argv[3] : "";
            cmd_assemble(infile, outfile);
            return 0;
        } else if (mode == "-t") {
            run_tests();
            return 0;
        } else {
            std::cerr << "Usage:\n";
            std::cerr << "  " << argv[0] << "            # run emulator\n";
            std::cerr << "  " << argv[0] << " -d infile [outfile]  # disassemble\n";
            std::cerr << "  " << argv[0] << " -a infile [outfile]  # assemble\n";
            std::cerr << "  " << argv[0] << " -t  # run tests\n";
            return 1;
        }
    }
    run_emulator();
    return 0;
}

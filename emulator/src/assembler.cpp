#include "assembler.hpp"
#include "binary.hpp"
#include <sstream>
#include <stdexcept>

// Static helper function for a single assembly pass
static void assemble_pass(const std::string &source, int pass, assembler_t &assembler, binary_writer_t &writer, std::ostringstream &error_stream)
{
    std::istringstream text_stream(source);
    std::string line;
    int line_number = 0;
    
    assembler.set_pass(pass);
    assembler.set_addr(addrs_t("P00-000"));
    
    while (std::getline(text_stream, line)) {
        line_number++;
        
        // Skip empty lines and comments
        if (line.empty() || line[0] == '/' || line[0] == ';') {
            continue;
        }
        
        try {
            iw_t iw{0, 0};
            if (assembler.assemble(line, iw)) {
                // Only write output on pass 1
                if (pass == 1) {
                    writer.append_iw(iw);
                }
            }
        }
        catch (const std::exception &e) {
            error_stream << "Pass " << pass << " Error on line " << line_number << ": " << e.what() << "\n";
            error_stream << "  " << line << "\n";
            throw; // Re-throw to stop assembly
        }
    }
}

// Free function to assemble source code into a binary_t
binary_t assemble(const std::string &source)
{
    assembler_t assembler;
    binary_writer_t writer;
    std::ostringstream error_stream;
    
    try {
        // Pass 0: Symbol collection and validation
        assemble_pass(source, 0, assembler, writer, error_stream);
        
        // Pass 1: Code generation
        assemble_pass(source, 1, assembler, writer, error_stream);
        
        return writer.take_binary();
    }
    catch (const std::exception &e) {
        // Include any collected errors in the exception message
        std::string full_error = error_stream.str();
        if (!full_error.empty()) {
            throw std::runtime_error(full_error);
        } else {
            throw; // Re-throw original exception
        }
    }
}

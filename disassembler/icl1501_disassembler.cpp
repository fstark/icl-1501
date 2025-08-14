#include "icl1501_disassembler.h"

SimpleICL1501Disassembler::SimpleICL1501Disassembler(bool enable_labels)
    : use_labels(enable_labels), start_address(0) {
}
    
    // Load from hex string (interpreting manual's octal format)
    bool SimpleICL1501Disassembler::loadFromHexString(const std::string& hex_str) {
        program_data.clear();
        std::string clean_hex = hex_str;
        
        // Remove spaces and make uppercase
        clean_hex.erase(std::remove_if(clean_hex.begin(), clean_hex.end(), ::isspace), clean_hex.end());
        std::transform(clean_hex.begin(), clean_hex.end(), clean_hex.begin(), ::toupper);
        
        if (clean_hex.length() % 2 != 0) {
            std::cerr << "Error: Hex string must have even number of characters" << std::endl;
            return false;
        }
        
        for (size_t i = 0; i < clean_hex.length(); i += 2) {
            try {
                uint8_t byte = static_cast<uint8_t>(std::stoi(clean_hex.substr(i, 2), nullptr, 16));
                program_data.push_back(byte);
            } catch (const std::exception& e) {
                std::cerr << "Error: Invalid hex string at position " << i << std::endl;
                return false;
            }
        }
        
        std::cout << "Loaded " << program_data.size() << " bytes from hex string" << std::endl;
        return true;
    }
    
    // Load octal pairs from manual format like "105-042"
    bool SimpleICL1501Disassembler::loadFromOctalPairs(const std::string& octal_pairs) {
        program_data.clear();
        start_address = 0;  // Default starting address
        
        std::string input = octal_pairs;
        
        // Check if input starts with Ppp-lll: format
        size_t colon_pos = input.find(':');
        if (colon_pos != std::string::npos && colon_pos > 0) {
            std::string addr_part = input.substr(0, colon_pos);
            if (addr_part.length() >= 7 && addr_part[0] == 'P' && addr_part[3] == '-') {
                try {
                    // Extract page and location
                    std::string page_str = addr_part.substr(1, 2);
                    std::string loc_str = addr_part.substr(4, 3);
                    
                    int page = std::stoi(page_str, nullptr, 8);
                    int location = std::stoi(loc_str, nullptr, 8);
                    
                    start_address = (page * 256) + location;
                    
                    // Remove the address prefix from input
                    input = input.substr(colon_pos + 1);
                    
                    // Trim leading whitespace
                    size_t start = input.find_first_not_of(" \t");
                    if (start != std::string::npos) {
                        input = input.substr(start);
                    }
                    
                    std::cout << "Starting address set to P" << std::setfill('0') << std::setw(2) << std::oct 
                              << page << "-" << std::setw(3) << location << std::dec 
                              << " (linear address " << start_address << ")" << std::endl;
                } catch (const std::exception& e) {
                    std::cerr << "Error: Invalid starting address format: " << addr_part << std::endl;
                    return false;
                }
            }
        }
        
        std::istringstream ss(input);
        std::string pair;
        
        while (ss >> pair) {
            size_t dash_pos = pair.find('-');
            if (dash_pos == std::string::npos) {
                std::cerr << "Error: Invalid octal pair format: " << pair << std::endl;
                return false;
            }
            
            try {
                std::string octal1_str = pair.substr(0, dash_pos);
                std::string octal2_str = pair.substr(dash_pos + 1);
                
                uint8_t byte1 = static_cast<uint8_t>(std::stoi(octal1_str, nullptr, 8));
                uint8_t byte2 = static_cast<uint8_t>(std::stoi(octal2_str, nullptr, 8));
                
                program_data.push_back(byte1);
                program_data.push_back(byte2);
                         
            } catch (const std::exception& e) {
                std::cerr << "Error: Invalid octal pair: " << pair << std::endl;
                return false;
            }
        }
        
        std::cout << "Loaded " << program_data.size() << " bytes from octal pairs" << std::endl;
        return true;
    }
    
    void SimpleICL1501Disassembler::disassemble() {
        if (program_data.empty()) {
            std::cerr << "No data loaded for disassembly" << std::endl;
            return;
        }
        
        if (use_labels) {
            // First pass: generate labels for branch targets
            symbol_table.clear();
            int address = start_address;
            int offset = 0;
            while (offset < static_cast<int>(program_data.size()) - 1) {
                if (isBRU(offset)) {
                    // Extract target address and create label
                    uint8_t byte1 = program_data[offset];
                    uint8_t byte2 = program_data[offset + 1];
                    uint8_t page = byte1 & 0x07;
                    uint8_t location = byte2;
                    int target_address = (page * 256) + location;  // Convert to linear address
                    
                    // Only create label if target is within our program range
                    int program_end = start_address + static_cast<int>(program_data.size());
                    if (target_address >= start_address && target_address < program_end) {
                        symbol_table.createLabelAtAddress(target_address);
                    }
                }
                offset += 2;
                address += 2;
            }
        }
        
        printHeader();
        
        // Second pass: actual disassembly with labels
        ICL1501Formatter formatter;
        int address = start_address;
        int offset = 0;
        while (offset < static_cast<int>(program_data.size()) - 1) {
            if (decodeBRU(address, offset, formatter)) {
                offset += 2; // BRU is 2 bytes
                address += 2;
            } else {
                // Not a BRU instruction, just show as data
                printUnknown(address, offset, formatter);
                offset += 2;
                address += 2;
            }
        }
    }
    
void SimpleICL1501Disassembler::printHeader() {
        ICL1501Formatter formatter;
        formatter.printHeader();
    }
    
    bool SimpleICL1501Disassembler::isBRU(int offset) {
        if (offset >= static_cast<int>(program_data.size()) - 1) {
            return false;
        }
        
        uint8_t byte1 = program_data[offset];
        uint8_t byte2 = program_data[offset + 1];
        
        // BRU binary format: 01000AAA AAAAAAA0
        return (byte1 & 0xF8) == 0x40 && (byte2 & 0x01) == 0;
    }
    
    bool SimpleICL1501Disassembler::decodeBRU(int addr, int offset, ICL1501Formatter& formatter) {
        if (!isBRU(offset)) {
            return false;
        }
        
        uint8_t byte1 = program_data[offset];
        uint8_t byte2 = program_data[offset + 1];
        
        // Extract address bits from both bytes
        uint8_t page = byte1 & 0x07;  // Last 3 bits of first byte
        uint8_t location = byte2;      // Second byte is the location
        
        printBRU(addr, byte1, byte2, page, location, formatter);
        return true;
    }
    
    void SimpleICL1501Disassembler::printBRU(int addr, uint8_t byte1, uint8_t byte2, uint8_t page, uint8_t location, ICL1501Formatter& formatter) {
        std::string label = "";
        std::string operands;
        
        // Get label for this address if labels are enabled
        if (use_labels) {
            label = symbol_table.getLabelAtAddress(addr);
        }
        
        // Determine operands - use label for target if available, otherwise use page/location
        if (use_labels) {
            int target_address = (page * 256) + location;
            std::string target_label = symbol_table.getLabelAtAddress(target_address);
            if (!target_label.empty()) {
                operands = target_label;
            } else {
                std::ostringstream oss;
                oss << "P" << std::setfill('0') << std::setw(2) << (int)page 
                    << "; " << std::setw(3) << std::setfill('0') << (int)location;
                operands = oss.str();
            }
        } else {
            std::ostringstream oss;
            oss << "P" << std::setfill('0') << std::setw(2) << (int)page 
                << "; " << std::setw(3) << std::setfill('0') << (int)location;
            operands = oss.str();
        }
        
        // Use formatter directly for BRU instruction
        int target_address = (page * 256) + location;
        
        formatter.startLine();
        formatter.writeAddress(addr);
        formatter.writeBytes(byte1, byte2);
        formatter.writeLabel(label);
        formatter.writeVerb("BRU,");
        formatter.writeOperands(operands + ".");
        
        // Create descriptive comment with target address
        std::ostringstream target_stream;
        int target_page = target_address / 256;
        int target_location = target_address % 256;
        target_stream << "P" << std::setfill('0') << std::setw(2) << target_page
                      << "; " << std::setw(3) << target_location;
        
        std::string comment = "Branch to " + target_stream.str();
        
        // If operands is a label (starts with 'L'), add it in parentheses
        if (!operands.empty() && operands[0] == 'L') {
            comment += " (" + operands + ")";
        }
        
        formatter.writeComment(comment);
        formatter.endLine();
    }
    
    void SimpleICL1501Disassembler::printUnknown(int addr, int offset, ICL1501Formatter& formatter) {
        if (offset < static_cast<int>(program_data.size()) - 1) {
            uint8_t byte1 = program_data[offset];
            uint8_t byte2 = program_data[offset + 1];
            
            std::string label = "";
            if (use_labels) {
                label = symbol_table.getLabelAtAddress(addr);
            }
            
            formatter.startLine();
            formatter.writeAddress(addr);
            formatter.writeBytes(byte1, byte2);
            formatter.writeLabel(label);
            formatter.writeVerb("???");
            formatter.writeOperands("");
            formatter.writeComment("UNKNOWN INSTRUCTION");
            formatter.endLine();
        }
    }

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Simple ICL-1501 Disassembler (BRU instruction only)" << std::endl;
        std::cout << "Usage:" << std::endl;
        std::cout << "  " << argv[0] << " -o <octal_pairs> [--labels]  - Disassemble from octal pairs (manual format)" << std::endl;
        std::cout << "  " << argv[0] << " -x <hex_string> [--labels]   - Disassemble from hex string" << std::endl;
        std::cout << "  " << argv[0] << " --test                       - Run test suite" << std::endl;
        std::cout << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << "  --labels    - Enable two-pass disassembly with label generation" << std::endl;
        std::cout << std::endl;
        std::cout << "Starting Address:" << std::endl;
        std::cout << "  Prefix octal pairs with Ppp-lll: to specify starting address" << std::endl;
        std::cout << "  where pp is page (octal) and lll is location (octal)" << std::endl;
        std::cout << "  Default is P00-000 if not specified" << std::endl;
        std::cout << std::endl;
        std::cout << "Examples:" << std::endl;
        std::cout << "  " << argv[0] << " -o \"105-042 106-144\"" << std::endl;
        std::cout << "  " << argv[0] << " -o \"P01-100: 102-000 104-006\" --labels" << std::endl;
        std::cout << "  " << argv[0] << " -o \"P02-000: 100-002 100-000\" --labels" << std::endl;
        std::cout << "  " << argv[0] << " -x \"8522 8664\"" << std::endl;
        std::cout << "  " << argv[0] << " --test" << std::endl;
        return 1;
    }
    
    // Check for --labels flag
    bool enable_labels = false;
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "--labels") {
            enable_labels = true;
            break;
        }
    }
    
    SimpleICL1501Disassembler disassembler(enable_labels);
    
    if (std::string(argv[1]) == "--test") {
        return disassembler.runTests() ? 0 : 1;
    } else if (argc >= 3) {
        if (std::string(argv[1]) == "-o") {
            // Load from octal pairs
            if (!disassembler.loadFromOctalPairs(argv[2])) {
                return 1;
            }
        } else if (std::string(argv[1]) == "-x") {
            // Load from hex string
            if (!disassembler.loadFromHexString(argv[2])) {
                return 1;
            }
        } else {
            std::cerr << "Unknown option: " << argv[1] << std::endl;
            return 1;
        }
        
        disassembler.disassemble();
    } else {
        std::cerr << "Unknown option: " << argv[1] << std::endl;
        return 1;
    }
}

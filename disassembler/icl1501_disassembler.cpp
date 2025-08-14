#include "icl1501_disassembler.h"

SimpleICL1501Disassembler::SimpleICL1501Disassembler(bool enable_labels)
    : use_labels(enable_labels), start_address(0)
{
}

// Load from hex string (interpreting manual's octal format)
bool SimpleICL1501Disassembler::loadFromHexString(std::string_view hex_str)
{
    program_data.clear();
    std::string clean_hex{hex_str};

    // Remove spaces and make uppercase using ranges (C++20 compatible)
    clean_hex.erase(std::remove_if(clean_hex.begin(), clean_hex.end(), ::isspace), clean_hex.end());
    std::transform(clean_hex.begin(), clean_hex.end(), clean_hex.begin(), ::toupper);

    if (clean_hex.length() % 2 != 0)
    {
        std::cerr << "Error: Hex string must have even number of characters" << std::endl;
        return false;
    }

    for (size_t i = 0; i < clean_hex.length(); i += 2)
    {
        try
        {
            uint8_t byte = static_cast<uint8_t>(std::stoi(clean_hex.substr(i, 2), nullptr, 16));
            program_data.push_back(byte);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error: Invalid hex string at position " << i << std::endl;
            return false;
        }
    }

    std::cout << "Loaded " << program_data.size() << " bytes from hex string" << std::endl;
    return true;
}

// Load octal pairs from manual format like "105-042"
bool SimpleICL1501Disassembler::loadFromOctalPairs(std::string_view octal_pairs)
{
    program_data.clear();
    start_address = 0; // Default starting address

    std::string input{octal_pairs};

    // Check if input starts with Ppp-lll: format
    size_t colon_pos = input.find(':');
    if (colon_pos != std::string::npos && colon_pos > 0)
    {
        std::string addr_part = input.substr(0, colon_pos);
        if (addr_part.length() >= 7 && addr_part[0] == 'P' && addr_part[3] == '-')
        {
            try
            {
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
                if (start != std::string::npos)
                {
                    input = input.substr(start);
                }

                std::cout << "Starting address set to P" << std::setfill('0') << std::setw(2) << std::oct
                          << page << "-" << std::setw(3) << location << std::dec
                          << " (linear address " << start_address << ")" << std::endl;
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error: Invalid starting address format: " << addr_part << std::endl;
                return false;
            }
        }
    }

    std::istringstream ss(input);
    std::string pair;

    while (ss >> pair)
    {
        size_t dash_pos = pair.find('-');
        if (dash_pos == std::string::npos)
        {
            std::cerr << "Error: Invalid octal pair format: " << pair << std::endl;
            return false;
        }

        try
        {
            std::string octal1_str = pair.substr(0, dash_pos);
            std::string octal2_str = pair.substr(dash_pos + 1);

            uint8_t byte1 = static_cast<uint8_t>(std::stoi(octal1_str, nullptr, 8));
            uint8_t byte2 = static_cast<uint8_t>(std::stoi(octal2_str, nullptr, 8));

            program_data.push_back(byte1);
            program_data.push_back(byte2);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error: Invalid octal pair: " << pair << std::endl;
            return false;
        }
    }

    std::cout << "Loaded " << program_data.size() << " bytes from octal pairs" << std::endl;
    return true;
}

void SimpleICL1501Disassembler::disassemble()
{
    if (program_data.empty())
    {
        std::cerr << "No data loaded for disassembly" << std::endl;
        return;
    }

    // Call the span version and print the results
    auto lines = disassemble(program_data, start_address, use_labels, false);
    for (const auto &line : lines)
    {
        std::cout << line << '\n';
    }
}

std::vector<std::string> SimpleICL1501Disassembler::disassemble(std::span<const uint8_t> data, int start_addr, bool enable_labels, bool terse)
{
    std::vector<std::string> result;

    if (data.empty())
    {
        return result;
    }

    // Save current state
    std::vector<uint8_t> original_data = program_data;
    ICL1501SymbolTable original_symbols = symbol_table;
    bool original_use_labels = use_labels;
    int original_start_address = start_address;

    // Set up for this disassembly - copy span to vector for compatibility with existing code
    program_data.assign(data.begin(), data.end());
    use_labels = enable_labels;
    start_address = start_addr;

    if (use_labels)
    {
        // First pass: generate labels for branch targets
        symbol_table.clear();
        int address = start_address;
        int offset = 0;
        while (offset < static_cast<int>(program_data.size()) - 1)
        {
            if (isBRU(offset) || isBRE(offset) || isBRH(offset) || isBRL(offset))
            {
                // Extract target address and create label
                uint8_t byte1 = program_data[offset];
                uint8_t byte2 = program_data[offset + 1];
                uint8_t page = byte1 & 0x07;
                uint8_t location = byte2 & 0xFE;              // Extract 7-bit address field (LSB is opcode)
                int target_address = (page * 256) + location; // Convert to linear address

                // Only create label if target is within our program range
                int program_end = start_address + static_cast<int>(program_data.size());
                if (target_address >= start_address && target_address < program_end)
                {
                    symbol_table.createLabelAtAddress(target_address);
                }
            }
            else if (isTLJ(offset))
            {
                // Create label for TLJ target
                createJumpLabel(address, offset);
            }
            else if (isTMJ(offset))
            {
                // Create label for TMJ target
                createJumpLabel(address, offset);
            }
            offset += 2;
            address += 2;
        }
    }

    // Capture output to string stream
    std::ostringstream output_stream;
    ICL1501Formatter formatter(output_stream);

    // Add header only if not in terse mode
    if (!terse)
    {
        formatter.printHeader();
    }

    // Second pass: actual disassembly with labels
    int address = start_address;
    int offset = 0;
    while (offset < static_cast<int>(program_data.size()) - 1)
    {
        if (decodeTLX(address, offset, formatter))
        {
            offset += 2; // TLX is 2 bytes
            address += 2;
        }
        else if (decodeTMX(address, offset, formatter))
        {
            offset += 2; // TMX is 2 bytes
            address += 2;
        }
        else if (decodeTLJ(address, offset, formatter))
        {
            offset += 2; // TLJ is 2 bytes
            address += 2;
        }
        else if (decodeTMJ(address, offset, formatter))
        {
            offset += 2; // TMJ is 2 bytes
            address += 2;
        }
        else if (decodeBRU(address, offset, formatter))
        {
            offset += 2; // BRU is 2 bytes
            address += 2;
        }
        else if (decodeBRE(address, offset, formatter))
        {
            offset += 2; // BRE is 2 bytes
            address += 2;
        }
        else if (decodeBRH(address, offset, formatter))
        {
            offset += 2; // BRH is 2 bytes
            address += 2;
        }
        else if (decodeBRL(address, offset, formatter))
        {
            offset += 2; // BRL is 2 bytes
            address += 2;
        }
        else
        {
            // Not a known branch instruction, just show as data
            printUnknown(address, offset, formatter);
            offset += 2;
            address += 2;
        }
    }

    // Split output into lines
    std::string output = output_stream.str();
    std::istringstream line_stream(output);
    std::string line;
    while (std::getline(line_stream, line))
    {
        result.push_back(line);
    }

    // Restore original state
    program_data = original_data;
    symbol_table = original_symbols;
    use_labels = original_use_labels;
    start_address = original_start_address;

    return result;
}

void SimpleICL1501Disassembler::printHeader()
{
    ICL1501Formatter formatter;
    formatter.printHeader();
}

std::string SimpleICL1501Disassembler::formatSectionRelativeAddress(int target_address, int context_address)
{
    // Calculate section from context address
    int context_section = context_address / 2048; // Each section is 2K = 2048 bytes

    // Calculate target page and location within section
    int target_page = (target_address % 2048) / 256; // Page within section (0-7)
    int target_location = target_address % 256;      // Location within page

    // Format as Psl where s=section, l=page
    std::ostringstream oss;
    oss << "P" << context_section << target_page
        << "; " << std::setfill('0') << std::setw(3) << target_location;
    return oss.str();
}

// Helper function to reduce duplication in branch instruction detection
bool SimpleICL1501Disassembler::isBranchInstruction(int offset, uint8_t byte1_pattern, uint8_t byte2_lsb)
{
    if (offset >= static_cast<int>(program_data.size()) - 1)
    {
        return false;
    }

    uint8_t byte1 = program_data[offset];
    uint8_t byte2 = program_data[offset + 1];

    return (byte1 & 0xF8) == byte1_pattern && (byte2 & 0x01) == byte2_lsb;
}

// Helper function to reduce duplication in branch instruction decoding
bool SimpleICL1501Disassembler::decodeBranchInstruction(int addr, int offset, ICL1501Formatter &formatter,
                                                        const std::string &mnemonic,
                                                        const std::string &description)
{
    uint8_t byte1 = program_data[offset];
    uint8_t byte2 = program_data[offset + 1];

    // Extract address bits from both bytes
    uint8_t page = byte1 & 0x07;     // Last 3 bits of first byte
    uint8_t location = byte2 & 0xFE; // 7-bit address field (LSB is opcode)

    printBranch(addr, byte1, byte2, page, location, mnemonic, description, formatter);
    return true;
}

bool SimpleICL1501Disassembler::isBRU(int offset)
{
    return isBranchInstruction(offset, 0x40, 0);
}

bool SimpleICL1501Disassembler::isBRE(int offset)
{
    return isBranchInstruction(offset, 0x40, 1);
}

bool SimpleICL1501Disassembler::isBRH(int offset)
{
    return isBranchInstruction(offset, 0x48, 0);
}

bool SimpleICL1501Disassembler::isBRL(int offset)
{
    return isBranchInstruction(offset, 0x48, 1);
}

bool SimpleICL1501Disassembler::isTLJ(int offset)
{
    if (offset >= static_cast<int>(program_data.size()) - 1)
    {
        return false;
    }

    uint8_t byte1 = program_data[offset];

    // TLJ binary format: 000JJJJ0 LLLLLLLL (where JJJJ = jump count, L = literal)
    // First byte pattern: 000xxxxx (top three bits must be 000)
    return (byte1 & 0xE0) == 0x00;
}

bool SimpleICL1501Disassembler::isTMJ(int offset)
{
    if (offset >= static_cast<int>(program_data.size()) - 1)
    {
        return false;
    }

    uint8_t byte1 = program_data[offset];

    // TMJ binary format: 001NNNN0 MMMMMMMM or 001NNNN1 MMMMMMMM (where NNNN = jump count, M = mask)
    // First byte pattern: 001xxxxx (top three bits must be 001)
    return (byte1 & 0xE0) == 0x20;
}

bool SimpleICL1501Disassembler::isTLX(int offset)
{
    if (offset >= static_cast<int>(program_data.size()) - 1)
    {
        return false;
    }

    uint8_t byte1 = program_data[offset];

    // TLX binary format: 00000000 LLLLLLLL (test literal and exit - jump count = 0)
    // First byte pattern: 00000000 (all bits must be 0)
    return byte1 == 0x00;
}

bool SimpleICL1501Disassembler::isTMX(int offset)
{
    if (offset >= static_cast<int>(program_data.size()) - 1)
    {
        return false;
    }

    uint8_t byte1 = program_data[offset];

    // TMX binary format: 00100000 MMMMMMMM (test mask and exit - jump count = 0)
    // First byte pattern: 00100000 (exactly 0x20)
    return byte1 == 0x20;
}

bool SimpleICL1501Disassembler::decodeBRU(int addr, int offset, ICL1501Formatter &formatter)
{
    return isBRU(offset) && decodeBranchInstruction(addr, offset, formatter, "BRU,", "Branch to");
}

bool SimpleICL1501Disassembler::decodeBRE(int addr, int offset, ICL1501Formatter &formatter)
{
    return isBRE(offset) && decodeBranchInstruction(addr, offset, formatter, "BRE,", "Branch on equal to");
}

bool SimpleICL1501Disassembler::decodeBRH(int addr, int offset, ICL1501Formatter &formatter)
{
    return isBRH(offset) && decodeBranchInstruction(addr, offset, formatter, "BRH,", "Branch on high to");
}

bool SimpleICL1501Disassembler::decodeBRL(int addr, int offset, ICL1501Formatter &formatter)
{
    return isBRL(offset) && decodeBranchInstruction(addr, offset, formatter, "BRL,", "Branch on low to");
}

bool SimpleICL1501Disassembler::decodeTLJ(int addr, int offset, ICL1501Formatter &formatter)
{
    if (!isTLJ(offset))
    {
        return false;
    }

    uint8_t byte1 = program_data[offset];
    uint8_t byte2 = program_data[offset + 1];

    printJumpInstruction(addr, byte1, byte2, "TLJ,", "If ", formatter);
    return true;
}

bool SimpleICL1501Disassembler::decodeTMJ(int addr, int offset, ICL1501Formatter &formatter)
{
    if (!isTMJ(offset))
    {
        return false;
    }

    uint8_t byte1 = program_data[offset];
    uint8_t byte2 = program_data[offset + 1];

    printJumpInstruction(addr, byte1, byte2, "TMJ,", "If mask ", formatter);
    return true;
}

bool SimpleICL1501Disassembler::decodeTLX(int addr, int offset, ICL1501Formatter &formatter)
{
    if (!isTLX(offset))
    {
        return false;
    }

    uint8_t byte1 = program_data[offset];
    uint8_t byte2 = program_data[offset + 1];

    // TLX is like TLJ but with jump count = 0 (exit/halt)
    printExitInstruction(addr, byte1, byte2, "TLX,", "Exit if ", formatter);
    return true;
}

bool SimpleICL1501Disassembler::decodeTMX(int addr, int offset, ICL1501Formatter &formatter)
{
    if (!isTMX(offset))
    {
        return false;
    }

    uint8_t byte1 = program_data[offset];
    uint8_t byte2 = program_data[offset + 1];

    // TMX is like TMJ but with jump count = 0 (exit/halt)
    printExitInstruction(addr, byte1, byte2, "TMX,", "Exit if mask ", formatter);
    return true;
}

void SimpleICL1501Disassembler::printBranch(int addr, uint8_t byte1, uint8_t byte2, uint8_t page, uint8_t location, const std::string &mnemonic, const std::string &description, ICL1501Formatter &formatter)
{
    std::string label = "";
    std::string operands;

    // Get label for this address if labels are enabled
    if (use_labels)
    {
        label = symbol_table.getLabelAtAddress(addr);
    }

    // Determine operands - use label for target if available, otherwise use page/location
    if (use_labels)
    {
        int target_address = (page * 256) + location;
        std::string target_label = symbol_table.getLabelAtAddress(target_address);
        if (!target_label.empty())
        {
            operands = target_label;
        }
        else
        {
            operands = formatSectionRelativeAddress(target_address, addr);
        }
    }
    else
    {
        int target_address = (page * 256) + location;
        operands = formatSectionRelativeAddress(target_address, addr);
    }

    // Use formatter directly for branch instruction
    int target_address = (page * 256) + location;

    formatter.startLine();
    formatter.writeAddress(addr);
    formatter.writeBytes(byte1, byte2);
    formatter.writeLabel(label);
    formatter.writeVerb(mnemonic);
    formatter.writeOperands(operands + ".");

    // Create descriptive comment with target address
    std::string target_addr_str = formatSectionRelativeAddress(target_address, addr);

    std::string comment = description + " " + target_addr_str;

    // If operands is a label (starts with 'L'), add it in parentheses
    if (!operands.empty() && operands[0] == 'L')
    {
        comment += " (" + operands + ")";
    }

    formatter.writeComment(comment);
    formatter.endLine();
}

void SimpleICL1501Disassembler::printJumpInstruction(int addr, uint8_t byte1, uint8_t byte2,
                                                     const std::string &mnemonic,
                                                     const std::string &comment_prefix,
                                                     ICL1501Formatter &formatter)
{
    // Extract jump count and direction from first byte
    uint8_t jump_count = (byte1 >> 1) & 0x0F; // Bits 4-1: jump count (0-15 instructions)
    bool is_backward = (byte1 & 0x01) != 0;   // Bit 0: direction (0=forward, 1=backward)
    uint8_t data_byte = byte2;                // Second byte: literal or mask

    std::string label = "";
    if (use_labels)
    {
        label = symbol_table.getLabelAtAddress(addr);
    }

    // Format the jump offset in bytes (instruction count * 2)
    int jump_bytes = jump_count * 2;
    std::string direction = is_backward ? "-" : "+";

    // Format operands - all data bytes use DEC format
    std::ostringstream operands_stream;
    operands_stream << direction << std::setfill('0') << std::setw(2) << std::dec << jump_bytes;
    operands_stream << "; DEC:" << std::setfill('0') << std::setw(3) << std::dec << static_cast<int>(data_byte) << ".";

    formatter.startLine();
    formatter.writeAddress(addr);
    formatter.writeBytes(byte1, byte2);
    formatter.writeLabel(label);
    formatter.writeVerb(mnemonic);
    formatter.writeOperands(operands_stream.str());

    // Create descriptive comment with target label if available
    std::string comment;
    if (mnemonic == "TMJ,")
    {
        // For TMJ, display mask in hex format
        std::ostringstream mask_stream;
        mask_stream << "0x" << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << static_cast<int>(data_byte);
        comment = comment_prefix + mask_stream.str() + ", jump " + direction + std::to_string(jump_bytes);
    }
    else
    {
        // For TLJ, display literal in decimal format
        comment = comment_prefix + std::to_string(static_cast<int>(data_byte)) + ", jump " + direction + std::to_string(jump_bytes);
    }

    // Add target label to comment if labels are enabled
    if (use_labels)
    {
        int target_address = is_backward ? (addr - jump_bytes) : (addr + jump_bytes);
        std::string target_label = symbol_table.getLabelAtAddress(target_address);
        if (!target_label.empty())
        {
            comment += " (" + target_label + ")";
        }
    }

    formatter.writeComment(comment);
    formatter.endLine();
}

void SimpleICL1501Disassembler::printExitInstruction(int addr, uint8_t byte1, uint8_t byte2,
                                                     const std::string &mnemonic,
                                                     const std::string &comment_prefix,
                                                     ICL1501Formatter &formatter)
{
    // For TLX and TMX, jump count is always 0 (exit/halt)
    uint8_t data_byte = byte2; // Second byte: literal or mask

    std::string label = "";
    if (use_labels)
    {
        label = symbol_table.getLabelAtAddress(addr);
    }

    // Format operands - all data bytes use DEC format, always show 000 for exit instructions
    std::ostringstream operands_stream;
    operands_stream << "000; DEC:" << std::setfill('0') << std::setw(3) << std::dec << static_cast<int>(data_byte) << ".";

    formatter.startLine();
    formatter.writeAddress(addr);
    formatter.writeBytes(byte1, byte2);
    formatter.writeLabel(label);
    formatter.writeVerb(mnemonic);
    formatter.writeOperands(operands_stream.str());

    // Create descriptive comment
    std::string comment;
    if (mnemonic == "TMX,")
    {
        // For TMX, display mask in hex format
        std::ostringstream mask_stream;
        mask_stream << "0x" << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << static_cast<int>(data_byte);
        comment = comment_prefix + mask_stream.str();
    }
    else
    {
        // For TLX, display literal in decimal format
        comment = comment_prefix + std::to_string(static_cast<int>(data_byte));
    }

    formatter.writeComment(comment);
    formatter.endLine();
}

void SimpleICL1501Disassembler::createJumpLabel(int address, int offset)
{
    uint8_t byte1 = program_data[offset];
    uint8_t jump_count = (byte1 >> 1) & 0x0F; // Bits 4-1: jump count (0-15 instructions)
    bool is_backward = (byte1 & 0x01) != 0;   // Bit 0: direction (0=forward, 1=backward)

    int jump_bytes = jump_count * 2;
    int target_address = is_backward ? (address - jump_bytes) : (address + jump_bytes);

    // Only create label if target is within our program range
    int program_end = start_address + static_cast<int>(program_data.size());
    if (target_address >= start_address && target_address < program_end)
    {
        symbol_table.createLabelAtAddress(target_address);
    }
}

void SimpleICL1501Disassembler::printUnknown(int addr, int offset, ICL1501Formatter &formatter)
{
    if (offset < static_cast<int>(program_data.size()) - 1)
    {
        uint8_t byte1 = program_data[offset];
        uint8_t byte2 = program_data[offset + 1];

        std::string label = "";
        if (use_labels)
        {
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

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cout << "Simple ICL-1501 Disassembler (BRU/BRE/BRH/BRL/TLJ/TMJ/TLX/TMX instructions)" << std::endl;
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
    for (int i = 1; i < argc; i++)
    {
        if (std::string(argv[i]) == "--labels")
        {
            enable_labels = true;
            break;
        }
    }

    SimpleICL1501Disassembler disassembler(enable_labels);

    if (std::string(argv[1]) == "--test")
    {
        return runICL1501DisassemblerTests() ? 0 : 1;
    }
    else if (argc >= 3)
    {
        if (std::string(argv[1]) == "-o")
        {
            // Load from octal pairs
            if (!disassembler.loadFromOctalPairs(argv[2]))
            {
                return 1;
            }
        }
        else if (std::string(argv[1]) == "-x")
        {
            // Load from hex string
            if (!disassembler.loadFromHexString(argv[2]))
            {
                return 1;
            }
        }
        else
        {
            std::cerr << "Unknown option: " << argv[1] << std::endl;
            return 1;
        }

        disassembler.disassemble();
    }
    else
    {
        std::cerr << "Unknown option: " << argv[1] << std::endl;
        return 1;
    }
}

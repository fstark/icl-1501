#include "icl1501_disassembler.h"

// Helper function to normalize whitespace for robust testing
static std::string normalizeWhitespace(const std::string &str)
{
    std::string result;
    bool in_space = false;

    for (char c : str)
    {
        if (std::isspace(c))
        {
            if (!in_space)
            {
                result += ' '; // Replace any whitespace with single space
                in_space = true;
            }
        }
        else
        {
            result += c;
            in_space = false;
        }
    }

    // Trim trailing space
    if (!result.empty() && result.back() == ' ')
    {
        result.pop_back();
    }

    return result;
}

// Helper function to parse octal pairs and starting address
static std::pair<std::vector<uint8_t>, int> parseOctalPairs(std::string_view octal_pairs)
{
    std::vector<uint8_t> data;
    int start_address = 0; // Default starting address

    std::string input{octal_pairs};

    // Check if input starts with Ppp-lll: format
    size_t colon_pos = input.find(':');
    if (colon_pos != std::string::npos && colon_pos > 0)
    {
        std::string addr_part = input.substr(0, colon_pos);
        if (addr_part.length() >= 7 && addr_part[0] == 'P' && addr_part[3] == '-')
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
        }
    }

    std::istringstream ss(input);
    std::string pair;

    while (ss >> pair)
    {
        size_t dash_pos = pair.find('-');
        if (dash_pos != std::string::npos)
        {
            std::string octal1_str = pair.substr(0, dash_pos);
            std::string octal2_str = pair.substr(dash_pos + 1);

            uint8_t byte1 = static_cast<uint8_t>(std::stoi(octal1_str, nullptr, 8));
            uint8_t byte2 = static_cast<uint8_t>(std::stoi(octal2_str, nullptr, 8));

            data.push_back(byte1);
            data.push_back(byte2);
        }
    }

    return std::make_pair(data, start_address);
}

// Standalone test function
bool runICL1501DisassemblerTests()
{
    std::cout << "Running ICL-1501 Disassembler Test Suite" << std::endl;
    std::cout << "=====================================" << std::endl;
    std::cout << std::endl;

    struct TestCase
    {
        std::string input;
        std::string expected_output;
        std::string description;
    };

    // Test cases from the manual
    std::vector<TestCase> tests = {
        {"102-000", "P00-000: 102-000.              BRU,  P02; 000.", "Basic BRU to page 2"},
        {"100-100", "P00-000: 100-100.              BRU,  P00; 064.", "BRU to page 0, location 64"},
        {"104-000", "P00-000: 104-000.              BRU,  P04; 000.", "BRU to page 4"},
        {"101-200", "P00-000: 101-200.              BRU,  P01; 128.", "BRU to page 1, location 200 octal"},
        {"107-374", "P00-000: 107-374.              BRU,  P07; 252.", "BRU to page 7, max location"},
        {"100-001", "P00-000: 100-001.              BRE,  P00; 000.", "Basic BRE to page 0"},
        {"102-101", "P00-000: 102-101.              BRE,  P02; 064.", "BRE to page 2, location 64"},
        {"110-000", "P00-000: 110-000.              BRH,  P00; 000.", "Basic BRH to page 0"},
        {"112-100", "P00-000: 112-100.              BRH,  P02; 064.", "BRH to page 2, location 64"},
        {"110-001", "P00-000: 110-001.              BRL,  P00; 000.", "Basic BRL to page 0"},
        {"112-101", "P00-000: 112-101.              BRL,  P02; 064.", "BRL to page 2, location 64"},

        // SBU/SBE/SBH/SBL (Stack and Branch) tests - Class 1 instructions
        {"122-000", "P00-000: 122-000.              SBU,  P02; 000.", "Basic SBU to page 2"},
        {"120-100", "P00-000: 120-100.              SBU,  P00; 064.", "SBU to page 0, location 64"},
        {"124-000", "P00-000: 124-000.              SBU,  P04; 000.", "SBU to page 4"},
        {"120-001", "P00-000: 120-001.              SBE,  P00; 000.", "Basic SBE to page 0"},
        {"122-101", "P00-000: 122-101.              SBE,  P02; 064.", "SBE to page 2, location 64"},
        {"130-000", "P00-000: 130-000.              SBH,  P00; 000.", "Basic SBH to page 0"},
        {"132-100", "P00-000: 132-100.              SBH,  P02; 064.", "SBH to page 2, location 64"},
        {"130-001", "P00-000: 130-001.              SBL,  P00; 000.", "Basic SBL to page 0"},
        {"132-101", "P00-000: 132-101.              SBL,  P02; 064.", "SBL to page 2, location 64"},

        // TLJ (Test Literal and Jump) tests - Class 0 instructions
        {"012-015", "P00-000: 012-015.              TLJ,  +10; DEC:013.", "TLJ forward jump with literal 013"},
        {"013-016", "P00-000: 013-016.              TLJ,  -10; DEC:014.", "TLJ backward jump with literal 014"},
        {"006-015", "P00-000: 006-015.              TLJ,  +06; DEC:013.", "TLJ forward jump with literal 013"},
        {"004-017", "P00-000: 004-017.              TLJ,  +04; DEC:015.", "TLJ forward jump with literal 015"},
        // TLJ edge cases - jump offsets
        {"036-025", "P00-000: 036-025.              TLJ,  +30; DEC:021.", "TLJ maximum forward jump (+30 bytes)"},
        {"037-030", "P00-000: 037-030.              TLJ,  -30; DEC:024.", "TLJ maximum backward jump (-30 bytes)"},
        // TMJ (Test Mask and Jump) tests - Class 0 instructions
        {"050-016", "P00-000: 050-016.              TMJ,  +08; DEC:014.", "TMJ forward jump with mask 014"},
        {"051-050", "P00-000: 051-050.              TMJ,  -08; DEC:040.", "TMJ backward jump with mask 040"},
        {"076-377", "P00-000: 076-377.              TMJ,  +30; DEC:255.", "TMJ maximum forward jump with mask 255"},
        {"044-004", "P00-000: 044-004.              TMJ,  +04; DEC:004.", "TMJ forward jump with mask 004"},
        // TMJ edge cases - jump offsets
        {"077-030", "P00-000: 077-030.              TMJ,  -30; DEC:024.", "TMJ maximum backward jump (-30 bytes)"},

        // TLX tests - Test Literal and Exit
        {"000-020", "P00-000: 000-020.              TLX,  000; DEC:016.", "TLX exit with literal 016 (was TLJ zero offset)"},
        {"000-017", "P00-000: 000-017.              TLX,  000; DEC:015.", "TLX exit with literal 015"},
        {"000-013", "P00-000: 000-013.              TLX,  000; DEC:011.", "TLX exit with literal 013"},
        {"000-377", "P00-000: 000-377.              TLX,  000; DEC:255.", "TLX exit with maximum literal"},
        {"000-000", "P00-000: 000-000.              TLX,  000; DEC:000.", "TLX exit with literal 0"},

        // TMX tests - Test Mask and Exit
        {"040-020", "P00-000: 040-020.              TMX,  000; DEC:016.", "TMX exit with mask 020 (was TMJ zero offset)"},
        {"040-010", "P00-000: 040-010.              TMX,  000; DEC:008.", "TMX exit with mask 010"},
        {"040-310", "P00-000: 040-310.              TMX,  000; DEC:200.", "TMX exit with mask 310"},
        {"040-240", "P00-000: 040-240.              TMX,  000; DEC:160.", "TMX exit with mask 240"},
        {"040-377", "P00-000: 040-377.              TMX,  000; DEC:255.", "TMX exit with maximum mask"},

        // Section-relative addressing tests
        {"P13-000: 115-062", "P13-000: 115-062.              BRH,  P15; 050.", "BRH from section 1 shows P15"},
        {"P27-000: 112-100", "P27-000: 112-100.              BRH,  P22; 064.", "BRH from section 2 shows P22"},
        {"P35-000: 110-001", "P35-000: 110-001.              BRL,  P30; 000.", "BRL from section 3 shows P30"},
    };

    int passed = 0;
    int failed = 0;

    for (const auto &test : tests)
    {
        std::cout << "Testing: " << test.input << " - " << test.description << std::endl;

        // Parse input to get data and start address
        auto [data, start_addr] = parseOctalPairs(test.input);

        if (data.empty())
        {
            std::cout << "   FAIL: Could not parse test data" << std::endl;
            failed++;
            continue;
        }

        // Create disassembler and get output in terse mode (no headers)
        SimpleICL1501Disassembler disassembler;
        std::vector<std::string> output_lines = disassembler.disassemble(data, start_addr, false, true);

        if (output_lines.empty())
        {
            std::cout << "   FAIL: No output produced" << std::endl;
            failed++;
            continue;
        }

        // In terse mode, first line should be the instruction
        std::string actual_output = output_lines[0];

        // Normalize whitespace for robust comparison
        std::string normalized_actual = normalizeWhitespace(actual_output);
        std::string normalized_expected = normalizeWhitespace(test.expected_output);

        // Check that expected output is a prefix of actual output (ignoring comments)
        if (normalized_actual.length() >= normalized_expected.length() &&
            normalized_actual.substr(0, normalized_expected.length()) == normalized_expected)
        {
            std::cout << "   PASS: " << actual_output << std::endl;
            passed++;
        }
        else
        {
            std::cout << "   FAIL: Expected: " << test.expected_output << std::endl;
            std::cout << "         Got:      " << actual_output << std::endl;
            std::cout << "         Normalized Expected: " << normalized_expected << std::endl;
            std::cout << "         Normalized Got:      " << normalized_actual << std::endl;
            failed++;
        }
    }

    std::cout << std::endl;
    std::cout << "Test Results: " << passed << " passed, " << failed << " failed" << std::endl;

    if (failed == 0)
    {
        std::cout << "All tests passed! ✅" << std::endl;
        return true;
    }
    else
    {
        std::cout << "Some tests failed! ❌" << std::endl;
        return false;
    }
}

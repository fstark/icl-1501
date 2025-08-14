#include "icl1501_disassembler.h"

// Helper function to normalize whitespace for robust testing
static std::string normalizeWhitespace(const std::string& str) {
    std::string result;
    bool in_space = false;
    
    for (char c : str) {
        if (std::isspace(c)) {
            if (!in_space) {
                result += ' ';  // Replace any whitespace with single space
                in_space = true;
            }
        } else {
            result += c;
            in_space = false;
        }
    }
    
    // Trim trailing space
    if (!result.empty() && result.back() == ' ') {
        result.pop_back();
    }
    
    return result;
}

// Test harness implementation
bool SimpleICL1501Disassembler::runTests() {
    std::cout << "Running ICL-1501 Disassembler Test Suite" << std::endl;
    std::cout << "=====================================" << std::endl;
    std::cout << std::endl;
    
    struct TestCase {
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
    };
    
    int passed = 0;
    int failed = 0;
    
    for (const auto& test : tests) {
        std::cout << "Testing: " << test.input << " - " << test.description << std::endl;
        
        // Save current data
        std::vector<uint8_t> original_data = program_data;
        
        // Load test data (suppress output for cleaner test display)
        std::cout.setstate(std::ios_base::failbit);
        bool loaded = loadFromOctalPairs(test.input);
        std::cout.clear();
        
        if (!loaded) {
            std::cout << "  FAIL: Could not load test data" << std::endl;
            failed++;
            program_data = original_data;
            continue;
        }
        
        // Capture output by redirecting to stringstream
        std::ostringstream captured_output;
        std::streambuf* old_cout = std::cout.rdbuf();
        std::cout.rdbuf(captured_output.rdbuf());
        
        // Generate the instruction output (without header)
        if (program_data.size() >= 2) {
            uint8_t byte1 = program_data[0];
            uint8_t byte2 = program_data[1];
            uint8_t page = byte1 & 0x07;
            uint8_t location = byte2;
            
            ICL1501Formatter temp_formatter(captured_output);
            printBRU(start_address, byte1, byte2, page, location, temp_formatter);
        }
        
        // Restore cout
        std::cout.rdbuf(old_cout);
        
        // Get the captured line and remove trailing newline
        std::string actual_output = captured_output.str();
        if (!actual_output.empty() && actual_output.back() == '\n') {
            actual_output.pop_back();
        }
        
        // Remove any extra leading characters that might be from stream formatting
        size_t start_pos = actual_output.find("P00-000:");
        if (start_pos != std::string::npos) {
            actual_output = actual_output.substr(start_pos);
        }
        
        // Extract just the instruction part (before the comment)
        size_t comment_pos = actual_output.find("Branch to");
        if (comment_pos != std::string::npos) {
            actual_output = actual_output.substr(0, comment_pos);
        }
        
        // Normalize whitespace for robust comparison
        std::string normalized_actual = normalizeWhitespace(actual_output);
        std::string normalized_expected = normalizeWhitespace(test.expected_output);
        
        // Compare with expected output
        if (normalized_actual == normalized_expected) {
            std::cout << "  PASS: " << actual_output << std::endl;
            passed++;
        } else {
            std::cout << "  FAIL: Expected: " << test.expected_output << std::endl;
            std::cout << "        Got:      " << actual_output << std::endl;
            std::cout << "        Normalized Expected: " << normalized_expected << std::endl;
            std::cout << "        Normalized Got:      " << normalized_actual << std::endl;
            failed++;
        }
        
        // Restore original data
        program_data = original_data;
    }
    
    std::cout << std::endl;
    std::cout << "Test Results: " << passed << " passed, " << failed << " failed" << std::endl;
    
    if (failed == 0) {
        std::cout << "All tests passed! ✅" << std::endl;
        return true;
    } else {
        std::cout << "Some tests failed! ❌" << std::endl;
        return false;
    }
}

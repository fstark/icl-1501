#pragma once
#include <vector>
#include <string_view>
#include <cstdint>

std::string to_octal(uint8_t value, int w = 3);
std::vector<uint8_t> vector_from_hex(std::string_view hex_str);
std::vector<uint8_t> vector_from_octal_pairs(std::string_view octal_pairs);
std::vector<uint8_t> vector_from_octal(std::string_view octal_numbers);
std::vector<uint8_t> vector_from_ascii(std::string_view ascii_str, bool underline = false);

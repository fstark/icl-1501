#pragma once
#include <vector>
#include <string_view>
#include <cstdint>

std::vector<uint8_t> vector_from_hex(std::string_view hex_str);
std::vector<uint8_t> vector_from_octal_pairs(std::string_view octal_pairs);

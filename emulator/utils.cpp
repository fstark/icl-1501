
#include "utils.hpp"
#include <cctype>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>
#include <stdint.h>

std::string to_octal(uint8_t value, int w )
{
	char buffer[4];
	char format[5];
	snprintf(format, sizeof(format), "%%0%do", w);
	snprintf(buffer, sizeof(buffer), format, value);
	return std::string(buffer);
}

// Load from hex string
std::vector<uint8_t> vector_from_hex(std::string_view hex_str)
{
	std::vector<uint8_t> data;
	std::string clean_hex{hex_str};

	// Remove spaces and make uppercase using ranges (C++20 compatible)
	clean_hex.erase(std::remove_if(clean_hex.begin(), clean_hex.end(), ::isspace), clean_hex.end());
	std::transform(clean_hex.begin(), clean_hex.end(), clean_hex.begin(), ::toupper);

	if (clean_hex.length() % 2 != 0)
	{
		std::cerr << "Error: Hex string must have even number of characters" << std::endl;
		throw std::invalid_argument("Invalid hex string length");
	}

	for (size_t i = 0; i < clean_hex.length(); i += 2)
	{
		try
		{
			int val = std::stoi(clean_hex.substr(i, 2), nullptr, 16);
			if (val < 0 || val > 255)
			{
				throw std::out_of_range("Byte value out of range");
			}
			data.push_back(static_cast<uint8_t>(val));
		}
		catch (const std::exception &e)
		{
			std::cerr << "Error: Invalid hex string at position " << i << std::endl;
			throw std::invalid_argument("Invalid hex string format");
		}
	}

	std::cout << "Loaded " << data.size() << " bytes from hex string" << std::endl;
	return data;
}

// Load octal pairs from manual format like "105-042"
std::vector<uint8_t> vector_from_octal_pairs(std::string_view octal_pairs)
{
	std::vector<uint8_t> data;
	std::string input{octal_pairs};

	std::istringstream ss(input);
	std::string pair;

	while (ss >> pair)
	{
		size_t dash_pos = pair.find('-');
		if (dash_pos == std::string::npos)
		{
			std::cerr << "Error: Invalid octal pair format: " << pair << std::endl;
			throw std::invalid_argument("Invalid octal pair format");
		}

		try
		{
			std::string octal1_str = pair.substr(0, dash_pos);
			std::string octal2_str = pair.substr(dash_pos + 1);

			// Check if both strings contain only valid octal digits (0-7)
			if (octal1_str.empty() || octal2_str.empty() ||
				octal1_str.find_first_not_of("01234567") != std::string::npos ||
				octal2_str.find_first_not_of("01234567") != std::string::npos)
			{
				std::cerr << "Error: Invalid octal digit in pair: " << pair << std::endl;
				throw std::invalid_argument("Invalid octal digit");
			}

			int val1 = std::stoi(octal1_str, nullptr, 8);
			int val2 = std::stoi(octal2_str, nullptr, 8);

			if (val1 < 0 || val1 > 255 || val2 < 0 || val2 > 255)
			{
				throw std::out_of_range("Octal value out of range");
			}

			data.push_back(static_cast<uint8_t>(val1));
			data.push_back(static_cast<uint8_t>(val2));
		}
		catch (const std::exception &e)
		{
			std::cerr << "Error: Invalid octal pair: " << pair << std::endl;
			throw std::invalid_argument("Invalid octal pair");
		}
	}

	std::cout << "Loaded " << data.size() << " bytes from octal pairs" << std::endl;
	return data;
}

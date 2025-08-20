#pragma once

#include <cstdint>
#include <algorithm>
#include <cassert>
#include <vector>

#include "addrs.hpp"
#include "iw.hpp"

class memory_t
{
	uint8_t data[16384];

public:
	memory_t() { std::fill(std::begin(data), std::end(data), 0); }

	uint8_t &operator[](size_t index)
	{
		assert(index < sizeof(data));
		return data[index];
	}

	const uint8_t &operator[](size_t index) const
	{
		assert(index < sizeof(data));
		return data[index];
	}

	uint8_t &operator[](addrs_t addr)
	{
		return (*this)[addr.linear()];
	}

	const uint8_t &operator[](addrs_t addr) const
	{
		return (*this)[addr.linear()];
	}

	void get(const addrs_t adrs, uint8_t &b0, uint8_t &b1) const
	{
		b0 = (*this)[adrs];
		b1 = (*this)[adrs + 1];
	}

	void set(const addrs_t adrs, uint8_t b0, uint8_t b1)
	{
		(*this)[adrs] = b0;
		(*this)[adrs + 1] = b1;
	}

	iw_t get_instruction(const addrs_t adrs) const
	{
		uint8_t b0, b1;
		get(adrs, b0, b1);
		return iw_t(b0, b1);
	}

	void set_instruction(const addrs_t adrs, const iw_t &iw)
	{
		set(adrs, iw.iwl(), iw.iwr());
	}

	addrs_t get_addrs(const addrs_t adrs) const
	{
		uint8_t b0, b1;
		get(adrs, b0, b1);
		return addrs_t(b0, b1);
	}

	void set_addrs(const addrs_t adrs, const addrs_t &new_adrs)
	{
		set(adrs, new_adrs.high(), new_adrs.low());
	}

	//  Fill memory from adrs with content of bytes
	void copy(addrs_t adrs, const std::vector<uint8_t> &bytes)
	{
		assert(adrs.linear() + bytes.size() <= sizeof(data));
		for (size_t i = 0; i < bytes.size(); ++i)
		{
			(*this)[adrs + i] = bytes[i];
		}
	}

	void dump_adrs( addrs_t from, size_t size = 16) const
	{
		assert(from.linear() + size <= sizeof(data));
		for (size_t i = 0; i < size; i+=2)
		{
			std::cout << (from+i).as_string() << ": ";
			addrs_t a = get_addrs(from+i);	
			std::cout << a.as_string() << " ";
			std::cout << std::endl;
		}
		std::cout << std::dec << std::endl;
	}

	void dump( addrs_t from, size_t size = 16) const
	{
		assert(from.linear() + size <= sizeof(data));
		std::cout << from.as_string() << ": ";
		for (size_t i = 0; i < size; i++)
		{
			std::cout << to_octal((*this)[from+i]) << " ";
		}
		std::cout << std::endl;
	}
};

void test_memory_t();

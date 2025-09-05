#pragma once

#include <cstdint>
#include <algorithm>
#include <cassert>
#include <vector>
#include <array>

#include "addrs.hpp"
#include "iw.hpp"

// Forward declaration
class binary_t;

//	A memory board
class memory_t
{
	uint8_t data[16384];
	std::array<bool,16384> monitored_;  //  If true, this address is monitored for changes (for CRT)
	bool changed_ = true;  //  If true, a monitored address has changed since last checked

public:
	memory_t() { std::fill(std::begin(data), std::end(data), 0); }

    void clear_crt_monitor()
	{
		std::fill(monitored_.begin(), monitored_.end(), false);
	}

	void monitor( const addrs_t &start, size_t length )
	{
		assert(start.linear()+length <= sizeof(data));
		for ( size_t i=0; i<length; i++ )
			monitored_[start.linear()+i] = true;
		std::cout << "Memory monitor from " << start.as_string() << " for " << length << " bytes" << std::endl;
	}

	bool changed()
	{
		bool was_changed = changed_;
		changed_ = false;
		return was_changed;
	}

	uint8_t get(const addrs_t adrs) const
	{
		return (*this)[adrs];
	}

	uint8_t operator[](size_t index) const
	{
		assert(index < sizeof(data));
		return data[index];
	}

	uint8_t operator[](addrs_t addr) const
	{
		return (*this)[addr.linear()];
	}

	void set(const addrs_t adrs, uint8_t b)
	{
		data[adrs.linear()] = b;
		if (monitored_[adrs.linear()])
		{
			changed_ = true;
			std::cout << "Memory changed at " << adrs.as_string() << " to " << to_octal(b) << std::endl;
		}
	}

	void get(const addrs_t adrs, uint8_t &b0, uint8_t &b1) const
	{
		b0 = (*this)[adrs];
		b1 = (*this)[adrs + 1];
	}

	void set(const addrs_t adrs, uint8_t b0, uint8_t b1)
	{
		set(adrs, b0);
		set(adrs + 1, b1);
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
			set(adrs + i, bytes[i]);
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

// Store all spans from binary_t into memory_t at their respective addresses
void store(memory_t& mem, const binary_t& binary);

void test_memory_t();

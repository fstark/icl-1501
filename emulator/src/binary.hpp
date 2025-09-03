#pragma once

#include "addrs.hpp"
#include "utils.hpp"
#include "iw.hpp"
#include <vector>
#include <string>
#include <sstream>
#include <stdexcept>
#include <cassert>

// A binary_span_t represents a contiguous block of data at a specific address
struct binary_span_t
{
    addrs_t start_address_;
    std::vector<uint8_t> data_;
    
    binary_span_t(const addrs_t& addr) : start_address_(addr) {}
    
    void add_byte(uint8_t byte)
    {
        data_.push_back(byte);
    }
    
    void add_word(uint8_t high_byte, uint8_t low_byte)
    {
        data_.push_back(high_byte);
        data_.push_back(low_byte);
    }
    
    size_t size() const
    {
        return data_.size();
    }
    
    addrs_t end_address() const;
};

// A binary_t, which is a set of binary_span_t, which is an address and associated data bytes
//	It can be created from a string of the form:
//	P00-000:000-001 002-055 P01-000:001-001 200-042 340-042 101-013 001-377 001-002 210-003 340-055 101-025 001-377 001-003 201-003 211-042 340-055 101-041 001-377 001-004 202-001 212-000 340-004 101-055 001-377 001-000
//	(ie whitespace separated. data can be two bytes (octal1-octal2) or 1 byte (octal1). There can be address specifiers to start new span. By default, starts at P00-000)
class binary_t
{
private:
    std::vector<binary_span_t> spans_;

public:
    // Default constructor creates empty binary
    binary_t() = default;
    
    // Constructor from string representation
    explicit binary_t(const std::string& binary_string)
    {
        parse_from_string(binary_string);
    }
    
    // Parse binary data from string representation
    void parse_from_string(const std::string& binary_string);
    
    // Get all spans
    const std::vector<binary_span_t>& spans() const
    {
        return spans_;
    }
    
    // Get all spans (non-const)
    std::vector<binary_span_t>& spans()
    {
        return spans_;
    }
    
    // Add a span
    void add_span(const binary_span_t& span)
    {
        spans_.push_back(span);
    }
    
    // Clear all spans
    void clear()
    {
        spans_.clear();
    }
    
    // Get total number of bytes across all spans
    size_t total_bytes() const;
};

// Free function to create binary from string representation
binary_t binary_from_string(const std::string& binary_string);

// Free function to convert a binary_span_t to a string representation
std::string to_string(const binary_span_t& span);

// Free function to convert a binary_t to a string representation
std::string to_string(const binary_t& binary);

// A binary_writer_t provides a convenient interface for building binary_t objects
class binary_writer_t
{
private:
    binary_t binary_;
    
    // Get the last span for writing (assumes we have at least one span)
    binary_span_t& get_current_span()
    {
        assert(!binary_.spans().empty());
        return binary_.spans().back();
    }
    
    // Get the last span for reading (assumes we have at least one span)
    const binary_span_t& get_current_span() const
    {
        assert(!binary_.spans().empty());
        return binary_.spans().back();
    }

public:
    // Default constructor starts at P00-000
    binary_writer_t()
    {
        // Create initial span at default address
        binary_.add_span(binary_span_t(addrs_t("P00-000")));
    }
    
    // Constructor with starting address
    explicit binary_writer_t(const addrs_t& start_address)
    {
        // Create initial span at specified address
        binary_.add_span(binary_span_t(start_address));
    }
    
    // Set the current address for writing - creates a new span
    void set_address(const addrs_t& addr)
    {
        if (get_current_span().size() == 0)
        {
            // If current span is empty, remove it
            binary_.spans().pop_back();
        }
        binary_.add_span(binary_span_t(addr));
    }
    
    // Get the current address (end of last span)
    addrs_t current_address() const
    {
        const auto& last_span = get_current_span();
        return last_span.end_address();
    }
    
    // Append a single byte
    void append_byte(uint8_t byte)
    {
        get_current_span().add_byte(byte);
    }
    
    // Append an iw_t (instruction word) - only if address is even
    void append_iw(const iw_t& instruction);
    
    // Get the built binary
    const binary_t& binary() const
    {
        return binary_;
    }
    
    // Move the built binary (for efficiency)
    binary_t take_binary()
    {
        return std::move(binary_);
    }
};

#include "binary.hpp"

// binary_span_t methods
addrs_t binary_span_t::end_address() const
{
    return start_address_ + static_cast<uint16_t>(data_.size());
}

// binary_t methods
void binary_t::parse_from_string(const std::string& binary_string)
{
    *this = binary_from_string(binary_string);
}

binary_t binary_from_string(const std::string& binary_string)
{
    std::istringstream iss(binary_string);
    std::string token;
    
    binary_writer_t writer;
    
    while (iss >> token)
    {
        if (token.back() == ':')
        {
            // Address specifier - remove trailing ':' and set address
            std::string addr_str = token.substr(0, token.length() - 1);
            writer.set_address(addrs_t(addr_str));
        }
        else
        {
            // Data token
            size_t dash_pos = token.find('-');
            
            if (dash_pos == std::string::npos)
            {
                // Single byte
                if (!is_octal(token))
                    throw std::invalid_argument("Invalid octal format: " + token);
                uint8_t byte = from_octal(token);
                writer.append_byte(byte);
            }
            else
            {
                // Two bytes - this is an instruction word
                iw_t instruction = iw_t::from_octal(token);
                writer.append_iw(instruction);
            }
        }
    }
    
    return writer.take_binary();
}

size_t binary_t::total_bytes() const
{
    size_t total = 0;
    for (const auto& span : spans_)
    {
        total += span.size();
    }
    return total;
}

// binary_writer_t methods
void binary_writer_t::append_iw(const iw_t& instruction)
{
    addrs_t current = current_address();
    if (current.linear() % 2 != 0)
    {
        throw std::invalid_argument("Cannot write iw_t at odd address: " + current.as_string());
    }
    
    append_byte(instruction.iwl());
    append_byte(instruction.iwr());
}

std::string to_string(const binary_span_t& span)
{
    std::ostringstream result;
    
    if (!span.data_.empty()) {
        result << span.start_address_.as_string() << ":";
        
        for (size_t i = 0; i < span.data_.size(); ++i) {
            result << " " << to_octal(span.data_[i]);
        }
    }
    
    return result.str();
}

std::string to_string(const binary_t& binary)
{
    std::ostringstream result;
    
    for (const auto& span : binary.spans()) {
        if (!span.data_.empty()) {
            result << to_string(span) << "\n";
        }
    }
    
    return result.str();
}

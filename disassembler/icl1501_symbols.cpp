#include "icl1501_symbols.h"
#include <sstream>
#include <iomanip>
#include <stdexcept>

ICL1501SymbolTable::ICL1501SymbolTable() : next_label_number(0) {}

std::string ICL1501SymbolTable::createLabelAtAddress(int address) {
    // Check if label already exists at this address
    auto it = label_table.find(address);
    if (it != label_table.end()) {
        return it->second;
    }
    
    // Create new label
    if (next_label_number >= 100) {
        throw std::runtime_error("Too many labels generated (limit: 100)");
    }
    
    std::ostringstream label_stream;
    label_stream << "L" << std::setfill('0') << std::setw(2) << next_label_number;
    std::string label = label_stream.str();
    
    label_table[address] = label;
    next_label_number++;
    
    return label;
}

std::string ICL1501SymbolTable::getLabelAtAddress(int address) const {
    auto it = label_table.find(address);
    if (it != label_table.end()) {
        return it->second;
    }
    return "";  // No label at this address
}

void ICL1501SymbolTable::clear() {
    label_table.clear();
    next_label_number = 0;
}

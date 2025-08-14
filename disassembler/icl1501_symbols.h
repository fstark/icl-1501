#ifndef ICL1501_SYMBOLS_H
#define ICL1501_SYMBOLS_H

#include <string>
#include <map>

class ICL1501SymbolTable {
private:
    std::map<int, std::string> label_table;  // address -> label mapping
    int next_label_number;
    
public:
    ICL1501SymbolTable();
    
    // Create a new label at the given address
    std::string createLabelAtAddress(int address);
    
    // Get the label at the given address (returns empty string if no label)
    std::string getLabelAtAddress(int address) const;
    
    // Clear all labels and reset counter
    void clear();
};

#endif // ICL1501_SYMBOLS_H

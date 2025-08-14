#ifndef ICL1501_FORMATTER_H
#define ICL1501_FORMATTER_H

#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>
#include <cstdint>

class ICL1501Formatter
{
private:
    // Field content widths (excluding trailing column separation space)
    static const int PPP_LLL_WIDTH = 8;   // "P00-000:" content width
    static const int MP_BYTES_WIDTH = 16; // "MP1-MP2-MP3-MP4." content width
    static const int LAB_WIDTH = 4;       // "LAB:" content width
    static const int VERB_WIDTH = 4;      // "VERB" content width
    static const int OPERANDS_WIDTH = 16; // "OPERANDS" content width
    static const int COMMENTS_WIDTH = 20; // "COMMENTS" content width

    std::ostream *output_stream;

public:
    ICL1501Formatter(std::ostream &os = std::cout);

    // Print the complete header
    void printHeader();

    // Start a new instruction line
    void startLine();

    // Field output methods
    void writeAddress(int address);
    void writeBytes(uint8_t byte1, uint8_t byte2);
    void writeLabel(const std::string &label);
    void writeVerb(const std::string &verb);
    void writeOperands(const std::string &operands);
    void writeComment(const std::string &comment);

    // Complete the line
    void endLine();

    // Get access to the output stream for special cases
    std::ostream &getStream() { return *output_stream; }
};

#endif // ICL1501_FORMATTER_H

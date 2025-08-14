#include "icl1501_formatter.h"

ICL1501Formatter::ICL1501Formatter(std::ostream &os) : output_stream(&os) {}

void ICL1501Formatter::printHeader()
{
	*output_stream << std::endl;
	*output_stream << "Simple ICL-1501 Disassembler (BRU/BRE/BRH/BRL/SBU/SBE/SBH/SBL/TLJ/TMJ/TLX/TMX/EXU/EXB instructions)" << std::endl;
	*output_stream << "=================================================" << std::endl;
	*output_stream << std::endl;

	// Build dynamic header using the same width constants
	*output_stream << std::setfill(' ') << std::left
				   << std::setw(PPP_LLL_WIDTH + 1) << "PPP-LLL:"
				   << std::setw(MP_BYTES_WIDTH + 1) << "MP1-MP2-MP3-MP4."
				   << std::setw(LAB_WIDTH + 1) << "LAB:"
				   << std::setw(VERB_WIDTH + 1) << "VERB"
				   << std::setw(OPERANDS_WIDTH + 1) << "OPERANDS"
				   << "COMMENTS" << std::endl;

	// Build dynamic separator line
	*output_stream << std::setfill('-') << std::left
				   << std::setw(PPP_LLL_WIDTH) << "" << " "
				   << std::setw(MP_BYTES_WIDTH) << "" << " "
				   << std::setw(LAB_WIDTH) << "" << " "
				   << std::setw(VERB_WIDTH) << "" << " "
				   << std::setw(OPERANDS_WIDTH) << "" << " "
				   << "--------" << std::endl;

	*output_stream << std::setfill(' ') << std::right; // Reset to normal formatting
}

void ICL1501Formatter::startLine()
{
	// Nothing special needed at start of line
}

void ICL1501Formatter::writeAddress(int address)
{
	std::ostringstream content_stream;
	int page = address / 256;
	int location = address % 256;

	content_stream << "P" << std::setfill('0') << std::setw(2) << std::oct
				   << page << "-" << std::setw(3) << location << std::dec << ":";

	*output_stream << std::setfill(' ') << std::setw(PPP_LLL_WIDTH + 1) << std::left << content_stream.str() << std::right;
}

void ICL1501Formatter::writeBytes(uint8_t byte1, uint8_t byte2)
{
	std::ostringstream content_stream;
	content_stream << std::setfill('0') << std::setw(3) << std::oct
				   << (int)byte1 << "-" << std::setw(3) << (int)byte2 << std::dec << ".";

	*output_stream << std::setfill(' ') << std::setw(MP_BYTES_WIDTH + 1) << std::left << content_stream.str() << std::right;
}

void ICL1501Formatter::writeLabel(const std::string &label)
{
	std::ostringstream content_stream;
	if (!label.empty())
	{
		content_stream << label << ": ";
	}

	*output_stream << std::setfill(' ') << std::setw(LAB_WIDTH + 1) << std::left << content_stream.str() << std::right;
}

void ICL1501Formatter::writeVerb(const std::string &verb)
{
	std::ostringstream content_stream;
	content_stream << verb;

	*output_stream << std::setfill(' ') << std::setw(VERB_WIDTH + 1) << std::left << content_stream.str() << std::right;
}

void ICL1501Formatter::writeOperands(const std::string &operands)
{
	std::ostringstream content_stream;
	content_stream << operands;

	*output_stream << std::setfill(' ') << std::setw(OPERANDS_WIDTH + 1) << std::left << content_stream.str() << std::right;
}

void ICL1501Formatter::writeComment(const std::string &comment)
{
	std::ostringstream content_stream;
	content_stream << comment; // Let column alignment handle spacing

	*output_stream << content_stream.str(); // Comments don't need width padding
}

void ICL1501Formatter::endLine()
{
	*output_stream << std::endl;
}

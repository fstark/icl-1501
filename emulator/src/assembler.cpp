#include "assembler.hpp"
#include "binary.hpp"
#include <sstream>
#include <stdexcept>
#include <algorithm>

// Member function implementations for assembler_t

void assembler_t::remove_symbol( const std::string & label )
{
	for ( auto it = symbols_.begin(); it!=symbols_.end(); it++ )
	{
		if (it->label==label)
		{
			symbols_.erase(it);
			return;
		}
	}
}

void assembler_t::set_symbol( const std::string & label, const addrs_t & addr )
{
	remove_symbol(label);
	symbols_.push_back({label,addr});
}

bool assembler_t::get_symbol( const std::string & label, addrs_t & addr )
{
	for ( auto &s : symbols_ )
	{
		if (s.label==label)
		{
			addr = s.addr;
			return true;
		}
	}

	// Symbol not found, fatal in pass1
	return pass_!=1;
}

void assembler_t::dump_symbols( std::ostream & out )
{
	out << "\n\nSymbol table:" << std::endl;
	for ( auto &s : symbols_ )
	{
		out << "  " << s.label << " = " << s.addr.as_string() << std::endl;
	}
}

uint8_t assembler_t::parse_op1( const std::string & op, char c )
{
	// Expect C#n where n is 0-7
	if (op.size()!=3 || op[0]!=c || op[1]!='#' || !isdigit(op[2]) || op[2]<'0' || op[2]>'7')
		throw std::runtime_error("Invalid IOC channel or register: " + op);
	return op[2]-'0';
}

bool assembler_t::try_parse_op1( const std::string & op, char c, uint8_t &reg)
{
	if (op.size()!=3 || op[0]!=c || op[1]!='#' || !isdigit(op[2]) || op[2]<'0' || op[2]>'7')
		return false;
	reg = op[2]-'0';
	return true;
}

uint8_t assembler_t::parse_indexing_register( const std::string & op, iw_t::eIndexingMode &mode )
{
	uint8_t reg;
	mode = iw_t::kUnchanged;
	if (try_parse_op1(op,iw_t::register_char(mode),reg))
		return reg;
	mode = iw_t::kIncrement;
	if (try_parse_op1(op,iw_t::register_char(mode),reg))
		return reg;
	mode = iw_t::kDecrement;
	if (try_parse_op1(op,iw_t::register_char(mode),reg))
		return reg;

	throw std::runtime_error("Invalid indexing register: " + op);
}

bool assembler_t::parse_page( const std::string & op, uint8_t &page )
{
	//	format is Pnn where nn is octal 00-77
	if (op.size()!=3 || op[0]!='P' || !::is_octal(op.substr(1,2)))
		return false;
	page = ::from_octal(op.substr(1,2)) & 0x3F;
	return true;
}

// prob to be moved to addrs.hpp
bool assembler_t::is_addrs( const std::string & op )
{
	// format is Pnn-nnn where nn is octal 00-77 and nnn is octal 000-377
	if (op.size()==7 && op[0]=='P' && op[3]=='-' && ::is_octal(op.substr(1,2)) && ::is_octal(op.substr(4,3)))
		return true;
	// format is Pn-nnn where n is octal 0-7 and nnn is octal 000-377
	if (op.size()==6 && op[0]=='P' && op[2]=='-' && ::is_octal(op.substr(1,1)) && ::is_octal(op.substr(3,3)))
		return true;
	return false;
}

bool assembler_t::get_addrs( const std::string & op, addrs_t &addr )
{
	if (!is_addrs(op))
		return false;
	addr = addrs_t(op);
	return true;
}

bool assembler_t::parse_binary8( const std::string & op, uint8_t &value )
{
	// format is 8 bits binary, e.g. 10101010
	if (op.size()!=8)
		return false;
	value = 0;
	for (char c: op)
	{
		if (c!='0' && c!='1')
			return false;
		value = (value<<1) | (c=='1' ? 1 : 0);
	}
	return true;
}

bool assembler_t::parse_shift( const std::string & op, uint8_t &shift )
{
	// format is n where n is 0-7
	if (op.size()!=1 || !isdigit(op[0]) || op[0]<'0' || op[0]>'7')
		return false;
	shift = op[0]-'0';
	return true;
}

// parse +U -U +V -V
void assembler_t::parse_uv( const std::string & op, bool &u, bool &v )
{
	u = false;
	v = false;
	// check size = 4
	if (op.size()!=4)
		throw std::runtime_error("Invalid U/V specifier: " + op);
	if (op[0]!='+' && op[0]!='-')
		throw std::runtime_error("Invalid U/V specifier: " + op);
	if (op[2]!='+' && op[2]!='-')
		throw std::runtime_error("Invalid U/V specifier: " + op);
	if (op[1]!='U' && op[1]!='V')
		throw std::runtime_error("Invalid U/V specifier: " + op);
	if (op[3]!='U' && op[3]!='V')
		throw std::runtime_error("Invalid U/V specifier: " + op);
	if (op[1]==op[3])
		throw std::runtime_error("Invalid U/V specifier: " + op);
	u = (op[0]=='+');
	v = (op[2]=='+');
}

bool assembler_t::assemble( const iw_t::instruction_def &id, const std::string & op1, const std::string & op2, iw_t &iw )
{

	//	base encoding for instrution
	iw = {(uint8_t)((id.value&0xff00)>>8), (uint8_t)(id.value&0x00ff)};

	//	We now parse what we need

	//	We look for all possible masks, and if the instruction uses them, we parse the corresponding operand

	// For now, just print it
	// std::cout << addr_.as_string() <<  ": parsing as [" << id.mnemonic << "] [" << op1 << "] [" << op2 << "]" << std::endl;

	for (int mask_bit=0;mask_bit!=32;mask_bit++)
	{
		int mask = 1<<mask_bit;
		if (id.decode&mask)
		{
			if (mask==iw_t::kDECODE_DBO)
			{
				iw = iw_t::from_octal( op1 );
			}
			if (mask==iw_t::kDECODE_MASK)
			{
				uint8_t m;
				if (op2.empty())
				{
					if (!parse_binary8(op1, m))
						return false;
				}
				else
				{
					if (!parse_binary8(op2, m))
						return false;
				}
				iw.set_literal(m);
			}
			if (mask==iw_t::kDECODE_JUMP)
			{
				int jump_count = std::stoi(op1);
				// std::cout << "JC=" << jump_count << std::endl;
				iw.set_signed_jump_count( jump_count );
			}
			if (mask==iw_t::kDECODE_INDEX_REGISTER)
			{
				int reg = parse_op1(op1,'R');
				iw.set_indexing_register(reg);
			}
			if (mask==iw_t::kDECODE_IOC_CHANNEL)
			{
				int channel = parse_op1(op1,'C');
				iw.set_ioc_channel(channel);
			}
			if (mask==iw_t::kDECODE_SECTION)
			{
				uint8_t section = parse_op1(op1,'S');
				iw.set_section1(section);
			}
		    if (mask==iw_t::kDECODE_UV)
		    {
				bool u,v;
				std::string uv = op2;
				if (uv.empty())
					uv = op1;
				parse_uv( uv, u, v );
				iw.set_u(u);
				iw.set_v(v);
		    }
			if (mask==iw_t::kDECODE_SHIFT)
			{
				uint8_t shift;
				if (parse_shift(op1,shift))
				{
					iw.set_shift_count( shift );
				}
				else
					return false;
			}
			if (mask==iw_t::kDECODE_OLITERAL)	
			{
					// TLX 000 => op1
					//	LDX R#1 000 => op2
				if (op2.empty())
				{
					if (!::is_octal(op1))
						return false;
					iw.set_iwr( ::from_octal( op1 ) );
				}
				else
				{
					if (!::is_octal(op2))
						return false;
					iw.set_iwr( ::from_octal( op2 ) );
				}
			}
			if (mask==iw_t::kDECODE_BLITERAL)
			{
				uint8_t b;
				if (!op2.empty())
				{
					if (!parse_binary8(op2,b))
						return false;
				}
				else if (!parse_binary8(op1,b))
					return false;
				iw.set_iwr(b);
			}
			if (mask==iw_t::kDECODE_INDEX_REGISTER_OP)
			{
				iw_t::eIndexingMode mode;
				int reg = parse_indexing_register(op1,mode);
				iw.set_indexing_register(reg);
				iw.set_indexing_mode(mode);
			}
			if (mask==iw_t::kDECODE_ZP_ADRS)
			{
				addrs_t a{0,0};
				if (!get_addrs(op1,a))
					return false;
				if (a.page()!=0)
					return false;		//	Should be page 0
				iw.set_iwr( a.location() );
			}
			if (mask==iw_t::kDECODE_PAGE_NUMBER)
			{
				uint8_t page;
				if (parse_page(op2,page))
					iw.set_page_number(page);
			}
			if (mask==iw_t::kDECODE_ADRS_LEVEL_BYTE)
			{
				addrs_t a{0,0};
				if (get_addrs(op1,a) || get_symbol(op1,a))
				{
					iw.set_address(a);
					// std::cout << iw.as_octal() << std::endl;
				}
				else
					return false;
			}
			if (mask==iw_t::kDECODE_SECTION_LEVEL)
			{
				// expect Psl where s is section 0-7 and l is level 0-7
				if (op2.size()!=3 || op2[0]!='P' || !isdigit(op2[1]) || !isdigit(op2[2]) || op2[1]<'0' || op2[1]>'7' || op2[2]<'0' || op2[2]>'7')
					return false;
				iw.set_section2( op2[1]-'0' );
				iw.set_level( op2[2]-'0' );
			}
		}
	}

	return true;
}

bool assembler_t::assemble( const std::string & mnemonic, const std::string & op1, const std::string & op2, iw_t &iw )
{
	// For now, just print it
	// std::cout << addr_.as_string() <<  ": [" << mnemonic << "] [" << op1 << "] [" << op2 << "]" << std::endl;

	for (auto &it: iw_t::types())
	{
		if (it.mnemonic == mnemonic)
		{
			if (assemble(it, op1, op2, iw))
			{
				// Increment address by 2 for next instruction
				addr_ = addr_.next_instruction();
				return true;
			}
		}
	}

	throw std::runtime_error("Can't assemble: " + mnemonic + " " + op1 + " " + op2	);
}

bool assembler_t::assemble( const std::string & line, iw_t &iw )
{
	//	Remove everything after a semicolon
	auto comment_pos = line.find(';');
	std::string code = line.substr(0, comment_pos);

	//	If only whitespace, ignore
	if (code.find_first_not_of(" \t\r\n") == std::string::npos)
	{
		return false;
	}

	//	If does not start with whitespace, consume a label
	//	and store it in label.
	//	Label ends with '=' or ':'
	//	any other is and error
	size_t first_non_ws = code.find_first_not_of(" \t\r\n");
	if (first_non_ws == 0)
	{
		std::string label;
		size_t label_end = code.find_first_of("=:", first_non_ws);
		if (label_end == std::string::npos)
			throw std::runtime_error("Label must end with '=' or ':'");
		label = code.substr(first_non_ws, label_end - first_non_ws);
		//	labels are [A-Z][A-Z0-9]*
		if (label.empty() || !isalpha(label[0]) || !std::all_of(label.begin()+1, label.end(), [](char c){ return c=='_' || (c>='A' && c<='Z') || (c>='0' && c<='9'); }))
			throw std::runtime_error("Invalid label: " + label);

// std::cout << "[" << label << "]" << std::endl;

		if (code[label_end] == '=')
		{
			// parse rest as an address
			std::string addr_str = code.substr(label_end + 1);
			addr_str.erase(0, addr_str.find_first_not_of(" \t\r\n"));
			addr_str.erase(addr_str.find_last_not_of(" \t\r\n") + 1);
			if (addr_str.empty())
				throw std::runtime_error("Address expected after '='");
			addrs_t addr(addr_str);
			set_symbol( label, addr );
			return false;
		}
		else
		{
			//	Label definition for current instruction
			set_symbol( label, addr_ );
			//	and continue parsing the rest of the line
			first_non_ws = label_end + 1;
		}
	}

	std::string rest_of_line = code.substr(first_non_ws);

	//	If empty after label, ignore
	if (rest_of_line.find_first_not_of(" \t\r\n") == std::string::npos)
	{
		return false;
	}

	//	Parse rest of line as 3 space-separated fields:
	//	Mnemonic op1 op2
	//	op1 and op2 are optional, depending on mnemonic
	//	For now, just print them
	std::string mnemonic;
	std::string op1;
	std::string op2;
	size_t pos = rest_of_line.find_first_not_of(" \t\r\n");
	if (pos == std::string::npos)
		throw std::runtime_error("Mnemonic expected");
	size_t end = rest_of_line.find_first_of(" \t\r\n", pos);
	mnemonic = rest_of_line.substr(pos, end - pos);
	pos = rest_of_line.find_first_not_of(" \t\r\n", end);
	if (pos != std::string::npos)
	{
		end = rest_of_line.find_first_of(" \t\r\n", pos);
		op1 = rest_of_line.substr(pos, end - pos);
		pos = rest_of_line.find_first_not_of(" \t\r\n", end);
		if (pos != std::string::npos)
		{
			end = rest_of_line.find_first_of(" \t\r\n", pos);
			op2 = rest_of_line.substr(pos, end - pos);
			pos = rest_of_line.find_first_not_of(" \t\r\n", end);
			if (pos != std::string::npos)
				throw std::runtime_error("Too many operands");
		}
	}
	
	if  (mnemonic=="ORG")
	{
		if (op1.empty())
			throw std::runtime_error("ORG requires an address");
		addr_ = op1;
		return false;
	}

	return assemble( mnemonic, op1, op2, iw );
}

// Static helper function for a single assembly pass
static void assemble_pass(const std::string &source, int pass, assembler_t &assembler, binary_writer_t &writer, std::ostringstream &error_stream)
{
    std::istringstream text_stream(source);
    std::string line;
    int line_number = 0;
    
    assembler.set_pass(pass);
    assembler.set_addr(addrs_t("P00-000"));
    
    while (std::getline(text_stream, line)) {
        line_number++;
        
        // Skip empty lines and comments
        if (line.empty() || line[0] == '/' || line[0] == ';') {
            continue;
        }
        
        try {
            iw_t iw{0, 0};
            auto prev_addr = assembler.addr();
            if (assembler.assemble(line, iw)) {
                // Only write output on pass 1
                if (pass == 1) {
                    writer.append_iw(iw);
                }
            }
            else if (prev_addr != assembler.addr() && pass == 1) {
                // Address changed, update writer
                writer.set_address(assembler.addr());
            }
        }
        catch (const std::exception &e) {
            error_stream << "Pass " << pass << " Error on line " << line_number << ": " << e.what() << "\n";
            error_stream << "  " << line << "\n";
            throw; // Re-throw to stop assembly
        }
    }
}

// Free function to assemble source code into a binary_t
binary_t assemble(const std::string &source)
{
    assembler_t assembler;
    binary_writer_t writer;
    std::ostringstream error_stream;
    
    try {
        // Pass 0: Symbol collection and validation
        assemble_pass(source, 0, assembler, writer, error_stream);
        
        // Pass 1: Code generation
        assemble_pass(source, 1, assembler, writer, error_stream);
        
        return writer.take_binary();
    }
    catch (const std::exception &e) {
        // Include any collected errors in the exception message
        std::string full_error = error_stream.str();
        if (!full_error.empty()) {
            throw std::runtime_error(full_error);
        } else {
            throw; // Re-throw original exception
        }
    }
}

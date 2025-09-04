#pragma once

#include "iw.hpp"

class assembler_t
{
	typedef struct
	{
		std::string label;
		addrs_t addr;
	} symbol_t;

	addrs_t addr_;
	int pass_ = 0;
	std::vector<symbol_t> symbols_;

	public:
	assembler_t() : addr_{0,0} {}

	void set_pass( int p ) { pass_ = p; }
	void set_addr( addrs_t a ) { addr_ = a; }
	addrs_t addr() const { return addr_; }

	void remove_symbol( const std::string & label );
	void set_symbol( const std::string & label, const addrs_t & addr );
	bool get_symbol( const std::string & label, addrs_t & addr );
	void dump_symbols( std::ostream & out );
	uint8_t parse_op1( const std::string & op, char c );
	bool try_parse_op1( const std::string & op, char c, uint8_t &reg);
	uint8_t parse_indexing_register( const std::string & op, iw_t::eIndexingMode &mode );
	bool parse_page( const std::string & op, uint8_t &page );
	bool is_addrs( const std::string & op );
	bool get_addrs( const std::string & op, addrs_t &addr );
	bool parse_binary8( const std::string & op, uint8_t &value );
	bool parse_shift( const std::string & op, uint8_t &shift );
	void parse_uv( const std::string & op, bool &u, bool &v );
	bool assemble( const iw_t::instruction_def &id, const std::string & op1, const std::string & op2, iw_t &iw );
	bool assemble( const std::string & mnemonic, const std::string & op1, const std::string & op2, iw_t &iw );
	bool assemble( const std::string & line, iw_t &iw );
};

// Forward declaration for binary_t
class binary_t;

// Free function to assemble source code into a binary_t
binary_t assemble(const std::string &source);

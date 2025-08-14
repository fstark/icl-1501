# ICL-1501 Disassembler

This directory contains the ICL-1501 disassembler implementation.

## Supported Instructions

- **BRU** (Branch Unconditionally) - Format: `01000AAA AAAAAAA0`
- **BRE** (Branch on Equal) - Format: `01001AAA AAAAAAA0`
- **BRH** (Branch on High) - Format: `01010AAA AAAAAAA0`
- **BRL** (Branch on Low) - Format: `01011AAA AAAAAAA0`

## Files

- `icl1501_disassembler.h` - Main disassembler class header
- `icl1501_disassembler.cpp` - Main disassembler implementation
- `icl1501_disassembler_tests.cpp` - Test suite implementation
- `icl1501_formatter.h` - Output formatting class header
- `icl1501_formatter.cpp` - Output formatting implementation
- `icl1501_symbols.h` - Symbol table management header
- `icl1501_symbols.cpp` - Symbol table management implementation
- `Makefile` - Build configuration

## Building

```bash
cd disassembler
make
```

## Running

```bash
# Run test suite
./icl1501_disassembler --test

# Disassemble all branch instruction types from octal pairs  
./icl1501_disassembler -o "102-000 110-004 120-002 130-006"

# Disassemble with labels for complex branch analysis
./icl1501_disassembler -o "P01-100: 102-006 110-104 120-102 130-100" --labels

# Show help
./icl1501_disassembler
```

## Architecture

- **SimpleICL1501Disassembler**: Main disassembler class with two-pass label generation
- **ICL1501Formatter**: Handles output formatting with dynamic column alignment
- **ICL1501SymbolTable**: Manages symbol/label generation and lookup

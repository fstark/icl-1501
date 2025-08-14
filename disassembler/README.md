# ICL-1501 Disassembler

This directory contains the ICL-1501 disassembler implementation.

## Supported Instructions

The disassembler supports all 8 ICL-1501 instruction types:

### Branch Instructions
- **BRU** (Branch Unconditionally) - Format: `01000AAA AAAAAAA0`
- **BRE** (Branch on Equal) - Format: `01000AAA AAAAAAA1` 
- **BRH** (Branch on High) - Format: `01001AAA AAAAAAA0`
- **BRL** (Branch on Low) - Format: `01001AAA AAAAAAA1`

### Jump Instructions  
- **TLJ** (Test Literal and Jump) - Format: `000JJJJ0 LLLLLLLL`
- **TMJ** (Test Mask and Jump) - Format: `001NNNN0/001NNNN1 MMMMMMMM`

### Exit Instructions
- **TLX** (Test Literal and Exit) - Format: `00000000 LLLLLLLL` 
- **TMX** (Test Mask and Exit) - Format: `00100000 MMMMMMMM`

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

# Disassemble from octal pairs  
./icl1501_disassembler -o "102-000 110-004 120-002 130-006"

# Disassemble with labels for complex branch analysis
./icl1501_disassembler -o "P01-100: 102-006 110-104 120-102 130-100" --labels

# Show help
./icl1501_disassembler
```

## Complete Instruction Showcase

Here's a comprehensive example demonstrating all 8 supported instruction types:

```bash
./icl1501_disassembler -o "P13-044: 102-000 050-020 112-100 036-025 013-016 051-050 000-017 040-310 100-001 104-046" --labels
```

Output:
```
Simple ICL-1501 Disassembler (BRU/BRE/BRH/BRL/TLJ/TMJ/TLX/TMX instructions)
=================================================

PPP-LLL: MP1-MP2-MP3-MP4. LAB: VERB OPERANDS         COMMENTS
-------- ---------------- ---- ---- ---------------- --------
P13-044: 102-000.              BRU, P12; 000.        Branch to P12; 000
P13-046: 050-020.         L01: TMJ, +08; DEC:016.    If mask 0x10, jump +8 (L00)
P13-050: 112-100.              BRH, P12; 064.        Branch on high to P12; 064
P13-052: 036-025.              TLJ, +30; DEC:021.    If 21, jump +30
P13-054: 013-016.              TLJ, -10; DEC:014.    If 14, jump -10
P13-056: 051-050.         L00: TMJ, -08; DEC:040.    If mask 0x28, jump -8 (L01)
P13-060: 000-017.         L02: TLX, 000; DEC:015.    Exit if 15
P13-062: 040-310.         L03: TMX, 000; DEC:200.    Exit if mask 0xC8
P13-064: 100-001.              BRE, P10; 000.        Branch on equal to P10; 000
P13-066: 104-046.              BRU, P14; 038.        Branch to P14; 038
```

This showcase demonstrates:
- **BRU/BRE/BRH/BRL**: All branch instruction variants
- **TLJ**: Test literal and conditional jump (forward/backward)  
- **TMJ**: Test mask and conditional jump (forward/backward)
- **TLX**: Test literal and exit/halt
- **TMX**: Test mask and exit/halt
- **Label generation**: Automatic L00-L03 labels for jump targets
- **Section addressing**: P13 section with proper address calculation

## Architecture

- **SimpleICL1501Disassembler**: Main disassembler class with two-pass label generation
- **ICL1501Formatter**: Handles output formatting with dynamic column alignment
- **ICL1501SymbolTable**: Manages symbol/label generation and lookup

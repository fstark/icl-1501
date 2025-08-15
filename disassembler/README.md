# ICL-1501 Disassembler

This directory contains the ICL-1501 disassembler implementation.

## Supported Instructions

The disassembler supports 23 ICL-1501 instruction types:

### Branch Instructions
- **BRU** (Branch Unconditionally) - Format: `01000AAA AAAAAAA0`
- **BRE** (Branch on Equal) - Format: `01000AAA AAAAAAA1` 
- **BRH** (Branch on High) - Format: `01001AAA AAAAAAA0`
- **BRL** (Branch on Low) - Format: `01001AAA AAAAAAA1`

### Stack-and-Branch Instructions
- **SBU** (Stack and Branch Unconditionally) - Format: `01010AAA AAAAAAA0`
- **SBE** (Stack and Branch on Equal) - Format: `01010AAA AAAAAAA1`
- **SBH** (Stack and Branch on High) - Format: `01011AAA AAAAAAA0`
- **SBL** (Stack and Branch on Low) - Format: `01011AAA AAAAAAA1`

### Jump Instructions  
- **TLJ** (Test Literal and Jump) - Format: `000JJJJ0 LLLLLLLL`
- **TMJ** (Test Mask and Jump) - Format: `001NNNN0/001NNNN1 MMMMMMMM`

### Exit Instructions
- **TLX** (Test Literal and Exit) - Format: `00000000 LLLLLLLL` 
- **TMX** (Test Mask and Exit) - Format: `00100000 MMMMMMMM`
- **EXU** (Exit Unconditional) - Format: `01100000 00000000`
- **EXB** (Exit and Branch) - Format: `01110AAA AAAAAAA0`

### Memory Management Instructions
- **SMS** (Set Memory Section) - Format: `01101000 000SSS00`
- **SMC** (Set Memory Control) - Format: `01101001 UV000000`
- **SSC** (Set Memory Section & Control) - Format: `01101010 UVSSS000`

### Control Instructions
- **SAC** (Set Arithmetic Condition) - Format: `01101011 00000000`
- **LSW** (Load Sense Switches) - Format: `01101100 00000000`
- **LPS** (Load Processor Status) - Format: `01101101 00000000`

### Interrupt Control Instructions
- **DPI** (Disable Processor Interrupt) - Format: `01101110 00000000`
- **EPI** (Enable Processor Interrupt) - Format: `01101110 00000001`
- **CPI** (Clear Processor Interrupt) - Format: `01101110 00000010`

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

Here's a comprehensive example demonstrating all 23 supported instruction types:

```bash
./icl1501_disassembler -o "P13-044: 102-000 100-001 110-000 110-001 122-100 120-001 130-200 130-001 052-100 010-020 000-017 040-310 140-000 160-020 150-004 151-100 152-110 153-000 154-000 155-000 156-000 156-001 156-002" --labels
```

Output:
```
Simple ICL-1501 Disassembler (BRU/BRE/BRH/BRL/SBU/SBE/SBH/SBL/TLJ/TMJ/TLX/TMX/EXU/EXB/SMS/SMC/SSC/SAC/LSW/LPS/DPI/EPI/CPI instructions)
=================================================

PPP-LLL: MP1-MP2-MP3-MP4. LAB: VERB OPERANDS         COMMENTS
-------- ---------------- ---- ---- ---------------- --------
P13-044: 102-000.              BRU, P12; 000.        Branch to P12; 000
P13-046: 100-001.              BRE, P10; 000.        Branch on equal to P10; 000
P13-050: 110-000.              BRH, P10; 000.        Branch on high to P10; 000
P13-052: 110-001.              BRL, P10; 000.        Branch on low to P10; 000
P13-054: 122-100.              SBU, P12; 064.        Stack and branch to P12; 064
P13-056: 120-001.              SBE, P10; 000.        Stack and branch on equal to P10; 000
P13-060: 130-200.              SBH, P10; 128.        Stack and branch on high to P10; 128
P13-062: 130-001.              SBL, P10; 000.        Stack and branch on low to P10; 000
P13-064: 052-100.              TMJ, +10; DEC:064.    If mask 0x40, jump +10 (L00)
P13-066: 010-020.              TLJ, +08; DEC:016.    If 16, jump +8 (L00)
P13-070: 000-017.         L01: TLX, 000; DEC:015.    Exit if 15
P13-072: 040-310.         L02: TMX, 000; DEC:200.    Exit if mask 0xC8
P13-074: 140-000.              EXU, 000.             Exit unconditional (return to stack)
P13-076: 160-020.         L00: EXB, P10; 016.        Exit and branch to P10; 016
P13-100: 150-004.              SMS, S#1.             Set memory section to S#1
P13-102: 151-100.              SMC, C#1.             Set memory control to C#1 (set V bit, reset U bit)
P13-104: 152-110.              SSC, S#1; C#1.        Set memory section S#1 and control C#1
P13-106: 153-000.              SAC, 000.             Set arithmetic condition from ACC bits 4-5
P13-110: 154-000.              LSW, 000.             Load sense switches to accumulator
P13-112: 155-000.              LPS, 000.             Load processor status word to accumulator
P13-114: 156-000.              DPI, 000.             Disable processor interrupt
P13-116: 156-001.              EPI, 000.             Enable processor interrupt
P13-120: 156-002.              CPI, 000.             Clear processor interrupt overflow
```

This showcase demonstrates:
- **BRU/BRE/BRH/BRL**: All branch instruction variants
- **SBU/SBE/SBH/SBL**: All stack-and-branch instruction variants
- **TLJ**: Test literal and conditional jump (forward/backward)  
- **TMJ**: Test mask and conditional jump (forward/backward)
- **TLX**: Test literal and exit/halt
- **TMX**: Test mask and exit/halt
- **EXU**: Exit unconditional (return to stack)
- **EXB**: Exit and branch to address
- **SMS**: Set memory section for branch addressing
- **SMC**: Set memory control bits (U & V flags)
- **SSC**: Set memory section and control bits combined
- **SAC**: Set arithmetic condition from accumulator bits
- **LSW**: Load sense switches to accumulator
- **LPS**: Load processor status word to accumulator
- **DPI**: Disable processor interrupt handling
- **EPI**: Enable processor interrupt handling  
- **CPI**: Clear processor interrupt overflow flag
- **Label generation**: Automatic L00-L02 labels for jump targets
- **Section addressing**: P13 section with proper address calculation

## Architecture

- **SimpleICL1501Disassembler**: Main disassembler class with two-pass label generation
- **ICL1501Formatter**: Handles output formatting with dynamic column alignment
- **ICL1501SymbolTable**: Manages symbol/label generation and lookup

## Files

- `icl1501_disassembler.h` - Main disassembler class header
- `icl1501_disassembler.cpp` - Main disassembler implementation
- `icl1501_disassembler_tests.cpp` - Test suite implementation
- `icl1501_formatter.h` - Output formatting class header
- `icl1501_formatter.cpp` - Output formatting implementation
- `icl1501_symbols.h` - Symbol table management header
- `icl1501_symbols.cpp` - Symbol table management implementation
- `Makefile` - Build configuration

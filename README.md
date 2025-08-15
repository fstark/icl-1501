# ICL-1501 / Cogar C4 Disassembler

A C++ disassembler for the ICL-1501/Cogar C4 minicomputer that converts binary machine code to the original DPL-1 assembly language format.

## Project Structure

- `/disassembler/` - ICL-1501 disassembler implementation (see `disassembler/README.md`)
- `/System_Programmers_Manual.md` - ICL-1501 programming reference

## Features

- **Complete DPL-1 Instruction Set Support**
  - Class 0: Jump and Conditional Exit Instructions
  - Class 1: Branch, Linkage-Control, and I/O Instructions  
  - Class 2: Data-Transfer and Arithmetic Instructions
  - Class 3: Boolean and Compare Instructions

- **Multiple Input Formats**
  - Binary files (.bin, etc.)
  - Hex string input via command line

- **Authentic Output Format**
  - Matches original ICL-1501 assembly syntax
  - Proper DPL-1 mnemonics and operand formatting
  - Octal notation as used in the original system

## Building

```bash
make
```

## Usage

### Decompile from binary file:
```bash
./icl1501_decompiler program.bin
```

### Decompile from hex string:
```bash
./icl1501_decompiler -x "200024 123174 237054"
```

## Examples

The decompiler converts machine code to authentic DPL-1 assembly:

**Input (hex):** `200024 123174 237054`

**Output:**
```
PPP-LLL:   MP1-MP2-MP3-MP4.   E SEQ. NO.   LAB:   VERB   OPERANDS     COMMENTS
--------   ----------------   ----------   ----   ----   --------     --------
000-000:   200-024.            XX-XXX.      LDA ; R#0; OCT:024        LOAD ACCUMULATOR
000-002:   123-174.            XX-XXX.      SBU ; P01; 123           STACK & BRANCH UNCOND
000-004:   237-054.            XX-XXX.      STA ; R#7; P00           STORE ACCUMULATOR
```

## Instruction Classes Supported

### Class 0: Jump Instructions
- `TLJ` - Test Literal and Jump
- `TMJ` - Test Mask and Jump  
- `TLX` - Test Literal and Exit
- `TMX` - Test Mask and Exit

### Class 1: Branch and I/O Instructions
- `BRU`, `BRE`, `BRH`, `BRL` - Branch instructions
- `SBU`, `SBE`, `SBH`, `SBL` - Stack and Branch instructions
- `EXU`, `EXB` - Exit instructions
- `SMS`, `SMC`, `SSC` - Memory control instructions
- `SAC` - Set Arithmetic Condition
- `LSW` - Load Sense Switches
- `LPS` - Load Processor Status
- `DPI` - Disable Processor Interrupt
- `EPI` - Enable Processor Interrupt
- `CPI` - Clear Processor Interrupt

### Class 2: Data Transfer and Arithmetic
- `LDA` - Load Accumulator
- `LDX` - Load Index Register  
- `LIA` - Load Instruction Address
- `STA` - Store Accumulator
- `ADX` - Add to Index Register
- `SUX` - Subtract from Index Register
- `ADA`, `SBA` - Arithmetic operations

### Class 3: Boolean and Compare
- `ANA` - AND to Accumulator
- `IRA` - Inclusive OR to Accumulator
- `ERA` - Exclusive OR to Accumulator  
- `CPA` - Compare Accumulator
- `CPX` - Compare Index Register

## Addressing Modes

The decompiler correctly handles all DPL-1 addressing modes:

- **Immediate**: `LDA, R#0; OCT:024`
- **Direct**: `LDA, R#0; 128`  
- **Indexed**: `LDA, R#3; P00` / `LDA, I#3; P00` / `LDA, D#3; P00`

## Technical Details

- Based on the 1972 ICL-1501 System Programmer's Manual
- Supports all documented instruction formats and opcodes
- Handles 2-byte instruction format (even byte boundaries)
- Proper octal formatting for addresses and literals
- Maintains original assembly syntax conventions

## Testing

Run the built-in tests:
```bash
make test
```

## References

- ICL-1501/Cogar C4/Singer 1501 System Programmer's Manual (1972)
- DPL-1 (Data Processing Language Level 1) specification
- Original Cogar batch assembler documentation

## License

Open source - see LICENSE file for details.

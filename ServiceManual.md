# SECTION 1
GENERAL DESCRIPTION  

## 1-1.  PURPOSE AND USE OF THE INTELLIGENT TERMINAL.

The Singer Model 1501 Intelligent Terminal is a desk-top unit that has a
wide range of data processing applications, such as inventory control,
payroll processing, sales analysis, and purchasing.  These various func-
tions can be accomplished by the use of different pre-recorded program
tapes.

The Model 1501, shown in figure 1-1, consists of a mini-computer, keyboard,
tape drives, and a display screen.  Within the unit are the processor,
memory, and input-output controller which comprise the computer.  These
components are printed circuit boards.  Two cartridge tape drives are
mounted on the upper right section of the unit.  Magnetic tape cartridges
loaded on these drives provide programs and store data for the computer.

A CRT and keyboard are housed in the terminal.  These components allow the
operator to perform the data processing for which the terminal is intended.
The keyboard allows the operator to enter data and control operation of the
computer, while the CRT displays data, questions, and instructions to
assist the operator.

The terminal can also communicate with other terminals and peripheral
equipment, such as tape units and printers, over a serial input-output
channel operated by the I/O controller.  With the addition of a communi-
cations adapter board to the main chassis and an external MODEM (modulator-
demodulator), the terminal also has the capability to transmit and receive
over telephone lines.

A well beneath the display screen houses the program load and system reset
switches.  After power is applied to the terminal, the first program is
loaded into the processor's memory from a tape drive cartridge.  This is
accomplished when the operator presses forward the program load switch.
This loads the program stored on tape into the processor memory.  From this
point on, the operation of the processor is under control of that program.

## 1-2.  MODELS AND OPTIONS AVAILABLE.

There are three basic models of the 1501.  They are: (1) the standard
1501; (2) the 1501-FF, which includes a transaction counter; and (3) the
1501-CL, which does not include tape drives and is intended to be operated
in the clustered mode with other terminals.  The suffix "CL" means
clustered.

There are several optional features that can be added to the terminal to
increase its capabilities.  These options and the basic Intelligent Terminal
part numbers are listed in table 1-1.  Where the suffix "XXX" appears in the
table, it indicates that the item is available in several different varia-
tions in which such things as power requirements, keyboard keytops, and
memory capacity differ.  Of course, other options may have been added or
part numbers changed since this manual was published so only the Illustrated
Parts Manual and the sales representative's catalog should be used for
selecting and ordering options.

Table 1-1.  Models and Options Available

---------------------------------------------------------------
Model                                           Part Number
---------------------------------------------------------------
1501 Intelligent Terminal . . . . . . . . . . . 001-003200-XXX
1501-FF Intelligent Terminal . . . . . . . . .  001-002994-XXX
1501-CL Intelligent Terminal . . . . . . . . .  001-001000-XXX
1534A Asynchronous Communications Adapter . . . 003-001907-XXX
1535A Binary Synchronous Communications Adapter  003-001907-039
1530-1, 13-Key Numeric Pad . . . . . . . . . .  003-003233-XXX
Booster Transformer Option for 100 VAC Input .  003-003350-006
*Dual SIO Option . . . . . . . . . . . . . . .  003-002830-008
1533 Dual Drive . . . . . . . . . . . . . . .   003-001574-XXX
---------------------------------------------------------------

*Must be included with 1501-CL

## 1-3. LOCATION OF MAJOR COMPONENTS.

The location of the tape drives, CRT, and keyboard assembly are shown in
figure 1-1.  However, most of the electronic components are mounted inside
the terminal, and the location of these components is shown in figure 1-2.

## 1-4. CONTROLS AND INDICATORS.

The Model 1501 Intelligent Terminal has relatively few operator controls
aside from the keyboard and pad.  All of these controls are described in
the Operator Instructions Manual; however, those of interest to the field
service engineer are summarized in the following paragraphs.



               CRT ASSEMBLY
                   |
            +---------------+
            |               |
  SWITCH    |               |   POWER SUPPLY
 ASSEMBLY   |               |        |
    |       +---------------+        |
    |                               TAPE DRIVE 1
    |                               TAPE DRIVE 2
    |
KEYBOARD ASSEMBLY
|
MOTION CONTROL
BOARDS
|
COMMUNICATIONS BOARD
|
+------------------+ ON/OFF SWITCH
| INPUT/OUTPUT | |
| CONTROLLER BOARD | TAPE AMPLIFIER
+------------------+ BOARD
|
PROCESSOR/MEMORY
BOARD

Figure 1-2. Location of Major Components


Following are the three controls used to apply power and start the terminal:

a. The POWER ON/OFF switch, located on the right rear bottom of the side
panel.

b. The PROGRAM LOAD switch, located in the well beneath the display
screen.  This switch, when pushed to the PROGRAM LOAD position, causes
the program to be loaded into the processor memory.  Except for the
1501-CL, the source of the program is the tape cartridge on tape drive
2.  In the 1501-CL, the program is available from another terminal
over the serial input-output channel.  This switch has three positions;
PROGRAM LOAD, off, and program interrupt.  The interrupt position,
which is not labeled, causes the processor program to jump to a
specific memory location (page 03, location 000) to perform an inter-
rupt servicing routine.

c. The SYSTEM RESET switch is located in the well beneath the display
screen.  This switch causes the processor program to jump to a
specific memory location (page 02, location 000) and start the program
located there.  The action the processor takes when the SYSTEM RESET
switch is used depends upon the program currently stored in the memory.

Both the normal operating programs and the diagnostic programs are stored
on tape cartridges.  Located on the outer case of the cartridge is a two-
position hole for the write pin.  If the write pin is inserted into the
position on top of the cartridge, the tape cannot be written on.  If the
write pin is inserted into the position on the bottom of the cartridge,
the tape may be written on; this is called the "write-enable" position.

Loading the cartridge is a simple procedure that is fully explained in the
Operator Instructions Manual.  Once the cartridge is placed in the loader,
the loader must be pushed down and the locking lever moved to the lock posi-
tion.  Of course, this procedure is reversed in order to unload a tape.

Normally, the processor rewinds the tape before instructions are given to
remove the cartridge.  If, however, the tape is to be removed before it
has been rewound by the processor, it is necessary to depress the red re-
wind pushbutton on the tape drive.  Once this button has been depressed,
the tape will be rewound and can be unloaded.

## 1-5. CAPABILITIES.

The capability of each terminal depends upon the options selected.  In the
following paragraphs, the capabilities of the standard components are de-
scribed first.  Following this description, the options available are
discussed.

### Display and Keyboard.

The terminal houses a 5-inch CRT on which a visual display is created.  Up
to 8 rows of 32 characters each can be shown.  Each character is formed on
a 5 x 7 dot matrix and is selected by the processor.  In the standard
character set, 64 different characters are available, including the alpha-
bet, numerals, and punctuation marks.  However, the keytops can be replaced
with any character set, and the processor program can create any character
set on the display.  The keyboard consists of a 52-key alphanumeric key-
board and an optional 13-key keypad.  A speaker is housed in the terminal
to provide audible cues to the operator.

### Processor.

Operation of the terminal is controlled by a computer housed within the
terminal.  This computer consists of a general-purpose processor, a memory,
and I/O controller.  The processor is a stored program, general-purpose
machine, with repertoire of 39 instructions.  The format of all instruc-
tions and data in the processor is organized into bytes (8 bits).  Instruc-
tion execution time ranges from three to six microseconds.

### Memory.

Memory capacity is a maximum of 16,384 bytes in 4096-byte increments.  The
memory is random access with read/write capability.

### I/O Controller.

An I/O controller is provided to handle data transfer between most I/O
devices and the processor.  This unit operates the cartridge tape drives,
the CRT and keyboard assembly, and a serial input-output channel.  This
channel can be cabled to as many as 64 terminals or peripheral equipment
units.  Cable length is a maximum of 1500 feet and cable impedance is
93 ohms.

### Cartridge Tape Units.

Except for the 1501-CL version, two cartridge tape units are standard in
the 1501.  Each holds one cartridge like that shown in figure 1-3.  The
data is recorded serially on a single channel tape at a density of 1600
bits per inch.  Capacity of each cartridge is 900 records of 143 bytes
each.  Reading and writing speed is 10 inches per second, and the tape is
driven at 40 inches per second during search and rewind operations.

```
A picture of the cartridge, with "write pin" and "leader eyelet" indicated.
```

(Figure 1-3. Tape Crtridge)

### Dual Serial Input-Output (SIO).

The standard terminal can communicate with other Series 1500 equipment
over one SIO channel.  The addition of a dual SIO board provides two SIO
channels.

### Communications Adapters.

The communications adapters are single boards that are added to the 1501
to allow the terminal to communicate with a MODEM and subsequently with
telephone lines.  Either the Model 1534 Asynchronous Communications Adapter
or the Model 1535 Binary Synchronous Communications Adapter can be used;
however, the terminal can accommodate only one such board.  This option is
either factory or field installable.

## 1-6. WEIGHT AND DIMENSIONS.

The Model 1501 Intelligent Terminal weighs approximately 64 pounds (29 kg).
Its dimensions are:

- Height - 10" (25 cm)  
- Width  - 18.5" (47 cm)  
- Depth  - 24" (60 cm)  

## 1-7. POWER REQUIREMENTS.

The Model 1501 Intelligent Terminal is available in versions that operate
on 100, 115, or 230-volt AC supplies.  However, the older models with the
Cogar power supply can operate only on 115 or 230-volt inputs.  These
models require the Booster Transformer Option if only a 100-volt input is
available.

- Voltage . . . . . . . . . . 100V RMS nominal, 50-60 Hz, Single Phase  
  Range . . . . . . . . . . .  90V - 110V RMS  

- Voltage . . . . . . . . . . 115V RMS nominal, 50-60 Hz, Single Phase  
  Range . . . . . . . . . . . 104V - 129V RMS  

- Voltage . . . . . . . . . . 230V RMS nominal, 50-60 Hz, Single Phase  
  Range . . . . . . . . . . . 208V - 258V RMS  

## 1-8. ENVIRONMENTAL REQUIREMENTS.

The Model 1501 Intelligent Terminal is designed to operate in a normal
office environment.  Listed below are the temperature and humidity limita-
tions for both normal operation and transportation and storage.

### Ambient Temperature

Operating:  +60°F to 90°F (15.6°C to 32°C)  
Storage:    -25°F to +135°F (-32°C to 57°C)  

### Relative Humidity

Operating:  20 to 80 percent  
Storage:    5 to 95 percent  


# SECTION 2  
FUNCTIONAL DESCRIPTION  

## 2-1. GENERAL.

The purpose and organization of the Model 1501 Intelligent Terminal were
briefly discussed in Section 1 of this manual.  This section begins with
a review of that material in order to introduce the detailed descriptions
that follow.

The Model 1501 is a small computer that includes a processor, a memory,
and an input-output controller.  These components are printed circuit
boards housed within the terminal.  Two tape drives, which are operated
by the computer, are mounted on the top right of the terminal.  Magnetic
tape cartridges loaded on these drives provide programs for the computer
and store data produced by the computer.

A CRT and keyboard, housed in the terminal, allow the operator to perform
the data processing for which the terminal is intended.  The keyboard
allows the operator to enter data and control operation of the computer,
while the CRT displays data, questions, and instructions to assist the
operator.

The terminal can also communicate with other terminals and peripheral
equipment, such as tape units and printers, over a serial input-output
channel operated by the I/O controller.  With the addition of a communi-
cations adapter board and an external MODEM (modulator-demodulator), the
terminal also has the capability to transmit and receive over telephone
lines.

The processor, memory, and I/O controller make up a small, stored-program,
general-purpose computer.  After power is applied to the terminal, the
first program is loaded into the processor's memory from the tape cartridge
on the second tape drive.  This is accomplished when the operator pushes
the PROGRAM LOAD switch, which causes the processor to perform a short
loading program stored in a read-only memory on the processor board.  This
program loads the program stored on the tape into the processor memory.
From this point on, the operation of the processor is under control of
that program.  The 1501-CL model does not have tape drives, however, and
it acquires programs over the serial input-output channel.

The processor has 41 different instructions in its instruction repertoire,
and there are many input-output instructions that are executed by the I/O
controller.  Since the data flow between units depends upon how the pro-
grammer uses these instructions, the following description discusses the
basic flow of data that can be accomplished rather than how data is pro-
cessed by any specific program.  Refer to the block diagram in figure 2-1
for the following discussion.



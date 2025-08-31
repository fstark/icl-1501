# RAM boards

This describes the RAM board found in a Singer 1501 reworked by ICL (replaced CPU+RAM) in 1982.
RAM datecodes indicate late 1980.

Top markings:
```
ICL
1501-1505 MEM BD
005-9019040-03
222113-3 (sticker)
```
Bottom:
```
040-9019341-02
0681
```

It's a 2 layer PCB

## Parts

| Location | Type    |
| -------- | ------- |
| A1       | MC7812CK|
| A3       | 75361   |
| A4       | 74S74   |
| A5       | 7400    |
| A6       | 74S02   |
| A7       | 74S197  |
| A8       | 74S10   |
| A9       | 74S11   |
| A10      | 74S04   |
| A11      | 7474    |
| A12      | 7400    |
| A13      | 7404    |
| A14      | 7474    |
| B3       | 7493    |
| B5       | 74S04   |
| B6       | 74S74   |
| B7       | 74S08   |
| B8       | 74S11   |
| B9       | 74S74   |
| B10      | 74S280  |
| B11      | 74S373  |
| B12      | 74S373  |
| B13      | 74S373  |
| B14      | 74180   |
| C1       | 4116-20 |
| C2       | 4116-20 |
| C3       | 4116-20 |
| C4       | 4116-20 |
| C5       | 4116-20 |
| C6       | 4116-20 |
| C7       | 4116-20 |
| C8       | 4116-20 |
| C9       | 4116-20 |
| C10      | 74S374  |
| C12      | D3242   |
| C13      | 7410    |
| C14      | 7402    |
| D1       | 4116-20 |
| D2       | 4116-20 |
| D3       | 4116-20 |
| D4       | 4116-20 |
| D5       | 4116-20 |
| D6       | 4116-20 |
| D7       | 4116-20 |
| D8       | 4116-20 |
| D9       | 4116-20 |
| D12      | 74S08   |
| D13      | 7485    |
| D14      | 74S139  |
| E10      | 74175   |
| E11      | 74S00   |
| E12      | 74S08   |
| E13      | 75451   |
| E14      | 7404    |
| F10      | 7442    |
| F11      | 7474    |
| F12      | 74S08   |

18.000 MHz quartz at location B3

Jumper configuration:
* J1: A-B
* J2: set
* J3: set
* J4: set
* J5: not set
* J6: not set

Bodges: 4 bodges, only related to VSS and VCC on RAM chips. Unsure why

## Connector pinout

Derived from the original COGAR Schematics, confirmed by checking the RAM board

| Signal     | Pin|Pin | Signal    |
| ---------- | ---|--- | --------- |
| 5V         |  1 | 2  | 5V        |
| 5V         |  3 | 4  | 5V        |
| Ground     |  5 | 6  | Ground    |
| Ground     |  7 | 8  | Ground    |
| 5V or NC*  |  9 | 10 | Ground    |
|            | 11 | 12 |           |
| Ground     | 13 | 14 |           |
|            | 15 | 16 |           |
|            | 17 | 18 | Ground    |
|            | 19 | 20 |           |
| Ground     | 21 | 22 |           |
|            | 23 | 24 |           |
|            | 25 | 26 | Ground    |
|            | 27 | 28 |           |
| Ground     | 29 | 30 |           |
|            | 31 | 32 |           |
|            | 33 | 34 | Ground    |
|            | 35 | 36 |           |
| Ground     | 37 | 38 | Ground    |
| 10 or 17.5V| 39 | 40 | 10 or 17.5V|
| Ground     | 41 | 42 |           |
|            | 43 | 44 |           |
| Ground     | 45 | 46 |           |
|            | 47 | 48 |           |
|            | 49 | 50 | Ground    |
|            | 51 | 52 |           |
| Ground     | 53 | 54 |           |
|            | 55 | 56 |           |
|            | 57 | 58 | Ground    |
|            | 59 | 60 |           |
| Ground     | 61 | 62 |           |
|            | 63 | 64 |           |
|            | 65 | 66 | Ground    |
|            | 67 | 68 |           |
| Ground     | 69 | 70 |           |
|            | 71 | 72 |           |
|            | 73 | 74 | Ground    |
|            | 75 | 76 |           |
| Ground     | 77 | 78 |           |
|            | 79 | 80 |           |
|            | 81 | 82 | Ground    |
|            | 83 | 84 |           |
| Ground     | 85 | 86 |           |
|            | 87 | 88 |           |
|            | 89 | 90 | Ground    |
| Ground     | 91 | 92 | Ground    |
| Ground     | 93 | 94 | Ground    |
| 5V         | 95 | 96 | 5V        |
| 5V         | 97 | 98 | 5V        |

Notes:
* Pin 9 is unconnected on our ram board, set to 5V on Cogar schematics
* Pins 39 and 40 are set to 10V on Cogar schematics. Our RAM board has a 7812 so it's at least 15. Singer documentation say this rail should be 17.5 for TI RAM so we'll go with that assumption.




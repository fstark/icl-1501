As the CPU board only sees bytes and no bits or flux transitions, the tape format focuses on provding bytes at certain tape position.

Overall format:

```
# This tape contains an infinite loop if loaded into P00-030 (normal boot load point)
# First number is the byte position in inches.
# As the tape is 100ft long, there are 1200 inches in a standard tape.
# Density is 1600bpi, so one bit every 0.000625, and one byte every 0.005 inches.
0030.0000: 100
0030.0050: 030
```

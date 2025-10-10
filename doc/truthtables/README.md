# Creating logic expression for the 2-valued encoding of 4-valued gates

## Example AND

Each variable is represented by two values, e.g, a = (a1, a2)
The 2-valued binary encoding of the 4-valued logic

* (0,0) = 0
* (1,0) = 1
* (0,1) = X
* (1,1) = Z

We know the truth table, e.g., for AND according to Verilog IEEE Standard 1364:
c= AND(a,b)
| 0 1 X Z
0 | 0 0 0 0
1 | 0 1 X X
X | 0 X X X
Z | 0 X X X
We can translate the truth table into the 2-bit binary encoding:
(c1,c2)= AND((a1,a2,(b1,b2))
| 00 10 01 11
00 | 00 00 00 00
10 | 00 10 01 01
01 | 00 01 01 01
11 | 00 01 01 01

This yields truth tables for c1 and c2. We can derive a DNF or CNF from the truth table.

# Optional optimization using logic synthesis

Sources:

* abc: http://people.eecs.berkeley.edu/~alanmi/abc/
* cadence.genlib: https://github.com/nianzelee/threABC/blob/master/cadence.genlib

We create a blif file that defines the two output functions for output c=(c1,c2) -- see and.blif

We use logic synthesis in `abc` to optimize that file, e.g., using the following commands:

* read_blif and.blif
* read_library cadence.genlib
* map
* resyn
* share
* resyn
* share
* write_verilog and.v

The and.v now contains readable expressions to be applied to the inputs.


# current issues

Our tool throws a 'scope not found' error during instantiation graph creation.
The vcd file generated by ghdl simulation writes out a for loop in "add\_sub\_eta2.vhd"
as three nested module scopes.
Yosys using the ghdl plugin to create a json file joins these nested modules as a single 
cell module
this creates a mismatch between vcd and json which our tool cannot handle.
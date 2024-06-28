# xtea Simulation

# iverilog 
* `iverilog -o xtea.vvp xtea_tb.v xtea.v`
* `vvp xtea.vvp`

# yosys
* `read_verilog xtea.v`
* `hierarchy`
* `proc`
* `memory`
* `opt`
* `write_json xtea.json`

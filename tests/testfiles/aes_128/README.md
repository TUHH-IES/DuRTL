# Simulation
## all modules in one file, see folder srcFiles

## iverilog:
* `iverilog -o aestest.vvp aes_128_tb.v aes_128.v`
* `vvp aestest.vvp` 

## yosys:
* 'read_verilog aes_128.v'
* 'hierarchy'
* 'proc'
* 'memory'
* 'opt'
* write_json aes_128.json


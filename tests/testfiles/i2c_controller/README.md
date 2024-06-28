# Simple_I2C Simulation

## iverilog

* `iverilog -o i2c_controller.vvp i2c_controller_tb.v i2c_controller.v i2c_slave_controller.v`
* `vvp i2c_controller.vvp`

## yosys

* `read_verilog i2c_controller.v`
* `hierarchy`
* `proc`
* `memory`
* `opt`
* `write_json i2c_controller.json`
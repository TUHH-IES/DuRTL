# Simulation simple_spi_top

## iverilog
* `iverilog -o simple_spi_top.vvp simple_spi_top_tb.v simple_spi_top.v fifo4.v spi_slave_model.v wb_master_model.v`
* `vvp simple_spi_top.vvp`

## yosys 
* `read_verilog simple_spi_top.v fifo4.v` 
* `hierarchy`
* `proc`
* `memory`
* `opt`
* `write_json simple_spi_top.json`



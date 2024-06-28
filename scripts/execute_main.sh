#!/bin/bash

cd ../cmake-build-debug/src/

./main -j ../../verilog/SPI_Master/SPI_Master.json -d ../../verilog/SPI_Master/SPI_Master.v -t ../../verilog/SPI_Master/SPI_Master.vcd -s 20

#./main -j ../../verilog/picorv32/picorv32.json -d ../../verilog/picorv32/picorv32.v -t ../../verilog/picorv32/picorv32.vcd -s 5000

#./main -j ../../verilog/y86/y86.json -d ../../verilog/y86/y86.v -t ../../verilog/y86/y86.vcd -s 100

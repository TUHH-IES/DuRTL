#!/bin/bash
rm -r build
mkdir build
(cd build;
cmake ..
make
(cd src;

./main -d ../../verilog/aes_128/aes_128.v -j ../../verilog/aes_128/aes_128.json -t ../../verilog/aes_128/aes_128_tb.v -v ../../verilog/aes_128/aes_128.vcd -s 5000
# stoi error
#./main -d ../../verilog/ethmac/ethmac.v -j ../../verilog/ethmac/ethmac.json -t ../../verilog/ethmac/ethmac_tb.v -v ../../verilog/ethmac/ethmac.vcd -s 5
# contains inout port
# ./main -d ../../verilog/i2c_controller/i2c_controller.v -j ../../verilog/i2c_controller/i2c_controller.json -t ../../verilog/i2c_controller/i2c_controller_tb.v -v ../../verilog/i2c_controller/i2c_controller.vcd -s 5
# designs running through without error message
#./main -d ../../verilog/mips_16_core_top/mips_16_core_top.v -j ../../verilog/mips_16_core_top/mips_16_core_top.json -t ../../verilog/mips_16_core_top/mips_16_core_top_tb.v -v ../../verilog/mips_16_core_top/mips_16_core_top.vcd -s 5
#./main -d ../../verilog/openMSP430/openMSP430.v -j ../../verilog/openMSP430/openMSP430.json -t ../../verilog/openMSP430/openMSP430_tb.v -v ../../verilog/openMSP430/openMSP430.vcd -s 5
#./main -d ../../verilog/PWM/PWM.v -j ../../verilog/PWM/PWM.json -t ../../verilog/PWM/PWM_tb.v -v ../../verilog/PWM/PWM.vcd -s 5

)
)
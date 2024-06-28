#! /usr/bin/bash

cd ../tests/testfiles/

designs=("aes_128" "aes_module_t" "analysis_test" "and" "binary_gates" "constants" "counter" "counter_ift" "fft" "fft16" "i2c_controller" "picorv32" "SPI_Master" "SPI_Slave" "usb2uart" "xtea" "y86")

for design in ${designs[@]}; do
yosys -p "read_verilog $design/$design.v; hierarchy -auto-top; proc; memory_collect; write_json $design/$design.json"
done
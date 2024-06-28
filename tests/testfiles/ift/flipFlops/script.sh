#! /usr/bin/bash

#design=$1
#
#cd $design
#
#iverilog -DINPUT_FILE_NAME=\ ${design}_tag_input.txt\  -DDUMP_FILE_NAME=\ ${design}_ift.vcd\  ${design}_ift.v ${design}_ift_tb.v ; ./a.out; rm a.out

#gtkwave ${design}_ift.vcd

array_ffs='dff dffe aldff adff aldffe adffe sdff sdffe dlatch adlatch dffsr dffsre dlatchsr mbit_dffsr mbit_dffsre sdffe_en_polarity dlatchsr_1bit'

for ff in $array_ffs
do
cd $ff
iverilog -DINPUT_FILE_NAME=\"${ff}_tag_input.txt\" -DDUMP_FILE_NAME=\"${ff}_ift.vcd\" ${ff}_ift.v ${ff}_ift_tb.v ; ./a.out; rm a.out
cd ..
done
#! /usr/bin/bash

#design=$1
#
#cd $design
#
#iverilog -DINPUT_FILE_NAME=\ ${design}_tag_input.txt\  -DDUMP_FILE_NAME=\ ${design}_ift.vcd\  ${design}_ift.v ${design}_ift_tb.v ; ./a.out; rm a.out

#gtkwave ${design}_ift.vcd

array_cells='shift sr pmux pmux_ue mux shiftx'

for cell in $array_cells
do
cd $cell
iverilog -DINPUT_FILE_NAME=\"${cell}_tag_input.txt\" -DDUMP_FILE_NAME=\"${cell}_ift.vcd\" ${cell}_ift.v ${cell}_ift_tb.v ; ./a.out; rm a.out
cd ..
done
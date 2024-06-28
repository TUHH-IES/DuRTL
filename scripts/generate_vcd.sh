#! /usr/bin/bash

## generating vcd data files for basic_gates_tests

### generating unary_gates_data
iverilog -o basic_gates/unary_output.vvp -DDUMP_FILE_NAME=\"$( dirname -- "$( readlink -f -- "$0"; )"; )/basic_gates/unary_gates_test.vcd\" basic_gates/unary_gates_test.v basic_gates/unary_gates_test_tb.v

vvp $( dirname -- "$( readlink -f -- "$0"; )"; )/basic_gates/unary_output.vvp

### generating binary_gates_data
iverilog -o basic_gates/binary_output.vvp -DDUMP_FILE_NAME=\"$( dirname -- "$( readlink -f -- "$0"; )"; )/basic_gates/binary_gates_test.vcd\" basic_gates/binary_gates_test.v basic_gates/binary_gates_test_tb.v

vvp $( dirname -- "$( readlink -f -- "$0"; )"; )/basic_gates/binary_output.vvp

## generating vcd data files for basic_gates_ift_tests

### generating unary_gates_ift_data
iverilog -o ift/unary_output.vvp -DINPUT_FILE_NAME=\"$( dirname -- "$( readlink -f -- "$0"; )"; )/ift/tag_input_unary_test.txt\" -DDUMP_FILE_NAME=\"$( dirname -- "$( readlink -f -- "$0"; )"; )/ift/unary_gates_ift_test.vcd\" ift/unary_gates_ift_test.v ift/unary_gates_ift_test_tb.v

vvp $( dirname -- "$( readlink -f -- "$0"; )"; )/ift/unary_output.vvp

### generating binary_gates_ift_data
iverilog -o ift/binary_output.vvp -DINPUT_FILE_NAME=\"$( dirname -- "$( readlink -f -- "$0"; )"; )/ift/tag_input_binary_test.txt\" -DDUMP_FILE_NAME=\"$( dirname -- "$( readlink -f -- "$0"; )"; )/ift/binary_gates_ift_test.vcd\" ift/binary_gates_ift_test.v ift/binary_gates_ift_test_tb.v

vvp $( dirname -- "$( readlink -f -- "$0"; )"; )/ift/binary_output.vvp

## generating flattened_verilog export test reference files

### generating and_gate_data
iverilog -o flattened_verilog/andGate_output.vvp -DDUMP_FILE_NAME=\"$( dirname -- "$( readlink -f -- "$0"; )"; )/flattened_verilog/andGate.vcd\" flattened_verilog/andGate.v flattened_verilog/andGate_tb.v

vvp $( dirname -- "$( readlink -f -- "$0"; )"; )/flattened_verilog/andGate_output.vvp

### generating or_gate_data
iverilog -o flattened_verilog/orGate_output.vvp -DDUMP_FILE_NAME=\"$( dirname -- "$( readlink -f -- "$0"; )"; )/flattened_verilog/orGate.vcd\" flattened_verilog/orGate.v flattened_verilog/orGate_tb.v

vvp $( dirname -- "$( readlink -f -- "$0"; )"; )/flattened_verilog/orGate_output.vvp

### adding the rest of the gates???
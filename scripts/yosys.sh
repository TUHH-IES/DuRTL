#! /usr/bin/bash

cd $1

export VERILOG_FILE_NAME="${2}.v"
export JSON_FILE_NAME="${2}.json"

yosys -c ../json.tcl
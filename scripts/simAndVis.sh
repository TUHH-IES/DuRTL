echo "Compiling with iverilog"
MOD=$1
iverilog $MOD.v $MOD_tb.v
echo "Simulating"
./a.out
echo "Showing waveforms"
gtkwave test.vcd &
echo "Producing json with yosys"
#yosys -p "read_verilog $MOD.v; hierarchy; proc; opt; write_json $MOD.json; show $MOD; exit"
yosys -p "read_verilog $MOD.v; hierarchy -auto-top; proc; write_json $MOD.json; show $MOD; exit"

if [ -e $MOD_verilator.cpp ]
then
  echo "Running verilator"
  echo "Producing xml output"
  verilator --xml-only --Wall --cc --exe $MOD_verilator.cpp $MOD.v
  echo "Simulation"
  verilator --Wall --cc --exe $MOD_verilator.cpp $MOD.v
  make -j -C obj_dir/ -f V$MOD.mk V$MOD
  ./obj_dir/V$MOD
fi


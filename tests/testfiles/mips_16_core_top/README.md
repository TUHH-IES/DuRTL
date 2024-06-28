# mips_16 Simulation

## iverilog
* `iverilog -o mips_16_core_top.vvp mips_16_core_top_tb.v mips_16_core_top.v alu.v data_mem.v EX_stage.v hazard_detection_unit.v ID_stage.v IF_stage.v instruction_mem.v MEM_stage.v register_file.v WB_stage.v `

* `vvp mips_16_core_top.vvp`

## yosys
* `mips_16_core_top.v alu.v data_mem.v EX_stage.v hazard_detection_unit.v ID_stage.v IF_stage.v instruction_mem.v MEM_stage.v register_file.v WB_stage.v`
* `hierarchy`
* `proc`
* `memory`
* `opt`
* `write_json mips_16_core_top.json`

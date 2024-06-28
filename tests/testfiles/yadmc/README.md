# yadmc Simulation

# iverilog 
* `iverilog -o yadmc.vvp yadmc_tb.v mt48lc16m16a2.v yadmc_dpram.v yadmc_spram.v yadmc_sync.v yadmc_sdram16.v yadmc.v`
* `vvp yadmc.vvp`

# yosys
* `read_verilog yadmc.v yadmc_dpram.v yadmc_spram.v yadmc_sync.v yadmc_sdram16.v`
* `hierarchy`
* `proc`
* `memory`
* `opt`
* `write_json yadmc.json`

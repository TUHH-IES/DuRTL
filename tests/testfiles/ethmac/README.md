# Ethmac Simulation

## iverilog

* `iverilog -o ethmac_test.vvp tb_ethernet.v ethmac.v wb_bus_mon.v wb_master_behavioral.v wb_slave_behavioral.v eth_maccontrol.v eth_macstatus.v eth_miim.v eth_phy.v eth_register.v eth_rxethmac.v eth_txethmac.v eth_wishbone.v wb_master32.v eth_clockgen.v eth_crc.v eth_fifo.v eth_outputcontrol.v eth_random.v eth_receivecontrol.v eth_registers.v eth_rxaddrcheck.v eth_rxcounters.v eth_rxstatem.v eth_shiftreg.v eth_spram_256x32.v eth_transmitcontrol.v eth_txcounters.v eth_txstatem.v`
* `vvp ethmac_test.vvp`

## yosys

* `read_verilog ethmac.v eth_maccontrol.v eth_macstatus.v eth_miim.v eth_register.v eth_rxethmac.v eth_txethmac.v eth_wishbone.v eth_clockgen.v eth_crc.v eth_fifo.v eth_outputcontrol.v eth_random.v eth_receivecontrol.v eth_registers.v eth_rxaddrcheck.v eth_rxcounters.v eth_rxstatem.v eth_shiftreg.v eth_spram_256x32.v eth_transmitcontrol.v eth_txcounters.v eth_txstatem.v`
* `hierarchy`
* `proc`
* `memory`
* `opt`
* `write_json ethmac.json`

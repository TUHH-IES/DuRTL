# OpenMSP430 Simulation

## iverilog

* `iverilog -o testOpenMSP.vvp tb_openMSP430.v openMSP430.v io_cell.v msp_debug.v omsp_clock_module.v omsp_dbg.v omsp_frontend.v omsp_mem_backbone.v omsp_multiplier.v omsp_sfr.v omsp_watchdog.v ram.v template_periph_16b.v template_periph_8b.v omsp_and_gate.v omsp_clock_gate.v omsp_dbg_uart.v omsp_execution_unit.v omsp_gpio.v omsp_scan_mux.v omsp_sync_cell.v omsp_sync_reset.v omsp_timerA.v omsp_wakeup_cell.v omsp_alu.v omsp_register_file.v`

* `vvp testOpenMSP.vvp`

## yosys

* `read_verilog openMSP430.v omsp_clock_module.v omsp_dbg.v omsp_frontend.v omsp_mem_backbone.v omsp_multiplier.v omsp_sfr.v omsp_watchdog.v template_periph_16b.v template_periph_8b.v omsp_and_gate.v omsp_clock_gate.v omsp_dbg_uart.v omsp_execution_unit.v omsp_gpio.v omsp_scan_mux.v omsp_sync_cell.v omsp_sync_reset.v omsp_timerA.v omsp_wakeup_cell.v omsp_alu.v omsp_register_file.v`
* `hierarchy`
* `proc`
* `memory`
* `opt`
* `write_json openMSP430.json`

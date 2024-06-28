module simple_spi_top( adr_i, clk_i, cyc_i, dat_i, miso_i, rst_i, stb_i, we_i, ack_o, dat_o, inta_o, mosi_o, sck_o);
input [1:0] adr_i;
input clk_i;
input cyc_i;
input [7:0] dat_i;
input miso_i;
input rst_i;
input stb_i;
input we_i;
output ack_o;
output [7:0] dat_o;
output inta_o;
output mosi_o;
output sck_o;
m_2 ins( adr_i, clk_i, cyc_i, dat_i, miso_i, rst_i, stb_i, we_i, ack_o, dat_o, inta_o, mosi_o, sck_o);
endmodule
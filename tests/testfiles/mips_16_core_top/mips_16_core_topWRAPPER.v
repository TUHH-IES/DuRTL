module mips_16_core_top( clk, rst, pc);
input clk;
input rst;
output [7:0] pc;
m_9 ins( clk, rst, pc);
endmodule
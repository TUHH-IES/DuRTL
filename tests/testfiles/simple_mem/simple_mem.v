module simple_mem(address, mem_data, clk);

input [7:0] address;
output [17:0] mem_data;
input clk;

reg [17:0] data_out;
wire [17:0] mem_data;

parameter MEM_SIZE = 4;
integer k;
reg [17:0] memory [0:MEM_SIZE - 1];

initial
begin
for (k = 0; k < MEM_SIZE - 1; k = k + 1)
begin
    memory[k] = k;
end
end

assign mem_data = data_out;

always@(posedge clk) begin
    data_out <= memory[address] ;
end

endmodule
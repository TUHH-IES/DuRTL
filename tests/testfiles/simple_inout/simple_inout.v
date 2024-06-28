module simple_inout(address, MEM_RW, MEM_OE, MEM_DATA_bidir, clk);

input [7:0] address;
input MEM_RW, MEM_OE;

//mem_oe=1 for memory output enable, mem_oe=0 output=zzzz...
//mem_rw=0 for read, mem_rw=1 for write

inout [17:0] MEM_DATA_bidir;
input clk;

reg [17:0] data_out;
wire [17:0] MEM_DATA;

parameter MEM_SIZE = 4;
integer k;
reg [17:0] Memory [0:MEM_SIZE - 1];

initial
begin
for (k = 0; k < MEM_SIZE - 1; k = k + 1)
begin
    Memory[k] = 0;
end
end

wire Read = MEM_OE && (!MEM_RW) ;
wire Write = MEM_OE && MEM_RW ;
assign MEM_DATA_bidir = Read ? data_out : 18'hz ;

always@(posedge clk) begin
    if (Write) Memory[address] <= MEM_DATA_bidir ;
    data_out <= Memory[address] ;
end

endmodule
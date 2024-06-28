module m_0 (CLK, CLK_t, D, D_t, Q, Q_t);

parameter WIDTH = 2;
parameter CLK_POLARITY = 1'b1;

input CLK;
input [WIDTH-1:0] D;
input [31:0] CLK_t;
input [31:0] D_t;
output reg [WIDTH-1:0] Q;
output reg [31:0] Q_t;
wire pos_clk = CLK == CLK_POLARITY;

initial begin
    Q_t = 0;
end

always @(posedge pos_clk) begin
	Q <= D;
	Q_t <= D_t;
end

endmodule

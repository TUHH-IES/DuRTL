module m_0 (CLK,CLK_t, D, D_t, SRST, SRST_t, Q, Q_t);

parameter WIDTH = 2;
parameter CLK_POLARITY = 1'b1;
parameter SRST_POLARITY = 1'b1;
parameter SRST_VALUE = 2;

input CLK, SRST;
input [WIDTH-1:0] D;
input [31:0] D_t;
input [31:0] SRST_t;
input [31:0] CLK_t;
output reg [WIDTH-1:0] Q;
output reg [31:0] Q_t;
wire pos_clk = CLK == CLK_POLARITY;
wire pos_srst = SRST == SRST_POLARITY;

initial begin
    Q_t = 0;
end

always @(posedge pos_clk) begin
	if (pos_srst) begin
		Q <= SRST_VALUE;
		Q_t <= (^D === 1'bx) ? 0 : ((D == SRST_VALUE) ? (D_t | SRST_t) : SRST_t);
		end
	else begin
		Q <= D;
		Q_t <= (^D === 1'bx) ? 0 : ((D == SRST_VALUE) ? (D_t | SRST_t) : D_t);
		end
end

endmodule


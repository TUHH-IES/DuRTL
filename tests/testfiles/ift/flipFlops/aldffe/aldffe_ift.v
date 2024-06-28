module m_0 (AD, AD_t, ALOAD, ALOAD_t, CLK, CLK_t, D, D_t, EN, EN_t, Q, Q_t);

parameter WIDTH = 2;
parameter CLK_POLARITY = 1'b1;
parameter EN_POLARITY = 1'b1;
parameter ALOAD_POLARITY = 1'b1;

input CLK, ALOAD, EN;
input [WIDTH-1:0] D;
input [WIDTH-1:0] AD;
input [31:0] CLK_t;
input [31:0] ALOAD_t;
input [31:0] EN_t;
input [31:0] D_t;
input [31:0] AD_t;
output reg [WIDTH-1:0] Q;
output reg [31:0] Q_t;
wire pos_clk = CLK == CLK_POLARITY;
wire pos_aload = ALOAD == ALOAD_POLARITY;

initial begin
    Q_t = 0;
end

always @(posedge pos_clk, posedge pos_aload) begin
	if (pos_aload) begin
		Q <= AD;
		Q_t <= (^D === 1'bx) ? 0 : ((D == ALOAD) ? (D_t | AD_t) : AD_t);
	end
	else if (EN == EN_POLARITY) begin
		Q <= D;
		Q_t <= (^D === 1'bx) ? 0 : (((D == ALOAD) ? (D_t | AD_t) : D_t) | EN_t);
	end
end

endmodule
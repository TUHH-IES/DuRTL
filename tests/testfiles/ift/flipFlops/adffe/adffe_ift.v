module m_0 (ARST, ARST_t, CLK, CLK_t, D, D_t, EN, EN_t, Q, Q_t);

parameter WIDTH = 2;
parameter CLK_POLARITY = 1'b1;
parameter EN_POLARITY = 1'b1;
parameter ARST_POLARITY = 1'b1;
parameter ARST_VALUE = 2;

input CLK, ARST, EN;
input [WIDTH-1:0] D;
input [31:0] CLK_t;
input [31:0] ARST_t;
input [31:0] D_t;
input [31:0] EN_t;
output reg [WIDTH-1:0] Q;
output reg [31:0] Q_t;
wire pos_clk = CLK == CLK_POLARITY;
wire pos_arst = ARST == ARST_POLARITY;

initial begin
    Q_t = 0;
end

always @(posedge pos_clk, posedge pos_arst) begin
	if (pos_arst)begin
		Q <= ARST_VALUE;
		Q_t <= (^D === 1'bx) ? 0 : ((D == ARST_VALUE) ? (D_t | ARST_t) : ARST_t);
		end
	else if (EN == EN_POLARITY)begin
		Q <= D;
		Q_t <= (^D === 1'bx) ? 0 : (((D == ARST_VALUE) ? (D_t | ARST_t) : D_t) | EN_t);
		end
end



endmodule

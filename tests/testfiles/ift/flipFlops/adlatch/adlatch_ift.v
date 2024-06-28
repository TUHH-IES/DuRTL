module m_0 (D, D_t,ARST, ARST_t, EN, EN_t, Q, Q_t);

parameter WIDTH = 2;
parameter EN_POLARITY = 1'b1;
parameter ARST_POLARITY = 1'b1;
parameter ARST_VALUE = 3;

input EN, ARST;
input [WIDTH-1:0] D;
input [31:0] EN_t;
input [31:0] D_t;
input [31:0] ARST_t;
output reg [WIDTH-1:0] Q;
output reg [31:0] Q_t;

initial begin
    Q_t = 0;
end

always @* begin
	if (ARST == ARST_POLARITY)begin
		Q <= ARST_VALUE;
		Q_t <= (^D === 1'bx) ? 0 : ((D == ARST_VALUE) ? (D_t | ARST_t) : ARST_t);
	end
	else if (EN == EN_POLARITY) begin
		Q <= D;
		Q_t <= (^D === 1'bx) ? 0 : (((D == ARST_VALUE) ? (D_t | ARST_t) : D_t) | EN_t);
	end
end

endmodule

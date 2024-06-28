module m_0( a, a_t, b, b_t, s, s_t, out, out_t);

	input a, b, s;
	output out;
	
	input [31:0] a_t;
	input [31:0] b_t;
	input [31:0] s_t;
	output [31:0] out_t;

	assign out = s ? a : b;

	assign out_t = (^s === 1'bx ? 0 : (s ? a_t : b_t)) | ((^(a^b) === 1'bx) ? 0 : (a ^ b ? s_t : 0)) | (a_t & s_t) | (b_t & s_t);


endmodule

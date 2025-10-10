module split( a, b, out);

	input [3:0] a;
	input [2:0] b;
	output [1:0] out;

	assign out = {a[3], b[2]} & b[1:0];

endmodule

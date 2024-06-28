module andVector( a, b, out);

	input [3:0] a;
	input [3:0] b;
	output [3:0] out;
	wire [3:0] out;

	assign out = a & b;

endmodule

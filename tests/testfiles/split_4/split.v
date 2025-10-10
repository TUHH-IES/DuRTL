module split( a, b, out);

	input [3:0] a;
	input [2:0] b;
	output [1:0] out;

	assign out[0] = a[3] & b[1];
	assign out[1] = a[2] & b[0];

endmodule

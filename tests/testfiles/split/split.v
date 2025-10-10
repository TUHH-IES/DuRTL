module split( a, b, out);

	input [3:0] a;
	input b;
	output out;

	assign out = a[0] & b;

endmodule

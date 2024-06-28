module EqCell( a, b, out);

	input [3:0] a;
	input [3:0] b;
	output out;
	wire out;

	assign out = a == b ? 1'b1 : 1'b0;

	//assign out = a & b & c;
	//assign out = AND(a, b, c);
  	//and(out, a,b,c);

endmodule

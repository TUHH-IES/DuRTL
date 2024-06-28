module posGate( a, out);

	input signed [2:0] a;
	output signed [2:0] out;
	wire signed [2:0] out;

	assign out = +a;
	
endmodule

module reduceXnorGate( a, out);

	input [2:0] a;
	output out;
	wire out;

	assign out = ~^a;
	
endmodule

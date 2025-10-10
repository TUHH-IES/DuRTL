module muxGate( a, b, s, c);

	input a;
	input b;
    input s;
	output c;

	assign c = s ? b:a;

endmodule

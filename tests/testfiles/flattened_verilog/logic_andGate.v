module logic_andGate( a, b, c, out);

	input a, b, c;
	output out;
	wire out;

	assign out = a && b && c;
	//assign out = AND(a, b, c);
  	//and(out, a,b,c);

endmodule

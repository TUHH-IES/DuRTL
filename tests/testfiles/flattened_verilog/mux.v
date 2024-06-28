module mux( a, b, s, out);

	input a, b, s;
	output reg out;

	always @(a, b, s)
		if (s)
			out <= a;
		else
			out <= b;

endmodule

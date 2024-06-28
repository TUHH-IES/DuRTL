module m_0( a, b, s, out);

	input a, b, s;
	output reg out;

	always @*
		if (s)
			out <= a;
		else
			out <= b;

endmodule

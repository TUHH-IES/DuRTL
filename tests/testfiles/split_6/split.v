module split_sub(f, g, h);

	input f, g;
	output [1:0] h;

	assign h[0] = f;
	assign h[1] = g;
endmodule

module split( a, b, out);

	input [3:0] a;
	input [2:0] b;
	output [1:0] out;
	
	split_sub mod(.f(a[3]), .g(b[2]), .h(out));

endmodule
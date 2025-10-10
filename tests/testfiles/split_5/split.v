module split_sub( g, h);

	input [1:0] g;
	output [1:0] h;

	assign h[0] = g[0];
	assign h[1] = g[1];
endmodule

module split( a, b, out);

	input [3:0] a;
	input [2:0] b;
	output [1:0] out;
	
	split_sub mod(.g({a[3], b[2]}), .h(out));

endmodule
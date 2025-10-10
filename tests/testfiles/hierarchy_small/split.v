module split_sub( g, h);

	input [1:0] g;
	output [1:0] h;

	assign h = g;
endmodule

module split( a, out);

	input [1:0] a;
	output [1:0] out;
	
	split_sub mod(.g(a), .h(out));

endmodule
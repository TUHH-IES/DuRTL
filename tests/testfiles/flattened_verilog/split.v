module split( a, b, out);

	input [5:0] a;
	input [5:0] b;
	output [5:0] out;
	wire [5:0] out;

  wire [3:0] tmp;

  // This is quite mean: 
  //   The bit-vectors are torn apart and combined in strange ways.
	assign tmp = a[3:0] &  { b[5:3], b[1]};
	assign out[1:0] = a[3:2] & { b[0], b[2]};

  assign out[5:2]= tmp;

endmodule

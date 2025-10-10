module pass_through(clk, a, c, d);

  input clk;
  input [2:0] a;

  output [2:0] c;
  output [2:0] d;
  
  reg [2:0] d;
  
  assign c = a ;

	always @(posedge clk)
    d = a ;

endmodule

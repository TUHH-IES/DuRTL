module mealy_moore(clk, a, b, c, d);

  input clk;
  input [2:0] a;
  input [2:0] b;

  output [2:0] c;
  output [2:0] d;
  
  reg [2:0] d;
  
  assign c = a & b;

	always @(posedge clk)
    d = a | b;

endmodule

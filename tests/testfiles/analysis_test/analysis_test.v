module analysis_test(
    clk, a, b,
    c, d);
  input wire a;
  input wire b;
  input wire clk;
  output wire c;
  output wire d;

  reg e, f;

  assign c = e;
  assign d = f;


always@(posedge clk)
  e <= a;

always@(negedge clk)
  e <= a;

always@(posedge clk)
  f <= a;

always@(negedge clk)
  f <= b;

endmodule
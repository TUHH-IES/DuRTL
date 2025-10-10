module dff(
    d, clk,
    q);
  input wire d;
  input wire clk;
  output reg q;
  
  always @ (posedge clk)
    q <= d;

endmodule
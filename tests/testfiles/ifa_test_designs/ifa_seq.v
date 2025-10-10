module ifa_seq(
    clk,a,b,c,
    d,e);
  input wire clk,a,b,c;
  output reg d,e;
  
  always @(posedge clk) begin
    d <= a & b & c;
    e <= a | b | c;
  end

endmodule
module dff_2x(
    d, clk,
    q);
  input wire d;
  input wire clk;
  output reg q;
  reg a;

  always @ (posedge clk) begin
    a <= d;
    q <= a;
  end

endmodule
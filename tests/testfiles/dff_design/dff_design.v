module dff_design(
    clk, a, b, c,
    d, e);
  input wire a,b,c;
  input wire clk;
  output wire d,e;
  wire x,y;
  reg o,p;

  assign x = a & b;
  assign y = a | b;

  always @ (posedge clk) begin
    o <= x;
    p <= o & c;
  end

  assign d = o;
  assign e = p & y;

endmodule
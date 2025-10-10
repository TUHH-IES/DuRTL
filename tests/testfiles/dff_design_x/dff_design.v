module dff_design(
    clk, a, b, c,
    d, e);
  input wire [7:0] a,b,c;
  input wire clk;
  output wire [7:0] d,e;
  wire [7:0] x,y;
  reg [7:0] o,p;

  assign x = a & b;
  assign y = a | b;

  always @ (posedge clk) begin
    o <= 8'bx;
    p <= o & c;
  end

  assign d = o;
  assign e = p & y;

endmodule
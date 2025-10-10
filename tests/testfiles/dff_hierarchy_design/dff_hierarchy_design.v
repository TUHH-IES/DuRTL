module dff_2(
  clk, a, b,
  c);
  input wire a,b;
  input wire clk;
  output reg c;
  
  always @ (posedge clk) begin
    c <= a & b;
  end
  
endmodule

module dff_hierarchy_design(
    clk, a, b, c,
    d, e);
  input wire a,b,c;
  input wire clk;
  output wire d,e;
  wire x,y,p;
  reg o;

  dff_2 dff_2 (
    .clk(clk),
    .a(o),
    .b(c),
    .c(p)
  );

  assign x = a & b;
  assign y = a | b;

  always @ (posedge clk) begin
    o <= x;
  end
  assign d = o;
  assign e = p & y;

endmodule



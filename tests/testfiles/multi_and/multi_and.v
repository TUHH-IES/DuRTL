module multi_and(
    a, b, c,
    g);
  input wire a;
  input wire b;
  input wire c;
  output wire g;
  wire e, f;
  assign e = a & b;
  assign f = a & c;
  assign g = e & f;

endmodule
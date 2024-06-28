module gateExample( 
    a, b, c, d,
    e, f);
  input wire a;
  input wire b;
  input wire c;
  input wire d;
  output wire e;
  output wire f;
  wire g;
  assign g = a & b;
  assign e = g | c; 
  assign f = e & d;

endmodule
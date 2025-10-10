module anda(
    a, b,
    c);
  input wire [4:0] a;
  input wire [4:0] b;
  output wire [4:0] c;
  assign c = a & b;

endmodule
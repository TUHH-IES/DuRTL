module anda(
    a,
    c);
  input wire a;
  output wire c;
  assign c = a & !a;

endmodule
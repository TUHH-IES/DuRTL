module not_(
    a,
    b, c);
  input wire [4:0] a;
  output wire b;
  output wire [4:0] c;
  assign b = !a;
  assign c = ~a;

endmodule
module UnaryTest(a, c, c1,c2,c3,c4, c5);
  input [31:0] a;
  output [31:0] c;
  output [31:0] c1;
  output [31:0] c2;
  output [31:0] c3;
  output [31:0] c4;
  output [31:0] c5;
  

  assign c = ~a;
  assign c1 = &a;
  assign c2 = |a;
  assign c3 = ^a;
  assign c4 = ~^a;
  assign c5 = -a;
  
endmodule
module signedness(a,b,c,au,bu, cu);
  input signed [2:0] a;
  input signed [1:0] b;
  output signed [4:0] c;
  assign c = a | b;
  input [2:0] au;
  input [1:0] bu;
  output [4:0] cu;
  assign cu = au | bu;
endmodule

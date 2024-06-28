module S(x,y,z);
 input [4:0] x,y;
 output [4:0] z;

 assign z = x & y;
endmodule


module M(e,f,g);
 input [4:0] e,f;
 output [4:0] g;
 wire [4:0] h;
 wire [4:0] i;

 S s1(e,f,i);
 assign h= f | i;

 S s2(e,h,g);
endmodule

module hierarchyVector(a, b, c);
 input [4:0] a,b;
 output [4:0] c;
 wire [4:0] d;

 M m1(a,b,d);
 M m2(a,d,c);
endmodule


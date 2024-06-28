module S(a,b,c);
 input a,b;
 output c;

 assign c = a & b;
endmodule


module M(a,b,c);
 input a,b;
 wire d;
 wire e, f;
 output c;
 
 S s1(e,b,d);
 S s2(f,d,c);
 
    assign e = a ^ b;
    assign f = a | b; 
 

endmodule

module hierarchy(a, b, c);
 input a,b;
 wire d;
 output c;

 M m1(a,b,d);
 M m2(a,d,c);

endmodule


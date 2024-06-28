module M(a,b,c);
 input a,b;
 output c;

 assign c = a & b;  

endmodule

module des(a, b, s, c);
 input a,b,s;
 wire d;
 output c;

 M m1(a,b,d);

 assign c = s ? d : !b;
endmodule


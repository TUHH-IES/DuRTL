module S(clk,a,b,c);
 input clk, a,b;
 output reg c;

 always@(posedge clk)  c <= a & b;
endmodule


module M(clk, a,b,c);
 input clk,a,b;
 wire d;
 wire e, f;
 output c;
 
 S s1(clk,e,b,d);
 S s2(clk,f,d,c);
 
    assign e = a ^ b;
    assign f = a | b; 
 

endmodule

module hierarchy_seq(clk, a, b, c);
 input clk,a,b;
 wire d;
 output c;

 M m1(clk,a,b,d);
 M m2(clk,a,d,c);

endmodule


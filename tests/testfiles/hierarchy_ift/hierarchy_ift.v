module S(a, a_t, b, b_t, c, c_t);
    input a,b;
    input [31:0] a_t, b_t;
    output c;
    output [31:0] c_t;

    assign c = a & b;
    assign c_t = ((a > 0 ? b_t : 0) | (b > 0 ? a_t : 0) | (a_t & b_t ));
endmodule


module M(a, a_t, b, b_t, c, c_t);
    input a,b;
    input [31:0] a_t, b_t;
    wire d;
    wire [31:0] d_t;
    wire e, f;
    wire [31:0] e_t, f_t;
    output c;
    output [31:0] c_t;
    
    S s1(.a(e),
        .a_t(e_t),
        .b(b), 
        .b_t(b_t), 
        .c(d), 
        .c_t(d_t));
    S s2(.a(f), 
        .a_t(f_t), 
        .b(d), 
        .b_t(d_t), 
        .c(c), 
        .c_t(c_t));
 
    assign e = a ^ b;
    assign e_t = a_t | b_t;
    assign f = a | b;
    assign f_t = ((a == 0 ? b_t : 0) | ( b == 0 ? a_t : 0) | (a_t & b_t)); 
 
endmodule

module hierarchy(a, a_t, b, b_t, c, c_t);
    input a,b;
    input [31:0] a_t, b_t;
    wire d;
    wire [31:0] d_t;
    output c;
    output [31:0] c_t;

    M m1(.a(a), .a_t(a_t), .b(b), .b_t(b_t), .c(d), .c_t(d_t));
    M m2(.a(a), .a_t(a_t), .b(d), .b_t(d_t), .c(c), .c_t(c_t));
endmodule


module A(A_a, A_b, A_c, A_d);
    input A_a, A_b, A_c;
    output A_d;
    assign A_d = A_a & A_b;
    //assign A_c = 1'b0;
endmodule

module B(B_a, B_b, B_c);
    input B_a, B_b;
    output B_c;
    A modins(
        .A_a(B_a),
        .A_b(B_b),
        .A_d(B_c)
    );
endmodule